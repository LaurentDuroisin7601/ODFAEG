#ifndef ODFAEG_VARIANT_HPP
#define ODFAEG_VARIANT_HPP
#include <cassert>
#include <functional>
#include <new>
#include <type_traits>
#include <utility>
#include "metaprog.hpp"
namespace odfaeg {
    namespace core {
        template <typename T>
        class recursive_wrapper
        {
        public:
            ~recursive_wrapper();
            template
                <
                typename U,
                typename Dummy =
                typename std::enable_if<std::is_convertible<U, T>::value, U>::type
                >
                recursive_wrapper(
                    const U& u);

            template
                <
                typename U,
                typename Dummy =
                typename std::enable_if<std::is_convertible<U, T>::value, U>::type
                >
                recursive_wrapper(U&& u);

            recursive_wrapper(const recursive_wrapper& rhs);

            recursive_wrapper(recursive_wrapper&& rhs);

            recursive_wrapper&
                operator=(const recursive_wrapper& rhs);

            recursive_wrapper&
                operator=(recursive_wrapper&& rhs);

            recursive_wrapper&
                operator=(const T& t);

            recursive_wrapper&
                operator=(T&& t);

            T& get();
            const T& get() const;

        private:
            T* m_t;

            template <typename U>
            void
                assign(U&& u);
        };

        struct true_ {};
        struct false_ {};

        namespace detail
        {
            template <typename T, typename Internal>
            T&
                get_value(T& t, const Internal&);

            template <typename T>
            T&
                get_value(recursive_wrapper<T>& t, const false_&);

            template <typename T>
            const T&
                get_value(const recursive_wrapper<T>& t, const false_&);
        }

        template
            <
            typename Internal,
            typename T,
            typename Storage,
            typename Visitor,
            typename... Args
            >
            typename Visitor::result_type
            visitor_caller(Internal&& internal,
                Storage&& storage, Visitor&& visitor, Args&&... args);
        template <typename First, typename... Types>
        class Variant
        {
        private:

            template <typename... AllTypes>
            struct do_visit
            {
                template
                    <
                    typename Internal,
                    typename VoidPtrCV,
                    typename Visitor,
                    typename... Args
                    >
                    typename Visitor::result_type
                    operator()
                    (
                        Internal&& internal,
                        size_t which,
                        VoidPtrCV&& storage,
                        Visitor& visitor,
                        Args&&... args
                        );
            };

            template <typename T>
            struct Sizeof
            {
                static constexpr size_t value = sizeof(T);
            };

            template <typename T>
            struct Alignof
            {
                static constexpr size_t value = alignof(T);
            };

            //size = max of size of each thing
            static constexpr size_t m_size =
                max
                <
                Sizeof,
                First,
                Types...
                >::value;

            struct constructor
            {
                typedef void result_type;

                constructor(Variant& self);

                template <typename T>
                void
                    operator()(const T& rhs) const;

            private:
                Variant& m_self;
            };

            struct move_constructor
            {
                typedef void result_type;

                move_constructor(Variant& self);

                template <typename T>
                void
                    operator()(T& rhs) const;

            private:
                Variant& m_self;
            };

            struct assigner
            {
                typedef void result_type;

                assigner(Variant& self, int rhs_which);

                template <typename Rhs>
                void
                    operator()(const Rhs& rhs) const;
            private:
                Variant& m_self;
                int m_rhs_which;
            };

            struct move_assigner
            {
                typedef void result_type;

                move_assigner(Variant& self, int rhs_which);

                template <typename Rhs>
                void
                    operator()(Rhs& rhs) const;

            private:
                Variant& m_self;
                int m_rhs_which;
            };

            struct destroyer
            {
                typedef void result_type;

                template <typename T>
                void
                    operator()(T& t) const;
            };

            template <size_t Which, typename... MyTypes>
            struct initialiser;

            template <size_t Which, typename Current, typename... MyTypes>
            struct initialiser<Which, Current, MyTypes...>
                : public initialiser<Which + 1, MyTypes...>
            {
                typedef initialiser<Which + 1, MyTypes...> base;
                using base::initialise;

                static void
                    initialise(Variant& v, Current&& current);

                static void
                    initialise(Variant& v, const Current& current);
            };

            template <size_t Which>
            struct initialiser<Which>
            {
                //this should never match
                void initialise();
            };

        public:

            Variant();

            ~Variant();

