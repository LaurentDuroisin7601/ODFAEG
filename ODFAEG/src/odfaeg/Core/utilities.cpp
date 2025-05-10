#include "../../../include/odfaeg/Core/utilities.h"
#include <iostream>
#include <filesystem>
namespace odfaeg {
    namespace core {
        using namespace std;
        using namespace sf;
        std::vector<std::string> split(const std::string& stringToBeSplitted, const std::string& delimeter)
        {
            std::vector<std::string> splittedString;
            int startIndex = 0;
            int  endIndex = 0;
            while( (endIndex = stringToBeSplitted.find(delimeter, startIndex)) < stringToBeSplitted.size() )
            {
                std::string val = stringToBeSplitted.substr(startIndex, endIndex - startIndex);
                splittedString.push_back(val);
                startIndex = endIndex + delimeter.size();
            }
            if(startIndex < stringToBeSplitted.size())
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

        Int32 conversionStringInt(std::string str)
        {
            std::stringstream ss(str);
            Int32 i;
            ss >> i;
            return i;
        }
        std::string conversionUIntString(const unsigned int& ui) {
            std::stringstream ss;
            ss << ui;
            return ss.str();
        }
        std::string conversionIntString(Int32 i)
        {
            std::stringstream ss;
            ss << i;

            return ss.str();
        }
        Int64 conversionStringLong(std::string str)
        {
            std::stringstream ss(str);
            Int64 i;
            ss >> i;
            return i;
        }
        sf::Uint64 conversionStringULong(std::string str) {
            std::stringstream ss(str);
            sf::Uint64 i;
            ss >> i;
            return i;
        }
        std::string conversionLongString(Int64 i)
        {
            std::stringstream ss;
            ss << i;

            return ss.str();
        }
        int conversionStringToHex(std::string str) {
            return strtoul(str.c_str(), NULL, 16);
        }
        void findFiles (std::string keyword, std::vector<std::string>& files, std::string startDir) {
            #if defined (ODFAEG_SYSTEM_LINUX)
            if (DIR* current = opendir(startDir.c_str())) {
                dirent* ent;
                while ((ent = readdir(current)) != NULL) {
                    if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
                        std::string path(ent->d_name);
                        if (ent->d_type == DT_DIR) {
                            findFiles(keyword, files, path);
                        } else if (path.find(keyword) != std::string::npos) {
                            files.push_back(path);
                        }
                    }
                }
                closedir(current);
            }
            #else if defined (ODFAEG_SYSTEM_WINDOWS)
            if (DIR* current = opendir(startDir.c_str())) {
                dirent* ent;
                while ((ent = readdir(current)) != NULL) {
                    if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
                       std::string path = startDir + "\\" + std::string(ent->d_name);
                       struct stat st;
                       stat(path.c_str(), &st);
                       //std::cout<<"path : "<<path<<" keyword : "<<keyword<<std::endl;
                       if (S_ISDIR(st.st_mode)) {
                           findFiles(keyword, files, path);
                       } else {
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
                                //path = std::string(ent->d_name);
                                files.push_back(path);
                           }
                       }
                    }
                }
            }
            #endif
        }
        bool is_number(const std::string& s)
        {
            std::string abs;
            if (!s.empty() && s.at(0) == '-') {
                abs = s.substr(1, s.length() - 1);
            } else {
                abs = s;
            }
            return !abs.empty() && std::find_if(abs.begin(),
            abs.end(), [](char c) { return !std::isdigit(c); }) == abs.end();
        }
        /// Try to find in the Haystack the Needle - ignore case
        int findString(const std::string & strHaystack, const std::string & strNeedle)
        {
              if(strNeedle > strHaystack) {
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
                       } else {
                           e = 0;
                       }
                   }
              }
            return -1;
        }
        std::string getCurrentPath() {
            /*char cCurrentPath[FILENAME_MAX];
            _getcwd(cCurrentPath,sizeof(cCurrentPath));*/
            return std::filesystem::current_path().string();
        }
        void memmv(void* src, void* dst, size_t size) {
            char* tmp = new char[size];
            char* csrc = (char*) src;
            char* cdst = (char*) dst;
            for (unsigned int i = 0; i < size; i++) {
                tmp[i] = csrc[i];
            }
            for (unsigned int i = 0; i < size; i++) {
                cdst[i] = tmp[i];
            }
            dst = cdst;
            delete tmp;
        }
    }
}
