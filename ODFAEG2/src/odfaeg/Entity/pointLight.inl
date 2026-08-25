namespace odfaeg {
    namespace entity {
        PointLight(math::Vec3f position, Color = entity::Color::Yellow) : GameObject("E_POINT_LIGHT") {
            this->position = position;
        }
        void PointLight::setLightSpace(std::array<math::Matrix4f, 6> lightSpace) {
            this->lightSpace = lightSpace;
        }
        std::array<math::Matrix4f, 6> PointLight::getLightSpace() {
            return lightSpace;
        }
        void PointLight::setShadowMapId(int shadowMapId) {
            this->shadowMapId = shadowMapId;
        }
        int PointLight::getShadowMapId() {
            return shadowMapId;
        }
        GameObject* PointLight::clone() {
            PointLight* pl = new PointLight(position);
            GameObject::copy(pl);
            return pl;
        }
        math::Vec3f PointLight::getPosition() {
            return position;
        }
    }
}