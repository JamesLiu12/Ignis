#pragma once

#include "pch.h"

namespace ignis 
{

	template <typename T>
	class Event
	{
	public:
		Event() = default;
		Event(const T& type, const std::string& name)
			: m_type(type), m_name(name) { }
		virtual ~Event() = default;
		
		T GetType() const { return m_type; }
		const std::string& GetName() const { return m_name; }

		bool Handled = false;

	protected:
		T m_type;
		std::string m_name;
	};

	template <typename T>
	class EventDispatcher
	{
	public:
		using Func = std::function<void(Event<T>&)>;
		using ListenerId = std::uint64_t;

		class Subscription
		{
		public:
			Subscription(EventDispatcher* dispatcher, const T& type, ListenerId id)
				: m_dispatcher(dispatcher), m_type(type), m_id(id) {}

			~Subscription()
			{
				if (m_dispatcher)

				{
					m_dispatcher->Unsubscribe(m_type, m_id);
				}
			}

			Subscription(const Subscription&) = delete;
			Subscription& operator=(const Subscription&) = delete;

			Subscription(Subscription&& other) noexcept
				: m_dispatcher(other.m_dispatcher), m_type(other.m_type), m_id(other.m_id)
			{
				other.m_dispatcher = nullptr;
			}

			Subscription& operator=(Subscription&& other) noexcept
			{
				if (this != &other)
				{
					if (m_dispatcher)
					{
						m_dispatcher->Unsubscribe(m_type, m_id);
					}
					m_dispatcher = other.m_dispatcher;
					m_type = other.m_type;
					m_id = other.m_id;
				}
				return *this;
			}

		private:
			EventDispatcher* m_dispatcher;
			T m_type;
			ListenerId m_id;
		};

		EventDispatcher() = default;

		Subscription Subscribe(const T& type, const Func& func)
		{
			std::lock_guard<std::mutex> lock(m_listeners_mutex);
			ListenerId id = ++m_next_id;
			m_listeners[type].emplace_back(id, std::move(func));
			return Subscription(this, type, id);
		}

		void Dispatch(Event<T>& event)
		{
			std::lock_guard<std::mutex> lock(m_listeners_mutex);
			auto it = m_listeners.find(event.GetType());
			if (it != m_listeners.end())
			{
				for (auto& [id, func] : it->second)
				{
					func(event);
					if (event.Handled) break;
				}
			}
		}

		void QueueEvent(const Event<T>& event)
		{
			std::lock_guard<std::mutex> lock(m_queue_mutex);
			m_event_queue.push(event);
		}

		void ProcessEventQueue()
		{
			std::queue<Event<T>> temp_queue;
			{
				std::lock_guard<std::mutex> lock(m_queue_mutex);
				temp_queue.swap(m_event_queue);
			}

			while (!temp_queue.empty())
			{
				auto event = std::move(temp_queue.front());
				temp_queue.pop();
				Dispatch(event);
			}
		}

	private:
		void Unsubscribe(const T& type, ListenerId id)
		{
			std::lock_guard<std::mutex> lock(m_listeners_mutex);
			auto it = m_listeners.find(type);
			if (it != m_listeners.end())
			{
				auto& vec = it->second;
				vec.erase(std::remove_if(vec.begin(), vec.end(),
					[id](const auto& pair) { return pair.first == id; }), vec.end());
				if (vec.empty())
				{
					m_listeners.erase(it);
				}
			}
		}

		std::unordered_map<T, std::vector<std::pair<ListenerId, Func>>> m_listeners;
		std::mutex m_listeners_mutex;

		std::queue<Event<T>> m_event_queue;
		std::mutex m_queue_mutex;

		std::atomic<ListenerId> m_next_id{ 0 };

		friend class Subscription;
	};
}