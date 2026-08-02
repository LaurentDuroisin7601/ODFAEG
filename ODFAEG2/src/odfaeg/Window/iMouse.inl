namespace odfaeg {
    namespace window {
        bool IMouse::isButtonPressed(Button button) {
            return MouseType::isButtonPressed(button);
        }
        math::Vector2i IMouse::getPosition() {
            return MouseType::getPosition();
        }
        math::Vector2i IMouse::getPosition(const Window& window) {
            return MouseType::getPosition(window.getImpl().getImplType());
        }
    }
}