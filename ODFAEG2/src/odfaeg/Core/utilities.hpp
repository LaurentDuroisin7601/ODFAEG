#ifndef ODFAEG_UTILITIES_HPP
#define ODFAEG_UTILITIES_HPP
#include <string>
#include <cstring>
#include  <regex>
#include  <vector>
#include  <sstream>
#include  <filesystem>
#include <odfaeg/config.hpp>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
namespace odfaeg {
    namespace core { 
        std::vector<std::string> split(const std::string& stringToBeSplitted, const std::string& delimeter);

        float conversionStringFloat(std::string str);
        std::string conversionFloatString(float f);

        std::int32_t conversionStringInt(std::string str);
        std::string conversionUIntString(const unsigned int& ui);
        std::string conversionIntString(std::int32_t i);
        std::int64_t conversionStringLong(std::string str);
        std::int64_t conversionStringULong(std::string str);
        std::string conversionLongString(std::int64_t i);
        int conversionStringToHex(std::string str);
        void findFiles(std::string keyword, std::vector<std::string>& files, std::string startDir);
        bool is_number(const std::string& s);
        int findString(const std::string& strHaystack, const std::string& strNeedle);
        std::string getCurrentPath();
    }
}
#include "utilities.inl"
#endif