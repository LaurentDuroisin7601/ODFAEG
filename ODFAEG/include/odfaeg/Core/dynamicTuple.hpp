#ifndef ODFAEG_DYNAMIC_TUPLE_HPP
#define ODFAEG_DYNAMIC_TUPLE_HPP
namespace odfaeg {
    namespace core {
        template < typename Tp, typename... List >
        struct contains : std::true_type {};

        template < typename Tp, typename Head, typename... Rest >
        struct contains<Tp, Head, Rest...>
        : std::conditional< std::is_same<Tp, Head>::value,
            std::true_type,
            contains<Tp, Rest...>
        >::type {};
        template <typename T, typename Tuple>
        struct has_type;

        template <typename T>
        struct has_type<T, std::tuple<>> : std::false_type {};

        template <typename T, typename U, typename... Ts>
        struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {};

        template <typename T, typename... Ts>
        struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};
        template < typename Tp >
        struct contains<Tp> : std::false_type {};

        template <typename T, typename U=void, typename... Types>
        constexpr size_t index() {
            return std::is_same<T, U>::value ? 0 : 1 + index<T, Types...>();
        }
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

        template<std::size_t N, typename T>
        auto remove_Nth(T& tuple);
        template <typename... TupleTypes>
        struct DynamicTuple {
            std::tuple<std::vector<TupleTypes>...> content;
            using types = typename std::tuple<TupleTypes...>;


            template <typename H, class = typename std::enable_if_t<contains<H, TupleTypes...>::value>>
            DynamicTuple add (H head) {
                std::get<std::vector<H>>(content).push_back(head);
                return *this;
            }
            template <typename H, class = typename std::enable_if_t<!contains<H, TupleTypes...>::value>>
            DynamicTuple <TupleTypes..., H> add (H head) {
                DynamicTuple<TupleTypes..., H> tuple;
                tuple.content = std::tuple_cat(content, std::make_tuple(std::vector<H>()));
                return tuple.add(head);
            }
            template <typename H, class = typename std::enable_if_t<!contains<H, TupleTypes...>::value>>
            DynamicTuple <TupleTypes..., H> addType () {
                DynamicTuple<TupleTypes..., H> tuple;
                tuple.content = std::tuple_cat(content, std::make_tuple(std::vector<H>()));
                return tuple;
            }
            template <typename T>
            constexpr size_t vectorSize() {
                return std::get<std::vector<T>>(content).size();
            }
            static constexpr size_t nbTypes() {
                return std::tuple_size<types>::value;
            }
            template <typename T>
            static constexpr size_t getIndexOfTypeT() {
                return index<T, TupleTypes...>();
            }
            template <typename T>
            T* get(unsigned int containerIdx) {
                constexpr size_t I = getIndexOfTypeT<T>();
                return (containerIdx < vectorSize<T>()) ? &std::get<I>(content)[containerIdx] : nullptr;
            }
            template <size_t I>
            auto get(unsigned int containerIdx) {
                return std::get<I>(content)[containerIdx];
            }
            template <size_t I>
            auto removeType() {
                return remove_Nth<I>(*this);
            }
            template <typename T>
            auto removeType() {
                return remove_Nth<index<T, TupleTypes...>()>(*this);
            }
            template <typename T>
            auto remove(T element) {
                std::vector<T> elements = std::get<index<T, TupleTypes...>()>(content);
                typename std::vector<T>::iterator it;
                for (it = elements.begin(); it != elements.end(); it++) {
                    if (*it == &element) {
                        elements.erase(it);
                    } else {
                        it++;
                    }
                }
                if (std::get<index<T, TupleTypes...>()>(content).size() == 0) {
                    return removeType<T>();
                }
                return *this;
            }
        };
        //Return a tuple with elements at indexes.
        template <typename T, size_t... Ints>
        auto select_tuple(T& tuple, std::index_sequence<Ints...>)
        {
            DynamicTuple<std::tuple_element_t<Ints, typename T::types>...> newTuple;
            newTuple.content = std::make_tuple(std::get<Ints>(tuple.content)...);
            return newTuple;
        }

        //Remove the Nth elements of a tuple.
        template<std::size_t N, typename T>
        auto remove_Nth(T& tuple)
        {
          constexpr auto size = tuple.nbTypes();
          using first = std::make_index_sequence<N>;
          using rest = offset_sequence_t<N+1,
                        std::make_index_sequence<size-N-1>>;
          using indices = cat_sequence_t<first, rest>;
          return select_tuple(tuple, indices{});
        }
    }
}
#endif
