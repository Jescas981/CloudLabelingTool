cmake_minimum_required(VERSION 3.15)
project(PointCloudTool VERSION 1.0.0 LANGUAGES CXX)

# Option to build against installed CloudCore or local development build
option(USE_LOCAL_CLOUDCORE "Use local CloudCore build from ../cloudanntool" ON)

if(USE_LOCAL_CLOUDCORE)
    message(STATUS "Using local CloudCore from ../cloudanntool")
    # Add cloudanntool as subdirectory for development
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cloudanntool ${CMAKE_BINARY_DIR}/cloudcore)
    set(CLOUDCORE_LIB cloudanncore)
else()
    message(STATUS "Looking for installed CloudAnnotationCore package")
    # Find installed CloudCore library
    find_package(CloudAnnotationCore REQUIRED)
    set(CLOUDCORE_LIB CloudAnnotationCore::cloudanncore)
endif()

# Point Cloud Tool sources
set(POINTCLOUDTOOL_SOURCES
    src/main.cpp
    src/PointCloudObject.cpp
    src/SceneController.cpp
    src/CameraController.cpp
    src/UIManager.cpp
    src/SelectionTool.cpp
)

add_subdirectory(external/nativefiledialog-extended)
# Point Cloud Tool executable
add_executable(pointcloudtool ${POINTCLOUDTOOL_SOURCES})

target_include_directories(pointcloudtool
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(pointcloudtool PRIVATE ${CLOUDCORE_LIB} nfd)

# Copy resources to build directory
if(USE_LOCAL_CLOUDCORE)
    # In development mode, use shaders from cloudanntool
    set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cloudanntool/shaders)

    # Copy shaders
    file(COPY ${SHADER_SOURCE_DIR} DESTINATION ${CMAKE_BINARY_DIR})

    # Copy assets if they exist
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../cloudanntool/assets)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../cloudanntool/assets DESTINATION ${CMAKE_BINARY_DIR})
    endif()
else()
    # When using installed CloudCore
    set(SHADER_SOURCE_DIR ${CMAKE_INSTALL_PREFIX}/share/cloudanncore/shaders)
    file(COPY ${SHADER_SOURCE_DIR} DESTINATION ${CMAKE_BINARY_DIR})

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
    endif()
endif()

# Installation
install(TARGETS pointcloudtool
    RUNTIME DESTINATION bin
)

# Install assets and shaders with the application
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    install(DIRECTORY assets/ DESTINATION share/pointcloudtool/assets)
endif()
