# Integration Guide

This document explains the SAR Simulator architecture and how to integrate custom hardware and software.

---

## System Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           SAR SIMULATOR                                      │
│                                                                             │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌───────────┐ │
│  │   Joystick   │    │    Video     │    │     HUD      │    │  Recorder │ │
│  │   Module     │    │    Module    │    │    Module    │    │   Module  │ │
│  └──────┬───────┘    └──────┬───────┘    └──────┬───────┘    └─────┬─────┘ │
│         │                   │                   │                  │       │
│         │    ┌──────────────┴──────────────┐    │                  │       │
│         │    │                             │    │                  │       │
│         └───►│         MAIN LOOP           │◄───┘                  │       │
│              │                             │◄─────────────────────►│       │
│              │   1. Poll joystick          │                       │       │
│              │   2. Get video frame        │                       │       │
│              │   3. Render HUD overlay     │                       │       │
│              │   4. Record if enabled      │                       │       │
│              │   5. Display frame          │                       │       │
│              │   6. Handle keyboard        │                       │       │
│              │                             │                       │       │
│              └──────────────┬──────────────┘                       │       │
│                             │                                      │       │
│                             ▼                                      │       │
│                      ┌─────────────┐                               │       │
│                      │   Display   │                               │       │
│                      │   Window    │                               │       │
│                      └─────────────┘                               │       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Data Flow

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  JOYSTICK   │     │   CAMERA    │     │   CONFIG    │     │    USER     │
│  (Hardware) │     │  (Hardware) │     │   (JSON)    │     │  (Keyboard) │
└──────┬──────┘     └──────┬──────┘     └──────┬──────┘     └──────┬──────┘
       │                   │                   │                   │
       │ USB HID           │ USB/RTSP          │ File              │ Events
       ▼                   ▼                   ▼                   ▼
┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│    SDL2      │    │   OpenCV     │    │  nlohmann    │    │   OpenCV     │
│  Joystick    │    │ VideoCapture │    │    JSON      │    │  waitKey()   │
└──────┬───────┘    └──────┬───────┘    └──────┬───────┘    └──────┬───────┘
       │                   │                   │                   │
       │ JoystickState     │ cv::Mat           │ Config            │ int key
       ▼                   ▼                   ▼                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                              MAIN LOOP                                       │
│                                                                             │
│   joystick.update() ──► video.getFrame() ──► hud.render() ──► cv::imshow() │
│         │                      │                   │                        │
│         │                      │                   │                        │
│         ▼                      ▼                   ▼                        │
│   Button callbacks      Frame cloning       Overlay drawing                 │
│   (record toggle)       (thread-safe)       (crosshair, text)              │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Module Interfaces

### Joystick Module

```
┌────────────────────────────────────────────────────────────────┐
│                      JOYSTICK MODULE                            │
├────────────────────────────────────────────────────────────────┤
│                                                                │
│  INPUT:                                                        │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  USB HID Device (Industrial Joystick)                   │  │
│  │  - Axes: Pan, Tilt, Zoom, Throttle, etc.                │  │
│  │  - Buttons: Trigger, Thumb, etc.                        │  │
│  │  - Hats: POV switches                                   │  │
│  └─────────────────────────────────────────────────────────┘  │
│                           │                                    │
│                           ▼                                    │
│  PROCESSING:                                                   │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  1. SDL_PollEvent() - get joystick events               │  │
│  │  2. Normalize axes: -32768..32767 → -1.0..1.0           │  │
│  │  3. Apply deadzone (configurable, default 10%)          │  │
│  │  4. Apply sensitivity multiplier                        │  │
│  │  5. Apply axis inversion if configured                  │  │
│  └─────────────────────────────────────────────────────────┘  │
│                           │                                    │
│                           ▼                                    │
│  OUTPUT:                                                       │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  JoystickState {                                        │  │
│  │    axes[]    : raw normalized values                    │  │
│  │    buttons[] : pressed states                           │  │
│  │    pan       : processed pan value (-1.0 to 1.0)        │  │
│  │    tilt      : processed tilt value (-1.0 to 1.0)       │  │
│  │    zoom      : processed zoom value (-1.0 to 1.0)       │  │
│  │    connected : true/false                               │  │
│  │    name      : device name string                       │  │
│  │  }                                                      │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

### Video Module

```
┌────────────────────────────────────────────────────────────────┐
│                       VIDEO MODULE                              │
├────────────────────────────────────────────────────────────────┤
│                                                                │
│  SUPPORTED SOURCES:                                            │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  "0", "1", "2"     → USB camera by index                │  │
│  │  "rtsp://..."      → IP camera RTSP stream              │  │
│  │  "http://..."      → MJPEG stream                       │  │
│  │  "video.mp4"       → Video file playback                │  │
│  └─────────────────────────────────────────────────────────┘  │
│                           │                                    │
│                           ▼                                    │
│  CAPTURE THREAD (Background):                                  │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  while (running) {                                      │  │
│  │    if (!connected) reconnect();                         │  │
│  │    frame = capture.read();                              │  │
│  │    mutex.lock();                                        │  │
│  │    latestFrame = frame.clone();                         │  │
│  │    mutex.unlock();                                      │  │
│  │  }                                                      │  │
│  └─────────────────────────────────────────────────────────┘  │
│                           │                                    │
│                           ▼                                    │
│  MAIN THREAD ACCESS:                                           │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  video.getFrame(frame)  → thread-safe frame copy        │  │
│  │  video.getWidth()       → current frame width           │  │
│  │  video.getHeight()      → current frame height          │  │
│  │  video.getFps()         → frames per second             │  │
│  │  video.isConnected()    → connection status             │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

