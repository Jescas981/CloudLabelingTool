// renderer/passes/BackgroundPass.h
#include <Perceptral/renderer/RenderPass.h>
#include <Perceptral/rendering/GridRenderer.h>
#include <Perceptral/core/Macros.h>

namespace Perceptral {

class Scene;

class GridPass : public RenderPass {
    std::unique_ptr<GridRenderer> m_renderer;

public:
    void initialize() override {
        m_renderer = std::make_unique<GridRenderer>();
        m_renderer->initialize();
    }

    void execute(Scene& scene, Camera& camera) override {
        UNUSED(scene);
        UNUSED(camera);

        if (!m_renderer) {
            m_renderer = std::make_unique<GridRenderer>();
            m_renderer->initialize();
        }

        m_renderer->render(camera);
    }

    void shutdown() override {
        m_renderer.reset();
    }

    int getPriority() const override { return 50; } // Render first
};

} // namespace Perceptral