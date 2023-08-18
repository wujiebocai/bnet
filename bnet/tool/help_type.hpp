
#pragma once

//#include <unordered_map>
//#include <functional>
//#include <assert.h>

namespace bnet {
#define CHECK_POINT(p)	\
	if (!p) {			\
		return false;	\
	}					\

	template <typename T>
	using unqualified = std::remove_cv<std::remove_reference_t<T>>;
	template <typename T>
	using unqualified_t = typename unqualified<T>::type;
	template<class T>
	struct is_shared_ptr_helper : std::false_type {};
	template<class T>
	struct is_shared_ptr_helper<std::shared_ptr<T>> : std::true_type {};
	template<class T>
	struct is_shared_ptr : is_shared_ptr_helper<unqualified_t<T>> {};
	template<class T>
	constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

	template<class T1, class T2>
	struct is_same_category : std::conditional<std::is_class_v<T1> && std::is_class_v<T2>, std::is_same<T1, T2>, std::false_type>::type {};
	template<class T1, class T2>
	constexpr bool is_same_category_t = is_same_category<std::decay_t<std::remove_pointer_t<T1>>, std::decay_t<std::remove_pointer_t<T2>>>::value;


	class func_proxy_base {
	public:
		func_proxy_base() = default;
		virtual ~func_proxy_base() = default;
	};

	template<class... Args>
	class func_proxy : public func_proxy_base {
	public:
		using func_type = std::function<void(Args...)>;
		using args_type = std::tuple<Args...>;
		//static constexpr std::size_t argc_nums = sizeof...(Args);

		explicit func_proxy(const func_type& fn) : fn_(fn) {}
		explicit func_proxy(func_type&& fn) : fn_(std::move(fn)) {}
		explicit func_proxy(const func_proxy<Args...>& other) : fn_(other.fn_) {}
		explicit func_proxy(func_proxy<Args...>&& other) : fn_(std::move(other.fn_)) {}

		template<class F, class ...C>
		explicit func_proxy(F&& f, C&&... c) {
			constexpr std::size_t argc = sizeof...(C);
			static_assert(argc == 0 || argc == 1, "c can only be none or one");
			this->bind(std::forward<F>(f), std::forward<C>(c)...);
		}

		template<class F>
		inline void bind(F&& f) {
			this->fn_ = func_type(std::forward<F>(f));
		}

		template<class F, class C>
		inline void bind(F&& f, C&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<C>> || is_shared_ptr_v<std::decay_t<C>>) {
				this->fn_ = [this, fn = std::forward<F>(f), s = std::forward<C>(c)](Args&&... args) mutable {
					((*s).*fn)(std::forward<Args>(args)...);
				};
			}
			else /*if constexpr (std::is_reference_v<C>)*/ {
				this->fn_ = [this, fn = std::forward<F>(f), s = std::forward<C>(c)](Args&&... args) mutable {
					(s.*fn)(std::forward<Args>(args)...);
				};
			}
		}

		inline void operator()(Args&&... args) {
			if (this->fn_)
				this->fn_(std::forward<Args>(args)...);
		}

		inline func_type move() { return std::move(this->fn_); }

	protected:
		func_type fn_;
	};

	template<typename IDXTYPE>
	class func_proxy_imp {
	public:
		template<class... Args>
		inline bool call(IDXTYPE evt, Args&&... args) {
			if (!this->check(evt)) {
				return false;
			}
			using func_proxy_type = func_proxy<Args...>;
			func_proxy_type* fp_ptr = static_cast<func_proxy_type*>(this->func_proxy_[evt].get());
			if (fp_ptr) {
				(*fp_ptr)(std::forward<Args>(args)...);
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

		template<class Ret, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(*f)(Args...)) {
			return this->bind_t(evt, func_proxy<Args...>(f));
		}
		template <typename Ret, typename... Args>
		inline bool bind(IDXTYPE evt, std::function<Ret(Args...)> f) {
			return this->bind_t(evt, func_proxy<Args...>(f));
		}
		template<class Ret, class ...Args>
		inline bool bind(IDXTYPE evt, std::function<Ret(Args...) const> f) {
			return this->bind_t(evt, func_proxy<Args...>(f));
		}
		template<class Ret, class ...Args>
		inline bool bind(IDXTYPE evt, std::function <Ret(Args...) volatile> f) {
			return this->bind_t(evt, func_proxy<Args...>(f));
		}
		template<class Ret, class ...Args>
		inline bool bind(IDXTYPE evt, std::function <Ret(Args...) const volatile> f) {
			return this->bind_t(evt, func_proxy<Args...>(f));
		}
		//reference OR point
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...), T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) volatile, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const volatile, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...)&, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const&, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) volatile&, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const volatile&, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...)&&, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const&&, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) volatile&&, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const volatile&&, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) noexcept, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const noexcept, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) volatile noexcept, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const volatile noexcept, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) & noexcept, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const& noexcept, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) volatile& noexcept, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const volatile& noexcept, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) && noexcept, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const&& noexcept, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) volatile&& noexcept, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Ret, class C, class ...Args, class T, typename = std::enable_if_t<is_same_category_t<C, T>>>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const volatile&& noexcept, T&& c) {
			if constexpr (std::is_pointer_v<std::decay_t<T>>) {
				CHECK_POINT(c);
			}
			return this->bind_t(evt, func_proxy<Args...>(f, std::forward<T>(c)));
		}
		template<class Callable, typename = std::enable_if_t<std::is_class<Callable>::value>>
		inline bool bind(IDXTYPE evt, Callable&& lam) {
			return this->bind(evt, &Callable::operator(), std::forward<Callable>(lam));
		}
		//std::shared_ptr
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...), std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) volatile, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const volatile, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...)&, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const&, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) volatile&, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const volatile&, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...)&&, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const&&, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) volatile&&, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const volatile&&, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) noexcept, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const noexcept, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) volatile noexcept, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const volatile noexcept, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) & noexcept, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const& noexcept, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) volatile& noexcept, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const volatile& noexcept, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) && noexcept, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const&& noexcept, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) volatile&& noexcept, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}
		template<class Ret, class C, class ...Args>
		inline bool bind(IDXTYPE evt, Ret(C::* f)(Args...) const volatile&& noexcept, std::shared_ptr<std::decay_t<C>> c) {
			CHECK_POINT(c);
			return this->bind_t(evt, func_proxy<Args...>(f, c));
		}

	protected:
		template<class T>
		inline bool bind_t(IDXTYPE evt, T&& fproxy) {
			if (this->check(evt)) {
				std::cout << " func is exist." << std::endl;
				return false;
			}
			this->func_proxy_[evt] = std::unique_ptr<func_proxy_base>(new T(std::forward<T>(fproxy)));
			return true;
		}

		std::unordered_map<unqualified_t<IDXTYPE>, std::unique_ptr<func_proxy_base>> func_proxy_;
	};
}