---

## Integration Points

### 1. Adding a New Joystick Type

```
YOUR INDUSTRIAL JOYSTICK
         │
         │  USB cable
         ▼
┌─────────────────────────────────────────┐
│           WINDOWS USB STACK             │
│                                         │
│  1. Install drivers (if needed)         │
│  2. Verify in joy.cpl                   │
│  3. Note axis/button mapping            │
│                                         │
└────────────────────┬────────────────────┘
                     │
                     │  HID reports
                     ▼
┌─────────────────────────────────────────┐
│              SDL2 LAYER                 │
│                                         │
│  SDL2 auto-detects HID joysticks        │
│  No code changes needed!                │
│                                         │
└────────────────────┬────────────────────┘
                     │
                     │  Axis indices
                     ▼
┌─────────────────────────────────────────┐
│          CONFIG (default.json)          │
│                                         │
│  Map YOUR joystick's axes:              │
│  {                                      │
│    "joystick": {                        │
│      "axis_mapping": {                  │
│        "pan": 0,    ← YOUR pan axis     │
│        "tilt": 1,   ← YOUR tilt axis    │
│        "zoom": 3    ← YOUR zoom axis    │
│      },                                 │
│      "invert_tilt": true  ← if needed   │
│    }                                    │
│  }                                      │
│                                         │
└─────────────────────────────────────────┘
```

**Steps:**
1. Plug in joystick
2. Run `sar_simulator.exe -l` to see device index
3. Run simulator and watch HUD telemetry
4. Move each axis and note which number changes
5. Update `config/default.json` with correct mapping

---

### 2. Connecting an IP Camera (RTSP)

```
┌─────────────────────┐         ┌─────────────────────┐
│   EO PAYLOAD        │         │   SAR SIMULATOR     │
│   CAMERA            │  WiFi/  │                     │
│                     │  Ethernet│                     │
│  ┌───────────────┐  │         │  ┌───────────────┐  │
│  │ Video Encoder │──┼────────►│──│ OpenCV Decode │  │
│  │ (H.264/MJPEG) │  │  RTSP   │  │               │  │
│  └───────────────┘  │  Stream │  └───────────────┘  │
│                     │         │                     │
└─────────────────────┘         └─────────────────────┘

RTSP URL Format:
rtsp://[username:password@]<ip-address>[:port]/[path]

Examples:
- rtsp://192.168.1.100:554/stream
- rtsp://admin:password@10.0.0.50/live/ch0
- rtsp://camera.local/h264
```

**Config:**
```json
{
  "video": {
    "source": "rtsp://192.168.1.100:554/stream",
    "width": 1920,
    "height": 1080,
    "fps": 30,
    "reconnect_delay_ms": 3000
  }
}
```

---

### 3. Adding Custom HUD Elements

