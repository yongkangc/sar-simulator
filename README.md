# SAR Simulator

**Search and Rescue Training Simulator** — Connect industrial joysticks to control payloads with live electro-optical (EO) video feeds.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)

## Features

- 🎮 **Industrial Joystick Support** — SDL2-based input with configurable axis mapping and deadzone
- 📹 **Live Video Feed** — USB cameras, RTSP streams, or video files via OpenCV
- 🎯 **HUD Overlay** — Crosshair, telemetry, joystick indicator, timestamp
- ⏺️ **Session Recording** — Record training sessions to MP4 with optional HUD
- ⚙️ **Fully Configurable** — JSON configuration for all settings
- 🔌 **Hot-plug Support** — Auto-detect joystick connect/disconnect

## Quick Start

### Prerequisites

- **Windows:** Visual Studio 2022 with C++ workload
- **Linux:** GCC 10+ or Clang 12+, CMake 3.20+
- **All platforms:** [vcpkg](https://github.com/microsoft/vcpkg)

### Build

```bash
# Clone the repository
git clone https://github.com/yongkangc/sar-simulator.git
cd sar-simulator

# Build with vcpkg (Windows)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release

# Build with vcpkg (Linux/macOS)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

### Run

```bash
# Windows
.\build\Release\sar_simulator.exe

# Linux/macOS
./build/sar_simulator
```

## Usage

```
SAR Simulator - Search and Rescue Training Simulator

Usage: sar_simulator [options]

Options:
  -c, --config <path>   Path to config file (default: config/default.json)
  -v, --video <source>  Video source (camera index or RTSP URL)
  -j, --joystick <idx>  Joystick device index (default: 0)
  -l, --list-joysticks  List available joysticks and exit
  -h, --help            Show this help message
```

### Keyboard Controls

| Key | Action |
|-----|--------|
| `R` | Toggle recording |
| `F` | Toggle fullscreen |
| `H` | Toggle HUD |
| `S` | Take screenshot |
| `Q` / `ESC` | Quit |

### Examples

```bash
# Use webcam (index 0) with default config
./sar_simulator

# Use RTSP camera
./sar_simulator -v "rtsp://192.168.1.100:554/stream"

# Use specific joystick
./sar_simulator -j 1

# Custom config file
./sar_simulator -c my_config.json
```

## Configuration

Edit `config/default.json` to customize:

```json
{
  "video": {
    "source": "0",           // Camera index or RTSP URL
    "width": 1280,
    "height": 720,
    "fps": 30
  },
  "joystick": {
    "device_index": 0,
    "deadzone": 0.1,
    "axis_mapping": {
      "pan": 0,              // Axis index for pan
      "tilt": 1,             // Axis index for tilt
      "zoom": 2              // Axis index for zoom
    },
    "invert_pan": false,
    "invert_tilt": false
  },
  "hud": {
    "show_crosshair": true,
    "show_telemetry": true,
    "show_timestamp": true
  },
  "recording": {
    "output_dir": "./recordings",
    "include_hud": true
  }
}
```

## Joystick Mapping

Industrial joysticks vary widely. Use `-l` to list connected devices, then adjust `axis_mapping` in the config:

```bash
# Find your joystick's axis indices
./sar_simulator -l
```

Common mappings:
- **Standard 2-axis:** pan=0, tilt=1
- **3-axis with twist:** pan=0, tilt=1, zoom=2
- **Throttle quadrant:** zoom=3 or 4

## Project Structure

```
sar-simulator/
├── CMakeLists.txt      # Build configuration
├── vcpkg.json          # Dependencies
├── config/
│   └── default.json    # Default configuration
├── src/
│   ├── main.cpp        # Application entry point
│   ├── config.cpp/h    # Configuration handling
│   ├── joystick.cpp/h  # Joystick input (SDL2)
│   ├── video.cpp/h     # Video capture (OpenCV)
│   ├── hud.cpp/h       # HUD overlay rendering
│   └── recorder.cpp/h  # Session recording
└── docs/
    └── SETUP.md        # Detailed setup guide
```

## Dependencies

Managed via vcpkg:
- **SDL2** — Joystick input
- **OpenCV** — Video capture and display
- **nlohmann-json** — Configuration parsing

## Contributing

Contributions welcome! Please open an issue or PR.

## License

MIT License — see [LICENSE](LICENSE) for details.

---

Built for SAR training operators. Stay safe out there. 🚁
