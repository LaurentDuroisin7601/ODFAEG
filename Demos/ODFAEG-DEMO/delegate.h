#ifndef ODFAEG_FAST_DELEGATE_HPP
#define ODFAEG_FAST_DELEGATE_HPP
#include "function.h"
#include "odfaeg/Core/erreur.h"
#include<memory>
#include<tuple>
#include<type_traits>
#include<utility>

namespace test {
template<class T>
struct IRefVal {
	IRefVal()=default;
	virtual T& get() =0;
	virtual std::unique_ptr<IRefVal> clone() const = 0;
	virtual ~IRefVal(){}

protected:
	IRefVal(const IRefVal&){}
	IRefVal& operator=(const IRefVal&)
	{ return *this; }
};

template<class T>
struct Ref : IRefVal<T> {
	Ref(const std::reference_wrapper<T>& r)
		: ref(r)
	{}
	T& get()
	{ return ref.get(); }
	std::unique_ptr<IRefVal<T>> clone() const
	{ return std::make_unique<Ref>(*this); }

private:
	std::reference_wrapper<T> ref;
};

template<class T>
struct Val : IRefVal<T> {
	Val(const T& t)
		: val(t)
	{}
	T& get()
	{ return val; }
	std::unique_ptr<IRefVal<T>> clone() const
	{ return std::make_unique<Val>(*this); }

private:
	T val;
};

template<class T>
struct RefVal {
	RefVal(const T& t)
		: rv(std::make_unique<Val<T>>(t))
	{}
	RefVal(const std::reference_wrapper<T>& r)
		: rv(std::make_unique<Ref<T>>(r))
	{}
	RefVal(const RefVal& rhs)
		: rv(rhs.rv->clone())
	{}
	RefVal& operator=(const RefVal& rhs)
	{ rv=rhs.rv->clone(); return *this; }
	T& get() const
	{ return rv->get(); }

private:
	std::unique_ptr<IRefVal<T>> rv;
};

template<class T>
struct ToStoreImpl
{ using type = T; };

template<class T>
struct ToStoreImpl<std::reference_wrapper<T>>
{ using type = T; };

template<class T>
struct ToStore
	: ToStoreImpl<std::remove_reference_t<T>>
{};

template<class T>
using ToStore_t = typename
	ToStore<T>::type;

template<class R>
struct Delegate {
	Delegate() =default;
	virtual std::unique_ptr<Delegate> clone() const = 0;
	virtual R operator()() = 0;
	virtual ~Delegate(){}

protected:
	Delegate(const Delegate&){}
	Delegate& operator=(const Delegate&)
	{ return *this; }
};

template<class R, class... ArgT>
struct FastDelegateImpl : Delegate<R> {
	template<class F, class... ArgU>
	FastDelegateImpl(F&& f, ArgU&&... arg)
		: func(std::forward<F>(f))
		, param(std::forward<ArgU>(arg)...)
	{}
	std::unique_ptr<Delegate<R>> clone() const
	{ return std::make_unique<FastDelegateImpl>(*this); }
	R operator()()
	{ return call(std::make_index_sequence<sizeof...(ArgT)>()); }
	template<class... ArgU>
	void setParam(ArgU&&... arg)
	{ param=std::make_tuple(std::forward<ArgU>(arg)...); }

private:
	template<std::size_t... I>
	R call(std::index_sequence<I...>)
	{ return func(std::get<I>(param).get()...); }

	std::function<R(ArgT&...)> func;
	std::tuple<RefVal<ArgT>...> param;
};

template<class R>
struct FastDelegate {
	template<class F, class... Arg>
	FastDelegate(F&& f, Arg&&... arg) :
		delegate(std::make_unique
			<FastDelegateImpl<R,ToStore_t<Arg>...>>
			(std::forward<F>(f),std::forward<Arg>(arg)...)
		)
	{}
	FastDelegate(FastDelegate& rhs)
		: delegate(rhs.delegate->clone())
	{}
	FastDelegate(const FastDelegate& rhs)
		: delegate(rhs.delegate->clone())
	{}
	FastDelegate(FastDelegate&& rhs) =default;
	FastDelegate& operator=(const FastDelegate& rhs)
	{ return operator=(FastDelegate(rhs)); }
	FastDelegate& operator=(FastDelegate&&) =default;
	R operator()() const
	{ return (*delegate)(); }
	template<class... Arg>
	void setParam(Arg&&... arg)
	{
		using DynamicType =
			FastDelegateImpl<R,ToStore_t<Arg>...>*;

		if(dynamic_cast<DynamicType>(delegate.get()))
			dynamic_cast<DynamicType>(delegate.get())->setParam(std::forward<Arg>(arg)...);
	}

private:
	std::unique_ptr<Delegate<R>> delegate;
};
}
#endif // PLACE_HOLDERS_H

