cmake_minimum_required(VERSION 3.10)

add_library(BitmapPlusPlus ${CMAKE_CURRENT_SOURCE_DIR}/BitmapPlusPlus.cpp)
target_include_directories(BitmapPlusPlus INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(bmp::BitmapPlusPlus ALIAS BitmapPlusPlus)

if (BPP_ENABLE_INSTALL)
  install(TARGETS BitmapPlusPlus EXPORT BitmapPlusPlus DESTINATION "lib")
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/BitmapPlusPlus.hpp DESTINATION "include")
endif ()
