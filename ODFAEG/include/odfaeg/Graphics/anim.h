#ifndef ODFAEG_ANIM_2D_HPP
#define ODFAEG_ANIM_2D_HPP
#include "animatedEntity.h"
#include "entityManager.h"
#include "mesh.hpp"
#include <SFML/System.hpp>
/**
  *\namespace odfaeg
  * the namespace of the Opensource Development Framework Adapted for Every Games.
  */
namespace odfaeg {
    namespace graphic {
        /**
          * \file anim.h
          * \class Anim
          * \brief Represent an animation which multiple frames.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          * Set up an animation, by adding different frames to animations.
          * Keep a pointer to the current frame to be displayed.
          * So each frames can be displayed one after one which the given framerate.
          * The frame must be an entity so it must inherits from the Entity class and a frame can be any type of entity that you want.
          *  Animations can also have parent's animations because animations are also entities.
          *  It's very usefull for bone's animations.
          * (By exemple, if you move the frame of the parent animation, the frame of the bone animation is also moved!
          *   Bone's animations are just child's animations!)
          *  The animations are updated by the AnimationUpdater class.
          * Note : in the next version of ODFAEG, a new class Frame'll be available, and'll contains informations about individual frames with
          * the entities to display for each frames.
          */
          /**
          * \file anim.h
          * \class Anim
          * \brief Represent an animation which multiple frames.
          * \author Duroisin.L
          * \version 1.0
          * \date 1/02/2014
          * Set up an animation, by adding different frames to animations.
          * Keep a pointer to the current frame to be displayed.
          * So each frames can be displayed one after one which the given framerate.
          * The frame must be an entity so it must inherits from the Entity class and a frame can be any type of entity that you want.
          *  Animations can also have parent's animations because animations are also entities.
          *  It's very usefull for bone's animations.
          * (By exemple, if you move the frame of the parent animation, the frame of the bone animation is also moved!
          *   Bone's animations are just child's animations!)
          *  The animations are updated by the AnimationUpdater class.
          * Note : in the next version of ODFAEG, a new class Frame'll be available, and'll contains informations about individual frames with
          * the entities to display for each frames.
          */
        class ODFAEG_GRAPHICS_API Anim : public AnimatedEntity {
            public :
                /** \fn default constructor
                */
                Anim(EntityFactory& factory);
                /**
                *  \fn Anim (float fr, math::Vec3f position, math::Vec3f size, Entity* parent = nullptr)
                *  \brief create an animation, with the given framerate, position and parent.
                *  \param float fr : the framerate of the animation. (The number of frames to display per seconds)
                *  \param math::Vec3f position : the position of the animation. (It corresponds to the minimal position for each frames)
                *  \param Entity* parent : the parent entity of the animation, can be null.
                */
                Anim (float fr, math::Vec3f position, math::Vec3f size, EntityFactory& factory, Entity* parent = nullptr);
                /**
                *  \fn addEntity(Entity* entity)
                *  \brief add a frame into the animation.
                *  \param Entity* entity : the frame to remove.
                */
                void setSelected(bool selected);
                void setLayer(unsigned int layer);
                void addFrame (Entity *entity);
                /**
                *  \fn removeEntity(Entity* entity)
                *  \brief remove a frame from the animation.
                *  \param Entity* entity : the frame to add.
                */
                void removeFrame (Entity *entity);
                /** \fn isCurrentEntityChanged();
                *   \brief tells if the current entity have being changed.
                *   \return true when the current entity is changed.
                */
                bool isCurrentFrameChanged();
                /** \fn setCurrentEntityChanged(bool b);
                *   \brief tells if the current frame have just been changed or not.
                *   \param true if the current entity is changed.
                */
                void setCurrentFrameChanged(bool b);
                /** \fn  play(bool loop)
                *   \brief play the animation, if loop is set to true, the animation is play in loop.
                *   \param bool loop : true if the animation have to be displayed in loop.
                */
                void play (bool loop=false);
                /** \fn stop
                *   \brief stop the animation.
                */
                void stop ();
                /** \fn setFrameRate (float fr);
                *   \brief change the current frame rate of the animation.
                *   \param fr : the framerate of the animation.
                */
                void setFrameRate (float fr);
                /** \fn float getFrameRate()
                *   \brief get the current framerate of the animation.
                *   \return the current frame rate of the animation.
                */
                float getFrameRate ();
                /** \fn sf::Time getElapsedTime()
                *   \brief get the ellapsed time since the last frame has been changed.
                *   \return the ellapsed time since the last frame has been changed.
                */
                sf::Time getElapsedTime ();
                /** \fn resetClock()
                *   \brief reset the clock for measuring elapsed time between several animations.
                */
                void resetClock();
                /** \fn nextImage()
                *   \brief set the next frame of the animation.
                */
                void computeNextFrame();
                /** \fn isRunning()
                *   \return true if the animation is currently playing.
                */
                bool isRunning ();
                /** \fn setCurrentTile(int index);
                *   \brief change the index of the current frame of the animation.
                *   \param the current index of the animation.
                */
                void setCurrentFrame(int index);
                /** \fn getCurrentTileIndex()
                *   \brief get the index of the current frame.
                *   \return the index of the current frame.
                */
                int getCurrentFrameIndex();
                /** \fn getCurrentEntity() const
                *   \brief get the current frame of the animation.
                *   \return Entity* the current frame of the animation.
                */
                Entity* getCurrentFrame () const;
                /** \fn getPreviousEntity()
                *   \brief get the previous frame of the animation.
                *   \return Entity* the previous frame of the animation.
                */
                Entity* getPreviousFrame();
                /** \fn void onDraw(RenderTarget &target, RenderStates states)
                *   \brief draw the current frame of the animation on the given render target.
                *   \param RenderTarget target : the target which draw the frame.
                *   \param RenderStates states : the states to render the frame with. (blendMode, shaders, etc...)
                */
                void onDraw (RenderTarget &target, RenderStates states);
                void onFrameChanged();
                /** \fn bool operator==(Entity& other)
                *   \brief compare two animations.
                *   \param Entity& other : the other entity.
                *   \return true if two entities are true.
                */
                bool operator== (Entity &other);
                /** \fn bool isAnimated()
                *   \brief abstract method of the entity class which must return true if the entity is animated.
                *   \return true because the entity is animated.
                */
                bool isAnimated() const {
                    return true;
                }
                /** \fn bool isAnimated()
                *   \brief abstract method of the entity class which must return true if the entity can have a shadow.
                *   \return false because the animations themselve doesn't generate shadows.
                */
                bool haveShadow() const {
                    return false;
                }
                /** \fn bool isAnimated()
                *   \brief abstract method of the entity class which must return true if the entity can be selected.
                *   \return true because the animations can be selected.
                */
                bool selectable() const {
                    return true;
                }
                /** \fn void vtserialize(Archive & ar)
                *   \brief write the animation onto an archive.
                *   \param Archive & ar : the archive where to put the animations.
                */
                template <typename Archive>
                void vtserialize(Archive & ar) {
                    AnimatedEntity::vtserialize(ar);
                    ar(fr);
                    ar(currentFrame);
                    ar(previousFrame);
                    ar(nextFrame);
                    ar(running);
                    ar(loop);
                    ar(currentFrameChanged);
                    ar(interpolatedFrame);
                }
                Entity* clone();
                void onMove(math::Vec3f& t);
                /**
                *  \fn ~Anim()
                *  destructor.
                */
                virtual ~Anim();
            private :
                void interpolate(Entity* currentFrame, Entity* nextFrame);
                void createFirstInterpolatedFrame(Entity* currentFrame);
                void changeInterpolatedFrame(Entity* currentFrame);
                /** \fn recomputeSize()
                *   \brief recompute the size of an animation.
                */
                void recomputeSize();
                unsigned int interpLevels, interpPerc, currentFrameIndex; /**> the index of the current tile*/
                sf::Clock clock; /**> the clock to measure times before frames.*/
                float fr; /**> the framerate of the animations.*/
                Entity *nextFrame, *currentFrame, *previousFrame; /**> previousCurrentEntity : the previous frame and the current frame of the entity*/
                bool running, loop, currentFrameChanged; /**> tells if the animation is currently playing in loop (or not) and if the current frame have just been changed.*/
                std::unique_ptr<Entity> interpolatedFrame;
        };
    }
}
#endif

