# Detailed Setup Guide

This guide covers setting up SAR Simulator from scratch on Windows.

## Prerequisites

### 1. Visual Studio 2022

1. Download [Visual Studio 2022 Community](https://visualstudio.microsoft.com/downloads/)
2. Run the installer
3. Select **"Desktop development with C++"** workload
4. Install

### 2. CMake

```powershell
winget install Kitware.CMake
```

Or download from [cmake.org](https://cmake.org/download/)

### 3. vcpkg (Package Manager)

```powershell
# Open PowerShell as Administrator
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Add to PATH (optional but recommended)
$env:PATH += ";C:\vcpkg"

# Install dependencies for SAR Simulator
.\vcpkg install sdl2:x64-windows opencv4:x64-windows nlohmann-json:x64-windows

# Integrate with Visual Studio
.\vcpkg integrate install
```

## Building SAR Simulator

### Option A: Command Line (Recommended)

```powershell
# Clone the repository
git clone https://github.com/yongkangc/sar-simulator.git
cd sar-simulator

# Configure (adjust vcpkg path if different)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build --config Release

# Run
.\build\Release\sar_simulator.exe
```

### Option B: Visual Studio

1. Open Visual Studio 2022
2. File → Open → CMake → Select `sar-simulator/CMakeLists.txt`
3. Select Release configuration
4. Build → Build All
5. Debug → Start Without Debugging

## Connecting Your Equipment

### Joystick Setup

1. Connect your industrial joystick via USB
2. Open Windows "Set up USB game controllers" (`joy.cpl`)
3. Verify your joystick appears and responds to input
4. Note the device name and test which axes control what

List joysticks in SAR Simulator:
```powershell
.\sar_simulator.exe -l
```

### Video Source Setup

#### USB Camera
- Simply connect and note the device index (usually 0)
- Use `source: "0"` in config

#### IP Camera (RTSP)
- Find your camera's RTSP URL (check manual or camera web interface)
- Common formats:
  - `rtsp://192.168.1.100:554/stream`
  - `rtsp://user:password@192.168.1.100:554/h264`
- Test with VLC first: Media → Open Network Stream → paste URL

#### Capture Card
- Install manufacturer drivers
- Camera appears as a numbered device (0, 1, etc.)

## Configuration

### Finding Your Joystick Axes

Most industrial joysticks have multiple axes. To find the correct mapping:

1. Run the simulator with default config
2. Watch the HUD telemetry (shows raw axis values)
3. Move each axis on your joystick and note which number changes
4. Update `config/default.json` with correct axis numbers

### Example: Typical 3-Axis Joystick

```json
"axis_mapping": {
  "pan": 0,      // Left-right
  "tilt": 1,     // Forward-back
  "zoom": 3      // Twist or slider
}
```

### Example: Industrial Control Panel

```json
"axis_mapping": {
  "pan": 2,      // Rotary encoder
  "tilt": 3,     // Slider
  "zoom": 0      // Thumbwheel
},
"invert_tilt": true
```

## Troubleshooting

### "No joysticks found"
- Check USB connection
- Verify in `joy.cpl`
- Try a different USB port
- Install joystick drivers if needed

### Black screen / No video
- Check camera is connected
- Verify camera index (try `-v 0`, `-v 1`, etc.)
- For RTSP: test URL in VLC first
- Check firewall isn't blocking video stream

### Low FPS / Lag
- Reduce resolution in config (`width`/`height`)
- Check CPU usage
- For RTSP: use hardware-accelerated codec on camera

### Build errors
- Ensure vcpkg integration: `vcpkg integrate install`
- Clean and rebuild: delete `build/` folder and re-run cmake
- Check Visual Studio C++ workload is installed

## Running in Production

For training sessions, recommended settings:

```json
{
  "video": {
    "width": 1920,
    "height": 1080,
    "fps": 30
  },
  "recording": {
    "enabled": true,
    "include_hud": true
  },
  "window": {
    "fullscreen": true
  }
}
```

Start with:
```powershell
.\sar_simulator.exe -c config/production.json
```

## Support

Open an issue on GitHub if you encounter problems.
