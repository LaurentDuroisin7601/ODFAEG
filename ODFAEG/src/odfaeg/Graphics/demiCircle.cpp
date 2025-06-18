#include "../../../include/odfaeg/Graphics/demiCircle.hpp"
namespace odfaeg {
    namespace graphic {
        DemiCircle::DemiCircle(float radius, float angle, Color color, int quality) : m_radius(radius), m_angle(angle), m_color(color), m_quality(quality) {
            m_vertices.setPrimitiveType(Triangles);
            float a = math::Math::toRadians(angle) / quality;
            Vertex v1 (math::Vec3f(0, 0, 0), color);
            for (unsigned int i = 0; i < quality; i++) {
                Vertex v2(math::Vec3f(radius * math::Math::cosinus(i * a), radius * math::Math::sinus(i * a), 0), color);
                Vertex v3(math::Vec3f(radius * math::Math::cosinus((i+1) * a), radius * math::Math::sinus((i + 1) * a), 0), color);
                m_vertices.append(v1);
                m_vertices.append(v2);
                m_vertices.append(v3);
            }
        }
        void DemiCircle::setRadius(float radius) {
            m_vertices.clear();
            float a = math::Math::toRadians(m_angle) / m_quality;
            Vertex v1 (math::Vec3f(0, 0, 0), m_color);
            for (unsigned int i = 0; i < m_quality; i++) {
                Vertex v2(math::Vec3f(radius * math::Math::cosinus(i * a), radius * math::Math::sinus(i * a), 0), m_color);
                Vertex v3(math::Vec3f(radius * math::Math::cosinus((i+1) * a), radius * math::Math::sinus((i + 1) * a), 0), m_color);
                m_vertices.append(v1);
                m_vertices.append(v2);
                m_vertices.append(v3);
            }
            m_radius = radius;
        }
        void DemiCircle::setAngle(float angle) {
            m_vertices.clear();
            float a = math::Math::toRadians(angle) / m_quality;
            Vertex v1 (math::Vec3f(0, 0, 0), m_color);
            for (unsigned int i = 0; i < m_quality; i++) {
                Vertex v2(math::Vec3f(m_radius * math::Math::cosinus(i * a), m_radius * math::Math::sinus(i * a), 0), m_color);
                Vertex v3(math::Vec3f(m_radius * math::Math::cosinus((i+1) * a), m_radius * math::Math::sinus((i + 1) * a), 0), m_color);
                m_vertices.append(v1);
                m_vertices.append(v2);
                m_vertices.append(v3);
            }
            m_angle = angle;
        }
        float DemiCircle::getAngle() {
            return m_angle;
        }
        float DemiCircle::getRadius() {
            return m_radius;
        }
        void DemiCircle::draw(RenderTarget& target, RenderStates states) {
            states.transform = getTransform();
            target.draw(m_vertices, states);
        }
    }
}

