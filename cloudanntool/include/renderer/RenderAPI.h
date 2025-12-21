#pragma once

#include <Eigen/Core>
#include <memory>
#include <renderer/Renderer.h>

namespace CloudCore {

class VertexArray;

// Abstract rendering API interface
class RenderAPI {
public:
    enum class API {
        None = 0,
        OpenGL = 1,
        Vulkan = 2,
        DirectX = 3
    };

    virtual ~RenderAPI() = default;

    // Initialization
    virtual void init() = 0;
    virtual void shutdown() = 0;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    // Viewport and clearing
    virtual void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
    virtual void setClearColor(const Eigen::Vector4f& color) = 0;
    virtual void clear() = 0;

    // Drawing
    virtual void drawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount) = 0;
    virtual void drawArrays(const std::shared_ptr<VertexArray>& vertexArray, uint32_t vertexCount, PrimitiveType type) = 0;

    // State
    virtual void setDepthTest(bool enabled) = 0;
    virtual void setDepthMask(bool enabled) = 0;
    virtual void setBlending(bool enabled) = 0;
    virtual void setWireframe(bool enabled) = 0;
    virtual void setPointSize(float size) = 0;
    virtual void setLineWidth(float size) = 0;

    // API info
    static API getAPI() { return s_API; }
    // static void setAPI(API api) { s_API = api; }

    // Factory
    static std::unique_ptr<RenderAPI> create();

private:
    static API s_API;
};

} // namespace CloudCore
