#include <Perceptral/core/layers/RenderLayer.h>
#include <Perceptral/core/Log.h>
#include <Perceptral/renderer/passes/BackgroundPass.h>
#include <Perceptral/renderer/passes/GridPass.h>
#include <Perceptral/renderer/passes/PointCloudPass.h>
#include <Perceptral/renderer/passes/AxisLinePass.h>

namespace Perceptral {

RenderLayer::RenderLayer(Scene& scene, Camera& camera)
    : Layer("RenderLayer")
    , m_scene(scene)
    , m_camera(camera) {
}

void RenderLayer::onAttach() {
    PC_CORE_INFO("RenderLayer attached");
    m_renderPipeline.addPass<BackgroundPass>();
    m_renderPipeline.addPass<PointCloudPass>();
    // m_renderPipeline.addPass<GridPass>();
    m_renderPipeline.addPass<AxisLinePass>();
    m_renderPipeline.initialize();
}

void RenderLayer::onDetach() {
    PC_CORE_INFO("RenderLayer detached");
    m_renderPipeline.shutdown();
}

void RenderLayer::onRender() {
    // Execute render pipeline
    m_renderPipeline.render(m_scene, m_camera);
}

void RenderLayer::onImGuiRender() {
   
}

} // namespace Perceptral