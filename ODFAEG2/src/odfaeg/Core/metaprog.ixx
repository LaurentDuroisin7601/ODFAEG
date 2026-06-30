module;
#include <type_traits>
#include <tuple>
export module odfaeg.core.metaprog;

export namespace odfaeg {
	namespace core {
            namespace meta {
            template <typename T, typename Tuple>
            struct contains;

            template <typename T>
            struct contains<T, std::tuple<>> : std::false_type {};

            template <typename T, typename U, typename... Rest>
            struct contains<T, std::tuple<U, Rest...>>
                : std::conditional_t<
                std::is_same<T, U>::value,
                std::true_type,
                contains<T, std::tuple<Rest...>>
                > {
            };
            template <typename T, typename U = void, typename... Types>
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
                using rest = offset_sequence_t<N + 1,
                    std::make_index_sequence<size - N - 1>>;
                using indices = cat_sequence_t<first, rest>;
                using type = typename select_tuple<T, indices>::type;
            };
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
                using f = typename append_if<Pred, typename get<IH - NB, T, Pred<std::tuple_element_t<IH - NB, T>>::value>::type, Pred<std::tuple_element_t<IH - NB, T>>::value, NB, std::index_sequence<IT...>>::f;
            };
            template <template<class...> class Pred, class T, std::size_t NB, std::size_t IH, std::size_t... IT>
            struct append_if<Pred, T, false, NB, std::index_sequence<IH, IT...>> {
                using f = typename append_if<Pred, typename get<IH - NB, T, Pred<std::tuple_element_t<IH - NB, T>>::value>::type, Pred<std::tuple_element_t<IH - NB, T>>::value, NB + 1, std::index_sequence<IT...>>::f;
            };
            template <template<class...> class Pred, class T, std::size_t NB, std::size_t IH>
            struct append_if<Pred, T, true, NB, std::index_sequence<IH>> {
                using f = typename get<IH - NB, T, Pred<std::tuple_element_t<IH - NB, T>>::value>::type;
            };
            template <template<class...> class Pred, class T, std::size_t NB, std::size_t IH>
            struct append_if<Pred, T, false, NB, std::index_sequence<IH>> {
                using f = typename get<IH - NB, T, Pred<std::tuple_element_t<IH - NB, T>>::value>::type;
            };

            template<template <class...> class Pred, typename T>
            struct copy_if {
                using f = typename append_if<Pred, T, Pred<std::tuple_element_t<0, T>>::value, 0, std::make_index_sequence<std::tuple_size<T>::value>>::f;
            };
            template<template <class...> class Pred>
            struct copy_if <Pred, std::tuple<>> {
                using f = std::tuple<>;
            };

            // Construit un tuple unique en accumulant
            template <typename Acc, typename Tuple>
            struct unique_impl;

            template <typename Acc>
            struct unique_impl<Acc, std::tuple<>> {
                using type = Acc;
            };

            template <typename Acc, typename T, typename... Rest>
            struct unique_impl<Acc, std::tuple<T, Rest...>> {
                using type = std::conditional_t <
                    contains<T, Acc>::value,
                    // déjà présent : on ne l'ajoute pas
                    typename unique_impl<Acc, std::tuple<Rest...>>::type,
                    // pas présent : on l'ajoute à Acc
                    typename unique_impl<
                    decltype(std::tuple_cat(Acc{}, std::tuple<T>{})),
                    std::tuple<Rest...>
                    > ::type
                > ;
            };

            // Interface publique
            template <typename Tuple>
            struct unique {
                using f = typename unique_impl<std::tuple<>, Tuple>::type;
            };

