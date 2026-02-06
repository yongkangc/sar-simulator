#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include "config.h"

namespace sar {

class Video {
public:
    Video();
    ~Video();
    
    bool init(const VideoConfig& config);
    void shutdown();
    
    bool getFrame(cv::Mat& frame);
    bool isConnected() const { return m_connected.load(); }
    
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    double getFps() const { return m_fps; }
    
private:
    void captureThread();
    bool openSource();
    
    VideoConfig m_config;
    cv::VideoCapture m_capture;
    
    std::thread m_thread;
    std::mutex m_frameMutex;
    cv::Mat m_latestFrame;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_connected{false};
    std::atomic<bool> m_newFrame{false};
    
    int m_width = 0;
    int m_height = 0;
    double m_fps = 0;
};

} // namespace sar
