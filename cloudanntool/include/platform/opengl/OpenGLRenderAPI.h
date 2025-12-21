#pragma once

#include "renderer/RenderAPI.h"

namespace CloudCore {
class OpenGLRenderAPI : public RenderAPI {
public:
    virtual void init() override;
    virtual void shutdown() override;

    void beginFrame() override;
    void endFrame() override;

    virtual void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    virtual void setClearColor(const Eigen::Vector4f& color) override;
    virtual void clear() override;

    virtual void drawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
    virtual void drawArrays(const std::shared_ptr<VertexArray>& vertexArray, uint32_t vertexCount, PrimitiveType type) override;

    virtual void setDepthTest(bool enabled) override;
    virtual void setDepthMask(bool enabled) override;
    virtual void setBlending(bool enabled) override;
    virtual void setWireframe(bool enabled) override;
    virtual void setPointSize(float size) override;
    virtual void setLineWidth(float size) override;
};

} // namespace CloudCore
