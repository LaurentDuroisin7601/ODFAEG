namespace odfaeg {
    namespace entity {
       TileSet::TileSet (unsigned int squareSize, physic::BoundingBox zone)
        : GameObject (zone.getPosition(), zone.getSize(), zone.getSize() * 0.5f, "E_HEIGHTMAP")
        squareSize(squareSize) {
            nbQuadsPerRow = zone.getSize().x() / squareSize;
       }
       void TileSet::addTile(Tile* tile) {
           addChild(tile);
       } 
       bool TileSet::getHeight(math::Vec2f point, float& height) {
            ////////std::cout<<"get height"<<std::endl;
            if (point.x() >= getGlobalBounds().getPosition().x() && point.x() < getGlobalBounds().getPosition().x() + getGlobalBounds().getSize().x()
                && point.y() >= getGlobalBounds().getPosition().z() && point.y() < getGlobalBounds().getPosition().z() + getGlobalBounds().getSize().z()) {
                math::Vec2f pos (0 - getPosition().x(), 0 - getPosition().z());
                int xPosition = (point.x() + pos.x()) / squareSize;
                int yPosition = (point.y() + pos.y()) / squareSize;
                int position = yPosition * nbQuadsPerRow + xPosition;
                math::Vec2f d(point.x() - xPosition * squareSize, point.y() - yPosition*squareSize);
                //Triangle du carré sur lequel le point se trouve.
                int triIndex = (d.x() < d.y()) ? 0 : 1;
                Height h1, h2, h3;
                h1 = getChildren()[position].getHeight(0);
                h2 = getChildren()[position].getHeight(1+triIndex);                     
                h3 = getChildren()[position].getHeight(2+triIndex);
                
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
                height = alpha * h1 + beta * h2 + gamma * h3;               
                return true;
             }
        }
        GameObject* TileSet::clone() {
            TileSet* ts = new TileSet(squareSize, getGlobalBounds());
            GameObject::copy(*ts);
            return ts;
        }
    }
}