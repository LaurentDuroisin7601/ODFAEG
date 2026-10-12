namespace odfaeg {
    namespace entity {
        Wall::Wall(Type type) : type(type), GameObject("E_WALL") {
        }
        Wall::Wall(Type wallType, math::Vec3f position, math::Vec3f size, math::Vec3f origin, std::string type, GameObject* parent) : type(wallType), GameObject(position, size, origin, "E_WALL", "", parent) {
        }
        bool Wall::operator==(GameObject& other) {
            return GameObject::operator==(other) && type == static_cast<Wall&>(other).type;
        }
        GameObject* Wall::clone() {
            Wall* w = new Wall(type, getPosition(), getSize(), getOrigin(), "E_WALL", getParent());
            GameObject::copy(w);
            return w;
        }
        Wall::Type Wall::getWallType() {
            return type;
        }
    }
}