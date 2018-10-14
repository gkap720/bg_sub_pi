CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(bg_sub)

FIND_PACKAGE(OpenCV 3.1.0 REQUIRED)
set(raspicam_DIR "/usr/local/lib/cmake")
find_package(raspicam REQUIRED)
find_library(wiringPi_LIB wiringPi)

INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS} "/usr/local/include")

ADD_EXECUTABLE(bg_sub bg_sub.cpp)

TARGET_LINK_LIBRARIES (bg_sub ${OpenCV_LIBS} ${raspicam_CV_LIBS} ${wiringPi_LIB})