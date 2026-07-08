module;
#include <tuple>
#include <functional>
#include <stdexcept>
#include <cassert>
#include <memory>
#include <string>
export module odfaeg.core.delegate;
import odfaeg.core.metaprog;
export namespace odfaeg {
    namespace core {
        template<std::size_t i, class T>
        struct Parameter
        {
            T value;
        };
        template<class... Placeholders>
        struct LateParameters : Parameter<Placeholders::index, typename Placeholders::type>... {

        };
        
        template<class T, class LateParamsT, bool isCopiable>
        struct IRefVal {
           
        };
        template<class T, class LateParamsT>
        struct IRefVal<T, LateParamsT, true> {
            /**\fn
            *  \brief default constructor
            */
            IRefVal() = default;
            /**\fn T& get()
            *  \brief get the reference of the wrapped type.
            *  \return the reference of the wrapped type.
            */
            virtual T& bind(void* params) = 0;
            virtual T& get() = 0;
            /**\fn std::unique_ptr<IRefVal<T>> clone() const = 0;
            *  \brief copy the wrapper.
            *  \return the copied wrapper.*/
            virtual std::unique_ptr<IRefVal<T, LateParamsT, true>> clone() = 0;
            /**\fn destructor*/
            virtual ~IRefVal() {}
        };
        template<class T, class LateParamsT>
        struct IRefVal<T, LateParamsT, false> {
            /**\fn
            *  \brief default constructor
            */
            IRefVal() = default;
            /**\fn T& get()
            *  \brief get the reference of the wrapped type.
            *  \return the reference of the wrapped type.
            */
            virtual T&& bind(void* params) = 0;
            virtual T&& get() = 0;
            /**\fn std::unique_ptr<IRefVal<T>> clone() const = 0;
            *  \brief copy the wrapper.
            *  \return the copied wrapper.*/
            virtual std::unique_ptr<IRefVal<T, LateParamsT, false>> transfer() = 0;
            /**\fn destructor*/
            virtual ~IRefVal() {}   
            IRefVal(const IRefVal&) = delete;
            IRefVal& operator=(const IRefVal&) = delete;            
        };
        template<class T, typename LateParamsT, bool isCopiable>
        struct Ref : IRefVal<T, LateParamsT, true> {            
            Ref(const std::reference_wrapper<T>& r)
                : ref(r)
            {
            }
            
            T& bind(void* params) {
                return ref.get();
            }
            
            T& get()
            {
                return ref.get();
            }
            
            std::unique_ptr<IRefVal<T, LateParamsT, true>> clone()
            {
                return std::make_unique<Ref>(*this);
            }
        private:
            std::reference_wrapper<T> ref; /**> the std::reference_wrapper which warp the reference.*/
        };
        /**
        *  \file  fastDelegate.h
        *  \class Val
        *  \brief Warp a value.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class T, class LateParamsT, bool isCopiable>
        struct Val : IRefVal<T, LateParamsT, isCopiable> {
        };
        template<class T, class LateParamsT>
        struct Val<T, LateParamsT, true> : IRefVal<T, LateParamsT, true> {
            Val(const T& t)
                : val(t)
            {
            }
            T& bind(void* params) {
                return val;
            }
            
            T& get()
            {
                return val;
            }
            std::unique_ptr<IRefVal<T, LateParamsT, true>> clone()
            {
                return std::make_unique<Val>(*this);
            }
        private:
            T val; /**> T val : keep the value of the wrapper.*/
        };
        template<class T, class LateParamsT>
        struct Val<T, LateParamsT, false> : IRefVal<T, LateParamsT, false> {
            Val(const T& t)
                : val(t)
            {
            }            
            
            std::unique_ptr<IRefVal<T, LateParamsT, true>> clone()
            {
                return std::make_unique<Val>(*this);
            }

            Val(T&& t)
                : val(std::move(t))
            {
            }

            Val(Val&& v) : val(std::move(v.val)) {
            }
            Val& operator=(Val&& v) {
                val = std::move(v.val);
                return *this;
            }

            T&& bind(void* params) {
                return std::move(val);
            }

            T&& get()
            {
                return std::move(val);
            }

