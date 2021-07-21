#ifndef ODFAEG_FUNCTION_HPP
#define ODFAEG_FUNCTION_HPP
#include "odfaeg/Core/tuple.h"
namespace test {
namespace helper {
template <int...> struct seq {};
template<int N, int ...S> struct gens : gens<N-1, N-1, S...> {

};
template<int ...S> struct gens<0, S...>{
    typedef seq<S...> type;
};
}
template<typename R> class Function {
};
template<typename R, typename ...A> class Function <R(A...)> {
    public :
    Function (R(*f)(A...)) : function(f) {

    }

    R operator()(A... args) {
        return  function(args...);
    }
    bool operator== (const Function<R(A...)>& other) {
        return function == other.function;
    }
    private :
    R(*function)(A...);

};



template <typename R> class Functor  {

};


template<typename R, typename F, typename ...A> class Functor<R(F, A...)> {
    public :

    Functor(R(*funct)(F, A...), F function) : funct(funct), function(function) {

    }

    bool operator== (const Functor<R(F, A...)>& other) {
        return funct == other.funct;
    }

    template<int... S>
    R operator()(std::tuple<A...> params) {
        return callFunc(typename helper::gens<sizeof...(A)>::type(), params);
    }
    template<int ...S>
    R callFunc(helper::seq<S...>, std::tuple<A...> params)
    {
          return funct(function, std::get<S>(params)...);
    }
    private :
    R(*funct)(F, A...);
    F function;

};

template<typename R> class MemberFunction {
};
template <typename R, typename O, typename ...A> class MemberFunction <R(O*, A...)> {
    public :

    MemberFunction (R(O::*f)(A...)) : function(f) {
        constFunction = nullptr;
    }
    MemberFunction (R(O::*f)(A...) const) : constFunction(f) {
        function = nullptr;
    }
    R operator() (O* object, A... args) {
        if (function != nullptr)
            return (object->*function)(args...);
        else
            return (object->*constFunction)(args...);
    }
    R operator() (O& object, A... args) {
        if (function != nullptr)
            return (object.*function)(args...);
        else
            return (object.*constFunction)(args...);
    }
    bool operator== (const MemberFunction<R(O*,  A...)>& other) {
        return function == other.function || constFunction == other.constFunction;
    }
    private :
    R(O::*function)(A...);
    R(O::*constFunction)(A...) const;
};
}
#endif // FUNCTION

