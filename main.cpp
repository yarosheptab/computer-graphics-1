#include <iostream>
#include <cmath>
#include <vector>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include "model.h"
#include "gif_writer.h"

void line(int x0, int y0, int x1, int y1, cv::Mat &image, cv::Vec3b color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x=x0; x<=x1; x++) {
        float t = (x-x0)/(float)(x1-x0);
        int y = y0*(1.-t) + y1*t;
         if (x > 0 && y >0)
         {


        if (steep ) {
            image.at<cv::Vec3b>(y, x) = color;

        } else {
           image.at<cv::Vec3b>(x, y) = color;
        }
         }
    }
}

cv::Vec3f cross(cv::Vec3f v1, cv::Vec3f v2) {
    return cv::Vec3f(v1[1]*v2[2] - v1[2]*v2[1], v1[2]*v2[0] - v1[0]*v2[2], v1[0]*v2[1] - v1[1]*v2[0]);
}

cv::Vec3f barycentric(cv::Vec3f A, cv::Vec3f B, cv::Vec3f C, cv::Vec3f P) {
    cv::Vec3f s[2];
    for (int i=2; i--; ) {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    cv::Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2])>1e-2)
        return cv::Vec3f(1.f-(u[0]+u[1])/u[2], u[1]/u[2], u[0]/u[2]);
    return cv::Vec3f(-1,1,1);
}

cv::Vec3f get_normal(cv::Vec3f *pts) {
    cv::Vec3f v1 = pts[1] - pts[0];
    cv::Vec3f v2 = pts[2] - pts[0];
    cv::Vec3f n = cross(v1, v2);
    float len = std::sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
    return cv::Vec3f(n[0]/len, n[1]/len, n[2]/len);
}

void triangle(cv::Vec3f *pts, cv::Mat &image, cv::Vec3b color, int width, int height, cv::Mat &z_buffer) {
    cv::Vec2f bboxmin(image.cols-1,  image.rows-1);
    cv::Vec2f bboxmax(0, 0);
    cv::Vec2f clamp(image.cols-1, image.rows-1);
    
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    
    // Calculate lighting in world space
    cv::Vec3f world_pts[3];
    for (int i=0; i<3; i++) {
        // Convert back to world space coordinates
        world_pts[i] = cv::Vec3f(
            (pts[i][0] - width/2.) * 2./width,
            (height/2. - pts[i][1]) * 2./height,
            pts[i][2]
        );
    }
    
    // Calculate normal in world space
    cv::Vec3f normal = get_normal(world_pts);
    
    // Static light direction in world space (from above and to the right)
    cv::Vec3f light_dir(0.7f, -0.7f, 0.0f);
    float len = std::sqrt(light_dir[0]*light_dir[0] + light_dir[1]*light_dir[1] + light_dir[2]*light_dir[2]);
    light_dir = cv::Vec3f(light_dir[0]/len, light_dir[1]/len, light_dir[2]/len);
    
    // Calculate lighting components
    float diffuse = std::max(0.0f, normal.dot(light_dir));
    float ambient = 0.4f;  // Slightly reduced ambient for more contrast
    float intensity = ambient + diffuse * 0.6f;  // Increased diffuse contribution
    
    // Ensure minimum visibility
    intensity = std::max(0.3f, std::min(1.0f, intensity));
    
    cv::Vec3b shaded_color(color[0] * intensity, color[1] * intensity, color[2] * intensity);
    
    cv::Vec3f P;
    for (P[0]=bboxmin[0]; P[0]<=bboxmax[0]; P[0]++) {
        for (P[1]=bboxmin[1]; P[1]<=bboxmax[1]; P[1]++) {
            cv::Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen[0]<0 || bc_screen[1]<0 || bc_screen[2]<0) continue;
            
            // Calculate z-coordinate using barycentric coordinates
            float z = pts[0][2] * bc_screen[0] + pts[1][2] * bc_screen[1] + pts[2][2] * bc_screen[2];
            
            // Only draw if this point is closer to the camera than what's already in the z-buffer
            if (z > z_buffer.at<float>(P[1], P[0])) {
                z_buffer.at<float>(P[1], P[0]) = z;
                image.at<cv::Vec3b>(P[1], P[0]) = shaded_color;
            }
        }
    }
}

int main()
{
    const int width  = 800;
    const int height = 800;
    cv::Mat gray(width, height, CV_8UC3, cv::Scalar(0));
    cv::Mat z_buffer(width, height, CV_32F, -std::numeric_limits<float>::infinity());
    
    Model *model = NULL;
    model = new Model("african_head.obj");
    cv::Vec3b grey(128, 128, 128);

    // Animation parameters
    float y_angle = 0.0f;
    const float ROTATION_SPEED = 1.0f;  // 1 degree per frame for smoother rotation
    const int FRAME_DELAY = 16;  // ~60 FPS for smoother preview
    const int TOTAL_FRAMES = 360;  // 360 degrees / 1 degree per frame = 360 frames
    
    // Store frames for GIF
    std::vector<cv::Mat> frames;
    
    // Record frames
    for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
        gray = cv::Scalar(0);
        z_buffer = -std::numeric_limits<float>::infinity();
        
        float rad_y_angle = y_angle * M_PI / 180.0f;
        float cos_y = cos(rad_y_angle);
        float sin_y = sin(rad_y_angle);

        for (int i=0; i<model->nfaces(); i++) {
            std::vector<int> face = model->face(i);
            cv::Vec3f pts[3];
            
            for (int j=0; j<3; j++) {
                cv::Vec3f v = model->vert(face[j]);
                
                float x_rot = v[0] * cos_y + v[2] * sin_y;
                float z_rot = -v[0] * sin_y + v[2] * cos_y;
                
                pts[j] = cv::Vec3f((x_rot+1.)*width/2., height - (v[1]+1.)*height/2., z_rot);
            }
            
            triangle(pts, gray, grey, width, height, z_buffer);
        }
        
        // Store frame
        frames.push_back(gray.clone());
        
        // Show preview
        cv::imshow("Recording preview", gray);
        cv::waitKey(FRAME_DELAY);
        
        y_angle += ROTATION_SPEED;
        if (y_angle >= 360.0f) y_angle -= 360.0f;
    }
    
    cv::destroyAllWindows();
    
    // Save frames as GIF with 30 FPS for better compatibility
    GifWriter::saveFramesToGif(frames, "head_rotation.gif", 30);
    
    return 0;
}
