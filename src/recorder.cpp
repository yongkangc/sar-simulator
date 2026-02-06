#include "recorder.h"
#include <iostream>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace sar {

Recorder::Recorder() {}

Recorder::~Recorder() {
    stop();
}

void Recorder::init(const RecordingConfig& config) {
    m_config = config;
    
    // Ensure output directory exists
    if (!m_config.output_dir.empty()) {
        std::filesystem::create_directories(m_config.output_dir);
    }
}

bool Recorder::start(int width, int height, double fps) {
    if (m_recording) {
        std::cout << "Already recording." << std::endl;
        return false;
    }
    
    if (!m_config.enabled) {
        std::cout << "Recording is disabled in config." << std::endl;
        return false;
    }
    
    m_currentFilename = generateFilename();
    
    // Get codec fourcc
    int fourcc;
    if (m_config.codec == "mp4v") {
        fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    } else if (m_config.codec == "avc1" || m_config.codec == "h264") {
        fourcc = cv::VideoWriter::fourcc('a', 'v', 'c', '1');
    } else if (m_config.codec == "xvid") {
        fourcc = cv::VideoWriter::fourcc('X', 'V', 'I', 'D');
    } else if (m_config.codec == "mjpg") {
        fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    } else {
        fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    }
    
    m_writer.open(m_currentFilename, fourcc, fps, cv::Size(width, height));
    
    if (!m_writer.isOpened()) {
        std::cerr << "Failed to open video writer: " << m_currentFilename << std::endl;
        return false;
    }
    
    m_recording = true;
    std::cout << "Recording started: " << m_currentFilename << std::endl;
    
    return true;
}

void Recorder::stop() {
    if (!m_recording) return;
    
    m_writer.release();
    m_recording = false;
    
    std::cout << "Recording stopped: " << m_currentFilename << std::endl;
    m_currentFilename.clear();
}

void Recorder::writeFrame(const cv::Mat& frame) {
    if (!m_recording || !m_writer.isOpened()) return;
    
    m_writer.write(frame);
}

std::string Recorder::generateFilename() {
    auto now = std::time(nullptr);
    auto tm = std::localtime(&now);
    
    std::stringstream ss;
    ss << m_config.output_dir << "/sar_";
    ss << std::put_time(tm, "%Y%m%d_%H%M%S");
    ss << "." << m_config.format;
    
    return ss.str();
}

} // namespace sar
