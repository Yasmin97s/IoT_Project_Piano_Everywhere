# CMakeList.txt : CMake project for OpenCV_Sandbox2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("OpenCV_Sandbox2")

set(OpenCV_DIR "C:/opencv/build/x64/vc15/lib")

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# Add source to this project's executable.
add_executable (OpenCV_Sandbox2 "OpenCV_Sandbox2.cpp" "OpenCV_Sandbox2.h" "Colors.h" "Detector.h" "MyIncludes.h" "PianoDetector.h"  "HandDetector.h"  "PianoHandler.h" )

# TODO: Add tests and install targets if needed.
target_link_libraries( OpenCV_Sandbox2 ${OpenCV_LIBS} )

include_directories("C:\\Program Files (x86)\\Intel RealSense SDK 2.0\\include")
link_directories("C:\\Program Files (x86)\\Intel RealSense SDK 2.0\\lib\\x64")

target_link_libraries(OpenCV_Sandbox2 realsense2.lib)