#include "../../../../include/odfaeg/Graphics/GUI/table.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            Table::Table (unsigned int nbRows, unsigned int nbColumns) {
                this->nbRows = nbRows;
                this->nbColumns = nbColumns;
            }
            void Table::addElement (LightComponent* element, unsigned int rowIndex, unsigned int columnIndex) {
                float relSizeX = 1.f / nbColumns;
                float relSizeY = 1.f / nbRows;
                float relPosX = columnIndex * relSizeX;
                float relPosY = rowIndex * relSizeY;
                element->setRelPosition(relPosX, relPosY);
                element->setRelSize(relSizeX, relSizeY);
                element->setAutoResized(true);
            }
        }
    }
}
