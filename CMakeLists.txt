cmake_minimum_required(VERSION 3.21.0)
project(
  Chaos-DXR
  VERSION 0.1.0
  LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_CLANG_TIDY clang-tidy --extra-arg-before=-EHsc)
set(MSVC_DIR "${PROJECT_SOURCE_DIR}/MSVC")
set(MSVC_VER "14.51.36231")
set(SDK_DIR "${MSVC_DIR}/Windows Kits/10/Include/10.0.28000.0")

add_executable(Chaos-DXR src/main.cpp src/RechtRenderer.cpp src/Renderer.cpp)
target_compile_options(Chaos-DXR PRIVATE
  /W4
  /WX
  /permissive-
)
set_target_properties(Chaos-DXR PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  WIN32_EXECUTABLE ON  
)
target_include_directories(Chaos-DXR PRIVATE
  "${PROJECT_SOURCE_DIR}/src/include")
target_include_directories(Chaos-DXR SYSTEM PRIVATE
  "${PROJECT_SOURCE_DIR}/vendor/wil/include"
  "${MSVC_DIR}/vc/tools/msvc/${MSVC_VER}/include"
  "${SDK_DIR}/shared"
  "${SDK_DIR}/ucrt"
  "${SDK_DIR}/um"
  "${SDK_DIR}/winrt"
  "${SDK_DIR}/cppwinrt"
)

if(NOT CMAKE_HOST_WIN32)
  add_custom_command(
    TARGET Chaos-DXR
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory_if_different
      "${MSVC_DIR}/VC/Redist/MSVC/${MSVC_VER}/debug_nonredist/x64/Microsoft.VC145.DebugCRT"
      $<TARGET_FILE_DIR:Chaos-DXR>)

  add_custom_command(
    TARGET Chaos-DXR
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      "${MSVC_DIR}/Windows Kits/10/bin/10.0.28000.0/x64/ucrt/ucrtbased.dll"
      $<TARGET_FILE_DIR:Chaos-DXR>)
endif()
