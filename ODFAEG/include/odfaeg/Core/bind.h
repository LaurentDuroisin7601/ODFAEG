#ifndef BIND_H
#define BIND_H

#include "function.h"
#include "placeholders.h"
#include <tuple>
#include <set>
namespace odfaeg {

    template <typename R, typename...V> class Bind {
    };

    template <typename R, typename... A, typename... V> class Bind <R(A...), V...> {
        public :

        Bind (R(*f)(A...), V&&... v) : function(f) {
            values = std::forward_as_tuple(v...);
        }

        template<typename... P2>
        R operator ()(P2&&... args) {
            std::tuple<P2...> argList2 = forward_as_tuple(args...);
            argList1 = std::tuple_cat (values, argList2);
            return callFunc(typename helper::gens<sizeof...(A)>::type());
        }
        template<int ...S>
        R callFunc(helper::seq<S...>)
        {
            return function(std::get<S>(argList1)...);
        }
        bool operator== (const Bind<R(A..., V...)>& other) {
            return function == other.function;
        }
        private :
        std::tuple<A...> argList1;
        std::tuple <V...> values;
        R(*function)(A...);
    };
    template <typename R, typename O, typename... A, typename... V> class Bind <R(O*, A...), V...> {
        public:
        Bind (R(O::*f)(A&&...), O* object, V&&... v) : object(object), function(f) {
            values = std::forward_as_tuple(v...);
        }
        template <typename... P2>
        R operator ()(P2&&... args) {
            std::tuple<P2...> argList2 = std::forward_as_tuple(args...);
            argList1 = std::tuple_cat (values, argList2);
            return callFunc(typename helper::gens<sizeof...(A)>::type());
        }
        template<int ...S>
        R operator ()() {
            return callFunc(typename helper::gens<sizeof...(A)>::type());
        }
        template<int ...S>
        R callFunc(helper::seq<S...>)
        {
            return (object->*function)(std::get<S>(argList1)...);
        }
        bool operator== (const Functor<R(O*, A...)>& other) {
            return function == other.function;
        }
        private :
        O* object;
        std::tuple <A...> argList1;
        std::tuple <V...> values;
        R(O::*function)(A...);
    };

}
#endif // BIND
