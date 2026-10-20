cmake_minimum_required(VERSION 3.21.0)
project(
  Chaos-DXR
  VERSION 0.1.0
  LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(Chaos-DXR src/main.cpp src/RechtRenderer.cpp src/Renderer.cpp)
set_property(TARGET Chaos-DXR PROPERTY CXX_STANDARD 20)
set_property(TARGET Chaos-DXR PROPERTY WIN32_EXECUTABLE true)
set_target_properties(Chaos-DXR PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  WIN32_EXECUTABLE ON  
)
target_include_directories(Chaos-DXR PRIVATE "${PROJECT_SOURCE_DIR}/src/include" PRIVATE "${PROJECT_SOURCE_DIR}/vendor/wil/include")

if(NOT WIN32)
  add_custom_command(
    TARGET Chaos-DXR
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory_if_different
      "${PROJECT_SOURCE_DIR}/MSVC/VC/Redist/MSVC/14.51.36231/debug_nonredist/x64/Microsoft.VC145.DebugCRT"
      $<TARGET_FILE_DIR:Chaos-DXR>)

  add_custom_command(
    TARGET Chaos-DXR
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      "${PROJECT_SOURCE_DIR}/MSVC/Windows Kits/10/bin/10.0.26100.0/x64/ucrt/ucrtbased.dll"
      $<TARGET_FILE_DIR:Chaos-DXR>)
endif()
