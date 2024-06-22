#ifndef ODFAEG_VIEW_HPP
#define ODFAEG_VIEW_HPP
#include <SFML/Graphics/Rect.hpp>
#include "../Math/vec4.h"
#include "projMatrix.h"
#include "viewMatrix.h"
//#include "../Math/transformMatrix.h"
#include "viewportMatrix.h"
#include "../Physics/boundingBox.h"
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {

        /**
          * \file view.h
          * \class View
          * \brief Represent a view.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          *  This is the class for every 3D or 2D views, view can be moved, zoomed, rotated, etc...
          *  We can also change the axis direction and the axis units by using the setScale function.
          */
        class ODFAEG_GRAPHICS_API View {
            friend class RenderTarget;
        public :
            /**
            * \fn View()
            * \brief constructor.
            */
            View ();
            /**
            * \fn View (double width, double height, double zNear, double zFar, Vec3f up)
            * \brief constructor. (construct a 2D view)
            * \param width : the width of the view.
            * \param height : the height of the view.
            * \param zNear : the z near of the view.
            * \param zFar : the z far of the view.
            * \param up : the up vector of the view.
            */
            View (double width, double height, double zNear, double zFar);
            /**
            * \fn View (double width, double height, double zNear, double zFar, Vec3f up, Vec3f up)
            * \brief constructor. (construct a 3D view)
            * \param width : the width of the view.
            * \param height : the height of the view.
            * \param zNear : the z near of the view.
            * \param zFar : the z far of the view.
            * \param up : the up vector of the view.
            */
            View (double width, double height, double fovy, double zNear, double zFar);
            /**
            * \fn void move (float x, float y, float z)
            * \brief move the view with the given x, y and z translations.
            * \param x : the x.
            * \param y : the y.
            * \param z : the z.
            */
            void move(float x, float y, float z);
            /**
            * \fn void move (Vec3f dir, float delta);
            * \brief move the view in the given direction, with the given delta.
            * \param dir : the direction.
            * \param delta : the translation.
            */
            void move (math::Vec3f dir, float delta);
            /**
            * \fn void rotate (float teta, float phi);
            * \brief move the view with the given angles in polar coordinates.
            * \param teta : the teta angle.
            * \param phi : the phi angle.
            */
            void rotate (float teta, float phi);
            /**
            * \fn void rotate (float gamma)
            * \brief perfom a rotation around the z axis of the view.
            * \param the gamma.
            */
            void rotate (float gamma);
            /**
            * \fn void lookAt (float x, float y, float z)
            * \brief make the view look at a point in space.
            * \param x : the x.
            * \param y : the y.
            * \param z : the z.
            */
            void lookAt (float x, float y, float z);
            void setCenter(math::Vec3f center);
            /**
            * \fn Vec3f getForward()
            * \brief get the forward vector of the view.
            * \return the forward vector of the view.
            */
            math::Vec3f getForward();
            /**
            * \fn Vec3f getLeft()
            * \brief get the left vector of the view.
            * \return the left vector of the view.
            */
            math::Vec3f getLeft();
            /**
            * \fn Vec3f getUp()
            * \brief get the up vector of the view.
            * \return the up vector of the view.
            */
            math::Vec3f getUp();
            /**
            * \fn float getTeta()
            * \brief get the teta angle of the view.
            * \return the teta angle of the view.
            */
            float getTeta();
            /**
            * \fn float getPhi();
            * \brief get the phi angle of the view.
            * \return the phi angle of the view.
            */
            float getPhi();
            /**
            * \fn ViewMatrix getViewMatrix()
            * brief get the view matrix of the view.
            * \return the view matrix of the view.
            */
            ViewMatrix getViewMatrix();
            /**
            * \fn ProjMatrix getProjMatrix()
            * brief get the projection matrix of the view.
            * \return the projection matrix of the view.
            */
            ProjMatrix getProjMatrix();
            /**
            * \fn sf::FloatRect getViewport ()
            * \brief get the viewport of the view.
            * \param the viewport of the view.
            */
            physic::BoundingBox getViewport ();
            /**
            * \fn  void reset(sf::FloatRect rect);
            * \brief reset the rect of the view.
            * \param the rectangle of the view.
            */
            void reset(physic::BoundingBox rect);
            /**
            *   \fn Vec3f getSize()
            *   \brief get the size of the view.
            *   \return the size of the view.
            */
            math::Vec3f getSize();
            /**
            *   \fn Vec3f getPosition()
            *   \brief get the position of the view.
            *   \return the position of the view.
            */
            math::Vec3f getPosition();
            /**
            * \fn void setScale(float x, float y, float z);
            * \brief change the units and the orientations of the view axes.
            * \param x the x factor.
            * \param y the y factor.
            * \param z the z factor.
            */
            void setScale(float x, float y, float z);
            /**
            * \fn Vec3f getScale();
            * \brief get the scale of the view.
            * \param the scale of the view.
            */
            math::Vec3f getScale();
            /**
            * \fn BoundingBox getViewVolume();
            * \brief get the volume of the view.
            * \return the volume of the view.
            */
            physic::BoundingBox getViewVolume();
            /**
            * \fn BoundingBox getFrustum();
            * \brief get the frustum of the view.
            * \return the frustum of the view.
            */
            physic::BoundingBox getFrustum();
            void setConstrains(float lockTeta, float lockPhi);
            void update();
            float getDepth();
            float getGamma();
            void setFlipX(bool flipX);
            void setFlipY(bool flipY);
            bool isXFlipped();
            bool isYFlipped();
            void updated();
             /**
            * \fn void setPerspective(double left, double right, double top, double bottom, double zNear, double zFar);
            * \brief change the perspective of the view with the given frustum. (2D view)
            * \param left : the left.
            * \param right : the right.
            * \param top : the top.
            * \param bottom : the bottom.
            * \param zNear : the zNear.
            * \param zFar : the zFar.
            * \param up : the up vector of the view.
            */
            void setPerspective(double left, double right, double top, double bottom, double zNear, double zFar);
            /**
            * \fn void setPerspective (double left, double right, double top, double bottom, double fovy, double aspect, double zNear, double zFar);
            * \brief change the perspective of the view with the given frustum. (3D view)
            * \param left : the left.
            * \param right : the right.
            * \param top : the top.
            * \param bottom : the bottom.
            * \param fovy : the fovy of the view.
            * \param aspect : the aspect of the view.
            * \param zNear : the z near of the view.
            * \param zFar : the z far of the view.
            */
            void setPerspective (double left, double right, double top, double bottom, double fovy, double aspect, double zNear, double zFar);

            /**
            * \fn void computeVectorsFromAngles();
            * \brief compute the vectors from angles to pass form polar coordinates to cartesian coordinates.
            */
            bool isOrtho();
        private :

            void computeVectorsFromAngles();

            /**
            * \fn void computeAnglesFromVectors();
            * \brief compute the angles from vectors to pass form cartesian coordinates to polar coordinates.
            */
            void computeAnglesFromVectors();
            bool viewUpdated, ortho; /**> if the view matrix has been updated.*/
            ProjMatrix projMatrix; /**> The projection matrix.*/
            ViewMatrix viewMatrix; /**> The view matrix.*/
            math::Vec3f position, up, target, forward, left; /**> The position, the up vector, the target, the forward and the left vector of the view.*/
            float teta, gamma, phi, zoomFactor; /**> The angles and the zoom factor of the view.*/
            physic::BoundingBox viewport; /**the viewport of the view.*/
            float depth; /**> the depth of the view. (The distance between the near and the far plane. */
            float lockTeta, lockPhi;
            bool flipX, flipY;
        };
    }
}

#endif // 3D_VIEW
