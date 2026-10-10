namespace odfaeg {
    namespace entity {
       class TileSet : public GameObject {
        public :
            TileSet(unsigned int squareSize, physic::BoundingBox rect);
            void addTile(Tile* tile);
            bool getHeight(math::Vec2f point, float& height);            
            GameObject* clone();
            template <typename Archive>
            void vtserialize(Archive& ar) {
                GameObject::vtserialize(ar);
            }
        private :
           int nbQuadsPerRow;
           unsigned int squareSize;
       };
    }
}