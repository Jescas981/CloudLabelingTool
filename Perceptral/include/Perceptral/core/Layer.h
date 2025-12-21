#pragma once
#include <Perceptral/core/Event.h>
#include <Perceptral/core/Macros.h>
#include <Perceptral/core/DeltaTime.h>
#include <string>

// core/Layer.h
namespace Perceptral {
class Layer {
public:
  Layer(const std::string &name = "Layer") : m_debugName(name) {}
  virtual ~Layer() = default;

  // Lifecycle
  virtual void onAttach() {}
  virtual void onDetach() {}

  // Per-frame callbacks
  virtual void onUpdate(DeltaTime ts) { UNUSED(ts); }
  virtual void onRender() {}
  virtual void onImGuiRender() {}

  // Event handling
  virtual void onEvent(Event &e) { UNUSED(e); }

  const std::string &getName() const { return m_debugName; }

protected:
  std::string m_debugName;
};

} // namespace Perceptral