module;
#include <functional>
#include <type_traits>
export module odfaeg.math.distribution;



export namespace odfaeg
{
    namespace math {

        template <typename T>
        class Distribution;

        namespace detail
        {

            // Functor that returns always the same value (don't use lambda expression because of Clang compiler bug)
            template <typename T>
            struct Constant
            {
                explicit Constant(T value)
                    : value(value)
                {
                }

                T operator() () const
                {
                    return value;
                }

                T value;
            };

            // Metafunction for SFINAE and reasonable compiler errors
            template <typename Fn, typename T>
            struct IsCompatibleFunction
            {
                // General case: Fn is a functor/function -> if it's not convertible to T (and thus not a constant), accept it
                static const bool value = !std::is_convertible<Fn, T>::value;
            };

            template <typename U, typename T>
            struct IsCompatibleFunction<Distribution<U>, T>
            {
                // If Fn is another Distribution<U>, accept it iff U is convertible to T (like all functors, but clearer error message)
                static const bool value = std::is_convertible<U, T>::value;
            };

        } // namespace detail

        // ---------------------------------------------------------------------------------------------------------------------------


        /// @addtogroup Math
        /// @{

        /// @brief Class holding a rule to create values with predefined properties
        /// @details Contains a callback that returns values on demand. These can be constant (always the same value), according to a
        /// random distribution, or be read from a value elsewhere in your code. Generally, the callback can be any function, member
        /// function or functor returning a value of type T and taking no arguments.
        /// @n@n thor::Distribution can be linked to random distributions of the standard library:
        /// @code
        /// std::mt19937 engine;
        /// std::uniform_int_distribution<int> distr(0, 7);
        /// auto randomizer = std::bind(distr, engine);
        ///
        /// thor::Distribution<int> thorDistr(randomizer);
        /// @endcode
        template <typename T>
        class Distribution
        {
            // ---------------------------------------------------------------------------------------------------------------------------
            // Private types
        private:
            typedef std::function<T()> FactoryFn;


            // ---------------------------------------------------------------------------------------------------------------------------
            // Public member functions
        public:
            /// @brief Construct from constant
            /// @param constant Constant value convertible to T. The distribution's operator() will always return this value.
            template <typename U>
            Distribution(U constant,
                typename std::enable_if<std::is_convertible<U, T>::value>::type* = nullptr)
                : mFactory(detail::Constant<T>(constant))
            {
            }

            /// @brief Construct from distribution function
            /// @param function Callable convertible to std::function<T()>. Every time the operator() of this distribution is invoked,
            /// it returns the return value of the specified function.
            template <typename Fn>
            Distribution(Fn function,
                typename std::enable_if<detail::IsCompatibleFunction<Fn, T>::value>::type* = nullptr)
                : mFactory(function)
            {
            }

            /// @brief Returns a value according to the distribution.
            ///
            T	operator() () const
            {
                return mFactory();
            }

            /// @brief Exchanges the contents of *this with other.
            ///
            void	swap(Distribution<T>& other)
            {
                mFactory.swap(other.mFactory);
            }


            // ---------------------------------------------------------------------------------------------------------------------------
            // Private variables
        private:
            FactoryFn	mFactory;
        };

        /// @relates Distribution
        /// @brief Swaps two Distribution<T> instances.
        template <typename T>
        void swap(Distribution<T>& lhs, Distribution<T>& rhs)
        {
            lhs.swap(rhs);
        }
    }
    /// @}

}