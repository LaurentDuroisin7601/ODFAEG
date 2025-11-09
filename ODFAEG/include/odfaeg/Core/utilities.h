#ifndef UTILITIES
#define UTILITIES
#include <string>
#include <regex>
#include <vector>
#include <sstream>
#include "../config.hpp"
#include <dirent.h>
#ifndef ODFAEG_SYSTEM_LINUX
#include <io.h>
#include <direct.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include "export.hpp"
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
        ODFAEG_CORE_API std::vector<std::string> split (const std::string &chaine, const std::string &separator);
        /**\fn float conversionStringFloat(std::string str);
        *  \brief convert an std::string into a float.
        *  \param the std::string to convert.
        *  \return the float.
        */
        ODFAEG_CORE_API float conversionStringFloat(std::string str);
        /**\fn Int32 conversionStringInt(std::string str);
        *  \brief convert an std::string into an Int32.
        *  \param the std::string to convert.
        *  \return the int.
        */
        ODFAEG_CORE_API std::int32_t conversionStringInt(std::string str);
        /**\fn Int64 conversionStringLong(std::string str);
        *  \brief convert an std::string into an Int64.
        *  \param the std::string to convert.
        *  \return the int.
        */
        ODFAEG_CORE_API std::int64_t conversionStringLong(std::string str);
        ODFAEG_CORE_API std::int64_t conversionStringULong(std::string str);
        /**\fn std::string conversionFloatString(float f);
        *  \brief convert a float to an std::string.
        *  \param the float to convert.
        *  \return the std::string.
        */
        ODFAEG_CORE_API std::string conversionFloatString(float f);
        /**\fn std::string conversionIntString(sf::Int32 i);
        *  \brief convert a Int32 to an std::string.
        *  \param the Int32 to convert.
        *  \return the std::string.
        */
        ODFAEG_CORE_API std::string conversionIntString(std::int32_t i);
        /**\fn std::string conversionLongString(sf::Int64 i);
        *  \brief convert a Int64 to an std::string.
        *  \param the Int64 to convert.
        *  \return the std::string.
        */
        ODFAEG_CORE_API std::string conversionUIntString(const unsigned int& ui);
        ODFAEG_CORE_API std::string conversionLongString(std::int64_t i);
        ODFAEG_CORE_API int conversionStringToHex(std::string str);
        ODFAEG_CORE_API std::string getCurrentPath();
        ODFAEG_CORE_API void findFiles (const std::string keyword, std::vector<std::string>& files,std::string startDir = "/");
        ODFAEG_CORE_API bool is_number(const std::string& s);
        ODFAEG_CORE_API int findString(const std::string & strHaystack, const std::string & strNeedle);
        ODFAEG_CORE_API void memmv(void* src, void* dst, size_t size);
    }
}
#endif
