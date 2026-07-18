module;
#include <string>
//import odfaeg.graphic.model;
module odfaeg.graphic.model;
namespace odfaeg {
    namespace entity {
        Model::Model(): GameObject(math::Vec3f(0.f, 0.f, 0.f), math::Vec3f(0.f, 0.f, 0.f), math::Vec3f(0.f, 0.f, 0.f), "E_MODEL", "", nullptr) {

        }
        Model::Model (math::Vec3f position, math::Vec3f size, math::Vec3f origin, std::string type, GameObject *parent)
            : GameObject(position, size, origin, type, "", parent) {
        }
        GameObject* Model::clone() {
            Model* model = new Model();
            GameObject::copy(model);
            return model;
        }
        bool Model::operator==(GameObject& other) {
            return GameObject::operator==(other);
        }
    }
}//
// Created by laurent on 26/05/2026.
//
