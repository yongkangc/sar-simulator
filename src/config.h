#pragma once

#include <string>
#include <array>
#include <map>
#include <nlohmann/json.hpp>

namespace sar {

struct VideoConfig {
    std::string source = "0";
    int width = 1280;
    int height = 720;
    int fps = 30;
    int reconnect_delay_ms = 3000;
};

struct JoystickConfig {
    int device_index = 0;
    float deadzone = 0.1f;
    float sensitivity = 1.0f;
    std::map<std::string, int> axis_mapping;
    std::map<std::string, int> button_mapping;
    bool invert_pan = false;
    bool invert_tilt = false;
};

struct HudConfig {
    bool enabled = true;
    bool show_crosshair = true;
    bool show_telemetry = true;
    bool show_timestamp = true;
    bool show_joystick_indicator = true;
    std::array<int, 3> crosshair_color = {0, 255, 0};
    std::array<int, 3> text_color = {0, 255, 0};
    double font_scale = 0.6;
    std::string telemetry_position = "top_left";
};

struct RecordingConfig {
    bool enabled = true;
    std::string output_dir = "./recordings";
    std::string format = "mp4";
    std::string codec = "mp4v";
    bool include_hud = true;
};

struct WindowConfig {
    std::string title = "SAR Simulator - EO Feed";
    bool fullscreen = false;
    bool always_on_top = false;
};

struct Config {
    VideoConfig video;
    JoystickConfig joystick;
    HudConfig hud;
    RecordingConfig recording;
    WindowConfig window;
    
    static Config load(const std::string& path);
    void save(const std::string& path) const;
};

} // namespace sar
