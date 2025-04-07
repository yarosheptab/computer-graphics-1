from PIL import Image
import os
import glob
import argparse

def create_gif(input_dir='temp_frames', output_file='head_rotation.gif', fps=30):
    # Get all PNG files in the input directory
    frames = sorted(glob.glob(os.path.join(input_dir, 'frame_*.png')))
    
    if not frames:
        print("No frames found in", input_dir)
        return
    
    # Open first frame to get dimensions
    first_frame = Image.open(frames[0])
    width, height = first_frame.size
    
    # Create a black background image
    background = Image.new('RGB', (width, height), (0, 0, 0))
    
    # Process all frames
    images = []
    for frame in frames:
        # Open frame
        img = Image.open(frame)
        if img.mode != 'RGB':
            img = img.convert('RGB')
        
        # Create a new image with black background
        new_img = background.copy()
        # Paste the frame on top
        new_img.paste(img, (0, 0))
        images.append(new_img)
    
    # Calculate duration for each frame (in milliseconds)
    duration = int(1000 / fps)
    
    # Save as GIF with minimal settings
    images[0].save(
        output_file,
        save_all=True,
        append_images=images[1:],
        duration=duration,
        loop=0
    )
    
    # Close all images
    for img in images:
        img.close()
    first_frame.close()
    
    print(f"GIF created successfully: {output_file}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Create GIF from PNG frames')
    parser.add_argument('--fps', type=int, default=30, help='Frames per second')
    parser.add_argument('--input', type=str, default='temp_frames', help='Input directory containing frames')
    parser.add_argument('--output', type=str, default='head_rotation.gif', help='Output GIF filename')
    
    args = parser.parse_args()
    create_gif(args.input, args.output, args.fps) 
