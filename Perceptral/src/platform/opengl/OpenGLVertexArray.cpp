#include <Perceptral/platform/opengl/OpenGLVertexArray.h>
#include <glad/glad.h>

namespace Perceptral {

static GLenum shaderDataTypeToOpenGLBaseType(ShaderDataType type) {
  switch (type) {
  case ShaderDataType::Float:
  case ShaderDataType::Float2:
  case ShaderDataType::Float3:
  case ShaderDataType::Float4:
  case ShaderDataType::Mat3:
  case ShaderDataType::Mat4:
    return GL_FLOAT;
  case ShaderDataType::Int:
  case ShaderDataType::Int2:
  case ShaderDataType::Int3:
  case ShaderDataType::Int4:
    return GL_INT;
  case ShaderDataType::Bool:
    return GL_BOOL;
  default:
    return 0;
  }
}

OpenGLVertexArray::OpenGLVertexArray() { glGenVertexArrays(1, &rendererID_); }

OpenGLVertexArray::~OpenGLVertexArray() {
  glDeleteVertexArrays(1, &rendererID_);
}

void OpenGLVertexArray::bind() const { glBindVertexArray(rendererID_); }

void OpenGLVertexArray::unbind() const { glBindVertexArray(0); }

void OpenGLVertexArray::clearVertexBuffers() {
  vertexBuffers_.clear();
  vertexBufferIndex_ = 0;
}

void OpenGLVertexArray::addVertexBuffer(
    const std::shared_ptr<VertexBuffer> &vertexBuffer) {
  glBindVertexArray(rendererID_);
  vertexBuffer->bind();

  const auto &layout = vertexBuffer->getLayout();
  for (const auto &element : layout) {
    glEnableVertexAttribArray(vertexBufferIndex_);
    glVertexAttribPointer(
        vertexBufferIndex_, element.getComponentCount(),
        shaderDataTypeToOpenGLBaseType(element.type),
        element.normalized ? GL_TRUE : GL_FALSE, layout.getStride(),
        reinterpret_cast<const void *>(static_cast<intptr_t>(element.offset)));
    vertexBufferIndex_++;
  }

  vertexBuffers_.push_back(vertexBuffer);
}

void OpenGLVertexArray::setIndexBuffer(
    const std::shared_ptr<IndexBuffer> &indexBuffer) {
  glBindVertexArray(rendererID_);
  indexBuffer->bind();
  indexBuffer_ = indexBuffer;
}

} // namespace Perceptral
