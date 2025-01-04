cmake_minimum_required(VERSION 3.20)

# set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(TFLITE_INCLUDE PATH "C:/Users/kwoncy/dev/tensorflow_src")
set(STB_INCLUDE PATH "C:/Users/kwoncy/dev/stb")
# link_directories(${CMAKE_CURRENT_LIST_DIR}/..)

add_library(flutter_my_ai SHARED ${CMAKE_CURRENT_LIST_DIR}/../flutter_my_ai.cpp)

target_include_directories(flutter_my_ai PUBLIC ${TFLITE_INCLUDE} PUBLIC ${STB_INCLUDE})
target_link_directories(flutter_my_ai PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
# target_link_libraries(using_tfl_c_shared "tensorflowlite_c.dll.if")
target_link_libraries(flutter_my_ai "tensorflowlite_c.dll")

