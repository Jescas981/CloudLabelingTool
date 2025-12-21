#include <Perceptral/renderer/Buffer.h>
#include <Perceptral/renderer/RenderAPI.h>
#include <Perceptral/platform/opengl/OpenGLBuffer.h>
#include <Perceptral/core/Log.h>

namespace Perceptral {

uint32_t shaderDataTypeSize(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::Float:    return 4;
        case ShaderDataType::Float2:   return 4 * 2;
        case ShaderDataType::Float3:   return 4 * 3;
        case ShaderDataType::Float4:   return 4 * 4;
        case ShaderDataType::Mat3:     return 4 * 3 * 3;
        case ShaderDataType::Mat4:     return 4 * 4 * 4;
        case ShaderDataType::Int:      return 4;
        case ShaderDataType::Int2:     return 4 * 2;
        case ShaderDataType::Int3:     return 4 * 3;
        case ShaderDataType::Int4:     return 4 * 4;
        case ShaderDataType::Bool:     return 1;
        default:
            PC_CORE_ERROR("Unknown ShaderDataType!");
            return 0;
    }
}

uint32_t BufferElement::getComponentCount() const {
    switch (type) {
        case ShaderDataType::Float:   return 1;
        case ShaderDataType::Float2:  return 2;
        case ShaderDataType::Float3:  return 3;
        case ShaderDataType::Float4:  return 4;
        case ShaderDataType::Mat3:    return 3 * 3;
        case ShaderDataType::Mat4:    return 4 * 4;
        case ShaderDataType::Int:     return 1;
        case ShaderDataType::Int2:    return 2;
        case ShaderDataType::Int3:    return 3;
        case ShaderDataType::Int4:    return 4;
        case ShaderDataType::Bool:    return 1;
        default:
            PC_CORE_ERROR("Unknown ShaderDataType!");
            return 0;
    }
}

BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements)
    : elements_(elements) {
    calculateOffsetsAndStride();
}

void BufferLayout::calculateOffsetsAndStride() {
    uint32_t offset = 0;
    stride_ = 0;
    for (auto& element : elements_) {
        element.offset = offset;
        offset += element.size;
        stride_ += element.size;
    }
}

// VertexBuffer factory
std::shared_ptr<VertexBuffer> VertexBuffer::create(uint32_t size) {
    switch (RenderAPI::getAPI()) {
        case RenderAPI::API::OpenGL:
            return std::make_shared<OpenGLVertexBuffer>(size);
        default:
            PC_CORE_ERROR("Unknown RenderAPI!");
            return nullptr;
    }
}

std::shared_ptr<VertexBuffer> VertexBuffer::create(const void* vertices, uint32_t size) {
    switch (RenderAPI::getAPI()) {
        case RenderAPI::API::OpenGL:
            return std::make_shared<OpenGLVertexBuffer>(vertices, size);
        default:
            PC_CORE_ERROR("Unknown RenderAPI!");
            return nullptr;
    }
}

// IndexBuffer factory
std::shared_ptr<IndexBuffer> IndexBuffer::create(const uint32_t* indices, uint32_t count) {
    switch (RenderAPI::getAPI()) {
        case RenderAPI::API::OpenGL:
            return std::make_shared<OpenGLIndexBuffer>(indices, count);
        default:
            PC_CORE_ERROR("Unknown RenderAPI!");
            return nullptr;
    }
}

} // namespace Perceptral
