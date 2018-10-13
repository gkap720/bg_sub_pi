CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(bg_sub)

FIND_PACKAGE(OpenCV 3.1.0 REQUIRED)
set(raspicam_DIR "/usr/local/lib/cmake")
find_package(raspicam REQUIRED)
find_package(numpy REQUIRED)

INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS} "/usr/local/include")

ADD_EXECUTABLE(bg_sub bg_sub.cpp)

FIND_LIBRARY(BGSUBCNT_LIB bgsubcnt "/usr/local/lib")
TARGET_LINK_LIBRARIES (bg_sub ${BGSUBCNT_LIB} ${OpenCV_LIBS} ${raspicam_CV_LIBS})