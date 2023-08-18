#pragma once

//#include <unordered_map>

#include "help_type.hpp"

namespace bnet {
	struct check_struct_memfns_base {
		void ParseFromArray();
	};
	template<typename T>
	struct check_struct_memfns_derived
		: T, check_struct_memfns_base {
	};
	template<typename T, T>
	struct check_struct_memfns_check {
	};
	template<typename>
	char(&parse_memfn_checker(...))[2];
	template <typename T>
	char parse_memfn_checker(check_struct_memfns_check<void (check_struct_memfns_base::*)(), &check_struct_memfns_derived<T>::ParseFromArray>*);
	/*template<typename T> //无法处理final类
	struct is_pb_proto : std::integral_constant<bool, sizeof(parse_memfn_checker<T>(0)) != 1> {
	};*/
	template<typename T>
	struct proto_has_parse {
	private:
		template<typename U>
		static auto check(bool) -> decltype(std::declval<U>().ParseFromArray((const void*)(0), int(0)), std::true_type());
		template<typename U>
		static std::false_type check(...);
	public:
		static constexpr bool value = std::is_same_v<decltype(check<T>(true)), std::true_type>;
	};
	template<typename T>
	struct is_pb_proto : proto_has_parse<T> {
	};
	template<typename T>
	constexpr bool is_pb_proto_v = is_pb_proto<unqualified_t<T>>::value;
	template<typename T>
	constexpr bool is_struct_proto_v = std::is_pod<unqualified_t<T>>::value;


	class msg_func_proxy_base {
	public:
		msg_func_proxy_base() = default;
		virtual ~msg_func_proxy_base() = default;
	};

	template<class... Args>
	class msg_func_proxy : public msg_func_proxy_base {
	public:
		using func_type = std::function<void(const char*, std::size_t, Args...)>;
		using args_type = std::tuple<Args...>;

		explicit msg_func_proxy(const func_type& fn) : fn_(fn) {}
		explicit msg_func_proxy(func_type&& fn) : fn_(std::move(fn)) {}
		explicit msg_func_proxy(const msg_func_proxy<Args...>& other) : fn_(other.fn_) {}
		explicit msg_func_proxy(msg_func_proxy<Args...>&& other) : fn_(std::move(other.fn_)) {}

		template<class F, class Packet, class ...C>
		explicit msg_func_proxy(F&& f, Packet&& pcg, C&&... c) {
			constexpr std::size_t argc = sizeof...(C);
			static_assert(argc == 0 || argc == 1, "c can only be none or one");
			this->bind(std::forward<F>(f), std::forward<Packet>(pcg), std::forward<C>(c)...);
		}

		template<class F, class Packet>
		inline void bind(F&& f, Packet&& pcg) {
			this->fn_ = [this, fn = std::forward<F>(f), pcg = std::move(pcg)](const char* buf, std::size_t len, Args&&... args) mutable {
				using PacketType = unqualified_t<Packet>;
				if constexpr (is_pb_proto_v<PacketType>) { //pb 协议解析
					if (pcg.ParseFromArray(buf, len)) {
						fn(pcg, std::forward<Args>(args)...);
					}
					return;
				}
				else /*if constexpr (is_struct_proto_v<PacketType>)*/ { //默认struct 协议解析
					if (len == sizeof(PacketType)) {
						pcg = *(PacketType*)buf;
						fn(pcg, std::forward<Args>(args)...);
					}
					return;
				}
			};
		}

