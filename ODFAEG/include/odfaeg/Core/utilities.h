#ifndef UTILITIES
#define UTILITIES
#include <string>
#include <regex>
#include <vector>
#include <sstream>
#include <SFML/Config.hpp>
#include "../config.hpp"
#include <dirent.h>
#ifndef ODFAEG_SYSTEM_LINUX
#include <io.h>
#include <direct.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg {
    namespace core {
        /**
        *  \fn split (const std::string& chaine, const std::string& separator)
        *  \brief split a std::string with the given separator.
        *  \param const std::string& chaine : the std::string to split.
        *  \param const std==string& separator : the separator string.
        *  \return the parts of the splitted std::string.
        */
        std::vector<std::string> split (const std::string &chaine, const std::string &separator);
        /**\fn float conversionStringFloat(std::string str);
        *  \brief convert an std::string into a float.
        *  \param the std::string to convert.
        *  \return the float.
        */
        float conversionStringFloat(std::string str);
        /**\fn Int32 conversionStringInt(std::string str);
        *  \brief convert an std::string into an Int32.
        *  \param the std::string to convert.
        *  \return the int.
        */
        sf::Int32 conversionStringInt(std::string str);
        /**\fn Int64 conversionStringLong(std::string str);
        *  \brief convert an std::string into an Int64.
        *  \param the std::string to convert.
        *  \return the int.
        */
        sf::Int64 conversionStringLong(std::string str);
        sf::Uint64 conversionStringULong(std::string str);
        /**\fn std::string conversionFloatString(float f);
        *  \brief convert a float to an std::string.
        *  \param the float to convert.
        *  \return the std::string.
        */
        std::string conversionFloatString(float f);
        /**\fn std::string conversionIntString(sf::Int32 i);
        *  \brief convert a Int32 to an std::string.
        *  \param the Int32 to convert.
        *  \return the std::string.
        */
        std::string conversionIntString(sf::Int32 i);
        /**\fn std::string conversionLongString(sf::Int64 i);
        *  \brief convert a Int64 to an std::string.
        *  \param the Int64 to convert.
        *  \return the std::string.
        */
        std::string conversionUIntString(const unsigned int& ui);
        std::string conversionLongString(sf::Int64 i);
        int conversionStringToHex(std::string str);
        std::string getCurrentPath();
        void findFiles (const std::string keyword, std::vector<std::string>& files,std::string startDir = "/");
        bool is_number(const std::string& s);
        int findString(const std::string & strHaystack, const std::string & strNeedle);
        void memmv(void* src, void* dst, size_t size);
    }
}
#endif
