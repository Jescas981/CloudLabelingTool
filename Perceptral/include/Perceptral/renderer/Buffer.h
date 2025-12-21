#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

namespace Perceptral {

// Shader data types
enum class ShaderDataType {
    None = 0,
    Float, Float2, Float3, Float4,
    Mat3, Mat4,
    Int, Int2, Int3, Int4,
    Bool
};

// Get size of shader data type
uint32_t shaderDataTypeSize(ShaderDataType type);

// Buffer element (vertex attribute)
struct BufferElement {
    std::string name;
    ShaderDataType type;
    uint32_t size;
    uint32_t offset;
    bool normalized;

    BufferElement() = default;

    BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
        : name(name), type(type), size(shaderDataTypeSize(type)), offset(0), normalized(normalized) {}

    uint32_t getComponentCount() const;
};

// Buffer layout (describes vertex structure)
class BufferLayout {
public:
    BufferLayout() = default;
    BufferLayout(const std::initializer_list<BufferElement>& elements);

    uint32_t getStride() const { return stride_; }
    const std::vector<BufferElement>& getElements() const { return elements_; }

    std::vector<BufferElement>::iterator begin() { return elements_.begin(); }
    std::vector<BufferElement>::iterator end() { return elements_.end(); }
    std::vector<BufferElement>::const_iterator begin() const { return elements_.begin(); }
    std::vector<BufferElement>::const_iterator end() const { return elements_.end(); }

private:
    void calculateOffsetsAndStride();

    std::vector<BufferElement> elements_;
    uint32_t stride_ = 0;
};

// Vertex Buffer
class VertexBuffer {
public:
    virtual ~VertexBuffer() = default;

    virtual void bind() const = 0;
    virtual void unbind() const = 0;

    virtual void setData(const void* data, uint32_t size) = 0;
    virtual std::size_t getSize() = 0;

    virtual const BufferLayout& getLayout() const = 0;
    virtual void setLayout(const BufferLayout& layout) = 0;

    static std::shared_ptr<VertexBuffer> create(uint32_t size);
    static std::shared_ptr<VertexBuffer> create(const void* vertices, uint32_t size);
};

// Index Buffer
class IndexBuffer {
public:
    virtual ~IndexBuffer() = default;

    virtual void bind() const = 0;
    virtual void unbind() const = 0;

    virtual uint32_t getCount() const = 0;

    static std::shared_ptr<IndexBuffer> create(const uint32_t* indices, uint32_t count);
};

} // namespace Perceptral
