cmake_minimum_required(VERSION 3.21.0)
project(
  Chaos-DXR
  VERSION 0.1.0
  LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_CXX_CLANG_TIDY clang-tidy --extra-arg-before=-EHsc)

# add_executable(Chaos-DXR src/main.cpp src/RechtRenderer.cpp src/Renderer.cpp)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

qt_standard_project_setup()

# qt_add_executable(Chaos-DXR src/main.cpp src/MovingBarRenderer.cpp src/Renderer.cpp src/TriangleRenderer.cpp)
# qt_add_executable(Chaos-DXR src/main.cpp src/Renderer.cpp src/Renderers/MovingBarRenderer/MovingBarRenderer.cpp)
qt_add_executable(Chaos-DXR src/main.cpp src/Renderer.cpp src/Renderers/TriangleRenderer/TriangleRenderer.cpp)

add_subdirectory("vendor/DirectX-Headers")
target_link_libraries(Chaos-DXR PRIVATE DirectX-Headers)


### Shader compilation

set(SHADER_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${SHADER_OUT_DIR}")
set(GENERATED_SHADER_HEADERS "")

set(HLSL_SHADER_FILES src/Renderers/TriangleRenderer/ConstColorVS.hlsl src/Renderers/TriangleRenderer/ConstColorPS.hlsl)
set_source_files_properties(src/Renderers/TriangleRenderer/ConstColorVS.hlsl PROPERTIES VS_SHADER_TYPE "vs")
set_source_files_properties(src/Renderers/TriangleRenderer/ConstColorPS.hlsl PROPERTIES VS_SHADER_TYPE "ps")

foreach(FILE ${HLSL_SHADER_FILES})
  get_filename_component(FILE_WE ${FILE} NAME_WE)
  list(APPEND H_SHADER_FILES ${CMAKE_BINARY_DIR}/${FILE_WE}.h)
  get_source_file_property(shadertype ${FILE} VS_SHADER_TYPE)

  set(CSO "${SHADER_OUT_DIR}/${FILE_WE}.cso")
  set(HDR "${SHADER_OUT_DIR}/${FILE_WE}.h")

  add_custom_command(
    OUTPUT  "${CSO}"
    COMMAND "dxc.exe" -T ${shadertype}_6_0
            $<$<CONFIG:Debug>:-Od>
            $<$<CONFIG:Debug>:-Zi>
            $<$<CONFIG:Debug>:-Fd>
            $<$<CONFIG:Debug>:${SHADER_OUT_DIR}/${FILE_WE}.pdb>
            $<$<NOT:$<CONFIG:Debug>>:-O3>
            -Fo "${CSO}" "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}"
    COMMENT "Compiling HLSL ${FILE}"
    VERBATIM
    COMMAND_EXPAND_LISTS)

  add_custom_command(
    OUTPUT  "${HDR}"
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/bin2header.py"
            "${CSO}" "${HDR}" "${FILE_WE}"
    DEPENDS "${CSO}" "${PROJECT_SOURCE_DIR}/tools/bin2header.py"
    COMMENT "Creating bytearray header for ${FILE}"
    VERBATIM)

  list(APPEND GENERATED_SHADER_HEADERS "${HDR}")

endforeach(FILE)

target_sources(Chaos-DXR PRIVATE ${GENERATED_SHADER_HEADERS})
set_source_files_properties(${GENERATED_SHADER_HEADERS} PROPERTIES GENERATED TRUE)

target_link_libraries(Chaos-DXR PRIVATE Qt6::Widgets)

target_compile_options(Chaos-DXR PRIVATE
  /W4
  /WX
  /permissive-
  /EHsc
  /Zc:__cplusplus
)
set_target_properties(Chaos-DXR PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED ON
  WIN32_EXECUTABLE ON
)
target_include_directories(Chaos-DXR PRIVATE
  "${PROJECT_SOURCE_DIR}/src/include"
  "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_include_directories(Chaos-DXR SYSTEM PRIVATE
  "${PROJECT_SOURCE_DIR}/vendor/wil/include")

add_custom_command(
  TARGET Chaos-DXR
  POST_BUILD
  COMMAND Qt6::windeployqt
          --no-translations
          --no-compiler-runtime
          --no-system-d3d-compiler
          $<$<CONFIG:Debug>:--pdb>
          "$<TARGET_FILE:Chaos-DXR>"
  COMMENT "Deploying Qt runtime next to Chaos-DXR.exe")


if(NOT CMAKE_HOST_WIN32)
  set(MSVC_DIR "${PROJECT_SOURCE_DIR}/MSVC")
  set(MSVC_VER "14.51.36231")
  set(SDK_DIR "${MSVC_DIR}/Windows Kits/10/Include/10.0.28000.0")
  target_include_directories(Chaos-DXR SYSTEM PRIVATE
    "${MSVC_DIR}/vc/tools/msvc/${MSVC_VER}/include"
    "${SDK_DIR}/shared"
    "${SDK_DIR}/ucrt"
    "${SDK_DIR}/um"
    "${SDK_DIR}/winrt"
    "${SDK_DIR}/cppwinrt"
  )

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


if(CMAKE_HOST_WIN32)
  target_compile_definitions(Chaos-DXR PRIVATE WIN_HOST=1)
else()
  target_compile_definitions(Chaos-DXR PRIVATE WIN_HOST=0)
endif()
