#include "../../../include/odfaeg/Graphics/billBoard.h"
namespace odfaeg {
    namespace graphic {
        BillBoard::BillBoard(View view, Entity& entity) :
        view(view),
        entity(entity)
        {

        }
        View BillBoard::getView() {
            return view;
        }
        void BillBoard::setView(View view) {
            this->view = view;
            math::Vec3f forward = view.getPosition() - entity.getCenter();
            forward = forward.normalize();
            float angle = math::Math::toDegrees(forward.getAngleBetween(view.getForward(), view.getUp()));
            math::Vec3f left = forward.cross(view.getUp()).normalize();
            math::Vec3f axis = left.cross(forward);
            TransformMatrix &tm = entity.getTransform();
            tm.setRotation(axis, angle);
            tm.update();
        }
    }
}
