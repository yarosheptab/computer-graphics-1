#ifndef __GIF_WRITER_H__
#define __GIF_WRITER_H__

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <string>
#include <vector>

class GifWriter {
public:
    static void saveFramesToGif(const std::vector<cv::Mat>& frames, 
                               const std::string& output_filename,
                               int fps = 30);
};

#endif //__GIF_WRITER_H__ 