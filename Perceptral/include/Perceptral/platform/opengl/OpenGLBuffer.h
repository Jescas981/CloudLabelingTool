#pragma once

#include <Perceptral/renderer/Buffer.h>

namespace Perceptral {

class OpenGLVertexBuffer : public VertexBuffer {
public:
    OpenGLVertexBuffer(uint32_t size);
    OpenGLVertexBuffer(const void* vertices, uint32_t size);
    virtual ~OpenGLVertexBuffer();

    virtual void bind() const override;
    virtual void unbind() const override;
    
    std::size_t getSize() override;

    virtual void setData(const void* data, uint32_t size) override;

    virtual const BufferLayout& getLayout() const override { return layout_; }
    virtual void setLayout(const BufferLayout& layout) override { layout_ = layout; }

private:
    uint32_t rendererID_;
    BufferLayout layout_;
    std::size_t size_;
};

class OpenGLIndexBuffer : public IndexBuffer {
public:
    OpenGLIndexBuffer(const uint32_t* indices, uint32_t count);
    virtual ~OpenGLIndexBuffer();

    virtual void bind() const override;
    virtual void unbind() const override;

    virtual uint32_t getCount() const override { return count_; }

private:
    uint32_t rendererID_;
    uint32_t count_;
};

} // namespace Perceptral
