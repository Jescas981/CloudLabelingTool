// rendering/GridRenderer.cpp
#include <Perceptral/rendering/GridRenderer.h>
#include <Perceptral/core/Camera.h>
#include <Perceptral/core/Log.h>
#include <Perceptral/renderer/Buffer.h>
#include <Perceptral/renderer/Renderer.h>
#include <glad/glad.h>

namespace Perceptral {

GridRenderer::GridRenderer() {}

GridRenderer::~GridRenderer() {}

void GridRenderer::initialize() {
    vertexArray_ = VertexArray::create();
    shader_ = Shader::create("shaders/grid.glsl");
    
    // Create a simple static grid immediately
    createSimpleGrid();
}

void GridRenderer::render(const Camera& camera) {
    if (!settings_.enabled || vertexCount_ == 0) {
        return;
    }
    
    // Save OpenGL state
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    
    // Set state for grid
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Bind shader and set uniforms
    shader_->bind();
    shader_->setMat4("uViewProjection", camera.getViewProjectionMatrix());
    shader_->setMat4("uModel", Eigen::Matrix4f::Identity());
    
    // Draw grid
    vertexArray_->bind();
    glDrawArrays(GL_LINES, 0, vertexCount_);
    
    // Check for errors immediately after draw
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        PC_CORE_ERROR("OpenGL error in grid render: {}", err);
    }
    
    vertexArray_->unbind();
    shader_->unbind();
    
    // Restore state
    if (!depthTestEnabled) glDisable(GL_DEPTH_TEST);
    if (!blendEnabled) glDisable(GL_BLEND);
}

void GridRenderer::createSimpleGrid() {
    // Create a very simple 10x10 grid centered at origin
    struct GridVertex {
        float x, y, z;        // position
        float r, g, b, a;     // color
    };
    
    std::vector<GridVertex> vertices;
    float gridSize = 10.0f;
    int divisions = 10;
    float step = gridSize / divisions;
    
    // Lines parallel to X axis
    for (int i = 0; i <= divisions; i++) {
        float z = -gridSize/2.0f + i * step;
        vertices.push_back({-gridSize/2.0f, 0.0f, z, 0.5f, 0.5f, 0.5f, 1.0f});
        vertices.push_back({ gridSize/2.0f, 0.0f, z, 0.5f, 0.5f, 0.5f, 1.0f});
    }
    
    // Lines parallel to Z axis
    for (int i = 0; i <= divisions; i++) {
        float x = -gridSize/2.0f + i * step;
        vertices.push_back({x, 0.0f, -gridSize/2.0f, 0.5f, 0.5f, 0.5f, 1.0f});
        vertices.push_back({x, 0.0f,  gridSize/2.0f, 0.5f, 0.5f, 0.5f, 1.0f});
    }
    
    vertexCount_ = vertices.size();
    
    PC_CORE_INFO("Creating grid with {} vertices", vertexCount_);
    
    // Create buffer
    auto vertexBuffer = VertexBuffer::create(vertices.data(), 
                                             vertices.size() * sizeof(GridVertex));
    vertexBuffer->setLayout({
        {ShaderDataType::Float3, "aPosition"},
        {ShaderDataType::Float4, "aColor"}
    });
    
    vertexArray_->addVertexBuffer(vertexBuffer);
    
    PC_CORE_INFO("Grid buffer created successfully");
}


} // namespace Perceptral