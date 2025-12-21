#pragma once

#include <Perceptral/renderer/VertexArray.h"

namespace Perceptral {

#pragma once

#include <cstddef>
#include <vector>

namespace Perceptral {

class OpenGLVertexBuffer {
public:
  OpenGLVertexBuffer();
  ~OpenGLVertexBuffer();

  // Initialize VAO/VBO
  void create();
  void destroy();

  // Upload data
  void uploadData(const void *data, size_t size, bool dynamic = false);

  // Bind/unbind
  void bind() const;
  void unbind() const;

  // Vertex attributes
  void setVertexAttribute(unsigned int index, int size, unsigned int type,
                          bool normalized, int stride, size_t offset);
  void enableVertexAttribute(unsigned int index);
  void disableVertexAttribute(unsigned int index);

  // Getters
  unsigned int getVAO() const { return vao_; }
  unsigned int getVBO() const { return vbo_; }
  size_t getVertexCount() const { return vertexCount_; }

  void setVertexCount(size_t count) { vertexCount_ = count; }

private:
  unsigned int vao_;
  unsigned int vbo_;
  size_t vertexCount_;
};

} // namespace Perceptral