            // Spécialisation vide (optionnelle, mais cohérente avec ton code)
            template <>
            struct unique<std::tuple<>> {
                using f = std::tuple<>;
            };
            template <std::size_t I, std::size_t J, class T, bool B>
            struct swap_if {
            };
            template <std::size_t I, std::size_t J, class T>
            struct swap_if<I, J, T, true> {
                using first = std::make_index_sequence<I>;
                using second = std::index_sequence<J>;
                using third = offset_sequence_t<I + 1, std::make_index_sequence<J - I - 1>>;
                using four = std::index_sequence<I>;
                using last = offset_sequence_t<J + 1, std::make_index_sequence<std::tuple_size<T>() - J - 1>>;
                using indices = cat_sequence_t<first, cat_sequence_t<second, cat_sequence_t<third, cat_sequence_t<four, last>>>>;
                using type = typename select_tuple<T, indices>::type;
            };
            template <std::size_t I, std::size_t J, class T>
            struct swap_if<I, J, T, false> {
                using type = T;
            };
            template <class Comp, typename T, std::size_t IH1, std::size_t IH2, std::size_t N>
            struct swap_elements {
                using f = typename swap_elements<Comp, typename swap_if<IH1, IH2, T, !Comp::template f<std::tuple_element_t<IH1, T>, std::tuple_element_t<IH2, T>>::value>::type, IH1, IH2 + 1, N>::f;
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
                using f = typename sort_impl<Comp, typename swap_elements<Comp, T, IH, IH + 1, std::tuple_size<T>::value - 1>::f, IH + 1, IH == std::tuple_size<T>::value - 2, (std::tuple_size<T>::value > 1)>::f;
            };
            template <typename Comp, typename T, std::size_t IH>
            struct sort_impl<Comp, T, IH, true, true> {
                using f = typename swap_elements<Comp, T, IH - 1, IH, std::tuple_size<T>::value - 1>::f;
            };
            template <typename Comp, typename T, std::size_t IH>
            struct sort_impl<Comp, T, IH, false, false> {
                using f = T;
            };
            template <typename Comp, typename T>
            struct sort {
                using f = typename sort_impl<Comp, T, 0, (std::tuple_size<T>::value - 2 == 0), (std::tuple_size<T>::value > 1)>::f;
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
        namespace detail
        {
            template
                <
                template <typename> class Size,
                typename SoFar,
                typename... Args
                >
                struct max_helper;

            template
                <
                template <typename> class Size,
                typename SoFar
                >
                struct max_helper<Size, SoFar>
            {
                static constexpr decltype(Size<SoFar>::value) value =
                    Size<SoFar>::value;
                typedef SoFar type;
            };

            template
                <
                template <typename> class Size,
                typename SoFar,
                typename Next,
                typename... Args
                >
                struct max_helper<Size, SoFar, Next, Args...>
            {
            private:
                typedef typename std::conditional
                    <
                    (Size<Next>::value > Size<SoFar>::value),
                    max_helper<Size, Next, Args...>,
                    max_helper<Size, SoFar, Args...>
            >::type m_next;

            public:
                static constexpr decltype(Size<SoFar>::value) value =
                    m_next::value;

                typedef typename m_next::type type;
            };
        }

        template <template <typename> class Size, typename... Args>
        struct max;

        template
            <
            template <typename> class Size,
            typename First,
            typename... Args
            >
            struct max<Size, First, Args...>
        {
        private:
            typedef decltype(Size<First>::value) m_size_type;
            typedef detail::max_helper
                <
                Size,
                First,
                Args...
                > m_helper;

        public:
            static constexpr m_size_type value = m_helper::value;
            typedef typename m_helper::type type;
        };
        //Utilitaire pour ajouter un type à la fin d'un tuple
        template<class, class>
        struct cat_type;

        template<class T, class... Arg>
        struct cat_type<std::tuple<Arg...>, T>
        {
            using type = std::tuple<Arg..., T>;
        };


        //Classe qui surcharge accept pour les types concret
        template<class Abstract, class>
        struct acceptable : Abstract
        {
            void accept(const typename Abstract::visitor_type& v)
            {
                v.visit(*this);
            }
        };


        //Classe de base des visiteurs
        //La visite se fait sur le type acceptable qui contient l'information sur le type réel
        template<class, class...>
        struct visitor;

        //Récursion
        template<class Abstract, class Concrete, class... Concrete_Tail>
        struct visitor<Abstract, Concrete, Concrete_Tail...>
            : visitor<Abstract, Concrete_Tail...>
        {
            using visitor<Abstract, Concrete_Tail...>::visit;
            virtual void visit(acceptable<Abstract, Concrete>&) const = 0;
        };

        //Condition d'arrêt
        template<class Abstract, class Concrete>
        struct visitor<Abstract, Concrete>
        {
            visitor() = default;
            visitor(const visitor&) = delete;

            virtual ~visitor()
            {
            }

            visitor& operator=(const visitor&) = delete;

            virtual void visit(acceptable<Abstract, Concrete>&) const = 0;
        };


