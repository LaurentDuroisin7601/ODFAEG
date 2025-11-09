#include "../../../include/odfaeg/Graphics/blendMode.hpp"
#include <iostream>
namespace odfaeg {
    namespace graphic {
        unsigned int BlendMode::nbBlendModes = 0;
        std::vector<BlendMode*> BlendMode::blendModes = std::vector<BlendMode*>();
        std::vector<BlendMode*> BlendMode::sameBlendModes= std::vector<BlendMode*>();
        ////////////////////////////////////////////////////////////
        // Commonly used blending modes
        ////////////////////////////////////////////////////////////
        const BlendMode BlendAlpha(BlendMode::SrcAlpha, BlendMode::OneMinusSrcAlpha, BlendMode::Add,
                                   BlendMode::One, BlendMode::OneMinusSrcAlpha, BlendMode::Add);
        const BlendMode BlendAdd(BlendMode::SrcAlpha, BlendMode::One, BlendMode::Add,
                                 BlendMode::One, BlendMode::One, BlendMode::Add);
        const BlendMode BlendMultiply(BlendMode::DstColor, BlendMode::Zero);
        const BlendMode BlendNone(BlendMode::One, BlendMode::Zero);


        ////////////////////////////////////////////////////////////
        BlendMode::BlendMode() :
        colorSrcFactor(BlendMode::SrcAlpha),
        colorDstFactor(BlendMode::OneMinusSrcAlpha),
        colorEquation (BlendMode::Add),
        alphaSrcFactor(BlendMode::One),
        alphaDstFactor(BlendMode::OneMinusSrcAlpha),
        alphaEquation (BlendMode::Add)
        {
            blendModes.push_back(this);
        }


        ////////////////////////////////////////////////////////////
        BlendMode::BlendMode(Factor sourceFactor, Factor destinationFactor, Equation blendEquation) :
        colorSrcFactor(sourceFactor),
        colorDstFactor(destinationFactor),
        colorEquation (blendEquation),
        alphaSrcFactor(sourceFactor),
        alphaDstFactor(destinationFactor),
        alphaEquation (blendEquation)
        {
            blendModes.push_back(this);
        }


        ////////////////////////////////////////////////////////////
        BlendMode::BlendMode(Factor colorSourceFactor, Factor colorDestinationFactor,
                             Equation colorBlendEquation, Factor alphaSourceFactor,
                             Factor alphaDestinationFactor, Equation alphaBlendEquation) :
        colorSrcFactor(colorSourceFactor),
        colorDstFactor(colorDestinationFactor),
        colorEquation (colorBlendEquation),
        alphaSrcFactor(alphaSourceFactor),
        alphaDstFactor(alphaDestinationFactor),
        alphaEquation (alphaBlendEquation)
        {
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            blendModes.push_back(this);
        }
        BlendMode::BlendMode(const BlendMode& copy) :
        colorSrcFactor(copy.colorSrcFactor),
        colorDstFactor(copy.colorDstFactor),
        colorEquation (copy.colorEquation),
        alphaSrcFactor(copy.alphaSrcFactor),
        alphaDstFactor(copy.alphaDstFactor),
        alphaEquation (copy.alphaEquation) {
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            blendModes.push_back(this);
        }
        void BlendMode::countNbBlendMode() {
            nbBlendModes = 0;
            sameBlendModes.clear();
            for (unsigned int i = 0; i < blendModes.size(); i++) {
                if (!contains(*blendModes[i])) {
                    nbBlendModes++;
                    sameBlendModes.push_back(blendModes[i]);
                }
            }
        }
        void BlendMode::updateIds() {
          std::lock_guard<std::recursive_mutex> lock(rec_mutex);
          countNbBlendMode();
          for (unsigned int i = 0; i < sameBlendModes.size(); i++) {
               for (unsigned int j = 0; j < blendModes.size(); j++) {
                    if (*sameBlendModes[i] == *blendModes[j]) {
                        //////std::cout<<"id : "<<i<<std::endl;
                        blendModes[j]->id = i;
                    }
               }
           }
        }
        bool BlendMode::contains(BlendMode& blendMode) {
            for (unsigned int i = 0; i < sameBlendModes.size(); i++) {
                if (*sameBlendModes[i] == blendMode) {
                    return true;
                }
            }
            return false;
        }
        ////////////////////////////////////////////////////////////
        bool operator ==(const BlendMode& left, const BlendMode& right)
        {
            return (left.colorSrcFactor == right.colorSrcFactor) &&
                   (left.colorDstFactor == right.colorDstFactor) &&
                   (left.colorEquation  == right.colorEquation)  &&
                   (left.alphaSrcFactor == right.alphaSrcFactor) &&
                   (left.alphaDstFactor == right.alphaDstFactor) &&
                   (left.alphaEquation  == right.alphaEquation);
        }


        ////////////////////////////////////////////////////////////
        bool operator !=(const BlendMode& left, const BlendMode& right)
        {
            return !(left == right);
        }
        BlendMode::~BlendMode() {
            std::lock_guard<std::recursive_mutex> lock(rec_mutex);
            std::vector<BlendMode*>::iterator it;
            for (it = blendModes.begin(); it != blendModes.end(); ) {
                if (*it == this) {
                    it = blendModes.erase(it);
                } else {
                    it++;
                }
            }
        }
    }
}
