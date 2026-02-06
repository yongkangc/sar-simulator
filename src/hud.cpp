#include "hud.h"
#include <ctime>
#include <sstream>
#include <iomanip>

namespace sar {

Hud::Hud() {}

void Hud::init(const HudConfig& config) {
    m_config = config;
    
    // Convert BGR colors (OpenCV format)
    m_crosshairColor = cv::Scalar(
        config.crosshair_color[2],  // B
        config.crosshair_color[1],  // G
        config.crosshair_color[0]   // R
    );
    
    m_textColor = cv::Scalar(
        config.text_color[2],
        config.text_color[1],
        config.text_color[0]
    );
}

void Hud::render(cv::Mat& frame, const JoystickState& joystick, bool recording) {
    if (!m_config.enabled) return;
    
    if (m_config.show_crosshair) {
        drawCrosshair(frame);
    }
    
    if (m_config.show_telemetry) {
        drawTelemetry(frame, joystick, recording);
    }
    
    if (m_config.show_joystick_indicator) {
        drawJoystickIndicator(frame, joystick);
    }
    
    if (m_config.show_timestamp) {
        drawTimestamp(frame);
    }
}

void Hud::drawCrosshair(cv::Mat& frame) {
    int cx = frame.cols / 2;
    int cy = frame.rows / 2;
    int size = 30;
    int gap = 8;
    int thickness = 2;
    
    // Horizontal lines
    cv::line(frame, cv::Point(cx - size, cy), cv::Point(cx - gap, cy), m_crosshairColor, thickness);
    cv::line(frame, cv::Point(cx + gap, cy), cv::Point(cx + size, cy), m_crosshairColor, thickness);
    
    // Vertical lines
    cv::line(frame, cv::Point(cx, cy - size), cv::Point(cx, cy - gap), m_crosshairColor, thickness);
    cv::line(frame, cv::Point(cx, cy + gap), cv::Point(cx, cy + size), m_crosshairColor, thickness);
    
    // Center dot
    cv::circle(frame, cv::Point(cx, cy), 2, m_crosshairColor, -1);
}

void Hud::drawTelemetry(cv::Mat& frame, const JoystickState& joystick, bool recording) {
    int x, y;
    
    if (m_config.telemetry_position == "top_left") {
        x = 10;
        y = 25;
    } else if (m_config.telemetry_position == "top_right") {
        x = frame.cols - 200;
        y = 25;
    } else if (m_config.telemetry_position == "bottom_left") {
        x = 10;
        y = frame.rows - 100;
    } else {
        x = frame.cols - 200;
        y = frame.rows - 100;
    }
    
    int lineHeight = 22;
    
    // Connection status
    std::string status = joystick.connected ? "Joystick: CONNECTED" : "Joystick: DISCONNECTED";
    cv::Scalar statusColor = joystick.connected ? m_textColor : cv::Scalar(0, 0, 255);
    cv::putText(frame, status, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 
                m_config.font_scale, statusColor, 1);
    y += lineHeight;
    
    if (joystick.connected) {
        // Show joystick name
        cv::putText(frame, joystick.name.substr(0, 25), cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX,
                    m_config.font_scale * 0.8, m_textColor, 1);
        y += lineHeight;
        
        // Axis values
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "Pan: " << joystick.getPan() << "  Tilt: " << joystick.getTilt();
        cv::putText(frame, ss.str(), cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX,
                    m_config.font_scale, m_textColor, 1);
        y += lineHeight;
        
        ss.str("");
        ss << "Zoom: " << joystick.getZoom();
        cv::putText(frame, ss.str(), cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX,
                    m_config.font_scale, m_textColor, 1);
        y += lineHeight;
    }
    
    // Recording indicator
    if (recording) {
        cv::circle(frame, cv::Point(x + 8, y + 5), 8, cv::Scalar(0, 0, 255), -1);
        cv::putText(frame, "REC", cv::Point(x + 22, y + 10), cv::FONT_HERSHEY_SIMPLEX,
                    m_config.font_scale, cv::Scalar(0, 0, 255), 2);
    }
}

void Hud::drawJoystickIndicator(cv::Mat& frame, const JoystickState& joystick) {
    // Draw in bottom-right corner
    int size = 80;
    int margin = 20;
    int cx = frame.cols - margin - size / 2;
    int cy = frame.rows - margin - size / 2;
    
    // Background circle
    cv::circle(frame, cv::Point(cx, cy), size / 2, cv::Scalar(50, 50, 50), -1);
    cv::circle(frame, cv::Point(cx, cy), size / 2, m_crosshairColor, 1);
    
    // Crosshair
    cv::line(frame, cv::Point(cx - size/2, cy), cv::Point(cx + size/2, cy), 
             cv::Scalar(80, 80, 80), 1);
    cv::line(frame, cv::Point(cx, cy - size/2), cv::Point(cx, cy + size/2), 
             cv::Scalar(80, 80, 80), 1);
    
    // Joystick position
    if (joystick.connected) {
        int dx = static_cast<int>(joystick.getPan() * (size / 2 - 5));
        int dy = static_cast<int>(joystick.getTilt() * (size / 2 - 5));
        
        cv::circle(frame, cv::Point(cx + dx, cy + dy), 6, m_crosshairColor, -1);
    }
}

void Hud::drawTimestamp(cv::Mat& frame) {
    // Get current time
    auto now = std::time(nullptr);
    auto tm = std::localtime(&now);
    
    std::stringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    
    // Draw in top-right corner
    int baseline;
    cv::Size textSize = cv::getTextSize(ss.str(), cv::FONT_HERSHEY_SIMPLEX, 
                                         m_config.font_scale, 1, &baseline);
    
    int x = frame.cols - textSize.width - 10;
    int y = 25;
    
    cv::putText(frame, ss.str(), cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX,
                m_config.font_scale, m_textColor, 1);
}

} // namespace sar
