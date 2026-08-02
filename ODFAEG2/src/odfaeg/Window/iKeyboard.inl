namespace odfaeg {
	namespace window {
		bool IKeyboard::isKeyPressed(Key key) {
			return KeyboardType::isKeyPressed(key);
		}
	}
}