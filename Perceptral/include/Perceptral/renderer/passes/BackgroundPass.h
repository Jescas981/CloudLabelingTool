// renderer/passes/BackgroundPass.h
#include <Perceptral/renderer/RenderPass.h>
#include <Perceptral/rendering/BackgroundRenderer.h>
#include <Perceptral/core/Macros.h>

namespace Perceptral {

class Scene;

class BackgroundPass : public RenderPass {
    std::unique_ptr<BackgroundRenderer> m_renderer;

public:
    void initialize() override {
        m_renderer = std::make_unique<BackgroundRenderer>();
        m_renderer->initialize();
    }

    void execute(Scene& scene, Camera& camera) override {
        UNUSED(camera);

        auto& registry = scene.getRegistry();
        // Check if background settings exist
        if (!registry.ctx().contains<BackgroundSettings>()) {
            return;
        }
        
        auto& settings = registry.ctx().get<BackgroundSettings>();
        if (!settings.enabled) {
            return;
        }

        if (!m_renderer) {
            m_renderer = std::make_unique<BackgroundRenderer>();
            m_renderer->initialize(settings);
        }

        m_renderer->render();
    }

    void shutdown() override {
        m_renderer.reset();
    }

    int getPriority() const override { return 0; } // Render first
};

} // namespace Perceptral