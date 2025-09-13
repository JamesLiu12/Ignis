#include "LayerStack.h"

namespace ignis {

	LayerStack::~LayerStack()
	{
		for (auto& layer : m_layers)
		{
			layer->OnDetach();
		}
	}

	void LayerStack::PushLayer(std::unique_ptr<Layer> layer)
	{
		m_layers.emplace(m_layers.begin() + m_layer_insert_index, std::move(layer));
		m_layer_insert_index++;
		m_layers[m_layer_insert_index - 1]->OnAttach();
	}

	void LayerStack::PushOverlay(std::unique_ptr<Layer> overlay)
	{
		m_layers.emplace_back(std::move(overlay));
		m_layers.back()->OnAttach();
	}

}
