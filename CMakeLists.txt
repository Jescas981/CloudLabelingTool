cmake_minimum_required(VERSION 3.23)
project(PointCloudTool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# Point Cloud Tool sources
set(POINTCLOUDTOOL_SOURCES
    src/main.cpp
    # src/PointCloud.cpp
    # src/PointCloudObject.cpp
    # src/SceneController.cpp
    # src/CameraController.cpp
    # src/UIManager.cpp
    # src/SelectionTool.cpp
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(external/Perceptral)
add_subdirectory(external/nativefiledialog-extended)
# Point Cloud Tool executable
add_executable(pointcloudtool ${POINTCLOUDTOOL_SOURCES})

target_include_directories(pointcloudtool PRIVATE include)
target_link_libraries(pointcloudtool PRIVATE perceptral nfd)
target_compile_definitions(pointcloudtool PRIVATE PC_ASSETS=\"${CMAKE_CURRENT_SOURCE_DIR}/assets\")