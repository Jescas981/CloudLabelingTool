#pragma once
#include <Perceptral/core/Layer.h>
#include <memory>
#include <vector>

namespace Perceptral {

class LayerStack {
public:
    LayerStack() = default;
    ~LayerStack() = default;

    void pushLayer(std::unique_ptr<Layer> layer);
    void pushOverlay(std::unique_ptr<Layer> overlay);
    
    std::unique_ptr<Layer> popLayer(Layer* layer);
    std::unique_ptr<Layer> popOverlay(Layer* overlay);
    
    Layer* operator[](size_t index) { return m_layers[index].get(); }
    size_t size() const { return m_layers.size(); }
    
    // Iterator wrapper that returns Layer* instead of unique_ptr<Layer>&
    class Iterator {
        std::vector<std::unique_ptr<Layer>>::iterator it;
    public:
        Iterator(std::vector<std::unique_ptr<Layer>>::iterator i) : it(i) {}
        Layer* operator*() const { return it->get(); }
        Iterator& operator++() { ++it; return *this; }
        bool operator!=(const Iterator& other) const { return it != other.it; }
    };
    
    class ReverseIterator {
        std::vector<std::unique_ptr<Layer>>::reverse_iterator it;
    public:
        ReverseIterator(std::vector<std::unique_ptr<Layer>>::reverse_iterator i) : it(i) {}
        Layer* operator*() const { return it->get(); }
        ReverseIterator& operator++() { ++it; return *this; }
        bool operator!=(const ReverseIterator& other) const { return it != other.it; }
    };
    
    Iterator begin() { return Iterator(m_layers.begin()); }
    Iterator end() { return Iterator(m_layers.end()); }
    ReverseIterator rbegin() { return ReverseIterator(m_layers.rbegin()); }
    ReverseIterator rend() { return ReverseIterator(m_layers.rend()); }

private:
    std::vector<std::unique_ptr<Layer>> m_layers;
    size_t m_layerInsertIndex = 0;
};

} // namespace Perceptral