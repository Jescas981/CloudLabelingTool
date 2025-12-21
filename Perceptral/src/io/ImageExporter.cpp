#include <Perceptral/io/ImageExporter.h>
#include <spdlog/spdlog.h>
#include <glad/glad.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../external/glfw/deps/stb_image_write.h"

#include <filesystem>
#include <fstream>

namespace Perceptral {

bool ImageExporter::recording_ = false;
std::string ImageExporter::outputPath_ = "";
int ImageExporter::frameCount_ = 0;
int ImageExporter::width_ = 0;
int ImageExporter::height_ = 0;
int ImageExporter::fps_ = 30;

bool ImageExporter::saveScreenshot(const std::string& filepath, int width, int height)
{
    spdlog::info("Saving screenshot to: {}", filepath);

    // Allocate buffer for pixel data (RGB, 3 bytes per pixel)
    std::vector<unsigned char> pixels(width * height * 3);

    // Read pixels from framebuffer
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        spdlog::error("OpenGL error while reading pixels: {}", error);
        return false;
    }

    // Flip image vertically (OpenGL's origin is bottom-left, image formats expect top-left)
    std::vector<unsigned char> flippedPixels(width * height * 3);
    for (int y = 0; y < height; y++) {
        memcpy(
            flippedPixels.data() + (height - 1 - y) * width * 3,
            pixels.data() + y * width * 3,
            width * 3
        );
    }

    // Determine file format from extension
    std::string ext = std::filesystem::path(filepath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    int result = 0;
    if (ext == ".png") {
        result = stbi_write_png(filepath.c_str(), width, height, 3, flippedPixels.data(), width * 3);
    } else if (ext == ".jpg" || ext == ".jpeg") {
        result = stbi_write_jpg(filepath.c_str(), width, height, 3, flippedPixels.data(), 95); // 95% quality
    } else if (ext == ".bmp") {
        result = stbi_write_bmp(filepath.c_str(), width, height, 3, flippedPixels.data());
    } else if (ext == ".tga") {
        result = stbi_write_tga(filepath.c_str(), width, height, 3, flippedPixels.data());
    } else {
        spdlog::error("Unsupported image format: {}", ext);
        return false;
    }

    if (result == 0) {
        spdlog::error("Failed to write image file: {}", filepath);
        return false;
    }

    spdlog::info("Screenshot saved successfully: {}", filepath);
    return true;
}

void ImageExporter::startRecording(const std::string& outputPath, int width, int height, int fps)
{
    if (recording_) {
        spdlog::warn("Already recording, stopping previous recording first");
        stopRecording(false);
    }

    outputPath_ = outputPath;
    width_ = width;
    height_ = height;
    fps_ = fps;
    frameCount_ = 0;
    recording_ = true;

    // Create output directory if it doesn't exist
    std::filesystem::path path(outputPath_);
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }

    spdlog::info("Started recording to: {} ({}x{} @ {} fps)", outputPath_, width_, height_, fps_);
}

bool ImageExporter::stopRecording(bool compileToVideo)
{
    if (!recording_) {
        spdlog::warn("Not currently recording");
        return false;
    }

    recording_ = false;

    spdlog::info("Stopped recording. Captured {} frames", frameCount_);

    if (compileToVideo) {
        return compileFramesToVideo();
    }

    return true;
}

void ImageExporter::captureFrame()
{
    if (!recording_) {
        return;
    }

    // Generate frame filename
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/frame_%06d.png", outputPath_.c_str(), frameCount_);

    // Save the frame
    if (saveScreenshot(filename, width_, height_)) {
        frameCount_++;
    } else {
        spdlog::error("Failed to capture frame {}", frameCount_);
    }
}

bool ImageExporter::isRecording()
{
    return recording_;
}

int ImageExporter::getFrameCount()
{
    return frameCount_;
}

bool ImageExporter::compileFramesToVideo()
{
    spdlog::info("Compiling frames to video...");

    // Check if ffmpeg is available
    int result = system("which ffmpeg > /dev/null 2>&1");
    if (result != 0) {
        spdlog::error("ffmpeg not found. Please install ffmpeg to compile videos.");
        spdlog::info("Frames are available in: {}", outputPath_);
        return false;
    }

    // Generate output video filename
    std::string videoPath = outputPath_ + "/output.mp4";

    // Build ffmpeg command
    char command[512];
    snprintf(command, sizeof(command),
             "ffmpeg -y -framerate %d -i %s/frame_%%06d.png -c:v libx264 -pix_fmt yuv420p -crf 18 %s 2>&1 | tail -5",
             fps_, outputPath_.c_str(), videoPath.c_str());

    spdlog::info("Running ffmpeg: {}", command);
    result = system(command);

    if (result == 0) {
        spdlog::info("Video compiled successfully: {}", videoPath);

        // Optionally delete frame images
        spdlog::info("Frame images kept in: {}", outputPath_);
        return true;
    } else {
        spdlog::error("Failed to compile video");
        return false;
    }
}

} // namespace Perceptral
