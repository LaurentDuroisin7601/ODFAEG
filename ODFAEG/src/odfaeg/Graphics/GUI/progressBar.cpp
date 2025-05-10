#include "../../../../include/odfaeg/Graphics/GUI/progressBar.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            ProgressBar::ProgressBar(RenderWindow& rw, math::Vec3f position, math::Vec3f size, const Font& font, unsigned int charSize) :
            LightComponent(rw, position, size, size*0.5f) {
                bar = RectangleShape(math::Vec3f(0, size.y, size.z));
                bar.setPosition(position);
                barColor = sf::Color::Red;
                bar.setFillColor(barColor);
                this->value = 0;
                text.setFont(font);
                text.setCharacterSize(charSize);
                text.setSize(size);
                text.setPosition(position);
                text.setString(core::conversionIntString(value)+"/"+core::conversionIntString(maxi));
                text.setColor(sf::Color::Black);
            }
            void ProgressBar::setColor(sf::Color barColor) {
                this->barColor = barColor;
            }
            void ProgressBar::clear() {
                bar.setFillColor(barColor);
            }
            void ProgressBar::onDraw(RenderTarget& target, RenderStates states) {
                target.draw(bar);
                target.draw(text);
            }
            void ProgressBar::setMaximum(int maxi) {
                text.setString(core::conversionIntString(value)+"/"+core::conversionIntString(maxi));
                this->maxi = maxi;
            }
            void ProgressBar::setMinimum(int mini) {
                this->mini = mini;
            }
            void ProgressBar::setValue(int value) {
                //std::cout<<"value : "<<value<<std::endl;
                int sizeX = getSize().x / (maxi - mini) * value;
                if (getName() == "XPBAR") {
                    bar.setName("RXPBAR");
                }

                bar.setSize(math::Vec3f(sizeX, bar.getSize().y, bar.getSize().z));
                //std::cout<<"new size : "<<bar.getSize()<<std::endl;
                this->value = value;
                text.setString(core::conversionIntString(value)+"/"+core::conversionIntString(maxi));
            }
            int ProgressBar::getValue() {
                return value;
            }
        }
    }
}
