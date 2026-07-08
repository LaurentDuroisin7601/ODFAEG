module;
export module odfaeg.core.nonCopyable;
export namespace odfaeg {
    namespace core {
        class NonCopyable
        {
        protected:

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// Because this class has a copy constructor, the compiler
            /// will not automatically generate the default constructor.
            /// That's why we must define it explicitly.
            ///
            ////////////////////////////////////////////////////////////
            NonCopyable() {}

            ////////////////////////////////////////////////////////////
            /// \brief Default destructor
            ///
            /// By declaring a protected destructor it's impossible to
            /// call delete on a pointer of sf::NonCopyable, thus
            /// preventing possible resource leaks.
            ///
            ////////////////////////////////////////////////////////////
            ~NonCopyable() {}

        private:

            ////////////////////////////////////////////////////////////
            /// \brief Disabled copy constructor
            ///
            /// By making the copy constructor private, the compiler will
            /// trigger an error if anyone outside tries to use it.
            /// To prevent NonCopyable or friend classes from using it,
            /// we also give no definition, so that the linker will
            /// produce an error if the first protection was inefficient.
            ///
            ////////////////////////////////////////////////////////////
            NonCopyable(const NonCopyable&) = delete;

            ////////////////////////////////////////////////////////////
            /// \brief Disabled assignment operator
            ///
            /// By making the assignment operator private, the compiler will
            /// trigger an error if anyone outside tries to use it.
            /// To prevent NonCopyable or friend classes from using it,
            /// we also give no definition, so that the linker will
            /// produce an error if the first protection was inefficient.
            ///
            ////////////////////////////////////////////////////////////
            NonCopyable& operator=(const NonCopyable&) = delete;
        };
    }

} // namespace sf