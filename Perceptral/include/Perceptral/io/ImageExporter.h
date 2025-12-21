#pragma once

#include <string>

namespace Perceptral {

/**
 * @brief Image exporter for screenshots and video recording
 */
class ImageExporter {
public:
    /**
     * @brief Save the current framebuffer to an image file
     * @param filepath Output file path (supports .png, .jpg, .bmp, .tga)
     * @param width Frame width
     * @param height Frame height
     * @return true if successful
     */
    static bool saveScreenshot(const std::string& filepath, int width, int height);

    /**
     * @brief Start recording video frames
     * @param outputPath Output video file path (will add frame numbers)
     * @param width Frame width
     * @param height Frame height
     * @param fps Frames per second (for playback reference)
     */
    static void startRecording(const std::string& outputPath, int width, int height, int fps = 30);

    /**
     * @brief Stop recording and optionally compile frames to video
     * @param compileToVideo If true, attempt to create video using ffmpeg
     * @return true if successful
     */
    static bool stopRecording(bool compileToVideo = false);

    /**
     * @brief Capture current frame during recording
     */
    static void captureFrame();

    /**
     * @brief Check if currently recording
     */
    static bool isRecording();

    /**
     * @brief Get current frame count
     */
    static int getFrameCount();

private:
    static bool recording_;
    static std::string outputPath_;
    static int frameCount_;
    static int width_;
    static int height_;
    static int fps_;

    /**
     * @brief Compile captured frames into a video file using ffmpeg
     * @return true if successful
     */
    static bool compileFramesToVideo();
};

} // namespace Perceptral
