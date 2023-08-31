#pragma once

#include <shared_mutex>

namespace bnet::base {
	template<class SessionType>
	class session_mgr {
	public:
		using self = session_mgr<SessionType>;
		using key_type = typename SessionType::key_type;
		using session_ptr = std::shared_ptr<SessionType>;
	public:
		explicit session_mgr() {
			this->sessions_.reserve(64);
		}
		~session_mgr() = default;

		inline bool emplace(const session_ptr& session_ptr) {
			if (!session_ptr)
				return false;

			std::unique_lock<std::shared_mutex> guard(this->mutex_);
			return this->sessions_.try_emplace(session_ptr->hash_key(), session_ptr).second;
		}

		inline bool erase(const session_ptr& session_ptr) {
			if (!session_ptr)
				return false;

			std::unique_lock<std::shared_mutex> guard(this->mutex_);
			return (this->sessions_.erase(session_ptr->hash_key()) > 0);
		}

		inline void foreach(const std::function<void(session_ptr &)> & fn) {
			std::vector<session_ptr> sessions;

			{
				std::shared_lock<std::shared_mutex> guard(this->mutex_);

				sessions.reserve(this->sessions_.size());

				for (const auto& [k, session_ptr] : this->sessions_)
				{
					std::ignore = k;

					sessions.emplace_back(session_ptr);
				}
			}

			for (session_ptr& session_ptr : sessions) {
				fn(session_ptr);
			}

			//std::shared_lock<std::shared_mutex> guard(this->mutex_);
			//for (auto &[k, session_ptr] : this->sessions_) {
			//	fn(session_ptr);
			//}
		}

		inline session_ptr find(const key_type & key) {
			std::shared_lock<std::shared_mutex> guard(this->mutex_);
			auto iter = this->sessions_.find(key);
			return (iter == this->sessions_.end() ? session_ptr() : iter->second);
		}

		inline session_ptr find_if(const std::function<bool(session_ptr &)> & fn) {
			std::shared_lock<std::shared_mutex> guard(this->mutex_);
			auto iter = std::find_if(this->sessions_.begin(), this->sessions_.end(),
				[this, &fn](auto &pair)
			{
				return fn(pair.second);
			});
			return (iter == this->sessions_.end() ? session_ptr() : iter->second);
		}

		inline std::size_t size() {
			std::shared_lock<std::shared_mutex> guard(this->mutex_);
			return this->sessions_.size();
		}

		inline bool empty() {
			std::shared_lock<std::shared_mutex> guard(this->mutex_);
			return this->sessions_.empty();
		}

		inline session_ptr rand_get() {
			if (sessions_.size() <= 0) {
				return session_ptr();
			}
			
			std::random_device rd;
    		std::mt19937 gen(rd());

    		std::uniform_int_distribution<int> distribution(0, sessions_.size() - 1); // 适当调整范围
    		//int randomIndex = distribution(gen);
			auto random_it = std::next(std::begin(sessions_), distribution(gen));
			return (random_it == this->sessions_.end() ? session_ptr() : random_it->second);
		}
	protected:
		std::unordered_map<key_type, session_ptr> sessions_;
		std::shared_mutex mutex_;
	};
}
