namespace odfaeg {
    namespace core {
        //Make index sequences from an offset.
        template<std::size_t N, typename Seq> struct offset_sequence;

        template<std::size_t N, std::size_t... Ints>
        struct offset_sequence<N, std::index_sequence<Ints...>>
        {
         using type = std::index_sequence<Ints + N...>;
        };
        template<std::size_t N, typename Seq>
        using offset_sequence_t = typename offset_sequence<N, Seq>::type;
        //Concatenate two sequences of indexes into one.
        template<typename Seq1, typename Seq> struct cat_sequence;
        template<std::size_t... Ints1, std::size_t... Ints2>
        struct cat_sequence<std::index_sequence<Ints1...>,
                            std::index_sequence<Ints2...>>
        {
         using type = std::index_sequence<Ints1..., Ints2...>;
        };
        template<typename Seq1, typename Seq2>
        using cat_sequence_t = typename cat_sequence<Seq1, Seq2>::type;
        template<typename T, std::size_t... Ints>
        //Return a tuple with elements at indexes.
        auto select_tuple(std::index_sequence<Ints...>)
        {
         T tuple;
         return std::make_tuple(
            std::get<Ints>(std::forward<T>(tuple))...);
        }
        //Remove the Nth elements of a tuple.
        template<std::size_t N, typename T>
        auto remove_Nth()
        {
          constexpr auto size = std::tuple_size_v<T>;
          using first = std::make_index_sequence<N>;
          using rest = offset_sequence_t<N+1,
                        std::make_index_sequence<size-N-1>>;
          using indices = cat_sequence_t<first, rest>;
          return select_tuple<T>(indices{});
        }
        //Remove every types, that are not satisfied a condition.
        template<template<class...> class Pred>
        struct append_if {
            //If the type statisfy the condition we don't remove it and return the tuple itself.
            template <size_t I, class B, class T, class = typename std::enable_if<B::value>::type>
            static constexpr auto get(T tp) {
                return tp;
            }
            //If the type doesn't satisfy the condition we remove it from the tuple and return the new tuple.
            template <size_t I, class B, class T, class... D, class = typename std::enable_if<!B::value>::type>
            static constexpr auto get(T tp) {
                return remove_Nth<I, T>();
            }
            //final case : we stop recursion when we have browsed all the elements of the tuple and we check if the last element must be removed.
            template <size_t I=0, size_t NB=0, class T, class = typename std::enable_if<(I-NB == std::tuple_size<T>()-1 && Pred<std::tuple_element_t<I-NB, T>>::value)>::type>
            static constexpr auto f(T tp) {
                return get<I-NB, std::true_type>(tp);
            }
            template <size_t I=0, size_t NB=0, class T, class... D, class = typename std::enable_if<(I-NB == std::tuple_size<T>()-1 && !Pred<std::tuple_element_t<I-NB, T>>::value)>::type>
            static constexpr auto f(T tp) {
                return get<I-NB, std::false_type>(tp);
            }
            template <size_t I=0, size_t NB=0, class T, class... D, class... E, class = typename std::enable_if<(I-NB < std::tuple_size<T>()-1  && Pred<std::tuple_element_t<I-NB, T>>::value)>::type>
            static constexpr auto f(T tp) {
                return f<I+1, NB>(get<I-NB, std::true_type>(tp));
            }
            template <size_t I=0, size_t NB=0, class T, class... D, class... E, class ... F, class = typename std::enable_if<(I-NB < std::tuple_size<T>()-1 && !Pred<std::tuple_element_t<I-NB, T>>::value)>::type>
            static constexpr auto f(T tp) {
                return f<I+1, NB+1>(get<I-NB, std::false_type>(tp));
            }
        };
        //Copy every tuple elements which satisfied a predicate.
        template<template <class...> class Pred, typename T>
        struct copy_if {
            T tp;
            using f = decltype(append_if<Pred>::template f(tp));
        };
        template<template <class...> class Pred>
        struct copy_if <Pred, std::tuple<>> {
            using f = std::tuple<>;
        };
        //Sort every elements of a tuple with the specified comparator.
        template <class Comp>
        struct swap_if {
            //Case 1 : If the next element is greater than the current element, no need to swap, we just return the tuple itself.
            template <size_t I, size_t J, class T, class = typename std::enable_if<Comp::template f<std::tuple_element_t<I, T>, std::tuple_element_t<J, T>>::value>::type>
            static constexpr auto get (T tp) {
               return T();
            }
            //Case 2 : If the new element is smaller than the current element, we swap the tuple's elements.
            template <size_t I, size_t J, class T, class... D, class = typename std::enable_if<!Comp::template f<std::tuple_element_t<I, T>, std::tuple_element_t<J, T>>::value>::type>
            static constexpr auto get (T tp) {
               using first = std::make_index_sequence<I>;
               using second = std::index_sequence<J>;
               using third = offset_sequence_t<I+1,std::make_index_sequence<J-I-1>>;
               using four = std::index_sequence<I>;
               using last = offset_sequence_t<J+1,std::make_index_sequence<std::tuple_size<T>()-J-1>>;
               using indices = cat_sequence_t<first, cat_sequence_t<second, cat_sequence_t<third, cat_sequence_t<four, last>>>>;
               return select_tuple<T>(indices{});
            }
            //final case second recursions, we
            template <size_t I, size_t J, class T, class = typename std::enable_if<(J == std::tuple_size<T>()-1)>::type>
            static constexpr auto f2(T tp) {
                return get<I, J>(tp);
            }
            //General case second recursion, we check if we need to swap the tuple's elements for next elements.
            template <size_t I, size_t J, class T, class... D, class = typename std::enable_if<(J < std::tuple_size<T>()-1)>::type>
            static constexpr auto f2(T tp) {
                return f2<I, J+1>(get<I, J>(tp));
            }
            //final case, we stop the recursion and we check if the penultimate element and the last element need to be swapped.
            template <size_t I=0, class T, class = typename std::enable_if<(std::tuple_size<T>() > 1 && I == std::tuple_size<T>() - 2)>::type>
            static constexpr auto f(T tp) {
                return f2<I, I+1>(tp);
            }
            //General case : do recursion to check if tuple's elements need to be swapped.
            template <size_t I=0, class T, class... D, class = typename std::enable_if<(std::tuple_size<T>() > 1 && I < std::tuple_size<T>() - 2)>::type>
            static constexpr auto f(T tp) {
                return f<I+1>(f2<I, I+1>(tp));
            }
            template <size_t I=0, class T, class... D, class... E, class = typename std::enable_if<(std::tuple_size<T>() < 2)>::type>
            static constexpr auto f(T tp) {
                return tp;
            }
        };
        template <class Comp, typename T>
        struct sort {
            T tp;
            using f = decltype(swap_if<Comp>::template f(tp));
        };
        struct make_unique {
            template <size_t I, size_t J, class B, class T, class = typename std::enable_if<B::value>::type>
            static constexpr auto get(T tp) {
                return remove_Nth<I, T>();
            }
            template <size_t I, size_t J, class B, class T, class... D, class = typename std::enable_if<!B::value>::type>
            static constexpr auto get(T tp) {
                return tp;
            }
            //final case second recursions, we
            template <size_t I, size_t J, size_t NB, class T, class = typename std::enable_if<(J-NB == std::tuple_size<T>()-1 && std::is_same<std::tuple_element_t<I, T>, std::tuple_element_t<J, T>>::value)>::type>
            static constexpr auto f2(T tp) {
                return get<I, J-NB, std::true_type>(tp);
            }
            template <size_t I, size_t J, size_t NB, class T, class... D, class = typename std::enable_if<(J-NB == std::tuple_size<T>()-1 && !std::is_same<std::tuple_element_t<I, T>, std::tuple_element_t<J, T>>::value)>::type>
            static constexpr auto f2(T tp) {
                return get<I, J-NB, std::false_type>(tp);
            }
            //General case second recursion, we check if we need to swap the tuple's elements for next elements.
            template <size_t I, size_t J, size_t NB, class T, class... D, class... E, class = typename std::enable_if<(J-NB < std::tuple_size<T>()-1 && std::is_same<std::tuple_element_t<I, T>, std::tuple_element_t<J, T>>::value)>::type>
            static constexpr auto f2(T tp) {
                return f2<I, J+1, NB+1>(get<I, J-NB, std::true_type>(tp));
            }
            template <size_t I, size_t J, size_t NB, class T, class... D, class... E, class... F, class = typename std::enable_if<(J-NB < std::tuple_size<T>()-1 && !std::is_same<std::tuple_element_t<I, T>, std::tuple_element_t<J, T>>::value)>::type>
            static constexpr auto f2(T tp) {
                return f2<I, J+1, NB>(get<I, J-NB, std::false_type>(tp));
            }
            //final case, we stop the recursion and we check if the penultimate element and the last element need to be swapped.
            template <size_t I=0, class T, class = typename std::enable_if<(std::tuple_size<T>() > 1 && I == std::tuple_size<T>() - 2)>::type>
            static constexpr auto f(T tp) {
                return f2<I, I+1, 0>(tp);
            }
            //General case : do recursion to check if tuple's elements need to be swapped.
            template <size_t I=0, class T, class... D, class = typename std::enable_if<(std::tuple_size<T>() > 1 && I < std::tuple_size<T>() - 2)>::type>
            static constexpr auto f(T tp) {
                return f<I+1>(f2<I, I+1, 0>(tp));
            }
            template <size_t I=0, class T, class... D, class... E, class = typename std::enable_if<(std::tuple_size<T>() < 2)>::type>
            static constexpr auto f(T tp) {
                return tp;
            }
        };
        template <class T>
        struct unique {
            T tp;
            using f = decltype(make_unique::f(tp));
        };
        template <class T, template<class...> class R, class Seq>
        struct lift {
        };
        template <class T, template<class...> class R, size_t... Ints>
        struct lift <T, R, std::index_sequence<Ints...>> {
            using f = R<std::tuple_element_t<Ints, T>...>;
        };
    }
}