		template<class F, class Packet, class C>
		inline void bind(F&& f, Packet&& pcg, C&& c) {
			using PacketType = unqualified_t<Packet>;
			if constexpr (std::is_pointer_v<std::decay_t<C>> || is_shared_ptr_v<std::decay_t<C>>) {
				this->fn_ = [this, fn = std::forward<F>(f), pcg = std::move(pcg), s = std::forward<C>(c)](const char* buf, std::size_t len, Args&&... args) mutable {
					if constexpr (is_pb_proto_v<PacketType>) { //pb 协议解析
						if (pcg.ParseFromArray(buf, len)) {
							((*s).*fn)(pcg, std::forward<Args>(args)...);
						}
						return;
					}
					else /*if constexpr (is_struct_proto_v<PacketType>)*/ { //默认struct 协议解析
						if (len == sizeof(PacketType)) {
							pcg = *(PacketType*)buf;
							((*s).*fn)(pcg, std::forward<Args>(args)...);
						}
						return;
					}
				};
			}
			else {
				this->fn_ = [this, fn = std::forward<F>(f), pcg = std::move(pcg), s = std::forward<C>(c)](const char* buf, std::size_t len, Args&&... args) mutable {
					if constexpr (is_pb_proto_v<PacketType>) { //pb 协议解析
						if (pcg.ParseFromArray(buf, len)) {
							(s.*fn)(pcg, std::forward<Args>(args)...);
						}
						return;
					}
					else /*if constexpr (is_struct_proto_v<PacketType>)*/ { //默认struct 协议解析
						if (len == sizeof(PacketType)) {
							pcg = *(PacketType*)buf;
							(s.*fn)(pcg, std::forward<Args>(args)...);
						}
						return;
					}
				};
			}
		}

		inline void operator()(const char* buf, std::size_t len, Args&&... args) {
			if (this->fn_)
				this->fn_(buf, len, std::forward<Args>(args)...);
		}

		inline func_type move() { return std::move(this->fn_); }

	protected:
		func_type fn_;
	};

	//如果绑定函数类型不支持, 自行增加相应接口.
	template<typename IDXTYPE>
	class msg_func_proxy_imp {
	public:
		template<class... Args>
		inline bool call(IDXTYPE evt, const char* buf, std::size_t len, Args&&... args) {
			if (!this->check(evt)) {
				return false;
			}
			using func_proxy_type = msg_func_proxy<Args...>;
			func_proxy_type* fp_ptr = static_cast<func_proxy_type*>(this->func_proxy_[evt].get());
			if (fp_ptr) {
				(*fp_ptr)(buf, len, std::forward<Args>(args)...);
			}
			return true;
		}

		inline bool check(IDXTYPE evt) {
			//return this->func_proxy_[evt];
			const auto& item = this->func_proxy_.find(evt);
			if (item != this->func_proxy_.end()) {
				return true;
			}
			return false;
		}

		template<class Ret, class Packet, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(*f)(Packet, Args...)) {
			unqualified_t<Packet> pcg;
			return this->bind_t(evt, msg_func_proxy<Args...>(f, pcg));
		}
		//reference OR point
		template<class Ret, class C, class Packet, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Packet, Args...), T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			unqualified_t<Packet> pcg;
			return this->bind_t(evt, msg_func_proxy<Args...>(f, pcg, std::forward<T>(c)));
		}
		template<class Ret, class C, class Packet, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Packet, Args...) const, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			unqualified_t<Packet> pcg;
			return this->bind_t(evt, msg_func_proxy<Args...>(f, pcg, std::forward<T>(c)));
		}
		template<class Callable, typename = std::enable_if_t<std::is_class<Callable>::value>>
		inline bool bind(IDXTYPE evt, Callable&& lam) {
			return this->bind(evt, &Callable::operator(), std::forward<Callable>(lam));
		}
		//std::shared_ptr
		template<class Ret, class C, class Packet, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Packet, Args...), std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			unqualified_t<Packet> pcg;
			return this->bind_t(evt, msg_func_proxy<Args...>(f, pcg, c));
		}
	protected:
		template<class T>
		inline bool bind_t(IDXTYPE evt, T&& fproxy) {
			if (this->check(evt)) {
				std::cout << " func is exist." << std::endl;
				return false;
			}
			this->func_proxy_[evt] = std::unique_ptr<msg_func_proxy_base>(new T(std::forward<T>(fproxy)));
			return true;
		}

		std::unordered_map<unqualified_t<IDXTYPE>, std::unique_ptr<msg_func_proxy_base>> func_proxy_;
	};
	using MSGIDPROXYTYPE = msg_func_proxy_imp<int32_t>;
	typedef std::shared_ptr<MSGIDPROXYTYPE> MsgIdFuncProxyImpPtr;

	using MSGSTRPROXYTYPE = msg_func_proxy_imp<std::string>;
	typedef std::shared_ptr<MSGSTRPROXYTYPE> MsgStrFuncProxyImpPtr;

}

