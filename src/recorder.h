#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <filesystem>
#include "config.h"

namespace sar {

class Recorder {
public:
    Recorder();
    ~Recorder();
    
    void init(const RecordingConfig& config);
    
    bool start(int width, int height, double fps);
    void stop();
    
    void writeFrame(const cv::Mat& frame);
    
    bool isRecording() const { return m_recording; }
    std::string getCurrentFilename() const { return m_currentFilename; }
    
private:
    std::string generateFilename();
    
    RecordingConfig m_config;
    cv::VideoWriter m_writer;
    bool m_recording = false;
    std::string m_currentFilename;
};

} // namespace sar
