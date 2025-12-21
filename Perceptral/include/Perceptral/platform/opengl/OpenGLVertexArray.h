#pragma once

#include <Perceptral/renderer/VertexArray.h>

namespace Perceptral {

class OpenGLVertexArray : public VertexArray {
public:
    OpenGLVertexArray();
    virtual ~OpenGLVertexArray();

    virtual void bind() const override;
    virtual void unbind() const override;

    void clearVertexBuffers() override;

    virtual void addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
    virtual void setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

    virtual const std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() const override {
        return vertexBuffers_;
    }
    virtual const std::shared_ptr<IndexBuffer>& getIndexBuffer() const override {
        return indexBuffer_;
    }

private:
    uint32_t rendererID_;
    uint32_t vertexBufferIndex_ = 0;
    std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers_;
    std::shared_ptr<IndexBuffer> indexBuffer_;
};

} // namespace Perceptral
