#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <csignal>
#include <SDL.h>
#include <opencv2/opencv.hpp>

#include "config.h"
#include "joystick.h"
#include "video.h"
#include "hud.h"
#include "recorder.h"

using namespace sar;

// Global flag for clean shutdown
static volatile bool g_running = true;

void signalHandler(int signum) {
    (void)signum;
    g_running = false;
}

void printUsage(const char* programName) {
    std::cout << "SAR Simulator - Search and Rescue Training Simulator\n\n";
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -c, --config <path>   Path to config file (default: config/default.json)\n";
    std::cout << "  -v, --video <source>  Video source (camera index or RTSP URL)\n";
    std::cout << "  -j, --joystick <idx>  Joystick device index (default: 0)\n";
    std::cout << "  -l, --list-joysticks  List available joysticks and exit\n";
    std::cout << "  -h, --help            Show this help message\n\n";
    std::cout << "Keyboard Controls:\n";
    std::cout << "  R         Toggle recording\n";
    std::cout << "  F         Toggle fullscreen\n";
    std::cout << "  H         Toggle HUD\n";
    std::cout << "  S         Take screenshot\n";
    std::cout << "  Q / ESC   Quit\n";
}

void listJoysticks() {
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return;
    }
    
    int count = SDL_NumJoysticks();
    std::cout << "Found " << count << " joystick(s):\n";
    
    for (int i = 0; i < count; i++) {
        std::cout << "  [" << i << "] " << SDL_JoystickNameForIndex(i) << "\n";
    }
    
    if (count == 0) {
        std::cout << "  (none)\n";
    }
    
    SDL_Quit();
}

void takeScreenshot(const cv::Mat& frame) {
    auto now = std::time(nullptr);
    auto tm = std::localtime(&now);
    
    std::stringstream ss;
    ss << "screenshot_" << std::put_time(tm, "%Y%m%d_%H%M%S") << ".png";
    
    cv::imwrite(ss.str(), frame);
    std::cout << "Screenshot saved: " << ss.str() << std::endl;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string configPath = "config/default.json";
    std::string videoOverride;
    int joystickOverride = -1;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-l" || arg == "--list-joysticks") {
            listJoysticks();
            return 0;
        } else if ((arg == "-c" || arg == "--config") && i + 1 < argc) {
            configPath = argv[++i];
        } else if ((arg == "-v" || arg == "--video") && i + 1 < argc) {
            videoOverride = argv[++i];
        } else if ((arg == "-j" || arg == "--joystick") && i + 1 < argc) {
            joystickOverride = std::stoi(argv[++i]);
        }
    }
    
    // Set up signal handler
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Load configuration
    std::cout << "Loading config from: " << configPath << std::endl;
    Config config = Config::load(configPath);
    
    // Apply command line overrides
    if (!videoOverride.empty()) {
        config.video.source = videoOverride;
    }
    if (joystickOverride >= 0) {
        config.joystick.device_index = joystickOverride;
    }
    
    // Initialize SDL (for joystick and window events)
    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_EVENTS) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Initialize components
    Joystick joystick;
    if (!joystick.init(config.joystick)) {
        std::cerr << "Warning: Joystick initialization failed. Continuing without joystick." << std::endl;
    }
    
    Video video;
    if (!video.init(config.video)) {
        std::cerr << "Warning: Video initialization failed. Will retry in background." << std::endl;
    }
    
    Hud hud;
    hud.init(config.hud);
    
    Recorder recorder;
    recorder.init(config.recording);
    
    // Set up joystick button callback for recording toggle
    joystick.setButtonCallback([&](int button, bool pressed) {
        if (!pressed) return;  // Only handle press, not release
        
        // Check button mapping
        auto it = config.joystick.button_mapping.find("record_toggle");
        if (it != config.joystick.button_mapping.end() && button == it->second) {
            if (recorder.isRecording()) {
                recorder.stop();
            } else {
                recorder.start(video.getWidth(), video.getHeight(), video.getFps());
            }
        }
        
        it = config.joystick.button_mapping.find("snapshot");
        if (it != config.joystick.button_mapping.end() && button == it->second) {
            cv::Mat frame;
            if (video.getFrame(frame)) {
                takeScreenshot(frame);
            }
        }
    });
    
    // Create display window
    cv::namedWindow(config.window.title, cv::WINDOW_NORMAL);
    if (config.window.fullscreen) {
        cv::setWindowProperty(config.window.title, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
    }
    
    std::cout << "\nSAR Simulator running. Press Q or ESC to quit.\n" << std::endl;
    
    cv::Mat frame;
    cv::Mat displayFrame;
    bool fullscreen = config.window.fullscreen;
    bool hudEnabled = config.hud.enabled;
    
    // Main loop
    while (g_running) {
        // Update joystick
        joystick.update();
        
        // Get video frame
        if (video.getFrame(frame)) {
            // Clone for display (HUD overlay)
            displayFrame = frame.clone();
            
            // Render HUD if enabled
            if (hudEnabled) {
                hud.render(displayFrame, joystick.getState(), recorder.isRecording());
            }
            
            // Record frame (with or without HUD based on config)
            if (recorder.isRecording()) {
                if (config.recording.include_hud) {
                    recorder.writeFrame(displayFrame);
                } else {
                    recorder.writeFrame(frame);
                }
            }
            
            // Display
            cv::imshow(config.window.title, displayFrame);
        }
        
        // Handle keyboard
        int key = cv::waitKey(1) & 0xFF;
        
        if (key == 'q' || key == 'Q' || key == 27) {  // Q or ESC
            g_running = false;
        } else if (key == 'r' || key == 'R') {
            if (recorder.isRecording()) {
                recorder.stop();
            } else {
                recorder.start(video.getWidth(), video.getHeight(), video.getFps());
            }
        } else if (key == 'f' || key == 'F') {
            fullscreen = !fullscreen;
            cv::setWindowProperty(config.window.title, cv::WND_PROP_FULLSCREEN, 
                                  fullscreen ? cv::WINDOW_FULLSCREEN : cv::WINDOW_NORMAL);
        } else if (key == 'h' || key == 'H') {
            hudEnabled = !hudEnabled;
            std::cout << "HUD " << (hudEnabled ? "enabled" : "disabled") << std::endl;
        } else if (key == 's' || key == 'S') {
            if (!displayFrame.empty()) {
                takeScreenshot(displayFrame);
            }
        }
        
        // Check if window was closed
        if (cv::getWindowProperty(config.window.title, cv::WND_PROP_VISIBLE) < 1) {
            g_running = false;
        }
    }
    
    // Cleanup
    std::cout << "\nShutting down..." << std::endl;
    
    recorder.stop();
    video.shutdown();
    joystick.shutdown();
    
    cv::destroyAllWindows();
    SDL_Quit();
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}
