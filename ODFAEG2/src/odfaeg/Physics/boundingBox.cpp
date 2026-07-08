module;
#include <array>
#include <vector>
//import odfaeg.physic.boundingBox;
module odfaeg.physic.boundingBox;
import odfaeg.math.computer;
import odfaeg.entity.transformMatrix;
namespace odfaeg {
    namespace physic {
        //Create a bounding box at position 0, 0, 0 and with the dimensions 1, 1, 1.
        BoundingBox::BoundingBox() {
            x = y = z = 0;
            width = height = depth = 1;
            points.resize(24);
            points[0] = math::Vec3f(x, y, z);
            points[1] = math::Vec3f(x + width, y, z);
            points[2] = math::Vec3f(x + width, y + height, z);
            points[3] = math::Vec3f(x, y + height, z);
            points[4] = math::Vec3f(x, y, z + depth);
            points[5] = math::Vec3f(x + width, y, z + depth);
            points[6] = math::Vec3f(x + width, y + height, z + depth);
            points[7] = math::Vec3f(x, y + height, z + depth);
            points[8] = points[4];
            points[9] = points[5];
            points[10] = points[1];
            points[11] = points[0];
            points[12] = points[7];
            points[13] = points[6];
            points[14] = points[2];
            points[15] = points[3];
            points[16] = points[1];
            points[17] = points[5];
            points[18] = points[6];
            points[19] = points[2];
            points[20] = points[0];
            points[21] = points[4];
            points[22] = points[7];
            points[23] = points[3];
            computeVectors();
        }
        //Create a bounding box with the specified , y, z position and of width, height, depth dimensions.
        BoundingBox::BoundingBox(float x, float y, float z, float width, float height, float depth) {
            this->x = x;
            this->y = y;
            this->z = z;
            this->width = width;
            this->height = height;
            this->depth = depth;
            if (depth == 0) {
                points.resize(4);
                points[0] = math::Vec3f(x, y, z);
                points[1] = math::Vec3f(x + width, y, z);
                points[2] = math::Vec3f(x + width, y + height, z);
                points[3] = math::Vec3f(x, y + height, z);
                center = (points[0] + points[1] + points[2] + points[3]) * 0.25f;
            }
            else {
                points.resize(24);
                points[0] = math::Vec3f(x, y, z);
                points[1] = math::Vec3f(x + width, y, z);
                points[2] = math::Vec3f(x + width, y + height, z);
                points[3] = math::Vec3f(x, y + height, z);
                points[4] = math::Vec3f(x, y, z + depth);
                points[5] = math::Vec3f(x + width, y, z + depth);
                points[6] = math::Vec3f(x + width, y + height, z + depth);
                points[7] = math::Vec3f(x, y + height, z + depth);
                points[8] = points[4];
                points[9] = points[5];
                points[10] = points[1];
                points[11] = points[0];
                points[12] = points[7];
                points[13] = points[6];
                points[14] = points[2];
                points[15] = points[3];
                points[16] = points[1];
                points[17] = points[5];
                points[18] = points[6];
                points[19] = points[2];
                points[20] = points[0];
                points[21] = points[4];
                points[22] = points[7];
                points[23] = points[3];
                center = (points[0] + points[1] + points[2] + points[3]
                    + points[4] + points[5] + points[6] + points[7]) * 0.125f;
            }
            flat = (depth == 0) ? true : false;
            computeVectors();
        }
        void BoundingBox::computeVectors() {
            edgeNormals.clear();
            faceNormals.clear();
            edgeBissectors.clear();
            faceBissectors.clear();
            if (depth == 0) {
                math::Vec3f v1 = points[1] - points[0];
                math::Vec3f v2 = points[2] - points[1];
                math::Vec3f v3 = points[3] - points[2];
                math::Vec3f v4 = points[3] - points[0];
                math::Vec3f n = v1.cross(v2).normalize();
                faceNormals.push_back(n);
                math::Vec3f n1 = n.cross(v1).normalize();
                math::Vec3f n2 = n.cross(v2).normalize();
                math::Vec3f n3 = n.cross(v3).normalize();
                math::Vec3f n4 = n.cross(v4).normalize();
                math::Vec3f b1 = (points[0] + points[1]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                math::Vec3f b2 = (points[1] + points[2]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                math::Vec3f b3 = (points[2] + points[3]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                math::Vec3f b4 = (points[3] + points[0]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                faceBissectors.push_back(center);
            }
            else {
                //Front face.
                math::Vec3f v1 = points[1] - points[0];
                math::Vec3f v2 = points[2] - points[1];
                math::Vec3f v3 = points[3] - points[2];
                math::Vec3f v4 = points[3] - points[0];
                math::Vec3f fb = (points[0] + points[1] + points[2] + points[3]) * 0.25f;
                math::Vec3f n = v1.cross(v2).normalize();
                if (n.dot(fb - center) < 0)
                    n = -n;
                faceNormals.push_back(n);
                math::Vec3f n1 = n.cross(v1).normalize();
                math::Vec3f n2 = n.cross(v2).normalize();
                math::Vec3f n3 = n.cross(v3).normalize();
                math::Vec3f n4 = n.cross(v4).normalize();
                math::Vec3f b1 = (points[0] + points[1]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                math::Vec3f b2 = (points[1] + points[2]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                math::Vec3f b3 = (points[2] + points[3]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                math::Vec3f b4 = (points[3] + points[0]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                faceBissectors.push_back(fb);
                //Back face.
                v1 = points[4] - points[5];
                v2 = points[5] - points[6];
                v3 = points[6] - points[7];
                v4 = points[7] - points[4];
                fb = (points[0] + points[4] + points[5] + points[6]) * 0.25f;
                n = v1.cross(v2).normalize();
                if (n.dot(fb - center) < 0)
                    n = -n;
                faceNormals.push_back(n);
                n1 = n.cross(v1).normalize();
                n2 = n.cross(v2).normalize();
                n3 = n.cross(v3).normalize();
                n4 = n.cross(v4).normalize();
                b1 = (points[4] + points[5]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                b2 = (points[5] + points[6]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                b3 = (points[6] + points[7]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                b4 = (points[7] + points[4]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                faceBissectors.push_back(fb);
                //Top face.
                v1 = points[4] - points[5];
                v2 = points[5] - points[1];
                v3 = points[1] - points[0];
                v4 = points[0] - points[4];
                fb = (points[0] + points[1] + points[4] + points[5]) * 0.25f;
                n = v1.cross(v2).normalize();
                if (n.dot(fb - center) < 0)
                    n = -n;
                faceNormals.push_back(n);
                n1 = n.cross(v1).normalize();
                n2 = n.cross(v2).normalize();
                n3 = n.cross(v3).normalize();
                n4 = n.cross(v4).normalize();
                b1 = (points[4] + points[5]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                b2 = (points[5] + points[1]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                b3 = (points[1] + points[0]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                b4 = (points[0] + points[4]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                faceBissectors.push_back(fb);
                //Bottom face.
                v1 = points[7] - points[6];
                v2 = points[6] - points[2];
                v3 = points[2] - points[3];
                v4 = points[3] - points[7];
                fb = (points[2] + points[3] + points[6] + points[7]) * 0.25f;
                n = v1.cross(v2).normalize();
                if (n.dot(fb - center) < 0)
                    n = -n;
                faceNormals.push_back(n);
                n1 = n.cross(v1).normalize();
                n2 = n.cross(v2).normalize();
                n3 = n.cross(v3).normalize();
                n4 = n.cross(v4).normalize();
                b1 = (points[7] + points[6]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                b2 = (points[6] + points[2]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                b3 = (points[2] + points[3]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                b4 = (points[3] + points[7]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                faceBissectors.push_back(fb);
                //Left face.
                v1 = points[1] - points[5];
                v2 = points[5] - points[6];
                v3 = points[6] - points[2];
                v4 = points[2] - points[1];
                fb = (points[1] + points[2] + points[5] + points[6]) * 0.25f;
                n = v1.cross(v2).normalize();
                if (n.dot(fb - center) < 0)
                    n = -n;
                faceNormals.push_back(n);
                n1 = n.cross(v1).normalize();
                n2 = n.cross(v2).normalize();
                n3 = n.cross(v3).normalize();
                n4 = n.cross(v4).normalize();
                b1 = (points[1] + points[5]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                b2 = (points[5] + points[6]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                b3 = (points[6] + points[2]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                b4 = (points[2] + points[1]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                faceBissectors.push_back(fb);
                //Right face.
                v1 = points[1] - points[5];
                v2 = points[5] - points[6];
                v3 = points[6] - points[2];
                v4 = points[2] - points[1];
                fb = (points[1] + points[2] + points[5] + points[6]) * 0.25f;
                n = v1.cross(v2).normalize();
                if (n.dot(fb - center) < 0)
                    n = -n;
                faceNormals.push_back(n);
                n1 = n.cross(v1).normalize();
                n2 = n.cross(v2).normalize();
                n3 = n.cross(v3).normalize();
                n4 = n.cross(v4).normalize();
                b1 = (points[1] + points[5]) * 0.5f;
                if (n1.dot(b1 - center) < 0)
                    n1 = -n1;
                b2 = (points[5] + points[6]) * 0.5f;
                if (n2.dot(b2 - center) < 0)
                    n2 = -n2;
                b3 = (points[6] + points[2]) * 0.5f;
                if (n3.dot(b3 - center) < 0)
                    n3 = -n3;
                b4 = (points[2] + points[1]) * 0.5f;
                if (n4.dot(b4 - center) < 0)
                    n4 = -n4;
                edgeBissectors.push_back(b1);
                edgeBissectors.push_back(b2);
                edgeBissectors.push_back(b3);
                edgeBissectors.push_back(b4);
                edgeNormals.push_back(n1);
                edgeNormals.push_back(n2);
                edgeNormals.push_back(n3);
                edgeNormals.push_back(n4);
                faceBissectors.push_back(fb);
            }
        }        
        bool BoundingBox::isFlat() {
            return flat;
        }
        bool BoundingBox::intersects(BoundingBox& other) {
            if (width == 0 && height == 0 && depth == 0
                || other.width == 0 && other.height == 0 && other.depth == 0)
                return false;
            int hx1 = width * 0.5;
            int hy1 = height * 0.5;
            int hz1 = depth * 0.5;
            int hx2 = other.width * 0.5;
            int hy2 = other.height * 0.5;
            int hz2 = other.depth * 0.5;
            //Check the mins and max medians positions.
            float minX1 = center.x() - hx1, minX2 = other.center.x() - hx2;
            float minY1 = center.y() - hy1, minY2 = other.center.y() - hy2;
            float minZ1 = center.z() - hz1, minZ2 = other.center.z() - hz2;
            float maxX1 = center.x() + hx1, maxX2 = other.center.x() + hx2;
            float maxY1 = center.y() + hy1, maxY2 = other.center.y() + hy2;
            float maxZ1 = center.z() + hz1, maxZ2 = other.center.z() + hz2;
            //If the medians overlap, our two boxes intersects.
            for (int i = 0; i < 3; i++) {
                if (i == 0) {
                    if (minX1 > maxX2 || maxX1 < minX2)
                        return false;
                }
                else if (i == 1) {
                    if (minY1 > maxY2 || maxY1 < minY2)
                        return false;
                }
                else {
                    if (minZ1 > maxZ2 || maxZ1 < minZ2)
                        return false;
                }
            }
            return true;
        }        
        bool BoundingBox::isInside(BoundingBox& other) {
            for (unsigned int i = 0; i < other.getVertices().size(); i++) {
                if (!isPointInside(other.getVertices()[i])) {
                    return false;
                }
            }
            return true;
        }
        //Test if a point is inside our box.
        bool BoundingBox::isPointInside(math::Vec3f point) {
            //Check the min and max values of the medians of our box.
            float minX = center.x() - width * 0.5f;
            float maxX = center.x() + width * 0.5f;
            float minY = center.y() - height * 0.5f;
            float maxY = center.y() + height * 0.5f;
            float minZ = center.z() - depth * 0.5f;
            float maxZ = center.z() + depth * 0.5f;
            //If one of the point is on one of the x, y or z medians, the point is inside the box.
            return (point.x() >= minX && point.x() <= maxX && point.y() >= minY && point.y() <= maxY && point.z() >= minZ && point.z() <= maxZ);
        }        
        //Acceseurs.
        math::Vec3f BoundingBox::getCenter() {
            return center;
        }
        float BoundingBox::getWidth() {
            return width;
        }
        float BoundingBox::getHeight() {
            return height;
        }
        float BoundingBox::getDepth() {
            return depth;
        }
        math::Vec3f BoundingBox::getSize() {
            return math::Vec3f(width, height, depth);
        }
        math::Vec3f BoundingBox::getPosition() {
            return math::Vec3f(x, y, z);
        }
        void BoundingBox::setPosition(int x, int y, int z) {
            math::Vec3f t(x - this->x, y - this->y, z - this->z);
            move(t);
        }
        void BoundingBox::setCenter(int x, int y, int z) {
            math::Vec3f t(x - center.x(), y - center.y(), z - center.z());
            move(t);
        }
        void BoundingBox::setSize(int width, int height, int depth) {
            this->width = width;
            this->height = height;
            this->depth = depth;
            center[0] = x + width * 0.5f;
            center[1] = y + height * 0.5f;
            center[2] = z + depth * 0.5f;
        }
        BoundingBox BoundingBox::transform(entity::TransformMatrix& tm) {
            std::array<math::Vec3f, 8> points;
            //Devant
            points[0] = getPosition();
            points[1] = math::Vec3f(getPosition().x() + getWidth(), getPosition().y(), getPosition().z());
            points[2] = math::Vec3f(getPosition().x() + getWidth(), getPosition().y() + getHeight(), getPosition().z());
            points[3] = math::Vec3f(getPosition().x(), getPosition().y() + getHeight(), getPosition().z());
            //Derri�re
            points[4] = math::Vec3f(getPosition().x(), getPosition().y(), getPosition().z() + getDepth());
            points[5] = math::Vec3f(getPosition().x(), getPosition().y() + getHeight(), getPosition().z() + getDepth());
            points[6] = math::Vec3f(getPosition().x() + getWidth(), getPosition().y() + getHeight(), getPosition().z() + getDepth());
            points[7] = math::Vec3f(getPosition().x() + getWidth(), getPosition().y(), getPosition().z() + getDepth());
            for (unsigned int i = 0; i < points.size(); i++) {
                ////////std::cout<<"point i : "<<points[i]<<std::endl<<tm.getMatrix()<<std::endl;
                points[i] = tm.transform(points[i]);
                
            }
            std::array<std::array<float, 2>, 3> store = math::Computer::getExtends(points);
            BoundingBox bx(store[0][0], store[1][0], store[2][0], store[0][1] - store[0][0], store[1][1] - store[1][0], store[2][1] - store[2][0]);
            return bx;
        }
        BoundingBox BoundingBox::inverseTransform(entity::TransformMatrix& tm) {
            std::array<math::Vec3f, 8> points;
            //Devant
            points[0] = getPosition();
            points[1] = math::Vec3f(getPosition().x() + getWidth(), getPosition().y(), getPosition().z());
            points[2] = math::Vec3f(getPosition().x() + getWidth(), getPosition().y() + getHeight(), getPosition().z());
            points[3] = math::Vec3f(getPosition().x(), getPosition().y() + getHeight(), getPosition().z());
            //Derri�re
            points[4] = math::Vec3f(getPosition().x(), getPosition().y(), getPosition().z() + getDepth());
            points[5] = math::Vec3f(getPosition().x(), getPosition().y() + getHeight(), getPosition().z() + getDepth());
            points[6] = math::Vec3f(getPosition().x() + getWidth(), getPosition().y() + getHeight(), getPosition().z() + getDepth());
            points[7] = math::Vec3f(getPosition().x() + getWidth(), getPosition().y(), getPosition().z() + getDepth());
            for (unsigned int i = 0; i < points.size(); i++) {
                ////////std::cout<<"point i : "<<points[i]<<std::endl<<tm.getMatrix()<<std::endl;
                points[i] = tm.inverseTransform(points[i]);

                //////std::cout<<"transformed point i : "<<points[i]<<std::endl;
            }
            std::array<std::array<float, 2>, 3> store = math::Computer::getExtends(points);
            BoundingBox bx(store[0][0], store[1][0], store[2][0], store[0][1] - store[0][0], store[1][1] - store[1][0], store[2][1] - store[2][0]);
            return bx;
        }
        void BoundingBox::move(math::Vec3f t) {
            entity::TransformMatrix tm;            
            center = center + t;
            tm.setTranslation(center);
            for (unsigned int i = 0; i < points.size(); i++)
                points[i] = tm.transform(points[i]);
            std::array<std::array<float, 2>, 3> extends = math::Computer::getExtends(points);
            x = extends[0][0];
            y = extends[1][0];
            z = extends[2][0];
            width = extends[0][1] - extends[0][0];
            height = extends[1][1] - extends[1][0];
            depth = extends[2][1] - extends[2][0];
            computeVectors();
        }
        void BoundingBox::scale(math::Vec3f s) {
            entity::TransformMatrix tm;
            tm.setScale(s);            
            tm.setTranslation(center);
            for (unsigned int i = 0; i < points.size(); i++)
                points[i] = tm.transform(points[i]);
            std::array<std::array<float, 2>, 3> extends = math::Computer::getExtends(points);
            x = extends[0][0];
            y = extends[1][0];
            z = extends[2][0];
            width = extends[0][1] - extends[0][0];
            height = extends[1][1] - extends[1][0];
            depth = extends[2][1] - extends[2][0];
            computeVectors();
        }
        std::vector<math::Vec3f> BoundingBox::getVertices() {
            return points;
        }
        std::vector<math::Vec3f> BoundingBox::getEdgeBissectors() {
            return edgeBissectors;
        }
        std::vector<math::Vec3f> BoundingBox::getEdgeNormals() {
            return edgeNormals;
        }
        std::vector<math::Vec3f> BoundingBox::getFaceBissectors() {
            return faceBissectors;
        }
        std::vector<math::Vec3f> BoundingBox::getFaceNormals() {
            return faceNormals;
        }       
    }
}
