cmake_minimum_required(VERSION 3.23)
project(PointCloudTool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMANDS ON)

# Option to build against installed CloudCore or local development build
option(USE_LOCAL_CLOUDCORE "Use local CloudCore build from ../Perceptral" ON)
option(STANDALONE_BUNDLE "Create standalone application bundle" ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# Optimization flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "" FORCE)
elseif(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG" CACHE STRING "" FORCE)
endif()

if(USE_LOCAL_CLOUDCORE)
    message(STATUS "Using local CloudCore from ../Perceptral")
    # Add Perceptral as subdirectory for development
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Perceptral ${CMAKE_BINARY_DIR}/cloudcore)
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

# ============================================================================
# Resource Handling for Development Build
# ============================================================================
if(USE_LOCAL_CLOUDCORE)
    # Copy CloudCore shaders to build directory
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../Perceptral/shaders 
         DESTINATION ${CMAKE_BINARY_DIR}
         PATTERN "*.glsl")
    
    # Copy CloudCore assets if they exist
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../Perceptral/assets)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../Perceptral/assets 
             DESTINATION ${CMAKE_BINARY_DIR})
    endif()
endif()

# ============================================================================
# Installation - Standalone Bundle
# ============================================================================
if(STANDALONE_BUNDLE)
    # Everything goes into a single directory
    set(BUNDLE_DIR "PointCloudTool")
    
    # Install executable
    install(TARGETS pointcloudtool
        RUNTIME DESTINATION ${BUNDLE_DIR}
    )
    
    # Install all shaders in one place (CloudCore + app)
    if(USE_LOCAL_CLOUDCORE)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Perceptral/shaders/
            DESTINATION ${BUNDLE_DIR}/shaders
            FILES_MATCHING PATTERN "*.glsl"
        )
    else()
        # Copy from installed CloudCore location
        install(DIRECTORY ${CMAKE_INSTALL_PREFIX}/share/cloudanncore/shaders/
            DESTINATION ${BUNDLE_DIR}/shaders
            FILES_MATCHING PATTERN "*.glsl"
        )
    endif()
    
    # Install app-specific assets
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/
            DESTINATION ${BUNDLE_DIR}/assets
        )
    endif()
    
    # Install shared libraries (for Linux/macOS)
    if(UNIX AND NOT APPLE)
        install(FILES $<TARGET_FILE:cloudanncore>
            DESTINATION ${BUNDLE_DIR}/lib
        )
    endif()
    
    # On Windows, DLLs should be next to exe
    if (WIN32)
        find_package(PCL REQUIRED COMPONENTS common io)
        get_target_property(PCL_COMMON_DLL pcl_common IMPORTED_LOCATION_RELEASE)
        get_target_property(PCL_IO_DLL     pcl_io     IMPORTED_LOCATION_RELEASE)

        message(STATUS "PCL common DLL: ${PCL_COMMON_DLL}")
        message(STATUS "PCL io DLL:     ${PCL_IO_DLL}")

        install(FILES
            ${PCL_COMMON_DLL}
            ${PCL_IO_DLL}
            DESTINATION ${BUNDLE_DIR}
        )
    endif()
    
    # Install README or LICENSE if they exist
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
        install(FILES README.md DESTINATION ${BUNDLE_DIR})
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
        install(FILES LICENSE DESTINATION ${BUNDLE_DIR})
    endif()
    
    message(STATUS "Standalone bundle will be installed to: ${CMAKE_INSTALL_PREFIX}/${BUNDLE_DIR}")
    
# ============================================================================
# Installation - System-wide (FHS compliant)
# ============================================================================
else()
    # Traditional Unix layout
    install(TARGETS pointcloudtool
        RUNTIME DESTINATION bin
    )
    
    # Install CloudCore shaders to their own location
    if(USE_LOCAL_CLOUDCORE)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Perceptral/shaders/
            DESTINATION share/cloudanncore/shaders
            FILES_MATCHING PATTERN "*.glsl"
        )
    endif()
    
    # Install app assets
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/
            DESTINATION share/pointcloudtool/assets
        )
    endif()
endif()

# ============================================================================
# CPack Configuration for Distribution
# ============================================================================
if(STANDALONE_BUNDLE)
    set(CPACK_PACKAGE_NAME "PointCloudTool")
    set(CPACK_PACKAGE_VENDOR "YourName")
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Point Cloud Annotation Tool")
    
    # Create a simple zip/tar.gz
    if(WIN32)
        set(CPACK_GENERATOR "ZIP")
    else()
        set(CPACK_GENERATOR "TGZ")
    endif()
    
    include(CPack)
endif()