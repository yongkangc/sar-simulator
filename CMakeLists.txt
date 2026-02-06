cmake_minimum_required(VERSION 3.20)
project(sar_simulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find packages
find_package(SDL2 CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/joystick.cpp
    src/video.cpp
    src/hud.cpp
    src/recorder.cpp
    src/config.cpp
)

# Headers
set(HEADERS
    src/joystick.h
    src/video.h
    src/hud.h
    src/recorder.h
    src/config.h
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})

# Copy default config to build directory
configure_file(${CMAKE_SOURCE_DIR}/config/default.json ${CMAKE_BINARY_DIR}/config/default.json COPYONLY)

# Install
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY config/ DESTINATION bin/config)
