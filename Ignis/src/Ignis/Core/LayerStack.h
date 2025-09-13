#pragma once
#include "Layer.h"
#include "pch.h"

namespace ignis {
    class LayerStack {
    public:
        LayerStack() = default;
        ~LayerStack();
        void PushLayer(std::unique_ptr<Layer> layer);
        void PushOverlay(std::unique_ptr<Layer> overlay);
        
        std::vector<std::unique_ptr<Layer>>::iterator begin() { return m_layers.begin(); }
        std::vector<std::unique_ptr<Layer>>::iterator end() { return m_layers.end(); }
        std::vector<std::unique_ptr<Layer>>::reverse_iterator rbegin() { return m_layers.rbegin(); }
        std::vector<std::unique_ptr<Layer>>::reverse_iterator rend() { return m_layers.rend(); }
        
    private:
        std::vector<std::unique_ptr<Layer>> m_layers;
        unsigned int m_layer_insert_index = 0;
    };
}