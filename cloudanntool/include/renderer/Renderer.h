#pragma once
#include <Eigen/Core>
#include <memory>

namespace CloudCore {

class RenderAPI;
class VertexArray;

enum class PrimitiveType {
    Points,
    Lines,
    LineStrip,
    Triangles,
    TriangleStrip,
    TriangleFan
};


// Static interface for submitting render commands with state caching
class Renderer {
public:
    static void init();
    static void shutdown();
    
    static void beginFrame();
    static void endFrame();
    
    static void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    static void setClearColor(const Eigen::Vector4f& color);
    static void clear();
    
    static void drawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0);
    static void drawArrays(const std::shared_ptr<VertexArray>& vertexArray, uint32_t vertexCount, PrimitiveType type);
    
    static void setDepthTest(bool enabled);
    static void setDepthMask(bool enabled);
    static void setBlending(bool enabled);
    static void setWireframe(bool enabled);
    static void setPointSize(float size);
    static void setLineWidth(float size);
    static void setRenderAPI(std::unique_ptr<RenderAPI> renderAPI);

private:
    static std::unique_ptr<RenderAPI> s_RenderAPI;
    
    // Cached render state
    struct State {
        bool depthTest = true;
        bool depthMask = true;
        bool blending = false;
        bool wireframe = false;
        float pointSize = 1.0f;
        float lineWidth = 1.0f;
        Eigen::Vector4f clearColor = Eigen::Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
    };
    
    static State s_State;
};

} // namespace CloudCore