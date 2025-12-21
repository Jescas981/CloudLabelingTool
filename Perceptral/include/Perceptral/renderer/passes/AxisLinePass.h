// renderer/passes/BackgroundPass.h
#include <Perceptral/renderer/RenderPass.h>
#include <Perceptral/rendering/AxisLineRenderer.h>

namespace Perceptral {

class Scene;

class AxisLinePass : public RenderPass {
    std::unique_ptr<AxisLineRenderer> m_renderer;

public:
    void initialize() override {
        m_renderer = std::make_unique<AxisLineRenderer>();
        m_renderer->initialize();
    }

    void execute(Scene& scene, Camera& camera) override {
        UNUSED(scene);
        UNUSED(camera);

        if (!m_renderer) {
            m_renderer = std::make_unique<AxisLineRenderer>();
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