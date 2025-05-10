#ifndef DEMI_CIRCLE_HPP
#define DEMI_CIRCLE_HPP
#include "vertexArray.h"
#include "transformable.h"
#include "renderTarget.h"
namespace odfaeg {
    namespace graphic {
        class ODFAEG_GRAPHICS_API DemiCircle : public Drawable, public Transformable {
            public :
            explicit DemiCircle(float radius = 0, float angle = 0, sf::Color color = sf::Color::White, int quality = 16);
            void setAngle(float angle);
            float getAngle();
            void setRadius(float radius);
            ////////////////////////////////////////////////////////////
            /// \brief Get the radius of the circle
            ///
            /// \return Radius of the circle
            ///
            /// \see setRadius
            ///
            ////////////////////////////////////////////////////////////
            float getRadius();
            void draw(RenderTarget& target, RenderStates states);
        private :

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            float        m_radius, m_angle;     ///< Radius of the circle
            sf::Color m_color;
            VertexArray m_vertices;
            int m_quality;
        };
    }
}
#endif // DEMI_CIRCLE_HPP
