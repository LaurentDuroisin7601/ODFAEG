#include "../../../include/odfaeg/Math/matrix2.h"
namespace odfaeg {
    namespace math {
        using namespace std;
        Matrix2f::Matrix2f () {
            m11 = m22 = 1;
            m12 = m21 = 0;
        }
        Matrix2f::Matrix2f (float m11, float m12, float m21, float m22) {
            this->m11 = m11;
            this->m12 = m12;
            this->m21 = m21;
            this->m22 = m22;
        }
        void Matrix2f::identity () {
            m11 = 1.f;
            m12 = 0.f;
            m21 = 0.f;
            m22 = 1.f;
        }
        void Matrix2f::zero() {
            m11 = m12 = m21 = m22 = 0.f;
        }
        void Matrix2f::set (float m11, float m12, float m21, float m22) {
            this->m11 = m11;
            this->m12 = m12;
            this->m21 = m21;
            this->m22 = m22;
        }
        Matrix2f Matrix2f::operator+ (const Matrix2f &other) {
            return Matrix2f (m11 + other.m11, m12 + other.m12, m21 + other.m21, m22 + other.m22);
        }
        Matrix2f Matrix2f::operator- (const Matrix2f &other) {
            return Matrix2f (m11 - other.m11, m12 - other.m12, m21 - other.m21, m22 - other.m22);
        }
        Matrix2f Matrix2f::operator* (const Matrix2f &other) {
            float m11 = this->m11 * other.m11 + this->m12 * other.m21;
            float m12 = this->m11 * other.m12 + this->m12 * other.m22;
            float m21 = this->m21 * other.m11 + this->m22 * other.m21;
            float m22 = this->m21 * other.m12 + this->m22 * other.m22;
            return Matrix2f (m11, m12, m21, m22);
        }
        Vec2f Matrix2f::operator* (const Vec2f &vec2f) {
            float x = vec2f.x * m11 + vec2f.y * m12;
            float y = vec2f.x * m21 + vec2f.y * m22;
            return Vec2f (x, y);
        }
        Matrix2f Matrix2f::operator* (const float scalar) {
            return Matrix2f (m11 * scalar, m12 * scalar, m21 * scalar, m22 * scalar);
        }
        Matrix2f& Matrix2f::operator= (const Matrix2f &other) {
            m11 = other.m11;
            m12 = other.m12;
            m21 = other.m21;
            m22 = other.m22;
            return *this;
        }
        Matrix2f Matrix2f::inverse () {
            float det = getDet();
            Matrix2f tmp;
            if (Math::abs(det) < 0.f) {
                return tmp;
            }
            else {
                tmp.m11 = this->m22;
                tmp.m12 = -this->m12;
                tmp.m21 = -this->m21;
                tmp.m22 = this->m11;
                float invDet = 1.f / det;
                return tmp * invDet;
            }
        }
        float Matrix2f::getDet() const {
            return m11 * m22 - m12 * m21;
        }
        ostream& operator<< (ostream &out, const Matrix2f &mat2) {
            out<<mat2.m11<<" "<<mat2.m12<<endl;
            out<<mat2.m21<<" "<<mat2.m22<<endl;
            return out;
        }
    }
}
