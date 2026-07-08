module;
//import odfaeg.graphic.projMatrix;
module odfaeg.graphic.projMatrix;
import odfaeg.math.matrix;
import odfaeg.math.vec;
namespace odfaeg {
	namespace graphic {
        ProjMatrix::ProjMatrix() {

        }
        
        void ProjMatrix::reset() {
            matrix4f.identity();
        }

        void ProjMatrix::setOrthoMatrix(double l, double r, double b, double t, double n, double f) {
            matrix4f[0][0] = 2 / (r - l);
            matrix4f[0][1] = 0;
            matrix4f[0][2] = 0;
            matrix4f[0][3] = ((r + l) / (r - l));
            matrix4f[1][0] = 0;
            matrix4f[1][1] = 2 / (t - b);
            matrix4f[1][2] = 0;
            matrix4f[1][3] = ((t + b) / (t - b));
            matrix4f[2][0] = 0;
            matrix4f[2][1] = 0;
            matrix4f[2][2] = 1 / (f - n);
            matrix4f[2][3] = -n / (f - n);
            matrix4f[3][0] = 0;
            matrix4f[3][1] = 0;
            matrix4f[3][2] = 0;
            matrix4f[3][3] = 1;            
            invMatrix4f = matrix4f.inverse();
        }        
        void ProjMatrix::setPerspectiveMatrix(double l, double r, double b, double t, double n, double f) {
            matrix4f[0][0] = (2 * n) / (r - l);
            matrix4f[0][1] = 0;
            matrix4f[0][2] = (r + l) / (r - l);
            matrix4f[0][3] = 0;
            matrix4f[1][0] = 0;
            matrix4f[1][1] = (2 * n) / (t - b);
            matrix4f[1][2] = (t + b) / (t - b);
            matrix4f[1][3] = 0;
            matrix4f[2][0] = 0;
            matrix4f[2][1] = 0;
            matrix4f[2][2] = f / (f - n);
            matrix4f[2][3] = -f * n / (f - n);
            matrix4f[3][0] = 0;
            matrix4f[3][1] = 0;
            matrix4f[3][2] = 1;
            matrix4f[3][3] = 0;            
            invMatrix4f = matrix4f.inverse();
        }        
        math::Matrix4f ProjMatrix::getMatrix() {
            return matrix4f;
        }
        //Projette un vecteur suivant la matrice de projection d�finie.
        math::Vec4f ProjMatrix::project(math::Vec4f vec4) {
            vec4[3] = 1;
            return matrix4f * vec4;
        }
        //D�projette un vecteur suivant la matrice de projection d�finie.
        math::Vec4f ProjMatrix::unProject(math::Vec4f vec4) {
            vec4[3] = 1;            
            return invMatrix4f * vec4;
        }        
    }
}

	