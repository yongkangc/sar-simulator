#pragma once

#include <opencv2/opencv.hpp>
#include "config.h"
#include "joystick.h"

namespace sar {

class Hud {
public:
    Hud();
    
    void init(const HudConfig& config);
    
    void render(cv::Mat& frame, const JoystickState& joystick, bool recording);
    
private:
    void drawCrosshair(cv::Mat& frame);
    void drawTelemetry(cv::Mat& frame, const JoystickState& joystick, bool recording);
    void drawJoystickIndicator(cv::Mat& frame, const JoystickState& joystick);
    void drawTimestamp(cv::Mat& frame);
    
    HudConfig m_config;
    cv::Scalar m_crosshairColor;
    cv::Scalar m_textColor;
};

} // namespace sar
