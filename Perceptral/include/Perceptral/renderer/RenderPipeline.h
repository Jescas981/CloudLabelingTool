#pragma once

#include <Perceptral/core/Log.h>
#include <Perceptral/renderer/RenderPass.h>
#include <memory>
#include <vector>

namespace Perceptral {

class RenderPipeline {
  std::vector<std::unique_ptr<RenderPass>> m_passes;
  bool m_initialized = false;

public:
  template <typename T, typename... Args> void addPass(Args &&...args) {
    auto pass = std::make_unique<T>(std::forward<Args>(args)...);
    m_passes.push_back(std::move(pass));
    sortPasses();
  }

  void initialize() {
    for (auto &pass : m_passes) {
      pass->initialize();
    }
    m_initialized = true;
  }

  void render(Scene &scene, Camera &camera) {
    if (!m_initialized) {
      PC_CORE_ERROR("RenderPipeline not initialized!");
      return;
    }

    for (auto &pass : m_passes) {
      pass->execute(scene, camera);
    }
  }

  void shutdown() {
    for (auto &pass : m_passes) {
      pass->shutdown();
    }
    m_passes.clear();
  }

private:
  void sortPasses() {
    std::sort(m_passes.begin(), m_passes.end(),
              [](const auto &a, const auto &b) {
                return a->getPriority() < b->getPriority();
              });
  }
};

} // namespace Perceptral