```cpp
// In src/hud.cpp, add to render():

void Hud::render(cv::Mat& frame, const JoystickState& joystick, bool recording) {
    // ... existing code ...
    
    // Add your custom element
    drawCustomElement(frame, joystick);
}

void Hud::drawCustomElement(cv::Mat& frame, const JoystickState& joystick) {
    // Example: Draw compass based on pan value
    int cx = frame.cols / 2;
    int cy = 50;
    int radius = 30;
    
    // Circle background
    cv::circle(frame, cv::Point(cx, cy), radius, cv::Scalar(50, 50, 50), -1);
    
    // Compass needle (rotates with pan)
    float angle = joystick.getPan() * M_PI;  // -π to π
    int nx = cx + static_cast<int>(radius * 0.8 * sin(angle));
    int ny = cy - static_cast<int>(radius * 0.8 * cos(angle));
    cv::line(frame, cv::Point(cx, cy), cv::Point(nx, ny), cv::Scalar(0, 255, 0), 2);
}
```

---

### 4. Sending Joystick Data to External System

```
┌─────────────────────────────────────────────────────────────────┐
│                     INTEGRATION OPTIONS                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  OPTION A: Serial/COM Port                                      │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  SAR Simulator ──► Serial Port ──► Gimbal Controller    │   │
│  │                                                         │   │
│  │  // Add to main loop:                                   │   │
│  │  serial.write(joystick.getPan(), joystick.getTilt());   │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  OPTION B: UDP Packets                                          │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  SAR Simulator ──► UDP Socket ──► Ground Station        │   │
│  │                                                         │   │
│  │  struct ControlPacket {                                 │   │
│  │    float pan;                                           │   │
│  │    float tilt;                                          │   │
│  │    float zoom;                                          │   │
│  │    uint32_t timestamp;                                  │   │
│  │  };                                                     │   │
│  │  udp.send(packet, "192.168.1.50", 5000);                │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  OPTION C: Shared Memory                                        │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  SAR Simulator ──► Shared Memory ──► Other Process      │   │
│  │                                                         │   │
│  │  // For local inter-process communication               │   │
│  │  // Use boost::interprocess or Windows shared memory    │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

### 5. Full System Integration Example

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      COMPLETE SAR TRAINING SETUP                             │
└─────────────────────────────────────────────────────────────────────────────┘

                    ┌─────────────────────────────────┐
                    │       OPERATOR STATION          │
                    │       (Windows Laptop)          │
                    │                                 │
                    │  ┌───────────────────────────┐  │
                    │  │     SAR SIMULATOR         │  │
                    │  │                           │  │
                    │  │  ┌─────────┐ ┌─────────┐  │  │
                    │  │  │  HUD    │ │  Video  │  │  │
                    │  │  │         │ │  Feed   │  │  │
                    │  │  └─────────┘ └─────────┘  │  │
                    │  │                           │  │
                    │  └───────────────────────────┘  │
                    │              ▲                  │
                    └──────────────┼──────────────────┘
                                   │
              ┌────────────────────┼────────────────────┐
              │                    │                    │
              ▼                    │                    ▼
┌─────────────────────┐            │         ┌─────────────────────┐
│  INDUSTRIAL         │            │         │   EO PAYLOAD        │
│  JOYSTICK           │            │         │   CAMERA            │
│                     │            │         │                     │
│  ┌───────────────┐  │     WiFi / │         │  ┌───────────────┐  │
│  │ Pan/Tilt/Zoom │  │    Ethernet│         │  │ Live Video    │  │
│  │ Controls      │──┼─ USB ──────┤         │  │ Stream        │──┤
│  │               │  │            │         │  │ (RTSP/H.264)  │  │
│  └───────────────┘  │            │         │  └───────────────┘  │
│                     │            │         │                     │
│  ┌───────────────┐  │            │         │  ┌───────────────┐  │
│  │ Buttons       │  │            │         │  │ Gimbal        │  │
│  │ (Record, etc) │  │            │         │  │ (optional)    │  │
│  └───────────────┘  │            │         │  └───────────────┘  │
│                     │            │         │                     │
└─────────────────────┘            │         └─────────────────────┘
                                   │
                          ┌────────┴────────┐
                          │    NETWORK      │
                          │    SWITCH       │
                          └─────────────────┘


DATA FLOW:
──────────

1. JOYSTICK → SIMULATOR
   [USB HID] → [SDL2] → [JoystickState] → [HUD Display]

2. CAMERA → SIMULATOR  
   [RTSP Stream] → [OpenCV] → [Frame Buffer] → [Display + Recording]

3. SIMULATOR → RECORDING
   [Frame + HUD] → [VideoWriter] → [MP4 File]


NETWORK CONFIG:
───────────────

Laptop IP:     192.168.1.10
Camera IP:     192.168.1.100
Subnet:        255.255.255.0
Camera URL:    rtsp://192.168.1.100:554/stream
```

