#include "joystick.h"
#include <iostream>
#include <cmath>

namespace sar {

// Static axis indices (set during init from config)
static int s_panAxis = 0;
static int s_tiltAxis = 1;
static int s_zoomAxis = 2;
static float s_deadzone = 0.1f;
static float s_sensitivity = 1.0f;
static bool s_invertPan = false;
static bool s_invertTilt = false;

float JoystickState::getPan() const {
    if (s_panAxis < static_cast<int>(axes.size())) {
        float val = axes[s_panAxis];
        if (std::abs(val) < s_deadzone) return 0.0f;
        float sign = val > 0 ? 1.0f : -1.0f;
        float adjusted = (std::abs(val) - s_deadzone) / (1.0f - s_deadzone) * sign * s_sensitivity;
        return s_invertPan ? -adjusted : adjusted;
    }
    return 0.0f;
}

float JoystickState::getTilt() const {
    if (s_tiltAxis < static_cast<int>(axes.size())) {
        float val = axes[s_tiltAxis];
        if (std::abs(val) < s_deadzone) return 0.0f;
        float sign = val > 0 ? 1.0f : -1.0f;
        float adjusted = (std::abs(val) - s_deadzone) / (1.0f - s_deadzone) * sign * s_sensitivity;
        return s_invertTilt ? -adjusted : adjusted;
    }
    return 0.0f;
}

float JoystickState::getZoom() const {
    if (s_zoomAxis < static_cast<int>(axes.size())) {
        float val = axes[s_zoomAxis];
        if (std::abs(val) < s_deadzone) return 0.0f;
        float sign = val > 0 ? 1.0f : -1.0f;
        return (std::abs(val) - s_deadzone) / (1.0f - s_deadzone) * sign * s_sensitivity;
    }
    return 0.0f;
}

Joystick::Joystick() {}

Joystick::~Joystick() {
    shutdown();
}

bool Joystick::init(const JoystickConfig& config) {
    m_config = config;
    
    // Store config values in statics for JoystickState accessors
    s_deadzone = config.deadzone;
    s_sensitivity = config.sensitivity;
    s_invertPan = config.invert_pan;
    s_invertTilt = config.invert_tilt;
    
    // Parse axis mapping
    auto it = config.axis_mapping.find("pan");
    if (it != config.axis_mapping.end()) s_panAxis = it->second;
    
    it = config.axis_mapping.find("tilt");
    if (it != config.axis_mapping.end()) s_tiltAxis = it->second;
    
    it = config.axis_mapping.find("zoom");
    if (it != config.axis_mapping.end()) s_zoomAxis = it->second;
    
    // Initialize SDL joystick subsystem if not already done
    if (!SDL_WasInit(SDL_INIT_JOYSTICK)) {
        if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
            std::cerr << "Failed to initialize SDL joystick: " << SDL_GetError() << std::endl;
            return false;
        }
    }
    
    // Enable joystick events
    SDL_JoystickEventState(SDL_ENABLE);
    
    // Try to open the configured device
    int numJoysticks = SDL_NumJoysticks();
    std::cout << "Found " << numJoysticks << " joystick(s)" << std::endl;
    
    for (int i = 0; i < numJoysticks; i++) {
        std::cout << "  [" << i << "] " << SDL_JoystickNameForIndex(i) << std::endl;
    }
    
    if (config.device_index < numJoysticks) {
        handleDeviceAdded(config.device_index);
    } else if (numJoysticks > 0) {
        std::cout << "Configured device index " << config.device_index 
                  << " not found, using device 0" << std::endl;
        handleDeviceAdded(0);
    } else {
        std::cout << "No joysticks connected. Will auto-detect when plugged in." << std::endl;
    }
    
    return true;
}

void Joystick::shutdown() {
    if (m_joystick) {
        SDL_JoystickClose(m_joystick);
        m_joystick = nullptr;
        m_instanceId = -1;
    }
    m_state = JoystickState();
}

void Joystick::update() {
    // Process SDL events for joystick
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_JOYDEVICEADDED:
                if (!m_joystick) {
                    handleDeviceAdded(event.jdevice.which);
                }
                break;
                
            case SDL_JOYDEVICEREMOVED:
                if (event.jdevice.which == m_instanceId) {
                    handleDeviceRemoved(event.jdevice.which);
                }
                break;
                
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                if (event.jbutton.which == m_instanceId) {
                    int button = event.jbutton.button;
                    bool pressed = (event.type == SDL_JOYBUTTONDOWN);
                    
                    if (button < static_cast<int>(m_state.buttons.size())) {
                        m_state.buttons[button] = pressed;
                    }
                    
                    if (m_buttonCallback) {
                        m_buttonCallback(button, pressed);
                    }
                }
                break;
                
            case SDL_JOYAXISMOTION:
                if (event.jaxis.which == m_instanceId) {
                    int axis = event.jaxis.axis;
                    if (axis < static_cast<int>(m_state.axes.size())) {
                        // Normalize from -32768..32767 to -1.0..1.0
                        m_state.axes[axis] = event.jaxis.value / 32767.0f;
                    }
                }
                break;
                
            case SDL_JOYHATMOTION:
                if (event.jhat.which == m_instanceId) {
                    int hat = event.jhat.hat;
                    if (hat < static_cast<int>(m_state.hats.size())) {
                        m_state.hats[hat] = event.jhat.value;
                    }
                }
                break;
        }
    }
}

void Joystick::handleDeviceAdded(int device_index) {
    m_joystick = SDL_JoystickOpen(device_index);
    if (!m_joystick) {
        std::cerr << "Failed to open joystick " << device_index << ": " << SDL_GetError() << std::endl;
        return;
    }
    
    m_instanceId = SDL_JoystickInstanceID(m_joystick);
    m_state.connected = true;
    m_state.name = SDL_JoystickName(m_joystick);
    
    // Initialize state vectors
    int numAxes = SDL_JoystickNumAxes(m_joystick);
    int numButtons = SDL_JoystickNumButtons(m_joystick);
    int numHats = SDL_JoystickNumHats(m_joystick);
    
    m_state.axes.resize(numAxes, 0.0f);
    m_state.buttons.resize(numButtons, false);
    m_state.hats.resize(numHats, 0);
    
    std::cout << "Joystick connected: " << m_state.name << std::endl;
    std::cout << "  Axes: " << numAxes << ", Buttons: " << numButtons << ", Hats: " << numHats << std::endl;
}

void Joystick::handleDeviceRemoved(SDL_JoystickID instance_id) {
    (void)instance_id;
    std::cout << "Joystick disconnected: " << m_state.name << std::endl;
    
    if (m_joystick) {
        SDL_JoystickClose(m_joystick);
        m_joystick = nullptr;
    }
    
    m_instanceId = -1;
    m_state = JoystickState();
}

float Joystick::applyDeadzone(float value) const {
    if (std::abs(value) < m_config.deadzone) {
        return 0.0f;
    }
    
    float sign = value > 0 ? 1.0f : -1.0f;
    return (std::abs(value) - m_config.deadzone) / (1.0f - m_config.deadzone) * sign;
}

std::vector<std::string> Joystick::enumerateDevices() {
    std::vector<std::string> devices;
    
    if (!SDL_WasInit(SDL_INIT_JOYSTICK)) {
        SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    }
    
    int numJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < numJoysticks; i++) {
        devices.push_back(SDL_JoystickNameForIndex(i));
    }
    
    return devices;
}

} // namespace sar
