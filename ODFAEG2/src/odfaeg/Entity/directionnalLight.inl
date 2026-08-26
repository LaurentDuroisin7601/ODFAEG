namespace odfaeg {
    namespace entity {
        DirectionnalLight::DirectionnalLight(math::Vec3f direction, Color color) : GameObject("E_DIRECTIONNAL_LIGHT") {
            this->dir = direction;
        }
        void DirectionnalLight::setLightSpace(std::array<math::Matrix4f, NB_CASCADES+1> lightSpace) {
            this->lightSpace = lightSpace;
        }
        std::array<math::Matrix4f, NB_CASCADES+1> DirectionnalLight::getLightSpace() {
            return lightSpace;
        }
        void DirectionnalLight::setShadowMapId(int shadowMapId) {
            this->shadowMapId = shadowMapId;
        }
        int DirectionnalLight::getShadowMapId() {
            return shadowMapId;
        }
        math::Vec3f DirectionnalLight::getDir() {
            return dir;
        }        
        GameObject* DirectionnalLight::clone() {
            DirectionnalLight* dl = new DirectionnalLight(dir);
            GameObject::copy(dl);
            return dl;
        }
    }
}