namespace odfaeg {
    namespace entity {
       bool HeightMap::getHeight(math::Vec2f point, float& height) {
            ////////std::cout<<"get height"<<std::endl;
             if (getSize().z() == 0) {
                if (point.x() >= getGlobalBounds().getPosition().x() && point.x() < getGlobalBounds().getPosition().x() + getGlobalBounds().getSize().x()
                 && point.y() >= getGlobalBounds().getPosition().y() && point.y() < getGlobalBounds().getPosition().y() + getGlobalBounds().getSize().y()) {
                     height = point.y();
                     ////////std::cout<<"2D height : "<<height<<std::endl;
                     return true;
                 } else {
                     height = 0;
                     ////////std::cout<<"2D height false : "<<height<<std::endl;
                     return false;
                 }
             } else {
                 if (point.x() >= getGlobalBounds().getPosition().x() && point.x() < getGlobalBounds().getPosition().x() + getGlobalBounds().getSize().x()
                     && point.y() >= getGlobalBounds().getPosition().z() && point.y() < getGlobalBounds().getPosition().z() + getGlobalBounds().getSize().z()) {
                     math::Vec2f pos (0 - getPosition().x(), 0 - getPosition().z());
                     int xPosition = (point.x() + pos.x()) / squareSize;
                     int yPosition = (point.y() + pos.y()) / squareSize;
                     int position = yPosition * nbTilesPerRow + xPosition;
                     int triIndex = (point.x() < point.y()) ? 0 : 1;
                     Height h1, h2, h3;
                     h1 = squares[position].getHeight(0);
                     h2 = squares[position].getHeight(1+triIndex);                     
                     h3 = squares[position].getHeight(2+triIndex);
                     
                     ////////std::cout<<"point  : "<<point<<"pos : "<<pos<<"tileSize : "<<tileSize<<std::endl;
                     // Vecteurs du triangle
                     math::Vec2f v0 = h2 - h1;
                     math::Vec2f v1 = h3 - h1;
                     math::Vec2f v2 = point - h1;

                     // Dot products
                     float d00 = v0.dor(v1);
                     float d01 = v0.dot(v1);
                     float d11 = v1.dot(v1);
                     float d20 = v2.dot(v0);
                     float d21 = v2.dot(v1);

                    // Déterminant
                     float denom = d00 * d11 - d01 * d01;

                     // Coordonnées barycentriques
                     float beta  = (d11 * d20 - d01 * d21) / denom;
                     float gamma = (d00 * d21 - d01 * d20) / denom;
                     float alpha = 1.0f - beta - gamma;

                     // Hauteur interpolée
                     /*height = alpha * hA + beta * hB + gamma * hC;
                     float dx = (point.x() - h2.x()) / squareSize;
                     float dy = (point.y() - h2.y()) / squareSize;
                     
                     ////////std::cout<<"h : "<<h1<<","<<h2<<","<<h3<<","<<h4<<std::endl;
                     height = ((1 - dx) * h1 + dx * h2) * (1 - dy) + ((1 - dx) * h4 + dx * h3) * dy;*/
                     ////////std::cout<<"3D height : "<<height<<std::endl;
                     return true;
                } else {
                     height = 0;
                     ////////std::cout<<"3D height false : "<<height<<std::endl;
                     return false;
                }
             }
        }
    }
}