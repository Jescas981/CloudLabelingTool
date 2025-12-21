#include <Perceptral/core/LayerStack.h>
#include <algorithm>

namespace Perceptral {

void LayerStack::pushLayer(std::unique_ptr<Layer> layer) {
    layer->onAttach();
    m_layers.emplace(m_layers.begin() + m_layerInsertIndex, std::move(layer));
    m_layerInsertIndex++;
}

void LayerStack::pushOverlay(std::unique_ptr<Layer> overlay) {
    overlay->onAttach();
    m_layers.emplace_back(std::move(overlay));
}

std::unique_ptr<Layer> LayerStack::popLayer(Layer* layer) {
    auto it = std::find_if(m_layers.begin(), m_layers.begin() + m_layerInsertIndex,
                           [layer](const auto& ptr) { return ptr.get() == layer; });
    
    if (it != m_layers.begin() + m_layerInsertIndex) {
        (*it)->onDetach();
        auto result = std::move(*it);
        m_layers.erase(it);
        m_layerInsertIndex--;
        return result;
    }
    return nullptr;
}

std::unique_ptr<Layer> LayerStack::popOverlay(Layer* overlay) {
    auto it = std::find_if(m_layers.begin() + m_layerInsertIndex, m_layers.end(),
                           [overlay](const auto& ptr) { return ptr.get() == overlay; });
    
    if (it != m_layers.end()) {
        (*it)->onDetach();
        auto result = std::move(*it);
        m_layers.erase(it);
        return result;
    }
    return nullptr;
}

} // namespace Perceptral