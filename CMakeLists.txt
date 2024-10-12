cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# add custom_board xiao
list(APPEND EXTRA_COMPONENT_DIRS "boards/components")

list(APPEND EXTRA_COMPONENT_DIRS "third_party/esp-adf/components")
list(APPEND EXTRA_COMPONENT_DIRS "third_party/esp-adf/examples/protocols/components/av_stream")

project(espidf_video_rtsp)
