namespace odfaeg {
    namespace window {
        ////////////////////////////////////////////////////////////
        /// \brief Get the list of all the supported fullscreen video modes
        ///
        /// \return Array filled with the fullscreen video modes
        ///
        ////////////////////////////////////////////////////////////
        std::vector<VideoMode> VideoModeImpl::getFullscreenModes() {
            return VideoModeImplType::getFullscreenModes();
        }

        ////////////////////////////////////////////////////////////
        /// \brief Get the current desktop video mode
        ///
        /// \return Current desktop video mode
        ///
        ////////////////////////////////////////////////////////////
        VideoMode VideoModeImpl::getDesktopMode() {
            return VideoModeImplType::getDesktopMode();
        }
    }
}