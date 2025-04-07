#include "gif_writer.h"
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <iomanip>

void GifWriter::saveFramesToGif(const std::vector<cv::Mat>& frames, 
                               const std::string& output_filename,
                               int fps) {
    // Create temporary directory for frames
    std::filesystem::create_directory("temp_frames");
    
    // Save each frame as PNG with high quality
    for (size_t i = 0; i < frames.size(); i++) {
        std::stringstream ss;
        ss << "temp_frames/frame_" << std::setw(3) << std::setfill('0') << i << ".png";
        cv::imwrite(ss.str(), frames[i]);
    }
    
    // Create GIF using Python script
    std::stringstream cmd;
    cmd << "python3 create_gif.py --fps " << fps;
    
    if (std::system(cmd.str().c_str()) != 0) {
        std::cerr << "Error creating GIF" << std::endl;
        // Clean up temporary files
        for (const auto& entry : std::filesystem::directory_iterator("temp_frames")) {
            std::filesystem::remove(entry.path());
        }
        std::filesystem::remove("temp_frames");
        return;
    }
    
    // Clean up temporary files
    // for (const auto& entry : std::filesystem::directory_iterator("temp_frames")) {
    //     std::filesystem::remove(entry.path());
    // }
    // std::filesystem::remove("temp_frames");
    
    // std::cout << "GIF created successfully: " << output_filename << std::endl;
} 