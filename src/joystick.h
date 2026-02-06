#pragma once

#include <SDL.h>
#include <string>
#include <vector>
#include <functional>
#include "config.h"

namespace sar {

struct JoystickState {
    std::vector<float> axes;          // Normalized -1.0 to 1.0
    std::vector<bool> buttons;
    std::vector<uint8_t> hats;
    bool connected = false;
    std::string name;
    
    // Convenience accessors with deadzone applied
    float getPan() const;
    float getTilt() const;
    float getZoom() const;
};

class Joystick {
public:
    using ButtonCallback = std::function<void(int button, bool pressed)>;
    
    Joystick();
    ~Joystick();
    
    bool init(const JoystickConfig& config);
    void shutdown();
    
    void update();
    
    const JoystickState& getState() const { return m_state; }
    bool isConnected() const { return m_joystick != nullptr; }
    
    void setButtonCallback(ButtonCallback callback) { m_buttonCallback = callback; }
    
    // Static utilities
    static std::vector<std::string> enumerateDevices();
    
private:
    float applyDeadzone(float value) const;
    void handleDeviceAdded(int device_index);
    void handleDeviceRemoved(SDL_JoystickID instance_id);
    
    SDL_Joystick* m_joystick = nullptr;
    SDL_JoystickID m_instanceId = -1;
    JoystickConfig m_config;
    JoystickState m_state;
    ButtonCallback m_buttonCallback;
};

} // namespace sar
