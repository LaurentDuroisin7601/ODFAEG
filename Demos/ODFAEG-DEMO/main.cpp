#include "application.h"
#include "odfaeg/Graphics/graphics.hpp"
#include "odfaeg/Audio/audio.hpp"
#include "odfaeg/Math/math.hpp"
#include "hero.hpp"



#include "odfaeg/Graphics/renderWindow.h"
#include "odfaeg/Graphics/font.h"
#include "odfaeg/Graphics/text.h"
#include "odfaeg/Graphics/sprite.h"
#include "odfaeg/Graphics/rectangleShape.h"
#include "odfaeg/Window/iEvent.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <iostream>
#include <math.h>
#include <functional>

using namespace odfaeg::math;
//using namespace odfaeg::physic;
using namespace odfaeg::graphic;
using namespace odfaeg::window;
using namespace odfaeg::audio;
using namespace sorrok;
/**
        *  \file  fastDelegate.h
        *  \class Parameter
        *  \brief Hold value for a placeholder.
        *  \param i : the placeholder's position.
        *  \param T : the placeholder's type.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<std::size_t i, class T>
        struct Parameter
        {
          T value;
        };
        /**
        *  \file  fastDelegate.h
        *  \class LateParameters
        *  \brief Wrap each placeholders's values using inheritance and variadic template.
        *  \param Placeholders : the placeholders.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class... Placeholders>
        struct LateParameters : Parameter<Placeholders::index, typename Placeholders::type>... {

        };
        /**
        *  \file  fastDelegate.h
        *  \class IRefVal
        *  \brief Interface for the warppers to references, values and pointers.
        *  This class is used store references, pointers and values to pass to the callack's functions.
        *  \param T : the type of the value to wrap.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class T, class LateParamsT, bool isCopiable>
        struct IRefVal {
        };
        template<class T, class LateParamsT>
        struct IRefVal<T, LateParamsT, true> {
            /**\fn
            *  \brief default constructor
            */
            IRefVal()=default;
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
            virtual ~IRefVal(){}

        protected:
            /**\fn IRefVal(const IRefVal&)
            *  \brief constructor, pass the reference to wrap.
            *  \param const IRefVal& : the reference to wrap.
            */
            IRefVal(const IRefVal&){}
            /** \fn IRefVal& operator=(const IRefVal&)
            *   \brief affector.
            *   \param const IRefVal& : the wrapper to affect.
            *   \return the affected wrapper.*/
            IRefVal& operator=(const IRefVal&)
            { return *this; }
        };
        template<class T, class LateParamsT>
        struct IRefVal<T, LateParamsT, false> {
            /**\fn
            *  \brief default constructor
            */
            IRefVal()=default;
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
            virtual ~IRefVal(){}

        protected:
            /**\fn IRefVal(const IRefVal&)
            *  \brief constructor, pass the reference to wrap.
            *  \param const IRefVal& : the reference to wrap.
            */
            IRefVal(const IRefVal&&){}
            /** \fn IRefVal& operator=(const IRefVal&)
            *   \brief affector.
            *   \param const IRefVal& : the wrapper to affect.
            *   \return the affected wrapper.*/
            IRefVal& operator=(const IRefVal&&)
            { return *this; }
        private :
            /**\fn IRefVal(const IRefVal&)
            *  \brief constructor, pass the reference to wrap.
            *  \param const IRefVal& : the reference to wrap.
            */
            IRefVal(const IRefVal&){}
            /** \fn IRefVal& operator=(const IRefVal&)
            *   \brief affector.
            *   \param const IRefVal& : the wrapper to affect.
            *   \return the affected wrapper.*/
            IRefVal& operator=(const IRefVal&)
            { return std::move(*this); }
        };
        /**
        *  \file  fastDelegate.h
        *  \class Ref
        *  \brief Warp a reference. (Use std::reference_wrapper so we can pass a reference with std::ref)
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */

        template<class T, typename LateParamsT, bool isCopiable>
        struct Ref : IRefVal<T, LateParamsT, true> {
            /**
            *\fn Ref(const std::reference_wrapper<T>& r)
            *\brief Constructor : pass an std::reference_wrapper to the wrapper.
            *\param std::reference_wrapper<T>& r : the reference_wrapper.
            */
            Ref(const std::reference_wrapper<T>& r)
                : ref(r)
            {}
            /**
            * \fn T& bind()
            * \brief return a reference to an object.
            * \param void* address of the wrapper to placeholder's values.
            * \return T& the reference to the object.
            */
            T& bind(void* params) {
              return ref.get();
            }
            /**
            * \fn T& get()
            * \brief return a reference to an object.
            * \return T& the reference to the object.
            */
            T& get()
            { return ref.get(); }
            /**
            * \fn std::unique_ptr<IRefVal<T>> clone() const
            * \brief copy the reference wrapper.
            * \return std::unique_ptr<IRefVal<T>> : the cloned wrapper.
            */
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
            /**\fn Val(const T& t)
            *  \brief pass a value to the wrapper.
            *  \param const T& t : the value to pass.
            */
            Val(const T& t)
                : val(t)
            {}
            /**
            * \fn T& bind()
            * \brief return a the value.
            * \param void* address of the wrapper to placeholder's values.
            * \return T& the value.
            */
            T& bind(void* params) {
              return val;
            }
            /** \fn
            *   \brief return the value
            *   \return T& : return the value.
            */
            T& get()
            { return val; }

            /**
            * \fn std::unique_ptr<IRefVal<T>> clone() const
            * \brief copy the value wrapper.
            * \return std::unique_ptr<IRefVal<T>> : the cloned wrapper.
            */
            std::unique_ptr<IRefVal<T, LateParamsT, true>> clone()
            { return std::make_unique<Val>(*this); }
        private:
            T val; /**> T val : keep the value of the wrapper.*/
        };
        template<class T, class LateParamsT>
        struct Val<T, LateParamsT, false> : IRefVal<T, LateParamsT, false> {
            /**\fn Val(const T& t)
            *  \brief pass a value to the wrapper.
            *  \param const T& t : the value to pass.
            */
            Val(T&& t)
                : val(std::move(t))
            {}
            Val(Val&& v) : val(std::move(v.val)) {
            }
            Val& operator=(Val&& v) {
                val = std::move(v.val);
                return *this;
            }
            /**
            * \fn T& bind()
            * \brief return a the value.
            * \param void* address of the wrapper to placeholder's values.
            * \return T& the value.
            */
            T&& bind(void* params) {
              return std::move(val);
            }
            /** \fn
            *   \brief return the value
            *   \return T& : return the value.
            */
            //template<class = typename std::enable_if_t<std::is_copy_constructible_v<T>>>
            T&& get()
            { return std::move(val); }
            /*template<class...D, class = typename std::enable_if_t<!std::is_copy_constructible_v<T>>>
            T& get()
            { return std::move(val); }*/
            /**
            * \fn std::unique_ptr<IRefVal<T>> clone() const
            * \brief copy the value wrapper.
            * \return std::unique_ptr<IRefVal<T>> : the cloned wrapper.
            */
            std::unique_ptr<IRefVal<T, LateParamsT, false>> transfer()
            { return std::make_unique<Val>(std::move(*this)); }
        private:
            Val(Val& v) : val(std::move(v.val)) {
            }
            Val& operator=(Val& v) {
                val = std::move(v.val);
                return *this;
            }
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
          using f = std::bool_constant<PlaceHolder1::index < PlaceHolder2::index>; /**> if the first placeholder position is smaller than the second placeholder's position.*/
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
        struct placeholder : IRefVal<T, LateParamsT, isCopiable> {
        };
        template<size_t I, class T, class LateParamsT>
        struct placeholder<I, T, LateParamsT, true> : IRefVal<T, LateParamsT, true>
        {
            /**
            * \fn std::unique_ptr<IRefVal<T, LateParamsT>> clone() const
            * \brief copy the placeholder wrapper.
            * \return std::unique_ptr<IRefVal<T, LateParamsT>> : the cloned wrapper.
            */
            std::unique_ptr<IRefVal<T, LateParamsT, true>> clone()
            { return std::make_unique<placeholder>(*this); }
            /**
            * \fn T& bind(void* params)
            * \brief extract the placeholder's value from the placeholers's holder.
            * \param void* address of the wrapper to placeholder's values.
            * \return T& the value.
            */
            T& bind(void* params) {
                //We cast from void* to the placeholders's holder type.
                LateParamsT& paramsT = *static_cast<LateParamsT*>(params);
                //We cast from base type to the derived type to extract the placeholder's value.
                return static_cast<Parameter<I, T>&>(paramsT).value;
            }
            /** \fn T& get()
            *   \brief we don't need to return anything, this function is never called anyway.
            *   \return T& : return's nothing.
            */
            T& get()
            {

            }
        };
        template<size_t I, class T, class LateParamsT>
        struct placeholder<I, T, LateParamsT, false> : IRefVal<T, LateParamsT, false>
        {
            /**
            * \fn std::unique_ptr<IRefVal<T, LateParamsT>> clone() const
            * \brief copy the placeholder wrapper.
            * \return std::unique_ptr<IRefVal<T, LateParamsT>> : the cloned wrapper.
            */
            placeholder(placeholder&& ph) {}
            placeholder& operator=(placeholder&& ph) {
                return *this;
            }
            std::unique_ptr<IRefVal<T, LateParamsT, false>> transfer()
            { return std::make_unique<placeholder>(std::move(*this)); }
            /**
            * \fn T& bind(void* params)
            * \brief extract the placeholder's value from the placeholers's holder.
            * \param void* address of the wrapper to placeholder's values.
            * \return T& the value.
            */
            T&& bind(void* params) {
                //We cast from void* to the placeholders's holder type.
                LateParamsT& paramsT = *static_cast<LateParamsT*>(params);
                //We cast from base type to the derived type to extract the placeholder's value.
                return std::move(static_cast<Parameter<I, T>&>(paramsT).value);
            }
            /** \fn T& get()
            *   \brief we don't need to return anything, this function is never called anyway.
            *   \return T& : return's nothing.
            */
            T&& get()
            {

            }
        private :
            placeholder(placeholder& ph) {}
            placeholder& operator=(placeholder& ph) {
                return *this;
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
            /**
            * \fn RefVal (const T& t)
            * \brief constructor : construct a wrapper to a value
            * \param const T& t : the value to
            */
            RefVal(const T& t)
            : rv(std::make_unique<Val<T, LateParamsT, true>>(t))
            {}
            /**
            * \fn RefVal (const std::reference_wrapper<T>& r)
            * \brief constructor : construct a wrapper to an std::reference_wrapper.
            * \param const std::reference_wrapper<T>& : the std::reference_wrapper to pass.
            */
            RefVal(const std::reference_wrapper<T>& r)
            : rv(std::make_unique<Ref<T, LateParamsT, true>>(r))
            {}
            /**
            * \fn RefVal (ph<I,T>&&)
            * \brief constructor : construct a wrapper to a placeholder from a user defined placeholder.
            * \param ph<I,T>&& the placeholder.
            */
            template<size_t I>
            RefVal(ph<I,T>&&) //we need to use a different placeholder class here to pass the palceholders's holder type for the static_cast.
                : rv(std::make_unique<placeholder<I,T,LateParamsT, true>>())
            {}
            /** \fn RefVal (const RefVal& rhs)
            *   \brief copy constructor, copy the wrapper with the right wrapper type.
            *   \param const RefVal& rhs : the wrapper to copy.
            */
            RefVal(const RefVal& rhs)
            {
                rv = rhs.rv->clone();
            }
            /** \fn RefVal& operator= (const RefVal& rhs)
            *   \brief affector.
            *   \param const RefVal& rhs : the wrapper to affect.
            */
            RefVal& operator=(const RefVal& rhs)
            { rv=rhs.rv->clone(); return *this; }
            /** \fn T& get() const
            *   \brief get the wrapper.
            *   \return a unique pointer to the generic wrapper interface.
            */
            T& bind(void* params) {
                return rv->bind(params);
            }
            T& get()
            { return rv->get(); }
        private:
            std::unique_ptr<IRefVal<T, LateParamsT, true>> rv; /**> a pointer to the generic wrapper interface.*/
        };
        template<class T, class LateParamsT>
        struct RefVal<T, LateParamsT, false> {
            RefVal() = default;
            /**
            * \fn RefVal (const T& t)
            * \brief constructor : construct a wrapper to a value
            * \param const T& t : the value to
            */
            RefVal(T&& t)
            : rv(std::make_unique<Val<T, LateParamsT, false>>(std::move(t)))
            {}
            /**
            * \fn RefVal (ph<I,T>&&)
            * \brief constructor : construct a wrapper to a placeholder from a user defined placeholder.
            * \param ph<I,T>&& the placeholder.
            */
            template<size_t I>
            RefVal(ph<I,T>&&) //we need to use a different placeholder class here to pass the palceholders's holder type for the static_cast.
                : rv(std::make_unique<placeholder<I,T,LateParamsT, false>>())
            {}
            /** \fn RefVal (const RefVal& rhs)
            *   \brief copy constructor, copy the wrapper with the right wrapper type.
            *   \param const RefVal& rhs : the wrapper to copy.
            */
            RefVal(RefVal&& rhs)
            {
                rv = rhs.rv->transfer();
            }
            /** \fn RefVal& operator= (const RefVal& rhs)
            *   \brief affector.
            *   \param const RefVal& rhs : the wrapper to affect.
            */
            RefVal& operator=(RefVal&& rhs)
            { rv=rhs.rv->transfer(); return *this; }
            /** \fn T& get() const
            *   \brief get the wrapper.
            *   \return a unique pointer to the generic wrapper interface.
            */
            T&& bind(void* params) {
                return std::move(rv->bind(params));
            }
            T&& get()
            { return std::move(rv->get()); }
        private:
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
        {};
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
        {};
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
        { using type = T; };
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
        { using type = T; };
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
        struct ToStoreImpl<ph<I,T>>
        { using type = T; };
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
        {};
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
        // --- Sélecteur pour choisir la bonne RefVal ---
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
        //Classe de trait pour déterminer le type à stocker
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
        template<class R, class C,class... ArgT>
        struct DynamicWrapper<true, R, C, ArgT...> {
            /**\fn DynamicWrapper(R(C::*pf)(ArgT...)) : pfunc(pf)
            *  \brief warp a pointer to a member function.
            *  \param R(C::*pf)(ArgT...) : the pointer to the member function.
            */
            DynamicWrapper(R(C::*pf)(ArgT...)) : pfunc(pf){}
            template<class O, class... ArgU>
            R operator()(O* o, ArgU&&... arg) const
            {
                ////////std::cout<<"class : "<<typeid(C).name()<<std::endl<<"object derived : "<<typeid(o).name()<<std::endl<<"address : "<<o<<std::endl;
                (o->*pfunc)(std::forward<ArgU>(arg)...);
            }
            template<class O, class... ArgU>
            R operator()(O o, ArgU&&... arg) const
            {
                ////////std::cout<<"class : "<<typeid(C).name()<<std::endl<<"object derived : "<<typeid(o).name()<<std::endl<<"address : "<<o<<std::endl;
                (o.*pfunc)(std::forward<ArgU>(arg)...);
            }

            /**\fn R operator()(O o, ArgU&&... arg) const
            *  \brief call the member function's pointer with he given object and the given argument
            *  \param O o : the object onwich to call the member function's pointer.
            *  \param ArgU&& arg : the arguments of the member function's pointer to call.
            */
            /*template<class O, class... ArgU>
            R operator()(O o, ArgU&&... arg) const {
                return (o.*pfunc)(std::forward<ArgU>(arg)...);
            }*/
        private:
            R (C::*pfunc)(ArgT...); /**> a pointer to a member's function.*/
        };
        template<class R, class C, class... ArgT>
        struct DynamicWrapper<false, R, C, ArgT...> {
            /**\fn DynamicWrapper(R(C::*pf)(ArgT...)) : pfunc(pf)
            *  \brief warp a pointer to a member function.
            *  \param R(C::*pf)(ArgT...) : the pointer to the member function.
            */
            DynamicWrapper(R(C::*pf)(ArgT...)) : pfunc(pf){}
            /**\fn R operator()(O* o, ArgU&&... arg) const
            *  \brief call the member function's pointer witht he given object and the given argument
            *  apply a dynamic_cast in case of the type of the object and the type of the classe containing the member function
            *  are not the same. (If the object is polymoprhic)
            *  If the cast fails it means that the object is not polymorphic so it throws an error.
            *  \param O* o : the object onwich to call the member function's pointer.
            *  \param ArgU&& arg : the arguments of the member function's pointer to call.
            */
            template<class O, class... ArgU>
            R operator()(O* o, ArgU&&... arg) const
            {
                ////////std::cout<<"class : "<<typeid(C).name()<<std::endl<<"object derived : "<<typeid(o).name()<<std::endl<<"address : "<<o<<std::endl;
                if(dynamic_cast<C*>(o))
                    return (dynamic_cast<C*>(o)->*pfunc)(std::forward<ArgU>(arg)...);
                throw odfaeg::core::Erreur(0, "Invalid cast : types are nor polymorphic!", 1);
            }
            template<class O, class... ArgU>
            R operator()(O& o, ArgU&&... arg) const
            {
                ////////std::cout<<"class : "<<typeid(C).name()<<std::endl<<"object derived : "<<typeid(o).name()<<std::endl<<"address : "<<o<<std::endl;
                if(dynamic_cast<C&>(o))
                    return (dynamic_cast<C&>(o).*pfunc)(std::forward<ArgU>(arg)...);
                throw odfaeg::core::Erreur(0, "Invalid cast : types are nor polymorphic!", 1);
            }
            R (C::*pfunc)(ArgT...);
        };
        /**
        *  \file  fastDelegate.h
        *  \class DynamicFunction
        *  \param F the type of the function.
        *  \brief Generic class for every functors type.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        template<class F>
        class DynamicFunction;
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
        class DynamicFunction<R(ArgT...)>
            : std::function<R(ArgT...)>
        {
            /**> just an alias to the type of the base class.*/
            using Base = std::function<R(ArgT...)>;

        public:
            /**
            * \fn DynamicFunction(F&& f)
            * \brief pass a functor to the std::function.
            * \param F&& f : the functor to pass to the std::function.
            */
            template<class F>
            DynamicFunction(F&& f) : Base(std::forward<F>(f))
            {}
            /** \fn DynamicFunction (R (C::*pf)(ArgU...))
            *   \brief pass a pointer to a member's function to the DynamicWrapper, and then
            *   pass this wrapper to the std::function, so, the std::function call the operator() of the DynamicWrapper class
            *   and not the operator() of the std::function class so we can perform the dynamic_cast if necessary.
            *   \param R(C::*pf)(ArgU...) : the pointer to the member's function.
            *   \
            */
            template<class C, class... ArgU>
            DynamicFunction(R(C::*pf)(ArgU...))
                : Base(DynamicWrapper<std::is_same<C, ToStore_t<std::remove_reference_t<std::tuple_element_t<0, std::tuple<ArgT...>>>>>::value, R,C, ArgU...>(pf))
            {}
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
            Delegate() =default;
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
            virtual ~Delegate(){}

        protected:
            /**
            * \fn Delegate (const Delegate&) {}
            * \brief copy constructor.
            * \param const Delegate& : the delegate to copy.
            */
            Delegate(const Delegate&){}
            /**
            * \fn Delegate& operator= (const Delegate&) {}
            * \brief affector.
            * \return Delegate& : return a reference to the current delegate.
            */
            Delegate& operator=(const Delegate&)
            {
                return *this;
            }
        };
        /**
        *  \file  fastDelegate.h
        *  \class FastDelegateImpl
        *  \brief Implementation of the delegate's interfaces.
        *  \author Duroisin.L
        *  \version 1.0
        *  \date 1/02/2014
        */
        // Copie ou move un tuple élément par élément
        template<class Tuple>
        Tuple copyOrMoveTuple(Tuple& src) {
            Tuple dst;
            copyOrMoveTupleImpl(src, dst, std::make_index_sequence<std::tuple_size<Tuple>::value>{});
            return dst;
        }
        // Copie si RefVal est copiable, sinon move
        template<class Stored, class LateParamsT, bool Copiable>
        void copyOrMove(RefVal<Stored, LateParamsT, Copiable>& s,
                        RefVal<Stored, LateParamsT, Copiable>& d) {
            if constexpr (Copiable) {
                d = s; // copie normale
            } else {
                d = RefVal<Stored, LateParamsT, false>(s.get()); // move-out
            }
        }

        template<class Tuple, std::size_t... I>
        void copyOrMoveTupleImpl(Tuple& src, Tuple& dst, std::index_sequence<I...>) {
            // expansion fold: applique copyOrMove sur chaque élément
            (copyOrMove(std::get<I>(src), std::get<I>(dst)), ...);
        }


        template<class R, class... ArgT>
        struct FastDelegateImpl : Delegate<R> {
            /** \fn FastDelegateImpl(F&& f, ArgU&&... arg)
            *   \brief constructor : create the delegate with the functor and the arguments value passed.
            */
            template<class F, class... ArgU>
            FastDelegateImpl(F&& f, ArgU&&... arg)
                : func(std::forward<F>(f))
                , param(std::forward<ArgU>(arg)...)
                , tmpParam(std::forward<ArgU>(arg)...)
            {}


            /** \fn std::unique_ptr<Delegate<R>> clone() const
            *   \brief make a copy of the delegate.
            *   \return the copied delegate.
            */
            std::unique_ptr<Delegate<R>> clone()
            {

                auto d = std::make_unique<FastDelegateImpl>(func);
                d->param    = copyOrMoveTuple(param);
                d->tmpParam = copyOrMoveTuple(tmpParam);
                return d;
            }

            /** \fn void bind(void* params)
            *   \brief bind each placeholders's parameters's values depending on the placeholders's positions and store them into the tuple.
            *   \param the address of the placeholders's holder.
            */
            void bind(void* params) {
              param = std::move(tmpParam);
              bindParams(params);
            }
            /** \fn R operator()()
            *   \brief changed the parameter's values of the std::function.
            *   \param ArgU&&... args : the values for the parameters to the delegate's function.
            */
            R operator()()
            { return call(std::make_index_sequence<sizeof...(ArgT)>()); }
            /** \fn setParams(ArgU&&... arg)
            *   \brief change parameters values of the delegate's function.
            *   \param ArgU&&... args : the values for the parameters to the delegate's function.
            */
            template<class... ArgU>
            void setParams(ArgU&&... arg)
            {
                param=std::make_tuple(std::forward<ArgU>(arg)...);
                tmpParam=std::make_tuple(std::forward<ArgU>(arg)...);
            }

        private:
            /**
            * \fn bindParams(void* params)
            * \brief final case, we stop recursion when each placeholder's parameters are binded.
            * \param the address of the placeholders's holder.
            */
            template <std::size_t I = 0>
            typename std::enable_if<I == sizeof...(ArgT)>::type
            bindParams(void* params) {

            }
            /**
            * \fn bindParams(void* params)
            * \brief intermediate case, we do recursion for each tuple elements to bind the placeholders's parameters's values.
            * \param the address of the placeholders's holder.
            */
            template <std::size_t I = 0>
            typename std::enable_if<(I < sizeof...(ArgT))>::type
            bindParams(void* params) {
                auto&& v = std::get<I>(param).bind(params);            // référence universelle
                std::get<I>(param) = std::forward<std::tuple_element_t<I, std::tuple<ArgT...>>>(v);     // copie si lvalue, move si rvalue

                bindParams<I+1>(params);
            }
            /** \fn R call(std::index_sequence<I...>)
            *   \brief pass each elements of the tuple to the std::function and unwarp them.
            *   \param std::index_sequence<I...> : a sequence of indexes to get every elements of the tuple.
            *   \return the value returned by the std::function.
            */
            template<std::size_t... I>
            R call(std::index_sequence<I...>)
            {
                return func(std::get<I>(param).get()...);
            }
            DynamicFunction<R(ArgT...)> func; /**> a functor whith hold the pointer to a callback function.*/
            /**
            * We need to remove every types which are not placeholders, we also need to remove same placeholder's types,
            * and finally we also need to sort them.
            * We also remove cv qualifier's and we pass the placeholders's types to the LateParameters class.
            */
            using tuple_t = typename odfaeg::core::meta::sort<LessPlaceceholder, typename odfaeg::core::meta::unique<typename odfaeg::core::meta::copy_if<is_placeholder, std::tuple<std::remove_cv_t<ArgT>...>>::f>::f>::f;
            using late_params_t = typename odfaeg::core::meta::lift<tuple_t, LateParameters, std::make_index_sequence<std::tuple_size<tuple_t>::value>>::f;    /**> alias to the placeholders's holder.*/
            //We need to use two tuples here, one to store parameters values and another to store parameters values after the placeholders's values are binded.
            //jln lib doesn't compile on all compilers so I created my own lib for meta functions.
            /*using late_params_t
      = jln::mp::copy_if<jln::mp::lift<is_placeholder>,
                    jln::mp::unique<jln::mp::sort<LessPlaceceholder,
                                        jln::mp::lift<LateParameters>>>>
        ::f<std::remove_cv_t<ArgT>...>;*/
            std::tuple<refval_t<ArgT, late_params_t, std::is_copy_constructible_v<ArgT> && std::is_copy_assignable_v<ArgT>>...> param; /**> the wrapped values of the parameters to pass to the callback's function.*/
            std::tuple<refval_t<ArgT, late_params_t, std::is_copy_constructible_v<ArgT> && std::is_copy_assignable_v<ArgT>>...> tmpParam; /**> the wrapped values of the temporary parameters (parameters with placeholder's types).*/
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
            /**\fn default constructor
            */
            FastDelegate() {
                delegate = nullptr;
            };
            /**\fn FastDelegate (F&& f, Arg&& arf)
            *  \param F&& f : the functor to pass.
            *  \param Arg&&... arg : arguments to pass to the functor.
            */
            template<class F, class... Arg>
            FastDelegate(F&& f, Arg&&... arg) :
                delegate(std::make_unique
                    <FastDelegateImpl<R, Arg...>>
                    (std::forward<F>(f),std::forward<Arg>(arg)...)
                )
            {}
            /**\fn FastDelegate (FastDelegate& rhs)
            *  \brief copy constructor.
            *  \param FastDelegate& rhs : the delegate to copy.
            */
            FastDelegate(FastDelegate& rhs)
                : delegate(rhs.delegate->clone())
            {}
            /**\fn FastDelegate (const FastDelegate& rhs)
            *  \brief copy constructor.
            *  \param const FastDelegate& rhs : the delegate to copy.
            */
            FastDelegate(const FastDelegate& rhs)
                : delegate(rhs.delegate->clone())
            {}
            /**\fn FastDelegate (FastDelegate& rhs)
            *  \brief move constructor.
            *  \param FastDelegate&& rhs : the delegate to move.
            */
            FastDelegate(FastDelegate&& rhs) =default;
            /**\fn FastDelegate& operator= (FastDelegate& rhs)
            *  \brief affect the content of the delegate to the delegate.
            *  \param const FastDelegate& rhs : the delegate affect.
            *  \return FastDelegate& : the affected delegate.
            */
            FastDelegate& operator=(const FastDelegate& rhs)
            { return operator=(FastDelegate(rhs)); }
            /**\fn FastDelegate& operator= (FastDelegate& rhs)
            *  \brief affect and move the content of the delegate to the delegate.
            *  \param const FastDelegate& rhs : the delegate to move and to affect.
            *  \return FastDelegate& : the moved and affected delegate.
            */
            FastDelegate& operator=(FastDelegate&&) =default;
            /**\fn void bind(Arg&&... arg)
            *  \brief bind each arguments for placeholders.
            *  \param the arguments the bind.
            */
            template <typename... Arg>
            void bind(Arg&&... arg)  {
                 void* params = bind_impl(std::index_sequence_for<Arg...>(), std::forward<Arg>(arg)...);
                 delegate->bind(params);
                 delete params;
            }
            /**\fn void bind_impl(std::index_sequence<Ints...>, Args&&... args)
            *  \brief bind each arguments for placeholders.
            *  \param std::index_sequence<Ints...> index sequence to pass each arguments.
            *  \param the arguments the bind.
            *  \return pointer to the placeholders's holder's type.
            */
            template<std::size_t... Ints, class... Args>
            void* bind_impl(std::index_sequence<Ints...>, Args&&... args)
            {
                //Alias to the placeholders's holder's type, we expand the parameter's packs, so the first argument is the type of placeholder 0, and so on.
                using params_t = LateParameters<ph<Ints, ToStore_t<Args>>...>;
                void* params = new params_t{std::forward<Args>(args)...};
                return params;
            }
            /**\fn R operator()() const;
            *  \brief call the std::function of the delegate.
            *  \return the value returned by the std::function.
            */
            R operator()()
            {
                if (delegate)
                    return  (*delegate)();
            }
            /**\fn setParams(Arg&&... arg)
            *  \brief change the parameter's values of the delegate.
            *  \return Arg&&... arg : the values of the parameters of the delegate.
            */
            template<class... Arg>
            void setParams(Arg... arg)
            {
                using DynamicType =
                    FastDelegateImpl<R, Arg...>*;
                if(dynamic_cast<DynamicType>(delegate.get()))
                    dynamic_cast<DynamicType>(delegate.get())->setParams(std::forward<Arg>(arg)...);
                else
                    throw odfaeg::core::Erreur(0, "Bad cast!", 5);
            }
        private:
            std::unique_ptr<Delegate<R>> delegate; /**> holds the pointer to the generic delegate.*/
        };
        void f(std::unique_ptr<int> prt) {
        }
int main(int argc, char *argv[]) {
    /*std::unique_ptr<int> ptr = std::make_unique<int>(5);
    FastDelegate<void> fd(&f, std::move(ptr));*/


    /*VkSettup instance;
    Device device(instance);

    RenderWindow window(VideoMode(800, 600), "test", device, Style::Default, ContextSettings(0, 0, 4, 4, 6));
    //RenderComponentManager rcm(window);
    Texture texture(device);
    texture.loadFromFile("tilesets/eau.png");
    Sprite sprite(texture, Vec3f(0, 0, 0), Vec3f(100, 50, 0), IntRect(0, 0, 100, 50));
    Sprite sprite2(texture, Vec3f(100, 50, 0), Vec3f(100, 50, 0), IntRect(0, 0, 100, 50));
    window.createDescriptorsAndPipelines();
    CircleShape circle(50);
    window.setPosition(Vector2i(10, 50));
    window.setSize(Vector2u(640, 480));
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(30);
    while(window.isOpen()) {
        window.clear(Color::Black);
        window.draw(sprite);

        window.draw(sprite2);
        window.submit(true);
        window.display();
        odfaeg::window::IEvent event;
        while (window.pollEvent(event))
        {
            // évènement "fermeture demandée" : on ferme la fenêtre
            if (event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED)
                window.close();


        }

    }*/


        //std::cout<<"size : "<<sizeof(std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::duration<float>>)<<std::endl;

    MyAppli app(VideoMode(800, 600), "Test odfaeg");
    return app.exec();
}