        //Utilitaire pour récupérer le type accepté par la hiérarchie depuis le visiteur
        template<class, class>
        struct accept_type;

        template<
            class Abstract, class... Concrete,
            class T
        >
        struct accept_type<visitor<Abstract, Concrete...>, T>
        {
            using type = acceptable<Abstract, T>;
        };


        //Utilitaire pour récupérer les types concrets de la hiérarchie depuis le visiteur
        template<class>
        struct concrete_type;

        template<class Abstract, class... Concrete>
        struct concrete_type<visitor<Abstract, Concrete...>>
        {
            using type = std::tuple<Concrete...>;
        };


        //Classe de base qui ajoute accept à la classe de base de la hiérarchie
        template<class Abstract, class... Concrete>
        struct accept_visitor
        {
            using visitor_type = visitor<Abstract, Concrete...>;

            accept_visitor() = default;
            accept_visitor(const accept_visitor&) = delete;

            virtual ~accept_visitor()
            {
            }

            accept_visitor& operator=(const accept_visitor&) = delete;

            virtual void accept(const visitor_type&) = 0;
        };


        //Classe dispatcher : c'est un visiteur et un foncteur

        //Condition d'arrêt sur les types de la hiérarchie
        template<
            class To_Visit, class, bool,
            class Visitor, class,
            class Fun
        >
        struct dispatcher : Visitor
        {
            dispatcher(To_Visit& tv, const Fun& f)
                : to_visit(tv), fun(f)
            {
            }

        protected:
            To_Visit& to_visit;
            Fun fun;
        };

        //Récursion sur les types de la hiérarchie
        template<
            class To_Visit, class Visited,
            class Visitor, class T, class... Concrete,
            class Fun
        >
        struct dispatcher<
            To_Visit, Visited, false,
            Visitor, std::tuple<T, Concrete...>,
            Fun
        >
            : dispatcher<
            To_Visit, Visited, false,
            Visitor, std::tuple<Concrete...>,
            Fun
            >
        {
        private:
            using base = dispatcher<
                To_Visit, Visited, false,
                Visitor, std::tuple<Concrete...>,
                Fun
            >;

        public:
            using base::base;

            //Déclenche la visite sur le bon paramètre
            void operator()() const
            {
                std::get
                    <std::tuple_size<Visited>::value>
                    (base::to_visit)
                    .accept(*this);
            }

            using base::visit;
            void visit(typename accept_type<Visitor, T>::type&) const
            {
                using new_visited = typename cat_type<Visited, T>::type;

                //Récursion sur les arguments d'appel
                dispatcher<
                    To_Visit, new_visited,
                    std::tuple_size<To_Visit>::value == std::tuple_size<new_visited>::value,
                    Visitor, typename concrete_type<Visitor>::type,
                    Fun
                >(base::to_visit, base::fun)();
            }
        };

        //Condition d'arrêt sur les paramètre d'appel
        template<
            class To_Visit, class... Visited,
            class Visitor, class Concrete,
            class Fun
        >
        struct dispatcher<
            To_Visit, std::tuple<Visited...>, true,
            Visitor, Concrete,
            Fun
        >
        {
            dispatcher(To_Visit& tv, const Fun& f)
                : to_visit(tv), fun(f)
            {
            }

            //Déclenche l'appel avec les bon types
            void operator()()
            {
                apply(std::make_index_sequence<sizeof...(Visited)>());
            }

        private:
            template<std::size_t... I>
            void apply(std::index_sequence<I...>)
            {
                fun(reinterpret_cast<Visited&>(std::get<I>(to_visit))...);
            }

            To_Visit& to_visit;
            Fun fun;
        };


        //Classe de base pour les foncteurs à dispatcher
        template<class Fun, class Abstract>
        struct dispatchable
        {
            dispatchable() = default;
            dispatchable(const dispatchable&) = default;

            dispatchable& operator=(const dispatchable&) = default;

            template<class... Arg>
            void apply(Arg&&... arg)
            {
                using used_visitor = typename Abstract::visitor_type;

                auto t = std::forward_as_tuple(std::forward<Arg>(arg)...);
                dispatcher<
                    decltype(t), std::tuple<>, false,
                    used_visitor, typename concrete_type<used_visitor>::type,
                    Fun
                >(t, static_cast<Fun&>(*this))();
            }

        protected:
            ~dispatchable()
            {
            }
        };
	}
}