            //enable_if disables this function if we are constructing with a Variant.
            //Unfortunately, this becomes Variant(Variant&) which is a better match
            //than Variant(const Variant& rhs), so it is chosen. Therefore, we disable
            //it.
            template
                <
                typename T,
                typename Dummy =
                typename std::enable_if
                <
                !std::is_same
                <
                typename std::remove_reference<Variant<First, Types...>>::type,
                typename std::remove_reference<T>::type
                >::value,
                T
                >::type
                >
                Variant(T&& t);

            Variant(const Variant& rhs);

            Variant(Variant&& rhs);

            Variant& operator=(const Variant& rhs);

            Variant& operator=(Variant&& rhs);
            int which() const;

            template <typename Internal, typename Visitor, typename... Args>
            typename Visitor::result_type
                apply_visitor(Visitor& visitor, Args&&... args);
            template <typename Internal, typename Visitor, typename... Args>
            typename Visitor::result_type
                apply_visitor(Visitor& visitor, Args&&... args) const;
        private:

            //TODO implement with alignas when it is implemented in gcc
            //alignas(max<Alignof, First, Types...>::value) char[m_size];
            union
            {
                char m_storage[m_size]; //max of size + alignof for each of Types...
                //the type with the max alignment
                typename max<Alignof, First, Types...>::type m_align;
            };

            int m_which;

            static std::function<void(void*)> m_handlers[1 + sizeof...(Types)];

            inline void indicate_which(int which) {
                m_which = which;
            }

            void* address();
            const void* address() const;

            template <typename Visitor>
            typename Visitor::result_type
                apply_visitor_internal(Visitor& visitor);

            template <typename Visitor>
            typename Visitor::result_type
                apply_visitor_internal(Visitor& visitor) const;

            void destroy();

            template <typename T>
            void
                construct(T&& t);
        };
        struct bad_get : public std::exception
        {
            virtual const char* what() const throw();
        };

        template <typename T>
        struct get_visitor
        {
            typedef T* result_type;

            result_type
                operator()(T& val) const;

            template <typename U>
            result_type
                operator()(const U& u) const;
        };

        template <typename Visitor, typename Visitable, typename... Args>
        typename Visitor::result_type
            apply_visitor(Visitor& visitor, Visitable& visitable, Args&&... args);
        template <typename Visitor, typename Visitable, typename... Args>
        typename Visitor::result_type
            apply_visitor(const Visitor& visitor, Visitable& visitable, Args&&... args);

        template <typename T, typename First, typename... Types>
        T*
            get(Variant<First, Types...>* var);
        template <typename T, typename First, typename... Types>
        const T*
            get(const Variant<First, Types...>* var);
        template <typename T, typename First, typename... Types>
        T&
            get(Variant<First, Types...>& var);
        template <typename T, typename First, typename... Types>
        const T&
            get(const Variant<First, Types...>& var);
        namespace details {
            template <typename Visitor, typename T>
            struct NaryVisitor {
                using result_type = typename Visitor::result_type;

                NaryVisitor(Visitor& visitor, T& t);
                Visitor& visitor;
                T& ref;

                template <typename... Args>
                auto operator()(Args&&... args) -> result_type;
            }; // struct NaryVisitor
            template <typename Visitor, typename Arg, typename... Vs>
            auto apply_nary_visitor_impl(
                Visitor& visitor, Arg& arg, Vs&&... vs
            )
                -> typename Visitor::result_type;

            template <typename Visitor, typename T0, typename... Ts, typename... Vs>
            auto apply_nary_visitor_impl(
                Visitor& visitor, Variant<T0, Ts...>&& v0, Vs&&... vs
            )
                -> typename Visitor::result_type;

            template <typename Visitor, typename Variant>
            struct NaryApplier {
                using result_type = typename Visitor::result_type;

                NaryApplier(Visitor& visitor, Variant& variant);

                Visitor& visitor;
                Variant& variant;

                template <typename T>
                auto apply() -> result_type;

                template <typename T, typename V0, typename... Vs>
                auto apply(V0&& v0, Vs&&... vs) -> result_type;
            }; // struct NaryApplier
            template <typename Visitor, typename Arg, typename... Vs>
            auto apply_nary_visitor_impl(
                Visitor& visitor, Arg& arg, Vs&&... vs
            )
                -> typename Visitor::result_type;
            template <typename Visitor, typename T0, typename... Ts, typename... Vs>
            auto apply_nary_visitor_impl(
                Visitor& visitor, Variant<T0, Ts...>& v0, Vs&&... vs
            )
                -> typename Visitor::result_type;

        } // namespace internal

        template <typename Visitor, typename... Variants>
        auto apply_nary_visitor(Visitor&& visitor, Variants&&... vs)
            -> typename Visitor::result_type;

        template <typename R = void>
        struct Visitor {
            typedef R result_type;
        };
    }
}
#endif