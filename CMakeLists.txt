cmake_minimum_required(VERSION 3.10.0)
project(
  Chaos-DXR
  VERSION 0.1.0
  LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(Chaos-DXR src/main.cpp)
set_property(TARGET Chaos-DXR PROPERTY CXX_STANDARD 20)

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
