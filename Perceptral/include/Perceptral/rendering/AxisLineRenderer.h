#pragma once
#include <memory>
#include <Perceptral/renderer/VertexArray.h>
#include <Perceptral/renderer/Shader.h>

namespace Perceptral {

class Camera;

class AxisLineRenderer {
public:
    AxisLineRenderer();
    ~AxisLineRenderer();

    void initialize();
    void render(const Camera& camera);

private:
    std::shared_ptr<VertexArray> vertexArray_;
    std::shared_ptr<Shader> shader_;
    size_t gridVertexCount_ = 0;
    size_t totalVertexCount_ = 0;
};

} // namespace Perceptral