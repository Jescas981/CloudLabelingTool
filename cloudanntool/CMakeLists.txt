cmake_minimum_required(VERSION 3.15)
project(CloudAnnotationCore VERSION 1.0.0 LANGUAGES CXX C)

# Options
option(BUILD_EXAMPLES "Build example applications" ON)

add_subdirectory(external)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(PCL REQUIRED COMPONENTS common io)

# Core library source files
set(CORE_SOURCES
    src/core/Log.cpp
    # Core
    src/core/Engine.cpp
    src/core/Window.cpp
    src/core/Renderer.cpp
    src/core/EventManager.cpp
    src/core/Camera.cpp
    src/core/Application.cpp
    src/core/Input.cpp
    src/core/LayerStack.cpp
    src/core/layers/RenderLayer.cpp

    # Old rendering (keeping for compatibility)
    src/rendering/PointCloudRenderer.cpp
    src/rendering/BackgroundRenderer.cpp
    src/rendering/GridRenderer.cpp
    src/rendering/AxisLineRenderer.cpp

    # New renderer abstraction
    src/renderer/RenderAPI.cpp
    src/renderer/Renderer.cpp
    src/renderer/Buffer.cpp
    src/renderer/VertexArray.cpp
    src/renderer/Shader.cpp
    src/renderer/ShaderLibrary.cpp

    # OpenGL Platform
    src/platform/opengl/OpenGLRenderAPI.cpp
    src/platform/opengl/OpenGLBuffer.cpp
    src/platform/opengl/OpenGLVertexArray.cpp
    src/platform/opengl/OpenGLShader.cpp

    # Scene (EnTT ECS)
    src/scene/Scene.cpp
    src/scene/Entity.cpp
    src/scene/Components.cpp
    src/scene/Systems.cpp
    src/scene/SceneManager.cpp
    src/scene/PointCloud.cpp
    src/scene/LabelDefinition.cpp

    # I/O
    src/io/PLYLoader.cpp
    src/io/PCDLoader.cpp
    src/io/ImageExporter.cpp
    src/io/PointCloudExporter.cpp

    # UI
    src/ui/ImGuiLayer.cpp
)

set(CORE_HEADERS
    include/core/Log.h
    # Core
    include/core/Engine.h
    include/core/Window.h
    include/core/EventManager.h
    include/core/Camera.h
    include/core/Application.h
    include/core/Timestep.h
    include/core/Input.h
    include/core/KeyCodes.h
    include/core/Event.h

    # Old rendering (keeping for compatibility)
    include/rendering/PointCloudRenderer.h
    include/rendering/GridRenderer.h
    include/rendering/BackgroundRenderer.h

    # New renderer abstraction
    include/renderer/RenderAPI.h
    include/renderer/Renderer.h
    include/renderer/Shader.h
    include/renderer/ShaderLibrary.h
    include/renderer/Buffer.h
    include/renderer/VertexArray.h

    # OpenGL Platform
    include/platform/opengl/OpenGLRenderAPI.h
    include/platform/opengl/OpenGLBuffer.h
    include/platform/opengl/OpenGLVertexArray.h
    include/platform/opengl/OpenGLShader.h

    # Scene (EnTT ECS)
    include/scene/Scene.h
    include/scene/Entity.h
    include/scene/Components.h
    include/scene/Systems.h
    include/scene/SceneManager.h
    include/scene/PointCloud.h
    include/scene/LabelDefinition.h

    # I/O
    include/io/PLYLoader.h
    include/io/PCDLoader.h
    include/io/ImageExporter.h
    include/io/PointCloudExporter.h

    # UI
    include/ui/ImGuiLayer.h
)


set(IMGUI_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

set(IMGUI_HEADERS
    external/imgui/imconfig.h
    external/imgui/imgui.h
    external/imgui/imgui_internal.h
    external/imgui/backends/imgui_impl_glfw.h
    external/imgui/backends/imgui_impl_opengl3.h
    external/imgui/backends/imgui_impl_opengl3_loader.h
)


# Create core library
add_library(cloudanncore STATIC ${CORE_SOURCES} ${CORE_HEADERS} ${IMGUI_SOURCES} ${IMGUI_HEADERS})

target_include_directories(cloudanncore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(cloudanncore
    PUBLIC
        pcl_common
        pcl_io
        OpenGL::GL
        EnTT::EnTT
        spdlog::spdlog_header_only
        glfw
        glad
)

target_include_directories(cloudanncore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/imgui>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends>
        $<INSTALL_INTERFACE:include/imgui>
        $<INSTALL_INTERFACE:include/imgui/backends>
)

# Compiler warnings
if(MSVC)
    target_compile_options(cloudanncore PRIVATE /W4)
else()
    target_compile_options(cloudanncore PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Installation
install(TARGETS cloudanncore glad glfw EnTT spdlog_header_only
    EXPORT CloudAnnotationCoreTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY shaders/ DESTINATION share/cloudanncore/shaders)

# Install external dependencies headers that are needed
install(DIRECTORY external/imgui/ DESTINATION include/imgui
    FILES_MATCHING PATTERN "*.h")
install(DIRECTORY external/imgui/backends/ DESTINATION include/imgui/backends
    FILES_MATCHING PATTERN "*.h")

# Export targets for find_package
install(EXPORT CloudAnnotationCoreTargets
    FILE CloudAnnotationCoreTargets.cmake
    NAMESPACE CloudAnnotationCore::
    DESTINATION lib/cmake/CloudAnnotationCore
)

# Create and install package config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CloudAnnotationCoreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CloudAnnotationCoreConfig.cmake
    INSTALL_DESTINATION lib/cmake/CloudAnnotationCore
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/CloudAnnotationCoreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CloudAnnotationCoreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/CloudAnnotationCoreConfigVersion.cmake
    DESTINATION lib/cmake/CloudAnnotationCore
)
