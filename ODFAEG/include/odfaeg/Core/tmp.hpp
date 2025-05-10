#ifndef ODFAEG_TMP_HPP
#define ODFAEG_TMP_HPP
#include <tuple>
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
            constexpr std::size_t index() {
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
            template<typename T, typename Seq>
            struct select_tuple;
            template<typename T, std::size_t... Ints>
            //Return a tuple with elements at indexes.
            struct select_tuple<T, std::index_sequence<Ints...>>
            {
             using type = std::tuple<std::tuple_element_t<Ints, T>...>;
            };
            //Remove the Nth elements of a tuple.
            template<std::size_t N, typename T>
            struct remove_Nth
            {
              static constexpr auto size = std::tuple_size_v<T>;
              using first = std::make_index_sequence<N>;
              using rest = offset_sequence_t<N+1,
                            std::make_index_sequence<size-N-1>>;
              using indices = cat_sequence_t<first, rest>;
              using type = typename select_tuple<T, indices>::type;
            };
            namespace meta {
                template <std::size_t I, class T, bool B>
                struct get;
                template <std::size_t I, class T>
                struct get <I, T, true>
                {
                    using type = T;
                };
                template <std::size_t I, class T>
                struct get <I, T, false> {
                    using type = typename remove_Nth<I, T>::type;
                };
                template <template<class...> class Pred, class T, bool B, std::size_t NB, class Seq>
                struct append_if {};

                template <template<class...> class Pred, class T, std::size_t NB, std::size_t IH, std::size_t... IT>
                struct append_if<Pred, T, true, NB, std::index_sequence<IH, IT...>> {
                    using f = typename append_if<Pred, typename get<IH-NB, T, Pred<std::tuple_element_t<IH-NB, T>>::value>::type, Pred<std::tuple_element_t<IH-NB, T>>::value, NB, std::index_sequence<IT...>>::f;
                };
                template <template<class...> class Pred, class T, std::size_t NB, std::size_t IH, std::size_t... IT>
                struct append_if<Pred, T, false, NB, std::index_sequence<IH, IT...>> {
                    using f = typename append_if<Pred, typename get<IH-NB, T, Pred<std::tuple_element_t<IH-NB, T>>::value>::type, Pred<std::tuple_element_t<IH-NB, T>>::value, NB+1, std::index_sequence<IT...>>::f;
                };
                template <template<class...> class Pred, class T, std::size_t NB, std::size_t IH>
                struct append_if<Pred, T, true, NB, std::index_sequence<IH>> {
                    using f = typename get<IH-NB, T, Pred<std::tuple_element_t<IH-NB, T>>::value>::type;
                };
                template <template<class...> class Pred, class T, std::size_t NB, std::size_t IH>
                struct append_if<Pred, T, false, NB, std::index_sequence<IH>> {
                    using f = typename get<IH-NB, T, Pred<std::tuple_element_t<IH-NB, T>>::value>::type;
                };

                template<template <class...> class Pred, typename T>
                struct copy_if {
                    using f = typename append_if<Pred, T, Pred<std::tuple_element_t<0, T>>::value, 0, std::make_index_sequence<std::tuple_size<T>::value>>::f;
                };
                template<template <class...> class Pred>
                struct copy_if <Pred, std::tuple<>> {
                    using f = std::tuple<>;
                };

                template <std::size_t NB, typename T, bool B, std::size_t IH, std::size_t IH2, std::size_t N>
                struct remove_if_sames {

                };

                template <std::size_t NB, typename T, std::size_t IH1, std::size_t IH2, std::size_t N>
                struct remove_if_sames<NB, T, true, IH1, IH2, N> {
                    using f = typename remove_if_sames<NB, typename get<IH2-NB, T, !std::is_same<std::tuple_element_t<IH1, T>, std::tuple_element_t<IH2-NB, T>>::value>::type, !std::is_same<std::tuple_element_t<IH1, T>, std::tuple_element_t<IH2-NB, T>>::value, IH1, IH2+1, N>::f;
                };
                template <std::size_t NB, typename T, std::size_t IH1, std::size_t IH2, std::size_t N>
                struct remove_if_sames<NB, T, false, IH1, IH2, N> {
                    using f = typename remove_if_sames<NB+1, typename get<IH2-NB, T, !std::is_same<std::tuple_element_t<IH1, T>, std::tuple_element_t<IH2-NB, T>>::value>::type, !std::is_same<std::tuple_element_t<IH1, T>, std::tuple_element_t<IH2-NB, T>>::value, IH1, IH2+1, N>::f;
                };
                template <std::size_t NB, typename T, std::size_t IH1, std::size_t N>
                struct remove_if_sames<NB, T, true, IH1, N, N> {
                    using f = typename get<N-NB, T, !std::is_same<std::tuple_element_t<IH1, T>, std::tuple_element_t<N-NB, T>>::value>::type;
                };
                template <std::size_t NB, typename T, std::size_t IH1, std::size_t N>
                struct remove_if_sames<NB, T, false, IH1, N, N> {
                    using f = typename get<N-NB-1, T, !std::is_same<std::tuple_element_t<IH1, T>, std::tuple_element_t<N-NB-1, T>>::value>::type;
                };




                template <typename T, std::size_t IH, bool B, bool B2>
                struct make_unique {
                    using f = typename make_unique<typename remove_if_sames<0, T, true, IH, IH+1, std::tuple_size<T>::value-1>::f, IH+1, IH == std::tuple_size<T>::value - 2, (std::tuple_size<T>::value > 1)>::f;
                };
                template <typename T, std::size_t IH>
                struct make_unique<T, IH, false, true> {
                    using f = typename make_unique<typename remove_if_sames<0, T, true, IH, IH+1, std::tuple_size<T>::value-1>::f, IH+1, IH == std::tuple_size<T>::value - 2, (std::tuple_size<T>::value > 1)>::f;
                };
                template <typename T, std::size_t IH>
                struct make_unique<T, IH, true, true> {
                    using f = typename remove_if_sames<0, T, true, IH-1, IH, std::tuple_size<T>::value-1>::f;
                };
                template <typename T, std::size_t IH>
                struct make_unique<T, IH, false, false> {
                    using f = T;
                };
                template <typename T>
                struct unique {
                    using f = typename make_unique<T, 0, false, (std::tuple_size<T>::value > 1)>::f;
                };
                template <>
                struct unique <std::tuple<>> {
                    using f = std::tuple<>;
                };
                template <std::size_t I, std::size_t J, class T, bool B>
                struct swap_if {
                };
                template <std::size_t I, std::size_t J, class T>
                struct swap_if<I, J, T, true> {
                   using first = std::make_index_sequence<I>;
                   using second = std::index_sequence<J>;
                   using third = offset_sequence_t<I+1,std::make_index_sequence<J-I-1>>;
                   using four = std::index_sequence<I>;
                   using last = offset_sequence_t<J+1,std::make_index_sequence<std::tuple_size<T>()-J-1>>;
                   using indices = cat_sequence_t<first, cat_sequence_t<second, cat_sequence_t<third, cat_sequence_t<four, last>>>>;
                   using type = typename select_tuple<T, indices>::type;
                };
                template <std::size_t I, std::size_t J, class T>
                struct swap_if<I, J, T, false> {
                   using type = T;
                };
                template <class Comp, typename T, std::size_t IH1, std::size_t IH2, std::size_t N>
                struct swap_elements {
                   using f = typename swap_elements<Comp, typename swap_if<IH1, IH2, T, !Comp::template f<std::tuple_element_t<IH1, T>, std::tuple_element_t<IH2, T>>::value>::type, IH1, IH2+1, N>::f;
                };
                template <class Comp, typename T, std::size_t IH1, std::size_t N>
                struct swap_elements<Comp, T, IH1, N, N> {
                   using f = typename swap_if<IH1, N, T, !Comp::template f<std::tuple_element_t<IH1, T>, std::tuple_element_t<N, T>>::value>::type;
                };
                template <typename Comp, typename T, std::size_t IH, bool B, bool B2>
                struct sort_impl {
                };
                template <typename Comp, typename T, std::size_t IH>
                struct sort_impl<Comp, T, IH, false, true> {
                    using f = typename sort_impl<Comp, typename swap_elements<Comp, T, IH, IH+1, std::tuple_size<T>::value-1>::f, IH + 1, IH ==  std::tuple_size<T>::value-2, (std::tuple_size<T>::value > 1)>::f;
                };
                template <typename Comp, typename T, std::size_t IH>
                struct sort_impl<Comp, T, IH, true, true> {
                    using f = typename swap_elements<Comp, T, IH-1, IH, std::tuple_size<T>::value-1>::f;
                };
                template <typename Comp, typename T, std::size_t IH>
                struct sort_impl<Comp, T, IH, false, false> {
                    using f = T;
                };
                template <typename Comp, typename T>
                struct sort {
                    using f = typename sort_impl<Comp, T, 0, false, (std::tuple_size<T>::value > 1)>::f;
                };
                template <typename Comp>
                struct sort <Comp, std::tuple<>> {
                    using f = std::tuple<>;
                };
                template <class T, template<class...> class R, class Seq>
                struct lift {
                };
                template <class T, template<class...> class R, std::size_t... Ints>
                struct lift <T, R, std::index_sequence<Ints...>> {
                    using f = R<std::tuple_element_t<Ints, T>...>;
                };
            }
        //Return a tuple with elements at indexes.
        template <typename T, std::size_t... Ints>
        auto select_tuple_vals(T& tuple, std::index_sequence<Ints...>)
        {
            return std::make_tuple(std::get<Ints>(tuple)...);
        }

        //Remove the Nth elements of a tuple.
        template<std::size_t N, typename T>
        auto remove_Nth_vals(T& tuple)
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