---

## Extending the Code

### Adding a New Module

```
src/
├── existing files...
└── telemetry.h / .cpp    ← NEW MODULE

Step 1: Create header (telemetry.h)
┌────────────────────────────────────────┐
│  #pragma once                          │
│  namespace sar {                       │
│    class Telemetry {                   │
│    public:                             │
│      void init(const Config& cfg);     │
│      void update(const JoystickState&);│
│      void sendToGCS();                 │
│    };                                  │
│  }                                     │
└────────────────────────────────────────┘

Step 2: Implement (telemetry.cpp)
┌────────────────────────────────────────┐
│  #include "telemetry.h"                │
│  namespace sar {                       │
│    void Telemetry::init(...) { }       │
│    void Telemetry::update(...) { }     │
│    void Telemetry::sendToGCS() { }     │
│  }                                     │
└────────────────────────────────────────┘

Step 3: Add to CMakeLists.txt
┌────────────────────────────────────────┐
│  set(SOURCES                           │
│    ...                                 │
│    src/telemetry.cpp    ← ADD          │
│  )                                     │
└────────────────────────────────────────┘

Step 4: Use in main.cpp
┌────────────────────────────────────────┐
│  #include "telemetry.h"                │
│  ...                                   │
│  Telemetry telemetry;                  │
│  telemetry.init(config);               │
│  ...                                   │
│  // In main loop:                      │
│  telemetry.update(joystick.getState());│
│  telemetry.sendToGCS();                │
└────────────────────────────────────────┘
```

---

## Troubleshooting Integration Issues

```
PROBLEM                          SOLUTION
─────────────────────────────────────────────────────────────────

Joystick not detected      →    1. Check USB connection
                                2. Run joy.cpl to verify Windows sees it
                                3. Install manufacturer drivers
                                4. Try different USB port

Wrong axis mapping         →    1. Run with default config
                                2. Watch HUD telemetry
                                3. Move each axis, note the number
                                4. Update axis_mapping in config

RTSP stream not working    →    1. Test URL in VLC first
                                2. Check firewall (allow port 554)
                                3. Verify camera IP is reachable (ping)
                                4. Check username/password if required

High latency video         →    1. Use wired Ethernet, not WiFi
                                2. Reduce resolution in config
                                3. Use H.264 instead of MJPEG
                                4. Check CPU usage

Recording fails            →    1. Verify video is connected first
                                2. Check output_dir exists and writable
                                3. Try different codec (mjpg is safest)
```

---

## Reference: Config Schema

```json
{
  "video": {
    "source": "string",           // Camera index or URL
    "width": 1280,                // Desired width
    "height": 720,                // Desired height  
    "fps": 30,                    // Desired FPS
    "reconnect_delay_ms": 3000    // Reconnect wait time
  },
  "joystick": {
    "device_index": 0,            // Which joystick (0 = first)
    "deadzone": 0.1,              // 10% deadzone
    "sensitivity": 1.0,           // Multiplier
    "axis_mapping": {
      "pan": 0,                   // Axis index for pan
      "tilt": 1,                  // Axis index for tilt
      "zoom": 2                   // Axis index for zoom
    },
    "button_mapping": {
      "record_toggle": 0,         // Button to toggle recording
      "snapshot": 1               // Button to take screenshot
    },
    "invert_pan": false,
    "invert_tilt": false
  },
  "hud": {
    "enabled": true,
    "show_crosshair": true,
    "show_telemetry": true,
    "show_timestamp": true,
    "show_joystick_indicator": true,
    "crosshair_color": [0, 255, 0],  // RGB
    "text_color": [0, 255, 0],
    "font_scale": 0.6
  },
  "recording": {
    "enabled": true,
    "output_dir": "./recordings",
    "format": "mp4",
    "codec": "mp4v",              // mp4v, avc1, xvid, mjpg
    "include_hud": true
  }
}
```
