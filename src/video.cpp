#include "video.h"
#include <iostream>
#include <chrono>

namespace sar {

Video::Video() {}

Video::~Video() {
    shutdown();
}

bool Video::init(const VideoConfig& config) {
    m_config = config;
    
    if (!openSource()) {
        std::cerr << "Warning: Could not open video source. Will retry in background." << std::endl;
    }
    
    // Start capture thread
    m_running = true;
    m_thread = std::thread(&Video::captureThread, this);
    
    return true;
}

void Video::shutdown() {
    m_running = false;
    
    if (m_thread.joinable()) {
        m_thread.join();
    }
    
    if (m_capture.isOpened()) {
        m_capture.release();
    }
    
    m_connected = false;
}

bool Video::openSource() {
    // Guard against concurrent access (init vs captureThread)
    std::lock_guard<std::mutex> lock(m_frameMutex);
    
    // Try to parse as integer (camera index) or string (URL/file)
    try {
        int cameraIndex = std::stoi(m_config.source);
        m_capture.open(cameraIndex);
    } catch (...) {
        // Not an integer, treat as URL or file path
        m_capture.open(m_config.source);
    }
    
    if (!m_capture.isOpened()) {
        m_connected = false;
        return false;
    }
    
    // Set capture properties
    m_capture.set(cv::CAP_PROP_FRAME_WIDTH, m_config.width);
    m_capture.set(cv::CAP_PROP_FRAME_HEIGHT, m_config.height);
    m_capture.set(cv::CAP_PROP_FPS, m_config.fps);
    
    // Get actual properties (may differ from requested)
    m_width = static_cast<int>(m_capture.get(cv::CAP_PROP_FRAME_WIDTH));
    m_height = static_cast<int>(m_capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    m_fps = m_capture.get(cv::CAP_PROP_FPS);
    
    if (m_fps <= 0) m_fps = 30.0; // Default fallback
    
    // Set sensible defaults if camera reports 0 (common with some drivers)
    if (m_width <= 0) m_width = m_config.width;
    if (m_height <= 0) m_height = m_config.height;
    
    std::cout << "Video source opened: " << m_config.source << std::endl;
    std::cout << "  Resolution: " << m_width << "x" << m_height << " @ " << m_fps << " fps" << std::endl;
    
    m_connected = true;
    return true;
}

void Video::captureThread() {
    cv::Mat frame;
    cv::VideoCapture localCapture;
    bool needsReconnect = false;
    
    while (m_running) {
        // Check if we need to reconnect
        {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            if (!m_capture.isOpened()) {
                needsReconnect = true;
            }
        }
        
        if (needsReconnect) {
            m_connected = false;
            
            // Try to reconnect
            std::this_thread::sleep_for(std::chrono::milliseconds(m_config.reconnect_delay_ms));
            
            if (m_running && openSource()) {
                std::cout << "Video source reconnected." << std::endl;
            }
            needsReconnect = false;
            continue;
        }
        
        // Read frame (don't hold lock during blocking read)
        bool readSuccess = false;
        {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            if (m_capture.isOpened()) {
                readSuccess = m_capture.read(frame);
            }
        }
        
        if (readSuccess && !frame.empty()) {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            m_latestFrame = frame.clone();
            m_newFrame = true;
            m_connected = true;
        } else if (!readSuccess) {
            // Read failed, probably disconnected
            {
                std::lock_guard<std::mutex> lock(m_frameMutex);
                m_capture.release();
            }
            m_connected = false;
            std::cout << "Video source disconnected. Attempting to reconnect..." << std::endl;
            needsReconnect = true;
        }
        
        // Small sleep to prevent busy-spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

bool Video::getFrame(cv::Mat& frame) {
    std::lock_guard<std::mutex> lock(m_frameMutex);
    
    if (m_latestFrame.empty()) {
        return false;
    }
    
    frame = m_latestFrame.clone();
    m_newFrame = false;
    return true;
}

} // namespace sar
