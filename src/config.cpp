#include "config.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace sar {

using json = nlohmann::json;

Config Config::load(const std::string& path) {
    Config config;
    
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file: " << path << std::endl;
        std::cerr << "Using default configuration." << std::endl;
        return config;
    }
    
    try {
        json j = json::parse(file);
        
        // Video config
        if (j.contains("video")) {
            auto& v = j["video"];
            if (v.contains("source")) config.video.source = v["source"].get<std::string>();
            if (v.contains("width")) config.video.width = v["width"].get<int>();
            if (v.contains("height")) config.video.height = v["height"].get<int>();
            if (v.contains("fps")) config.video.fps = v["fps"].get<int>();
            if (v.contains("reconnect_delay_ms")) config.video.reconnect_delay_ms = v["reconnect_delay_ms"].get<int>();
        }
        
        // Joystick config
        if (j.contains("joystick")) {
            auto& js = j["joystick"];
            if (js.contains("device_index")) config.joystick.device_index = js["device_index"].get<int>();
            if (js.contains("deadzone")) config.joystick.deadzone = js["deadzone"].get<float>();
            if (js.contains("sensitivity")) config.joystick.sensitivity = js["sensitivity"].get<float>();
            if (js.contains("invert_pan")) config.joystick.invert_pan = js["invert_pan"].get<bool>();
            if (js.contains("invert_tilt")) config.joystick.invert_tilt = js["invert_tilt"].get<bool>();
            
            if (js.contains("axis_mapping")) {
                for (auto& [key, val] : js["axis_mapping"].items()) {
                    config.joystick.axis_mapping[key] = val.get<int>();
                }
            }
            
            if (js.contains("button_mapping")) {
                for (auto& [key, val] : js["button_mapping"].items()) {
                    config.joystick.button_mapping[key] = val.get<int>();
                }
            }
        }
        
        // HUD config
        if (j.contains("hud")) {
            auto& h = j["hud"];
            if (h.contains("enabled")) config.hud.enabled = h["enabled"].get<bool>();
            if (h.contains("show_crosshair")) config.hud.show_crosshair = h["show_crosshair"].get<bool>();
            if (h.contains("show_telemetry")) config.hud.show_telemetry = h["show_telemetry"].get<bool>();
            if (h.contains("show_timestamp")) config.hud.show_timestamp = h["show_timestamp"].get<bool>();
            if (h.contains("show_joystick_indicator")) config.hud.show_joystick_indicator = h["show_joystick_indicator"].get<bool>();
            if (h.contains("font_scale")) config.hud.font_scale = h["font_scale"].get<double>();
            if (h.contains("telemetry_position")) config.hud.telemetry_position = h["telemetry_position"].get<std::string>();
            
            if (h.contains("crosshair_color")) {
                auto c = h["crosshair_color"].get<std::vector<int>>();
                if (c.size() >= 3) {
                    config.hud.crosshair_color = {c[0], c[1], c[2]};
                }
            }
            
            if (h.contains("text_color")) {
                auto c = h["text_color"].get<std::vector<int>>();
                if (c.size() >= 3) {
                    config.hud.text_color = {c[0], c[1], c[2]};
                }
            }
        }
        
        // Recording config
        if (j.contains("recording")) {
            auto& r = j["recording"];
            if (r.contains("enabled")) config.recording.enabled = r["enabled"].get<bool>();
            if (r.contains("output_dir")) config.recording.output_dir = r["output_dir"].get<std::string>();
            if (r.contains("format")) config.recording.format = r["format"].get<std::string>();
            if (r.contains("codec")) config.recording.codec = r["codec"].get<std::string>();
            if (r.contains("include_hud")) config.recording.include_hud = r["include_hud"].get<bool>();
        }
        
        // Window config
        if (j.contains("window")) {
            auto& w = j["window"];
            if (w.contains("title")) config.window.title = w["title"].get<std::string>();
            if (w.contains("fullscreen")) config.window.fullscreen = w["fullscreen"].get<bool>();
            if (w.contains("always_on_top")) config.window.always_on_top = w["always_on_top"].get<bool>();
        }
        
    } catch (const json::exception& e) {
        std::cerr << "Error parsing config file: " << e.what() << std::endl;
        std::cerr << "Using default configuration." << std::endl;
    }
    
    return config;
}

void Config::save(const std::string& path) const {
    json j;
    
    // Video
    j["video"]["source"] = video.source;
    j["video"]["width"] = video.width;
    j["video"]["height"] = video.height;
    j["video"]["fps"] = video.fps;
    j["video"]["reconnect_delay_ms"] = video.reconnect_delay_ms;
    
    // Joystick
    j["joystick"]["device_index"] = joystick.device_index;
    j["joystick"]["deadzone"] = joystick.deadzone;
    j["joystick"]["sensitivity"] = joystick.sensitivity;
    j["joystick"]["invert_pan"] = joystick.invert_pan;
    j["joystick"]["invert_tilt"] = joystick.invert_tilt;
    j["joystick"]["axis_mapping"] = joystick.axis_mapping;
    j["joystick"]["button_mapping"] = joystick.button_mapping;
    
    // HUD
    j["hud"]["enabled"] = hud.enabled;
    j["hud"]["show_crosshair"] = hud.show_crosshair;
    j["hud"]["show_telemetry"] = hud.show_telemetry;
    j["hud"]["show_timestamp"] = hud.show_timestamp;
    j["hud"]["show_joystick_indicator"] = hud.show_joystick_indicator;
    j["hud"]["crosshair_color"] = hud.crosshair_color;
    j["hud"]["text_color"] = hud.text_color;
    j["hud"]["font_scale"] = hud.font_scale;
    j["hud"]["telemetry_position"] = hud.telemetry_position;
    
    // Recording
    j["recording"]["enabled"] = recording.enabled;
    j["recording"]["output_dir"] = recording.output_dir;
    j["recording"]["format"] = recording.format;
    j["recording"]["codec"] = recording.codec;
    j["recording"]["include_hud"] = recording.include_hud;
    
    // Window
    j["window"]["title"] = window.title;
    j["window"]["fullscreen"] = window.fullscreen;
    j["window"]["always_on_top"] = window.always_on_top;
    
    // Create directory if needed
    std::filesystem::path filepath(path);
    if (filepath.has_parent_path()) {
        std::filesystem::create_directories(filepath.parent_path());
    }
    
    std::ofstream file(path);
    if (file.is_open()) {
        file << j.dump(2);
    } else {
        std::cerr << "Error: Could not write config file: " << path << std::endl;
    }
}

} // namespace sar
