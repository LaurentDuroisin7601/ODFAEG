namespace odfaeg {
    namespace core {  
        template<class T, typename LateParamsT, bool isCopiable>
        Ref<T, LateParamsT, isCopiable>::Ref(const std::reference_wrapper<T>& r)
            : ref(r)
        {
        }
        template<class T, typename LateParamsT, bool isCopiable>
        T& Ref<T, LateParamsT, isCopiable>::bind(void* params) {
            std::cout<<"get : "<<&ref.get()<<std::endl;
            return ref.get();
        }
        template<class T, typename LateParamsT, bool isCopiable>
        T& Ref<T, LateParamsT, isCopiable>::get()
        {
            return ref.get();
        }
        template<class T, typename LateParamsT, bool isCopiable>
        std::unique_ptr<IRefVal<T, LateParamsT, true>> Ref<T, LateParamsT, isCopiable>::clone()
        {
            return std::make_unique<Ref>(*this);
        }
        /**
        *  \file  fastDelegate.h
        *  \class Val
        *  \brief Warp a value.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class T, class LateParamsT>
        Val<T, LateParamsT, true>::Val(const T& t)
            : val(t)
        {
        }
        template<class T, class LateParamsT>
        T& Val<T, LateParamsT, true>::bind(void* params) {
            return val;
        }
        template<class T, class LateParamsT>
        T& Val<T, LateParamsT, true>::get()
        {
            return val;
        }
        template<class T, class LateParamsT>
        std::unique_ptr<IRefVal<T, LateParamsT, true>> Val<T, LateParamsT, true>::clone()
        {
            return std::make_unique<Val>(*this);
        } 
        template<class T, class LateParamsT>
        Val<T, LateParamsT, false>::Val(const T& t)
            : val(t)
        {
        } 
        /*template<class T, class LateParamsT>
        std::unique_ptr<IRefVal<T, LateParamsT, true>> Val<T, LateParamsT, false>::clone()
        {
            return std::make_unique<Val>(*this);
        }*/
        template<class T, class LateParamsT>
        Val<T, LateParamsT, false>::Val(T&& t)
            : val(std::move(t))
        {
        }
        template<class T, class LateParamsT>
        Val<T, LateParamsT, false>::Val(Val&& v) : val(std::move(v.val)) {
        }
        template<class T, class LateParamsT>
        Val<T, LateParamsT, false>& Val<T, LateParamsT, false>::operator=(Val&& v) {
            val = std::move(v.val);
            return *this;
        }
        template<class T, class LateParamsT>
        T&& Val<T, LateParamsT, false>::bind(void* params) {
            return std::move(val);
        }
        template<class T, class LateParamsT>
        T&& Val<T, LateParamsT, false>::get()
        {
            return std::move(val);
        }
        template<class T, class LateParamsT>
        std::unique_ptr<IRefVal<T, LateParamsT, false>> Val<T, LateParamsT, false>::transfer()
        {
            return std::make_unique<Val>(std::move(*this));
        }
        template<size_t I, class T, class LateParamsT>
        std::unique_ptr<IRefVal<T, LateParamsT, true>> Placeholder<I, T, LateParamsT, true>::clone()
        {
            return std::make_unique<Placeholder>(*this);
        }        
        template<size_t I, class T, class LateParamsT>
        T& Placeholder<I, T, LateParamsT, true>::bind(void* params) {
            //We cast from void* to the placeholders's holder type.
            LateParamsT& paramsT = *static_cast<LateParamsT*>(params);
            //We cast from base type to the derived type to extract the placeholder's value.
            //RefVal<T, LateParamsT, true> rv(static_cast<Parameter<I, T>&>(paramsT).value);
            RefVal<T, LateParamsT, true> rv(static_cast<Parameter<I, T>&>(paramsT).value);
            //std::cout<<"get : "<<&rv.get()<<std::endl;
            return rv.get();
        }
        template<size_t I, class T, class LateParamsT>
        T& Placeholder<I, T, LateParamsT, true>::get()
        {

        }        
        template<size_t I, class T, class LateParamsT>
        Placeholder<I, T, LateParamsT, false>::Placeholder(Placeholder&& ph) {}
        template<size_t I, class T, class LateParamsT>
        Placeholder<I, T, LateParamsT, false>& Placeholder<I, T, LateParamsT, false>::operator=(Placeholder&& ph) {
            return *this;
        } 
        template<size_t I, class T, class LateParamsT>          
        std::unique_ptr<IRefVal<T, LateParamsT, false>> Placeholder<I, T, LateParamsT, false>::transfer()
        {
            return std::make_unique<Placeholder>(std::move(*this));
        }
        template<size_t I, class T, class LateParamsT>
        T&& Placeholder<I, T, LateParamsT, false>::bind(void* params) {
            //We cast from void* to the placeholders's holder type.
            LateParamsT& paramsT = *static_cast<LateParamsT*>(params);
            //We cast from base type to the derived type to extract the placeholder's value.
            return std::move(static_cast<Parameter<I, T>&>(paramsT).value);
        }
        template<size_t I, class T, class LateParamsT>
        T&& Placeholder<I, T, LateParamsT, false>::get()
        {

        } 
        template<class T, class LateParamsT>
        RefVal<T, LateParamsT, true>::RefVal(const T& t)
            : rv(std::make_unique<Val<T, LateParamsT, true>>(t))
        {
        }
        template<class T, class LateParamsT>
        RefVal<T, LateParamsT, true>::RefVal(const std::reference_wrapper<T>& r)
            : rv(std::make_unique<Ref<T, LateParamsT, true>>(r))
        {
        }
        template<class T, class LateParamsT>
        template<size_t I>
        RefVal<T, LateParamsT, true>::RefVal(ph<I, T>&&) //we need to use a different placeholder class here to pass the palceholders's holder type for the static_cast.
            : rv(std::make_unique<Placeholder<I, T, LateParamsT, true>>())
        {
        }
        template<class T, class LateParamsT>
        RefVal<T, LateParamsT, true>::RefVal(const RefVal& rhs)
        {
            rv = rhs.rv->clone();
        }
        template<class T, class LateParamsT>
        RefVal<T, LateParamsT, true>& RefVal<T, LateParamsT, true>::operator=(const RefVal& rhs)
        {
            rv = rhs.rv->clone(); return *this;
        }
        template<class T, class LateParamsT>
        T& RefVal<T, LateParamsT, true>::bind(void* params) {
            return rv->bind(params);
        }
        template<class T, class LateParamsT>
        T& RefVal<T, LateParamsT, true>::get()
        {
            return rv->get();
        }        
        template<class T, class LateParamsT>
        RefVal<T, LateParamsT, false>::RefVal(T&& t)
            : rv(std::make_unique<Val<T, LateParamsT, false>>(std::move(t)))
        {
        }
        template<class T, class LateParamsT>
        template<size_t I>
        RefVal<T, LateParamsT, false>::RefVal(ph<I, T>&&) //we need to use a different placeholder class here to pass the palceholders's holder type for the static_cast.
            : rv(std::make_unique<Placeholder<I, T, LateParamsT, false>>())
        {
        }
        template<class T, class LateParamsT>
        RefVal<T, LateParamsT, false>::RefVal(RefVal&& rhs)
        {
            rv = rhs.rv->transfer();
        }
        template<class T, class LateParamsT>
        RefVal<T, LateParamsT, false>& RefVal<T, LateParamsT, false>::operator=(RefVal&& rhs)
        {
            rv = rhs.rv->transfer(); return *this;
        }
        template<class T, class LateParamsT>
        T&& RefVal<T, LateParamsT, false>::bind(void* params) {
            return std::move(rv->bind(params));
        }
        template<class T, class LateParamsT>
        T&& RefVal<T, LateParamsT, false>::get()
        {
            return std::move(rv->get());
        }
        template<class R, class C, class... ArgT>
        DynamicWrapper<true, R, C, ArgT...>::DynamicWrapper(R(C::* pf)(ArgT...)) : pfunc(pf) {}
        template<class R, class C, class... ArgT>
        template<class O, class... ArgU>
        R DynamicWrapper<true, R, C, ArgT...>::operator()(O* o, ArgU&&... arg) const
        {
            (o->*pfunc)(std::forward<ArgU>(arg)...);
        }
        template<class R, class C, class... ArgT>
        template<class O, class... ArgU>
        R DynamicWrapper<true, R, C, ArgT...>::operator()(O o, ArgU&&... arg) const
        {
            (o.*pfunc)(std::forward<ArgU>(arg)...);
        }
        template<class R, class C, class... ArgT>              
        DynamicWrapper<false, R, C, ArgT...>::DynamicWrapper(R(C::* pf)(ArgT...)) : pfunc(pf) {}
        template<class R, class C, class... ArgT>
        template<class O, class... ArgU>
        R DynamicWrapper<false, R, C, ArgT...>::operator()(O* o, ArgU&&... arg) const
        {

            if (dynamic_cast<C*>(o))
                return (dynamic_cast<C*>(o)->*pfunc)(std::forward<ArgU>(arg)...);
            throw std::runtime_error(std::string("Invalid cast : types + ") + typeid(C).name() + " et " + typeid(O).name()+" are nor polymorphic!");
        }   
        template<class R, class C, class... ArgT>         
        template<class O, class... ArgU>
        R DynamicWrapper<false, R, C, ArgT...>::operator()(O& o, ArgU&&... arg) const
        {

            if (dynamic_cast<C&>(o))
                return (dynamic_cast<C&>(o).*pfunc)(std::forward<ArgU>(arg)...);
            throw std::runtime_error(std::string("Invalid cast : types + ") + typeid(C).name() + " et " + typeid(O).name()+ " are nor polymorphic!");
        }
        template<class R, class... ArgT>
        template<class F>
        DynamicFunction<R(ArgT...)>::DynamicFunction(F&& f)
            requires (!std::is_same_v<std::decay_t<F>, DynamicFunction<R(ArgT...)>>)
        : Base(std::forward<F>(f))
        {
        }
        template<class R, class C, class... ArgT>
        template<class... ArgU>
        DynamicFunction<R(C::*)(ArgT...)>::DynamicFunction(R(C::* pf)(ArgU...))
            : Base(DynamicWrapper<std::is_same<C, ToStore_t<std::remove_reference_t<std::tuple_element_t<0, std::tuple<ArgT...>>>>>::value, R, C, ArgU...>(pf))
        {
        } 
        template<class Tuple>
        Tuple copyOrMoveTuple(Tuple& src)
        {
            Tuple dst;
            copyOrMoveTupleImpl(src, dst, std::make_index_sequence<std::tuple_size<Tuple>::value>{});
            return dst;
        }
        template<class Stored, class LateParamsT, bool Copiable>
        void copyOrMove(RefVal<Stored, LateParamsT, Copiable>& s, RefVal<Stored, LateParamsT, Copiable>& d)
        {
            if constexpr (Copiable) {
                d = s; // copie normale
            }
            else {
                d = RefVal<Stored, LateParamsT, false>(s.get()); // move-out
            }
        }
        template<class Tuple, std::size_t ...I>
        void copyOrMoveTupleImpl(Tuple& src, Tuple& dst, std::index_sequence<I...>)
        {
            (copyOrMove(std::get<I>(src), std::get<I>(dst)), ...);
        }
        template<class R, class... ArgT>
        template<class F, class... ArgU>
        FastDelegateImpl<R, ArgT...>::FastDelegateImpl(F&& f, ArgU&&... arg)
            : func(std::forward<F>(f))
            , param(std::forward<ArgU>(arg)...)
            , tmpParam(std::forward<ArgU>(arg)...)
        {
        }
        template<class R, class... ArgT>
        std::unique_ptr<Delegate<R>> FastDelegateImpl<R, ArgT...>::clone()
        {
            auto d = std::make_unique<FastDelegateImpl>(func);
            d->param = copyOrMoveTuple(param);
            d->tmpParam = copyOrMoveTuple(tmpParam);
            return d;
        }
        template<class R, class... ArgT>
        void FastDelegateImpl<R, ArgT...>::bind(void* params) {
            param = std::move(tmpParam);
            bindParams(params);
        }
        template<class R, class... ArgT>
        R FastDelegateImpl<R, ArgT...>::operator()()
        {
            return call(std::make_index_sequence<sizeof...(ArgT)>());
        }
        template<class R, class... ArgT>
        template<class... ArgU>
        void FastDelegateImpl<R, ArgT...>::setParams(ArgU&&... arg)
        {
            param = std::make_tuple(std::forward<ArgU>(arg)...);
            tmpParam = std::make_tuple(std::forward<ArgU>(arg)...);
        }   
        template<class R, class... ArgT>   
        template <std::size_t I>
        void FastDelegateImpl<R, ArgT...>::bindParams(void* params) requires IsLastRecursion<I, ArgT...> {
            auto&& v = std::get<I>(param).bind(params);            // r�f�rence universelle
            std::get<I>(param) = std::forward<std::tuple_element_t<I, std::tuple<extractTypeFromPh_t<ArgT>...>>>(v);     // copie si lvalue, move si rvalue
        }
        template<class R, class... ArgT>
        template <std::size_t I>
        void FastDelegateImpl<R, ArgT...>::bindParams(void* params) {
            auto&& v = std::get<I>(param).bind(params);            // r�f�rence universelle
            std::get<I>(param) = std::forward<std::tuple_element_t<I, std::tuple<extractTypeFromPh_t<ArgT>...>>>(v);     // copie si lvalue, move si rvalue

            bindParams<I + 1>(params);
        }
        template<class R, class... ArgT>
        template <std::size_t I>
        void FastDelegateImpl<R, ArgT...>::bindParams(void* params) requires IsEmpty<ArgT...> {
        }
        template<class R, class... ArgT>
        template<std::size_t... I>
        R FastDelegateImpl<R, ArgT...>::call(std::index_sequence<I...>)
        {
            return func(std::get<I>(param).get()...);
        }
        template<class R>
        template<class F, class... Arg>
        FastDelegate<R>::FastDelegate(F&& f, Arg... arg) :
            delegate(new
                FastDelegateImpl<R, Arg...>
                (std::forward<F>(f), std::forward<Arg>(arg)...)
            )
        {

        }
        template<class R>
        FastDelegate<R>::FastDelegate(FastDelegate& rhs)
            : delegate(rhs.delegate->clone())
        {
        }
        template<class R>
        FastDelegate<R>::FastDelegate(const FastDelegate& rhs)
            : delegate(rhs.delegate->clone())
        {
        }
        template<class R>        
        FastDelegate<R>& FastDelegate<R>::operator=(const FastDelegate& rhs)
        {
            delegate = rhs.delegate->clone();
            return *this;
        }
        template<class R>
        template <typename... Arg>
        void FastDelegate<R>::bind(Arg&&... arg) {
            void* params = bind_impl(std::index_sequence_for<Arg...>(), std::forward<Arg>(arg)...);
            delegate->bind(params);
            delete params;
        }
        template<class R>
        template<std::size_t... Ints, class... Args>
        void* FastDelegate<R>::bind_impl(std::index_sequence<Ints...>, Args&&... args)
        {
            //Alias to the placeholders's holder's type, we expand the parameter's packs, so the first argument is the type of placeholder 0, and so on.
            using params_t = LateParameters<ph<Ints, ToStore_t<Args>>...>;
            void* params = new params_t{ std::forward<Args>(args)... };
            return params;
        }
        template<class R>
        R FastDelegate<R>::operator()()
        {
            if (delegate)
                return  (*delegate)();              
        }
        template<class R>
        template<class... Arg>
        void FastDelegate<R>::setParams(Arg... arg)
        {
            using DynamicType =
                FastDelegateImpl<R, Arg...>*;
            if (dynamic_cast<DynamicType>(delegate.get()))
                dynamic_cast<DynamicType>(delegate.get())->setParams(std::forward<Arg>(arg)...);
            else
                throw std::runtime_error(std::string("Invalid cast : types + ") + typeid(DynamicType).name() + " et " + typied(*delegate.get()) + " are nor polymorphic!");
        }        
    }
}
