# SAR Simulator Implementation Plan

## Project Overview
A Search and Rescue (SAR) training simulator that connects industrial joysticks to control a simulated payload with live electro-optical (EO) video feed display.

## Architecture

```
┌─────────────────┐     ┌──────────────────┐     ┌─────────────────┐
│  Industrial     │────▶│   SAR Simulator  │────▶│  Display Window │
│  Joystick       │     │   Application    │     │  (EO Feed + HUD)│
└─────────────────┘     └──────────────────┘     └─────────────────┘
                               │
                               ▼
                        ┌──────────────────┐
                        │  Video Source    │
                        │  (Camera/RTSP)   │
                        └──────────────────┘
```

## Tech Stack
- **Language:** C++ (cross-platform potential, low latency)
- **Build:** CMake + vcpkg
- **Joystick Input:** SDL2 (broad HID support)
- **Video Capture:** OpenCV (simpler integration, overlays)
- **Config:** JSON (nlohmann/json)

## Module Breakdown

### Module 1: Joystick Input (`src/joystick.cpp`)
- Initialize SDL2 joystick subsystem
- Enumerate connected devices
- Read axis values (X, Y, Z, throttle)
- Read button states
- Configurable axis mapping via JSON
- Deadzone handling

### Module 2: Video Pipeline (`src/video.cpp`)
- OpenCV VideoCapture for camera/RTSP
- Frame buffering with separate thread
- Configurable resolution and FPS
- Reconnection handling for network streams

### Module 3: HUD Overlay (`src/hud.cpp`)
- Render joystick position indicator
- Display telemetry (axis values, button states)
- Crosshair/reticle overlay
- Recording indicator
- Timestamp overlay

### Module 4: Recording (`src/recorder.cpp`)
- Record sessions to MP4 (OpenCV VideoWriter)
- Overlay joystick data on recording
- Configurable output path

### Module 5: Configuration (`src/config.cpp`)
- Load/save JSON config
- Joystick axis mapping
- Video source URL/device
- HUD settings
- Keybindings

### Module 6: Main Application (`src/main.cpp`)
- Initialize all modules
- Main event loop
- Keyboard shortcuts (quit, toggle recording, etc.)
- Clean shutdown

## File Structure
```
sar-simulator/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── vcpkg.json              # Dependencies manifest
├── .gitignore
├── config/
│   └── default.json        # Default configuration
├── src/
│   ├── main.cpp
│   ├── joystick.h / .cpp
│   ├── video.h / .cpp
│   ├── hud.h / .cpp
│   ├── recorder.h / .cpp
│   └── config.h / .cpp
└── docs/
    └── SETUP.md            # Detailed setup instructions
```

## Implementation Phases

### Phase 1: Core Infrastructure (Parallel)
- [ ] CMakeLists.txt + vcpkg.json
- [ ] Config module (JSON loading)
- [ ] Basic main loop structure

### Phase 2: Input & Video (Parallel)
- [ ] Joystick module (enumeration, reading)
- [ ] Video module (capture, display)

### Phase 3: Integration
- [ ] HUD overlay rendering
- [ ] Connect joystick to HUD display
- [ ] Main loop integration

### Phase 4: Features
- [ ] Recording module
- [ ] Keyboard shortcuts
- [ ] Error handling & reconnection

### Phase 5: Polish
- [ ] README documentation
- [ ] Setup guide
- [ ] Code review & cleanup

## Dependencies (vcpkg.json)
- SDL2
- OpenCV4
- nlohmann-json

## Build Instructions
```bash
# Clone
git clone https://github.com/gouthambinu/sar-simulator.git
cd sar-simulator

# Configure with vcpkg
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build --config Release

# Run
./build/Release/sar_simulator
```

## Config Example
```json
{
  "video": {
    "source": "0",
    "width": 1280,
    "height": 720,
    "fps": 30
  },
  "joystick": {
    "device_index": 0,
    "deadzone": 0.1,
    "axis_mapping": {
      "pan": 0,
      "tilt": 1,
      "zoom": 2
    }
  },
  "hud": {
    "show_crosshair": true,
    "show_telemetry": true,
    "show_timestamp": true
  },
  "recording": {
    "output_dir": "./recordings",
    "format": "mp4"
  }
}
```

## Success Criteria
1. Detects and reads industrial joystick input
2. Displays live video feed (webcam or RTSP)
3. Renders HUD overlay with joystick state
4. Records training sessions
5. Configurable via JSON file
6. Clear documentation for setup
