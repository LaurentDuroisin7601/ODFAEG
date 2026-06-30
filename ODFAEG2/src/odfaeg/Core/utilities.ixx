module;
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
export module odfaeg.core.utilities;
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
export namespace odfaeg {
    namespace core { 
        std::vector<std::string> split(const std::string& stringToBeSplitted, const std::string& delimeter)
        {
            std::vector<std::string> splittedString;
            int startIndex = 0;
            int  endIndex = 0;
            while ((endIndex = stringToBeSplitted.find(delimeter, startIndex)) < stringToBeSplitted.size())
            {
                std::string val = stringToBeSplitted.substr(startIndex, endIndex - startIndex);
                splittedString.push_back(val);
                startIndex = endIndex + delimeter.size();
            }
            if (startIndex < stringToBeSplitted.size())
            {
                std::string val = stringToBeSplitted.substr(startIndex);
                splittedString.push_back(val);
            }
            return splittedString;
        }

        float conversionStringFloat(std::string str)
        {
            std::stringstream ss(str);
            float f;
            ss >> f;
            return f;
        }

        std::string conversionFloatString(float f)
        {
            std::stringstream ss;
            ss << f;
            return ss.str();
        }

        std::int32_t conversionStringInt(std::string str)
        {
            std::stringstream ss(str);
            std::int32_t i;
            ss >> i;
            return i;
        }
        std::string conversionUIntString(const unsigned int& ui) {
            std::stringstream ss;
            ss << ui;
            return ss.str();
        }
        std::string conversionIntString(std::int32_t i)
        {
            std::stringstream ss;
            ss << i;

            return ss.str();
        }
        std::int64_t conversionStringLong(std::string str)
        {
            std::stringstream ss(str);
            std::int64_t i;
            ss >> i;
            return i;
        }
        std::int64_t conversionStringULong(std::string str) {
            std::stringstream ss(str);
            std::int64_t i;
            ss >> i;
            return i;
        }
        std::string conversionLongString(std::int64_t i)
        {
            std::stringstream ss;
            ss << i;

            return ss.str();
        }
        int conversionStringToHex(std::string str) {
            return strtoul(str.c_str(), NULL, 16);
        }
        void findFiles(std::string keyword, std::vector<std::string>& files, std::string startDir) {
            if (DIR* current = opendir(startDir.c_str())) {
                dirent* ent;
                while ((ent = readdir(current)) != NULL) {
                    if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
                        std::string path = startDir + "\\" + std::string(ent->d_name);
                        struct stat st;
                        stat(path.c_str(), &st);
                        ////////std::cout<<"path : "<<path<<" keyword : "<<keyword<<std::endl;
                        if (S_ISDIR(st.st_mode)) {
                            findFiles(keyword, files, path);
                        }
                        else {
                            std::vector<std::string> parts = split(keyword, " ");
                            for (unsigned int i = 0; i < parts.size(); i++) {
                                if (path.find(parts[i]) != std::string::npos) {
                                    bool contains = false;
                                    for (unsigned int j = 0; j < files.size() && !contains; j++) {
                                        if (files[j] == path)
                                            contains = true;
                                    }
                                    if (!contains) {
                                        files.push_back(path);
                                    }
                                }
                            }
                            if (keyword == "*") {                               
                                files.push_back(path);
                            }
                        }
                    }
                }
            }
        }
        bool is_number(const std::string& s)
        {
            std::string abs;
            if (!s.empty() && s.at(0) == '-') {
                abs = s.substr(1, s.length() - 1);
            }
            else {
                abs = s;
            }
            return !abs.empty() && std::find_if(abs.begin(),
                abs.end(), [](char c) { return !std::isdigit(c); }) == abs.end();
        }        
        int findString(const std::string& strHaystack, const std::string& strNeedle)
        {
            if (strNeedle > strHaystack) {
                return false;
            }
            int e = 0, indx = -1;
            for (unsigned int i = 0; i < strHaystack.size(); i++) {
                if (strHaystack.at(i) == strNeedle.at(e)) {
                    if (e == 0) {
                        indx = i;
                    }
                    e++;
                    if (e == strNeedle.size()) {
                        return indx;
                    }
                    else {
                        e = 0;
                    }
                }
            }
            return -1;
        }
        std::string getCurrentPath() {            
            return std::filesystem::current_path().string();
        }
    }
}