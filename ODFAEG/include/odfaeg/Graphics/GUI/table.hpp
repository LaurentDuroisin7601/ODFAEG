#ifndef ODFAEG_TABLE_HPP
#define ODFAEG_TABLE_HPP
#include "../lightComponent.h"
#include "../rectangleShape.h"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class Table {
                public :
                Table (unsigned int nbRows, unsigned int nbColumns);
                void addElement (LightComponent* element, unsigned int rowIndex, unsigned int columnIndex);
                private :
                unsigned int nbRows;
                unsigned int nbColumns;
            };
        }
    }
}
#endif
