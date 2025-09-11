#pragma once

namespace ignis 
{
	class EventBase
	{
	public:
		EventBase() = default;
		virtual ~EventBase() = default;
		bool Handled = false;
	};

	template <typename T>
	class Event : public EventBase
	{
	public:
		Event() = default;
		Event(const T& type, const std::string& name)
			: m_type(type), m_name(name) { }
		virtual ~Event() = default;
		
		T GetType() const { return m_type; }
		const std::string& GetName() const { return m_name; }

	protected:
		T m_type;
		std::string m_name;
	};

	class EventDispatcher
	{
	public:
		using EventPtr = std::unique_ptr<EventBase>;
		using Func = std::function<void(EventBase&)>;
		using ListenerId = std::uint64_t;
		using EventTypeId = std::type_index;

		class Subscription
		{
		public:
			Subscription(EventDispatcher* dispatcher, EventTypeId type, ListenerId id)
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
					other.m_dispatcher = nullptr;
				}
				return *this;
			}

		private:
			EventDispatcher* m_dispatcher;
			EventTypeId m_type;
			ListenerId m_id;
		};

		EventDispatcher() = default;

		template<typename EventType>
			requires std::is_base_of_v<EventBase, EventType>
		Subscription Subscribe(std::function<void(EventType&)> func)
		{
			std::lock_guard<std::mutex> lock(m_listeners_mutex);
			EventTypeId type_id = std::type_index(typeid(EventType));
			ListenerId id = ++m_next_id;

			auto wrapper = [func = std::move(func)](EventBase& e) {
				func(static_cast<EventType&>(e));
				};

			m_listeners[type_id].emplace_back(id, std::move(wrapper));

			return Subscription(this, type_id, id);
		}

		void Dispatch(EventBase& event) 
		{
			std::vector<std::pair<ListenerId, Func>> listeners;
			{
				std::lock_guard<std::mutex> lock(m_listeners_mutex);
				auto it = m_listeners.find(std::type_index(typeid(event)));
				if (it != m_listeners.end()) listeners = it->second;
			}

			for (auto& [_, func] : listeners)
			{
				if (event.Handled) break;
				func(event);
			}

		}

		template<typename... Args>
		void Dispatch(EventBase& event, Args&&... args)
		{
			Dispatch(event, std::forward<Args>(args)...);
		}

		void QueueEvent(EventPtr event)
		{
			std::lock_guard<std::mutex> lock(m_queue_mutex);
			m_event_queue.push(std::move(event));
		}

		template<typename EventType, typename... Args>
			requires std::is_base_of_v<EventBase, EventType>
		void QueueEvent(Args&&... args)
		{
			QueueEvent(std::make_unique<EventType>(std::forward<Args>(args)...));
		}

		void ProcessEventQueue()
		{
			std::queue<EventPtr> temp_queue;
			{
				std::lock_guard<std::mutex> lock(m_queue_mutex);
				temp_queue.swap(m_event_queue);
			}

			while (!temp_queue.empty())
			{
				Dispatch(*temp_queue.front());
				temp_queue.pop();
			}
		}

	private:
		void Unsubscribe(EventTypeId type, ListenerId id)
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

		std::unordered_map<EventTypeId, std::vector<std::pair<ListenerId, Func>>> m_listeners;
		std::mutex m_listeners_mutex;

		std::queue<EventPtr> m_event_queue;
		std::mutex m_queue_mutex;

		std::atomic<ListenerId> m_next_id{ 0 };

		friend class Subscription;
	};
}