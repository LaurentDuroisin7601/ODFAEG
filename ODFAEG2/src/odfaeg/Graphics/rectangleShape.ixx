module;
export module odfaeg.graphic.rectangleShape;
import odfaeg.graphic.shape;
import odfaeg.graphic.device;
import odfaeg.math.vec;
namespace odfaeg {
	namespace graphic {
        export class RectangleShape : public Shape {
        public:

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// \param size Size of the rectangle
            ///
            ////////////////////////////////////////////////////////////
            explicit RectangleShape(Device& device, const math::Vec3f& size = math::Vec3f(0.f, 0.f, 0.f));
            //void onScale(Vec3f& s);
            ////////////////////////////////////////////////////////////
            /// \brief Get the size of the rectangle
            ///
            /// \return Size of the rectangle
            ///
            /// \see setSize
            ///
            ////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////
            /// \brief Get the number of points defining the shape
            ///
            /// \return Number of points of the shape
            ///
            ////////////////////////////////////////////////////////////
            virtual unsigned int getPointCount() const;

            ////////////////////////////////////////////////////////////
            /// \brief Get a point of the shape
            ///
            /// The result is undefined if \a index is out of the valid range.
            ///
            /// \param index Index of the point to get, in range [0 .. getPointCount() - 1]
            ///
            /// \return Index-th point of the shape
            ///
            ////////////////////////////////////////////////////////////
            virtual math::Vec3f getPoint(unsigned int index) const;
            
            void onResize(math::Vec3f& scale);
            void onScale(math::Vec3f& scale);
        private:
            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            math::Vec3f m_size; ///< Size of the rectangle
        };		
	}
}