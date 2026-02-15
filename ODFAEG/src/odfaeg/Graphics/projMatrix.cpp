#include "../../../include/odfaeg/Graphics/projMatrix.h"
/*Matrice de projection*/
namespace odfaeg {
    namespace graphic {
        ProjMatrix::ProjMatrix () {

        }
        //Réinitialise la matrice de projection.
        void ProjMatrix::reset () {
            matrix4f.identity ();
        }

        void ProjMatrix::setGlOrthoMatrix (double l, double r, double b, double t, double n, double f) {
            matrix4f[0][0] = -2 / (r - l);
            matrix4f[0][1] = 0;
            matrix4f[0][2] = 0;
            matrix4f[0][3] = -((r + l) / (r - l));
            matrix4f[1][0] = 0;
            #ifndef VULKAN
            matrix4f[1][1] = -2 / (t - b);
            matrix4f[1][2] = 0;
            matrix4f[1][3] = -((t + b) / (t - b));
            #else
            matrix4f[1][1] = 2 / (t - b);
            matrix4f[1][2] = 0;
            matrix4f[1][3] = ((t + b) / (t - b));
            #endif
            matrix4f[2][0] = 0;
            matrix4f[2][1] = 0;
            #ifndef VULKAN
            matrix4f[2][2] = -2 / (f - n);
            matrix4f[2][3] = -(f + n) / (f - n);
            #else
            matrix4f[2][2] = -1 / (n - f);
            matrix4f[2][3] = -f / (n - f);
            #endif
            matrix4f[3][0] = 0;
            matrix4f[3][1] = 0;
            matrix4f[3][2] = 0;
            matrix4f[3][3] = 1;
            this->l = l;
            this->r = r;
            this->b = b;
            this->t = t;
            this->n = n;
            this->f = f;
            invMatrix4f = matrix4f.inverse();
        }
        /*void ProjMatrix::setGlPerspectiveMatrix (double aspect, double tanHalFovy, double n, double f) {
            matrix4f.m11 = 1.0 / (aspect * tanHalFovy);
            matrix4f.m12 = 0;
            matrix4f.m13 = 0;
            matrix4f.m14 = 0;
            matrix4f.m21 = 0;
            matrix4f.m22 = 1.0 / tanHalFovy;
            matrix4f.m23 = 0;
            matrix4f.m24 = 0;
            matrix4f.m31 = 0;
            matrix4f.m32 = 0;
            #ifndef VULKAN
            matrix4f.m33 = (f + n) / (f - n);
            matrix4f.m34 = (2 * f * n) / (f - n);
            #else
            matrix4f.m33 = f / (f - n);
            matrix4f.m34 = (f * n) / (f - n);
            #endif
            matrix4f.m41 = 0;
            matrix4f.m42 = 0;
            matrix4f.m43 = -1;
            matrix4f.m44 = 0;
            this->l = l;
            this->r = r;
            this->b = b;
            this->t = t;
            this->n = n;
            this->f = f;
            invMatrix4f = matrix4f.inverse();
        }*/
        void ProjMatrix::setGlPerspectiveMatrix (double l, double r, double b, double t, double n, double f) {
            matrix4f[0][0] = (2 * n) / (r - l);
            matrix4f[0][1] = 0;
            matrix4f[0][2] = (r + l) / (r - l);
            matrix4f[0][3] = 0;
            matrix4f[1][0] = 0;
            #ifndef VULKAN
            matrix4f[1][1] = (2 * n) / (t - b);
            matrix4f[1][2] = (t + b) / (t - b);
            #else
            matrix4f[1][1] = -(2 * n) / (t - b);
            matrix4f[1][2] = -(t + b) / (t - b);
            #endif
            matrix4f[1][3] = 0;
            matrix4f[2][0] = 0;
            matrix4f[2][1] = 0;
            #ifndef VULKAN
            matrix4f[2][2] = ((f + n) / (f - n));
            matrix4f[2][3] = (2 * f * n) / (f - n);
            #else
            matrix4f[2][2] = n / (f - n);
            matrix4f[2][3] = (n * f) / (f - n);
            #endif
            matrix4f[3][0] = 0;
            matrix4f[3][1] = 0;
            matrix4f[3][2] = -1;
            matrix4f[3][3] = 0;
            this->l = l;
            this->r = r;
            this->b = b;
            this->t = t;
            this->n = n;
            this->f = f;
            invMatrix4f = matrix4f.inverse();
        }
        float* ProjMatrix::getGlMatrix () {
            float* matrix = new float[16];
            matrix[0] = matrix4f[0][0];
            matrix[1] = matrix4f[1][0];
            matrix[2] = matrix4f[2][0];
            matrix[3] = matrix4f[3][0];
            matrix[4] = matrix4f[0][1];
            matrix[5] = matrix4f[1][1];
            matrix[6] = matrix4f[2][1];
            matrix[7] = matrix4f[3][1];
            matrix[8] = matrix4f[0][2];
            matrix[9] = matrix4f[1][2];
            matrix[10] = matrix4f[2][2];
            matrix[11] = matrix4f[3][2];
            matrix[12] = matrix4f[0][3];
            matrix[13] = matrix4f[1][3];
            matrix[14] = matrix4f[2][3];
            matrix[15] = matrix4f[3][3];
            return matrix;
        }
        math::Matrix4f ProjMatrix::getMatrix() {
            return matrix4f;
        }
        //Projette un vecteur suivant la matrice de projection définie.
        math::Vec4f ProjMatrix::project (math::Vec4f vec4) {
            vec4[3] = 1;
            return matrix4f * vec4;
        }
        //Déprojette un vecteur suivant la matrice de projection définie.
        math::Vec4f ProjMatrix::unProject (math::Vec4f vec4) {
            vec4[3] = 1;
            ////////std::cout<<"matrix : "<<matrix4f<<"inv matrix : "<<invMatrix4f<<std::endl;
            return invMatrix4f * vec4;
        }
        physic::BoundingBox ProjMatrix::getFrustum() {
            return physic::BoundingBox(l,b,0,(r - l),(t - b), f);
        }
    }
}
