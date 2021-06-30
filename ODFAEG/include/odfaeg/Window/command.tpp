namespace odfaeg {
        namespace core {
        template <typename... A>
        void Command::setSigParams (A&&... args) {
            if (trigger != nullptr)
                trigger->setParams(std::forward<A>(args)...);
        }
        template <typename... A>
        void Command::setSlotParams (A&&... args) {
            slot.setParams(std::forward<A>(args)...);
        }
        template <typename... A>
        void Command::bindSigParams (A&&... args) {
            if (trigger != nullptr)
                trigger->bind(std::forward<A>(args)...);
        }
        template <typename... A>
        void Command::bindSlotParams (A&&... args) {
            slot.bind(std::forward<A>(args)...);
        }
    }
}