            std::unique_ptr<IRefVal<T, LateParamsT, false>> transfer()
            {
                return std::make_unique<Val>(std::move(*this));
            }
        private:            
            T val; /**> T val : keep the value of the wrapper.*/
        };
        /**
        *  \file  fastDelegate.h
        *  \class ph
        *  \brief user defined placeholders.
        *  \param I placeholder's position.
        *  \param T placeholder's type.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<size_t I, class T>
        struct ph
        {
            using type = T; /**> alias to the type.*/
            static constexpr std::size_t index = I; /** > alias to the position.*/
        };
        /**
        *  \file  fastDelegate.h
        *  \class LessPlaceceholder
        *  \brief class used to sort placeholder's from the first position to the last type's position.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        struct LessPlaceceholder
        {
            template<class PlaceHolder1, class PlaceHolder2>
            using f = std::bool_constant < PlaceHolder1::index < PlaceHolder2::index>; /**> if the first placeholder position is smaller than the second placeholder's position.*/
        };
        /**
        *  \file  fastDelegate.h
        *  \class placeholder
        *  \brief Warp a placeholder.
        *  \param I : the placeholder's position.
        *  \param T : the placeholder's type.
        *  \param LateParamsT : the type of the placeholders holder. (we need to pass it for the cast!)
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<size_t I, class T, class LateParamsT, bool isCopiable>
        struct Placeholder : IRefVal<T, LateParamsT, isCopiable> {
        };
        template<size_t I, class T, class LateParamsT>
        struct Placeholder<I, T, LateParamsT, true> : IRefVal<T, LateParamsT, true>
        {            
            std::unique_ptr<IRefVal<T, LateParamsT, true>> clone()
            {
                return std::make_unique<Placeholder>(*this);
            }
            
            T& bind(void* params) {
                //We cast from void* to the placeholders's holder type.
                LateParamsT& paramsT = *static_cast<LateParamsT*>(params);
                //We cast from base type to the derived type to extract the placeholder's value.
                return static_cast<Parameter<I, T>&>(paramsT).value;
            }
            
            T& get()
            {

            }
        };
        template<size_t I, class T, class LateParamsT>
        struct Placeholder<I, T, LateParamsT, false> : IRefVal<T, LateParamsT, false>
        {            
            Placeholder(Placeholder&& ph) {}
            
            Placeholder& operator=(Placeholder&& ph) {
                return *this;
            }            
            std::unique_ptr<IRefVal<T, LateParamsT, false>> transfer()
            {
                return std::make_unique<Placeholder>(std::move(*this));
            }
            
            T&& bind(void* params) {
                //We cast from void* to the placeholders's holder type.
                LateParamsT& paramsT = *static_cast<LateParamsT*>(params);
                //We cast from base type to the derived type to extract the placeholder's value.
                return std::move(static_cast<Parameter<I, T>&>(paramsT).value);
            }
            T&& get()
            {

            }
        };
        /**
        *  \file  fastDelegate.h
        *  \class RefVal
        *  \brief Wrap a pointer, a value or a reference and keep a pointer to the generic wrapper.
        *  Call the right constructor depending on the wrapper's or value's type.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class T, class LateParamsT, bool isCopiable>
        struct RefVal {
        };
        template<class T, class LateParamsT>
        struct RefVal<T, LateParamsT, true> {
            RefVal() = default;
            
            RefVal(const T& t)
                : rv(std::make_unique<Val<T, LateParamsT, true>>(t))
            {
            }
            
            RefVal(const std::reference_wrapper<T>& r)
                : rv(std::make_unique<Ref<T, LateParamsT, true>>(r))
            {
            }
            template<size_t I>
            RefVal(ph<I, T>&&) //we need to use a different placeholder class here to pass the palceholders's holder type for the static_cast.
                : rv(std::make_unique<Placeholder<I, T, LateParamsT, true>>())
            {
            }
            
            RefVal(const RefVal& rhs)
            {
                rv = rhs.rv->clone();
            }
            RefVal& operator=(const RefVal& rhs)
            {
                rv = rhs.rv->clone(); return *this;
            }
            T& bind(void* params) {
                return rv->bind(params);
            }
            
            T& get()
            {
                return rv->get();
            }
        private:
            std::unique_ptr<IRefVal<T, LateParamsT, true>> rv; /**> a pointer to the generic wrapper interface.*/
        };
        template<class T, class LateParamsT>
        struct RefVal<T, LateParamsT, false> {
            RefVal() = default;
            
            RefVal(T&& t)
                : rv(std::make_unique<Val<T, LateParamsT, false>>(std::move(t)))
            {
            }
            template<size_t I>
            RefVal(ph<I, T>&&) //we need to use a different placeholder class here to pass the palceholders's holder type for the static_cast.
                : rv(std::make_unique<Placeholder<I, T, LateParamsT, false>>())
            {
            }
            RefVal(RefVal&& rhs)
            {
                rv = rhs.rv->transfer();
            }
            RefVal& operator=(RefVal&& rhs)
            {
                rv = rhs.rv->transfer(); return *this;
            }
            T&& bind(void* params) {
                return std::move(rv->bind(params));
            }
            T&& get()
            {
                return std::move(rv->get());
            }
        private:

            /** \fn RefVal& operator= (const RefVal& rhs)
            *   \brief affector.
            *   \param const RefVal& rhs : the wrapper to affect.
            */
            RefVal(const RefVal& rhs) = delete;
            RefVal& operator=(const RefVal& rhs) = delete;
            std::unique_ptr<IRefVal<T, LateParamsT, false>> rv; /**> a pointer to the generic wrapper interface.*/
        };
        /**
        *  \file  fastDelegate.h
        *  \class is_placeholder
        *  \brief trait class if the type is not a placeholder the compiler choose this class's version.
        *  \param T the type.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class T>
        struct is_placeholder
            : std::false_type
        {
        };
        /**
        *  \file  fastDelegate.h
        *  \class is_placeholder
        *  \brief trait class specialization if the type is a placeholder the compiler choose this class's version.
        *  \param T the type.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<std::size_t I, class T>
        struct is_placeholder<ph<I, T>>
            : std::true_type
        {
        };
        template<typename X> inline constexpr bool is_placeholder_v = is_placeholder<X>::value;
        /**
        *  \file  fastDelegate.h
        *  \class ToStoreImpl
        *  \param T the type of the wrapper.
        *  \brief Trait class which use an alias on a wrapped type.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class T>
        struct ToStoreImpl
        {
            using type = T;
        };
        /**
        *  \file  fastDelegate.h
        *  \class ToStoreImpl
        *  \param T the type warpped in the warpper.
        *  \brief Trait class with keep an alias on the wrapped type.
        *  This class is specialized for std::_reference_wrapper type.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class T>
        struct ToStoreImpl<std::reference_wrapper<T>>
        {
            using type = T;
        };
        /**
        *  \file  fastDelegate.h
        *  \class ToStoreImpl
        *  \param T the type warpped in the warpper.
        *  \brief Trait class with keep an alias on the plcaholder's type.
        *  This class is specialized for placeholder's type.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<size_t I, class T>
        struct ToStoreImpl<ph<I, T>>
        {
            using type = T;
        };
        /**
        *  \file  fastDelegate.h
        *  \class ToStore
        *  \param T the type of the wrapper.
        *  \brief Trait class with keep an alias of the wrapper type. (without the reference)
        *  the inheritance use the right specialized templated class to hold the type of the wrapped object
        *  depending on the wrapper type.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class T>
        struct ToStore
            : ToStoreImpl<std::remove_reference_t<T>>
        {
        };
        /**
        *  \file  fastDelegate.h
        *  \class ToStore_t
        *  \param T the type of the wrapped object.
        *  \brief Trait class which hold an alias to the real type of the wrapped type.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class T>
        using ToStore_t = typename
            ToStore<T>::type;
        template<typename X> struct is_refwrap : std::false_type {};
        template<typename T> struct is_refwrap<std::reference_wrapper<T>> : std::true_type {};
        template<typename X> inline constexpr bool is_refwrap_v = is_refwrap<X>::value;
        // --- S�lecteur pour choisir la bonne RefVal ---
        template<typename ArgT, typename LateParamsT, bool defaultCopiable>
        struct refval_selector {
            using T = ToStore_t<ArgT>; // unwrapped underlying type used inside RefVal

            // Detect reference wrapper on the original ArgT (no decay beyond cvref)
            static constexpr bool isRefWrapper =
                is_refwrap_v<std::remove_cvref_t<ArgT>>;

            // Compute copyability on the underlying type T (after ToStore_t)
            using Base = std::remove_cvref_t<T>;
            static constexpr bool underlying_is_copyable =
                std::is_copy_constructible_v<Base> &&
                std::is_copy_assignable_v<Base>;

            // Never claim copyable if the underlying isn't; references force copyable path
            static constexpr bool isCopiable =
                isRefWrapper
                ? true
                : (defaultCopiable && underlying_is_copyable);

            using type = RefVal<T, LateParamsT, isCopiable>;
        };

        template<typename ArgT, typename LateParamsT, bool defaultCopiable>
        using refval_t = typename refval_selector<ArgT, LateParamsT, defaultCopiable>::type;

        template <typename T>
        struct extractTypeFromPh {
            using type = T;
        };
        template <size_t I, typename T>
        struct extractTypeFromPh<ph<I, T>> {
            using type = T;
        };
        template<class T>
        using extractTypeFromPh_t = typename
            extractTypeFromPh<T>::type;
        //Classe de trait pour d�terminer le type � stocker
        /**
        *  \file  fastDelegate.h
        *  \class DynamicWrapper
        *  \param R the return type of the wrapped functor.
        *  \param C the class type of the wrapped functor.
        *  \param ArgT the arguments types of the wrapped functor.
        *  \brief This class warp a function pointer to a member function.
        *  I don't use an std::function directly here to keep the class type of the member function pointer
        *  because if the passed object is polymorphic, I need to apply a dynamic cast
        *  before calling the member function pointer on the object.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<bool B, class R, class C, class... ArgT>
        struct DynamicWrapper {
        };
        template<class R, class C, class... ArgT>
        struct DynamicWrapper<true, R, C, ArgT...> {            
            DynamicWrapper(R(C::* pf)(ArgT...)) : pfunc(pf) {}
            template<class O, class... ArgU>
            R operator()(O* o, ArgU&&... arg) const
            {
                (o->*pfunc)(std::forward<ArgU>(arg)...);
            }
            template<class O, class... ArgU>
            R operator()(O o, ArgU&&... arg) const
            {
                (o.*pfunc)(std::forward<ArgU>(arg)...);
            }
        private:
            R(C::* pfunc)(ArgT...); /**> a pointer to a member's function.*/
        };
        template<class R, class C, class... ArgT>
        struct DynamicWrapper<false, R, C, ArgT...> {            
            DynamicWrapper(R(C::* pf)(ArgT...)) : pfunc(pf) {}

            
            template<class O, class... ArgU>
            R operator()(O* o, ArgU&&... arg) const
            {

                if (dynamic_cast<C*>(o))
                    return (dynamic_cast<C*>(o)->*pfunc)(std::forward<ArgU>(arg)...);
                throw std::runtime_error(std::string("Invalid cast : types + ") + typeid(C).name() + " et " + typeid(O).name()+" are nor polymorphic!");
            }            
            template<class O, class... ArgU>
            R operator()(O& o, ArgU&&... arg) const
            {

                if (dynamic_cast<C&>(o))
                    return (dynamic_cast<C&>(o).*pfunc)(std::forward<ArgU>(arg)...);
                throw std::runtime_error(std::string("Invalid cast : types + ") + typeid(C).name() + " et " + typeid(O).name()+ " are nor polymorphic!");
            }
        private:
            R(C::* pfunc)(ArgT...);
        };
                
        
        /**
        *  \file  fastDelegate.h
        *  \class DynamicFunction
        *  \param R the return type of the function.
        *  \param ArgT... the type of the arguments of the function.
        *  \brief Specialized template class for functors. (inherit from std::function)
        *  build a functor with the right constructor depending a the pointer's function type.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class R, class... ArgT>
                class DynamicFunction{};
        template<class R, class... ArgT>
        class DynamicFunction<R(ArgT...)>
            : std::function<R(ArgT...)>
        {
            /**> just an alias to the type of the base class.*/
            using Base = std::function<R(ArgT...)>;

        public:
            template<class F>
            DynamicFunction(F&& f)
                requires (!std::is_same_v<std::decay_t<F>, DynamicFunction<R(ArgT...)>>)
            : Base(std::forward<F>(f))
            {
            }
            /**> we use the operator() of the base class.*/
            using Base::operator();
        };
        template<class R, class C, class... ArgT>
        class DynamicFunction<R(C::*)(ArgT...)>
            : std::function<R(ArgT...)>
        {
            using Base = std::function<R(ArgT...)>;
            template<class... ArgU>
            DynamicFunction(R(C::* pf)(ArgU...))
                : Base(DynamicWrapper<std::is_same<C, ToStore_t<std::remove_reference_t<std::tuple_element_t<0, std::tuple<ArgT...>>>>>::value, R, C, ArgU...>(pf))
            {
            }
            /**> we use the operator() of the base class.*/
            using Base::operator();
        };
        /**
        *  \file  fastDelegate.h
        *  \class Delegate
        *  \param R the return type of the function.
        *  \brief Interface with can hold a delegate to every delegates types.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class R>
        struct Delegate {
            /**\fn Delegate()
            *  \brief default constructor.
            */
            Delegate() = default;
            /**\fn virtual std::unique_ptr<Delegate> clone() const = 0;
            *  \brief pure virtual function to redefine in the subclass to copy the delegate.
            *  \return std::unique_ptr<Delegate> a pointer to the copied delegate.
            */
            virtual std::unique_ptr<Delegate> clone() = 0;
            /**\fn void bind(void* params) = 0;
            *  \brief pure virtual function to redefines to bind parameters.
            *  \param void* : address of placeholders's holder.
            */
            virtual void bind(void* params) = 0;
            /**\fn R operator()() = 0;
            *  \brief pure virtual function to redefines to call the std::function of the delegate.
            *  \return R : return the value returned by the std::function.
            */
            virtual R operator()() = 0;
            /** /fn virtual Delegate()
            * \brief destructor
            */
            virtual ~Delegate() {

            }

        protected:
            Delegate(const Delegate&) {}


            Delegate& operator=(const Delegate&) {}
        };
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
        /**
        *  \file  fastDelegate.h
        *  \class FastDelegateImpl
        *  \brief Implementation of the delegate's interfaces.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template <std::size_t I = 0, typename... ArgT>
        concept IsLastRecursion =
            I == sizeof...(ArgT) - 1;
        template <typename... ArgT>
        concept IsEmpty =
            sizeof...(ArgT) == 0;
        template<class R, class... ArgT>
        struct FastDelegateImpl : Delegate<R> {
            template<class F, class... ArgU>
            FastDelegateImpl(F&& f, ArgU&&... arg)
                : func(std::forward<F>(f))
                , param(std::forward<ArgU>(arg)...)
                , tmpParam(std::forward<ArgU>(arg)...)
            {
            }
            std::unique_ptr<Delegate<R>> clone()
            {
                auto d = std::make_unique<FastDelegateImpl>(func);
                d->param = copyOrMoveTuple(param);
                d->tmpParam = copyOrMoveTuple(tmpParam);
                return d;
            }

            void bind(void* params) {
                param = std::move(tmpParam);
                bindParams(params);
            }

            R operator()()
            {
                return call(std::make_index_sequence<sizeof...(ArgT)>());
            }

            template<class... ArgU>
            void setParams(ArgU&&... arg)
            {
                param = std::make_tuple(std::forward<ArgU>(arg)...);
                tmpParam = std::make_tuple(std::forward<ArgU>(arg)...);
            }

        private:
            template <std::size_t I=0>
            void bindParams(void* params) requires IsLastRecursion<I, ArgT...> {
                auto&& v = std::get<I>(param).bind(params);            // r�f�rence universelle
                std::get<I>(param) = std::forward<std::tuple_element_t<I, std::tuple<extractTypeFromPh_t<ArgT>...>>>(v);     // copie si lvalue, move si rvalue
            }

            template <std::size_t I=0>
            void bindParams(void* params) {
                auto&& v = std::get<I>(param).bind(params);            // r�f�rence universelle
                std::get<I>(param) = std::forward<std::tuple_element_t<I, std::tuple<extractTypeFromPh_t<ArgT>...>>>(v);     // copie si lvalue, move si rvalue

                bindParams<I + 1>(params);
            }

            template <std::size_t I=0>
            void bindParams(void* params) requires IsEmpty<ArgT...> {
            }

            template<std::size_t... I>
            R call(std::index_sequence<I...>)
            {
                return func(std::get<I>(param).get()...);
            }
            DynamicFunction<R(extractTypeFromPh_t<ArgT>...)> func; /**> a functor whith hold the pointer to a callback function.*/
            /**
            * We need to remove every types which are not placeholders, we also need to remove same placeholder's types,
            * and finally we also need to sort them.
            * We also remove cv qualifier's and we pass the placeholders's types to the LateParameters class.
            */
            using tuple_t = typename meta::sort<LessPlaceceholder, typename meta::unique<typename meta::copy_if<is_placeholder, std::tuple<std::remove_cv_t<ArgT>...>>::f>::f>::f;
            using late_params_t = typename meta::lift<tuple_t, LateParameters, std::make_index_sequence<std::tuple_size<tuple_t>::value>>::f;    /**> alias to the placeholders's holder.*/
            //We need to use two tuples here, one to store parameters values and another to store parameters values after the placeholders's values are binded.
            //jln lib doesn't compile on all compilers so I created my own lib for meta functions.
            /*using late_params_t
      = jln::mp::copy_if<jln::mp::lift<is_placeholder>,
                    jln::mp::unique<jln::mp::sort<LessPlaceceholder,
                                        jln::mp::lift<LateParameters>>>>
        ::f<std::remove_cv_t<ArgT>...>;*/
            std::tuple<refval_t<ArgT, late_params_t, std::is_copy_constructible_v<ArgT>&& std::is_copy_assignable_v<ArgT>>...> param; /**> the wrapped values of the parameters to pass to the callback's function.*/
            std::tuple<refval_t<ArgT, late_params_t, std::is_copy_constructible_v<ArgT>&& std::is_copy_assignable_v<ArgT>>...> tmpParam; /**> the wrapped values of the temporary parameters (parameters with placeholder's types).*/
        };
        /**
        *  \file  fastDelegate.h
        *  \class FastDelegate
        *  \brief Class used for the type erasure,
        *  which allow the user be able to store a set of different callback's functions types with the same return type.
        * I use this class essentially to connect signals and slots.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class R>
        struct FastDelegate {            
            template<class F, class... Arg>
            FastDelegate(F&& f, Arg... arg) :
                delegate(new
                    FastDelegateImpl<R, Arg...>
                    (std::forward<F>(f), std::forward<Arg>(arg)...)
                )
            {

            }
            FastDelegate(FastDelegate& rhs)
                : delegate(rhs.delegate->clone())
            {
            }
            
            FastDelegate(const FastDelegate& rhs)
                : delegate(rhs.delegate->clone())
            {
            }

            
            FastDelegate& operator=(const FastDelegate& rhs)
            {
                delegate = rhs.delegate->clone();
                return *this;
            }
            
            template <typename... Arg>
            void bind(Arg&&... arg) {
                void* params = bind_impl(std::index_sequence_for<Arg...>(), std::forward<Arg>(arg)...);
                delegate->bind(params);
                delete params;
            }
            
            template<std::size_t... Ints, class... Args>
            void* bind_impl(std::index_sequence<Ints...>, Args&&... args)
            {
                //Alias to the placeholders's holder's type, we expand the parameter's packs, so the first argument is the type of placeholder 0, and so on.
                using params_t = LateParameters<ph<Ints, ToStore_t<Args>>...>;
                void* params = new params_t{ std::forward<Args>(args)... };
                return params;
            }
            
            R operator()()
            {
                if (delegate)
                    return  (*delegate)();              
            }
            
            template<class... Arg>
            void setParams(Arg... arg)
            {
                using DynamicType =
                    FastDelegateImpl<R, Arg...>*;
                if (dynamic_cast<DynamicType>(delegate.get()))
                    dynamic_cast<DynamicType>(delegate.get())->setParams(std::forward<Arg>(arg)...);
                else
                    throw std::runtime_error(std::string("Invalid cast : types + ") + typeid(DynamicType).name() + " et " + typied(*delegate.get()) + " are nor polymorphic!");
            }
        private:
            std::unique_ptr<Delegate<R>> delegate; /**> holds the pointer to the generic delegate.*/
        };        
        
    }
}
