 namespace odfaeg {
    namespace core {
        
        template <typename T>
        recursive_wrapper<T>::~recursive_wrapper()
        {
            delete m_t;
        }
        template <typename T>
        template
            <
            typename U,
            typename Dummy
            >
            recursive_wrapper<T>::recursive_wrapper(
                const U& u)
            : m_t(new T(u))
        {
        }
        template <typename T>
        template
            <
            typename U,
            typename Dummy
            >
            recursive_wrapper<T>::recursive_wrapper(U&& u)
            : m_t(new T(std::forward<U>(u))) {
        }
        template <typename T>
        recursive_wrapper<T>::recursive_wrapper(const recursive_wrapper& rhs)
            : m_t(new T(rhs.get())) {
        }
        template <typename T>
        recursive_wrapper<T>::recursive_wrapper(recursive_wrapper&& rhs)
            : m_t(rhs.m_t)
        {
            rhs.m_t = nullptr;
        }
        template <typename T>
        recursive_wrapper<T>&
            recursive_wrapper<T>::operator=(const recursive_wrapper& rhs)
        {
            assign(rhs.get());
            return *this;
        }
        template <typename T>
        recursive_wrapper<T>&
            recursive_wrapper<T>::operator=(recursive_wrapper&& rhs)
        {
            delete m_t;
            m_t = rhs.m_t;
            rhs.m_t = nullptr;
            return *this;
        }
        template <typename T>
        recursive_wrapper<T>&
            recursive_wrapper<T>::operator=(const T& t)
        {
            assign(t);
            return *this;
        }
        template <typename T>
        recursive_wrapper<T>&
            recursive_wrapper<T>::operator=(T&& t)
        {
            assign(std::move(t));
            return *this;
        }
        template <typename T>
        T& recursive_wrapper<T>::get() { return *m_t; }
        template <typename T>
        const T& recursive_wrapper<T>::get() const { return *m_t; }
        template <typename T>
        template <typename U>
        void
            recursive_wrapper<T>::assign(U&& u)
        {
            *m_t = std::forward<U>(u);
        }

        namespace detail
        {
            template <typename T, typename Internal>
            T&
                get_value(T& t, const Internal&)
            {
                return t;
            }

            template <typename T>
            T&
                get_value(recursive_wrapper<T>& t, const false_&)
            {
                return t.get();
            }

            template <typename T>
            const T&
                get_value(const recursive_wrapper<T>& t, const false_&)
            {
                return t.get();
            }
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
                Storage&& storage, Visitor&& visitor, Args&&... args)
        {
            typedef typename std::conditional
                <
                std::is_const<
                typename std::remove_extent<
                typename std::remove_reference<Storage>::type>::type
                >::value,
                const T,
                T
                >::type ConstType;

            return visitor(detail::get_value(*reinterpret_cast<ConstType*>(storage),
                internal), std::forward<Args>(args)...);
        }
        template <typename First, typename... Types>
        template <typename... AllTypes>
        template
            <
            typename Internal,
            typename VoidPtrCV,
            typename Visitor,
            typename... Args
            >
            typename Visitor::result_type
            Variant<First, Types...>::do_visit<AllTypes...>::operator()
            (
                Internal&& internal,
                size_t which,
                VoidPtrCV&& storage,
                Visitor& visitor,
                Args&&... args
                )
        {
            typedef typename Visitor::result_type(*whichCaller)
                (Internal&&, VoidPtrCV&&, Visitor&&, Args&&...);

            static whichCaller callers[sizeof...(AllTypes)] =
            {
                &visitor_caller<Internal&&, AllTypes,
                VoidPtrCV&&, Visitor, Args&&...>...
            }
            ;

            assert(which >= 0 && which < sizeof...(AllTypes));

            return (*callers[which])
                (
                    std::forward<Internal>(internal),
                    std::forward<VoidPtrCV>(storage),
                    std::forward<Visitor>(visitor),
                    std::forward<Args>(args)...
                    );
        }  
        template <typename First, typename... Types>
        Variant<First, Types...>::constructor::constructor(Variant& self)
            : m_self(self)
        {
        }
        template <typename First, typename... Types>
        template <typename T>
        void
        Variant<First, Types...>::constructor::operator()(const T& rhs) const
        {
            m_self.construct(rhs);
        }        
        template <typename First, typename... Types>
        Variant<First, Types...>::move_constructor::move_constructor(Variant& self)
            : m_self(self)
        {
        }
        template <typename First, typename... Types>
        template <typename T>
        void
                Variant<First, Types...>::move_constructor::operator()(T& rhs) const
        {
            m_self.construct(std::move(rhs));
        }           
        template <typename First, typename... Types>
        Variant<First, Types...>::assigner::assigner(Variant& self, int rhs_which)
            : m_self(self), m_rhs_which(rhs_which)
        {
        }
        template <typename First, typename... Types>
        template <typename Rhs>
        void
            Variant<First, Types...>::assigner::operator()(const Rhs& rhs) const
        {
            if (m_self.which() == m_rhs_which)
            {
                //the types are the same, so just assign into the lhs
                *reinterpret_cast<Rhs*>(m_self.address()) = rhs;
            }
            else
            {
                Rhs tmp(rhs);
                m_self.destroy(); //nothrow
                m_self.construct(std::move(tmp)); //nothrow (please)
            }
        }   
        template <typename First, typename... Types>          
        Variant<First, Types...>::move_assigner::move_assigner(Variant& self, int rhs_which)
            : m_self(self), m_rhs_which(rhs_which)
        {
        }
        template <typename First, typename... Types>
        template <typename Rhs>
        void
            Variant<First, Types...>::move_assigner::operator()(Rhs& rhs) const
        {
            typedef typename std::remove_const<Rhs>::type RhsNoConst;
            if (m_self.which() == m_rhs_which)
            {
                //the types are the same, so just assign into the lhs
                *reinterpret_cast<RhsNoConst*>(m_self.address()) = std::move(rhs);
            }
            else
            {
                m_self.destroy(); //nothrow
                m_self.construct(std::move(rhs)); //nothrow (please)
            }
        }
        template <typename First, typename... Types>
        template <typename T>
        void
            Variant<First, Types...>::destroyer::operator()(T& t) const
        {
            t.~T();
        }
        template <typename First, typename... Types>
        template <size_t Which, typename Current, typename... MyTypes>
        void
            Variant<First, Types...>::initialiser<Which, Current, MyTypes...>::initialise(Variant& v, Current&& current)
        {
            v.construct(std::forward<Current>(current));
            v.indicate_which(Which);
        }
        template <typename First, typename... Types>
        template <size_t Which, typename Current, typename... MyTypes>
        void
            Variant<First, Types...>::initialiser<Which, Current, MyTypes...>::initialise(Variant& v, const Current& current)
        {
            v.construct(current);
            v.indicate_which(Which);
        }
        template <typename First, typename... Types>
        Variant<First, Types...>::Variant()
        {
            //try to construct First
            //if this fails then First is not default constructible
            construct(First());
            indicate_which(0);
        }
        template <typename First, typename... Types>
        Variant<First, Types...>::~Variant()
        {
            destroy();
        } 
        template <typename First, typename... Types>
        template
            <
            typename T,
            typename Dummy
            >
            Variant<First, Types...>::Variant(T&& t)
        {
            static_assert(
                !std::is_same<Variant<First, Types...>&, T>::value,
                "why is Variant(T&&) instantiated with a Variant?");

            //compile error here means that T is not unambiguously convertible to
            //any of the types in (First, Types...)
            initialiser<0, First, Types...>::initialise(*this, std::forward<T>(t));
        }
        template <typename First, typename... Types>
        Variant<First, Types...>::Variant(const Variant& rhs)
        {
            constructor c(*this);
            rhs.apply_visitor_internal(c);
            indicate_which(rhs.which());
        }
        template <typename First, typename... Types>
        Variant<First, Types...>::Variant(Variant&& rhs)
        {
            move_constructor mc(*this);
            rhs.apply_visitor_internal(mc);
            indicate_which(rhs.which());
        }
        template <typename First, typename... Types>
        Variant<First, Types...>& Variant<First, Types...>::operator=(const Variant& rhs)
        {
            if (this != &rhs)
            {
                assigner a(*this, rhs.which());
                rhs.apply_visitor_internal(a);
                indicate_which(rhs.which());
            }
            return *this;
        }
        template <typename First, typename... Types>
        Variant<First, Types...>& Variant<First, Types...>::operator=(Variant&& rhs)
        {
            if (this != &rhs)
            {
                move_assigner ma(*this, rhs.which());
                rhs.apply_visitor_internal(ma);
                indicate_which(rhs.which());
            }
            return *this;
        }
        template <typename First, typename... Types>
        int Variant<First, Types...>::which() const { return m_which; }
        template <typename First, typename... Types>
        template <typename Internal, typename Visitor, typename... Args>
        typename Visitor::result_type
            Variant<First, Types...>::apply_visitor(Visitor& visitor, Args&&... args)
        {
            return do_visit<First, Types...>()(Internal(), m_which, m_storage,
                visitor, std::forward<Args>(args)...);
        }
        template <typename First, typename... Types>
        template <typename Internal, typename Visitor, typename... Args>
        typename Visitor::result_type
            Variant<First, Types...>::apply_visitor(Visitor& visitor, Args&&... args) const
        {
            return do_visit<First, Types...>()(Internal(), m_which, m_storage,
                visitor, std::forward<Args>(args)...);
        }
        template <typename First, typename... Types>
        void* Variant<First, Types...>::address() { return m_storage; }
        template <typename First, typename... Types>
        const void* Variant<First, Types...>::address() const { return m_storage; }
        template <typename First, typename... Types>
        template <typename Visitor>
        typename Visitor::result_type
            Variant<First, Types...>::apply_visitor_internal(Visitor& visitor)
        {
            return apply_visitor<true_, Visitor>(visitor);
        }
        template <typename First, typename... Types>
        template <typename Visitor>
        typename Visitor::result_type
            Variant<First, Types...>::apply_visitor_internal(Visitor& visitor) const
        {
            return apply_visitor<true_, Visitor>(visitor);
        }
        template <typename First, typename... Types>
        void Variant<First, Types...>::destroy()
        {
            destroyer d;
            apply_visitor_internal(d);
        }
        template <typename First, typename... Types>
        template <typename T>
        void
            Variant<First, Types...>::construct(T&& t)
        {
            typedef typename std::remove_reference<T>::type type;
            new(m_storage) type(std::forward<T>(t));
        }
        const char* bad_get::what() const throw()
        {
            return "bad_get";
        }  
        template <typename T>
        get_visitor<T>::result_type
            get_visitor<T>::operator()(T& val) const
        {
            return &val;
        }
        template <typename T>
        template <typename U>
        get_visitor<T>::result_type
            get_visitor<T>::operator()(const U& u) const
        {
            return nullptr;
        }       
       
        template <typename Visitor, typename Visitable, typename... Args>
        typename Visitor::result_type
            apply_visitor(Visitor& visitor, Visitable& visitable, Args&&... args)
        {
            return visitable.template apply_visitor<false_>
                (visitor, std::forward<Args>(args)...);
        }

        template <typename Visitor, typename Visitable, typename... Args>
        typename Visitor::result_type
            apply_visitor(const Visitor& visitor, Visitable& visitable, Args&&... args)
        {
            return visitable.template apply_visitor<false_>
                (visitor, std::forward<Args>(args)...);
        }

        template <typename T, typename First, typename... Types>
        T*
            get(Variant<First, Types...>* var)
        {
            return apply_visitor(get_visitor<T>(), *var);
        }

        template <typename T, typename First, typename... Types>
        const T*
            get(const Variant<First, Types...>* var)
        {
            return apply_visitor(get_visitor<const T>(), *var);
        }

        template <typename T, typename First, typename... Types>
        T&
            get(Variant<First, Types...>& var)
        {
            T* t = apply_visitor(get_visitor<T>(), var);
            if (t == nullptr) { throw bad_get(); }

            return *t;
        }

        template <typename T, typename First, typename... Types>
        const T&
            get(const Variant<First, Types...>& var)
        {
            const T* t = apply_visitor(get_visitor<const T>(), var);
            if (t == nullptr) { throw bad_get(); }

            return *t;
        }
        namespace details {
            template <typename Visitor, typename T>
            NaryVisitor<Visitor, T>::NaryVisitor(Visitor& visitor, T& t) : visitor(visitor), ref(t) {}
            template <typename Visitor, typename T>
            template <typename... Args>
            auto NaryVisitor<Visitor, T>::operator()(Args&&... args) -> result_type {
                return visitor(ref, std::forward<Args>(args)...);
            } // apply
            
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
            NaryApplier<Visitor, Variant>::NaryApplier(Visitor& visitor, Variant& variant) :
                visitor(visitor), variant(variant) {
            }
            template <typename Visitor, typename Variant>
            template <typename T>
            auto NaryApplier<Visitor, Variant>::apply() -> result_type {
                return visitor(get<T>(variant));
            }
            template <typename Visitor, typename Variant>
            template <typename T, typename V0, typename... Vs>
            auto NaryApplier<Visitor, Variant>::apply(V0&& v0, Vs&&... vs) -> result_type {
                NaryVisitor<Visitor, T> nary{
                    visitor,
                    get<T>(variant)
                };
                return apply_nary_visitor_impl(nary,
                    std::forward<V0>(v0),
                    std::forward<Vs>(vs)...);
            }
            
            template <typename Visitor, typename Arg, typename... Vs>
            auto apply_nary_visitor_impl(
                Visitor& visitor, Arg& arg, Vs&&... vs
            )
                -> typename Visitor::result_type
            {
                return visitor(arg);
            }
            template <typename Visitor, typename T0, typename... Ts, typename... Vs>
            auto apply_nary_visitor_impl(
                Visitor& visitor, Variant<T0, Ts...>& v0, Vs&&... vs
            )
                -> typename Visitor::result_type
            {
                using result_type = typename Visitor::result_type;

                using Variant = Variant<T0, Ts...>;
                using Applier = details::NaryApplier<Visitor, Variant>;
                using Member = result_type(Applier::*)(Vs&&...);

                static Member const members[] = {
                    (&Applier::template apply<T0, Vs...>),
                    (&Applier::template apply<Ts, Vs...>)...
                };

                Member const m = members[v0.which()];
                Applier a{ visitor, v0 };
                return (a.*m)(std::forward<Vs>(vs)...);
            } // apply_nary_visitor_impl

        } // namespace internal

        template <typename Visitor, typename... Variants>
        auto apply_nary_visitor(Visitor&& visitor, Variants&&... vs)
            -> typename Visitor::result_type
        {
            return details::apply_nary_visitor_impl(visitor,
                std::forward<Variants>(vs)...);
        } // apply_nary_visitor
    }
}