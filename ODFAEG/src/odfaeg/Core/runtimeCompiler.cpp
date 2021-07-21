#include "../../../include/odfaeg/Core/runtimeCompiler.hpp"
#include <iostream>
namespace odfaeg {
    namespace core {
        RuntimeCompiler::RuntimeCompiler(std::string functionName) : funcName(functionName), isDllOpened(false) {

        }
        void RuntimeCompiler::compile() {
            if (isDllOpened) {
                #if defined (ODFAEG_SYSTEM_LINUX)
                dlclose(flib);
                #else if defined (ODFAEG_SYSTEM_WINDOWS)
                FreeLibrary(flib);
                #endif
                isDllOpened = false;
            }
            std::ofstream file(funcName+".DEF");
            file<<"LIBRARY \""+funcName+"\"\n";
            file<<"EXPORTS\n";
            for (unsigned int i = 0; i < functions.size(); i++) {
                file<<functions[i]+"\n";
            }
            file<<"SECTIONS\n";
            file<<".shared READ WRITE SHARED\n";
            file.close();
            std::string command;
            for (unsigned int s = 0; s < sourceFiles.size(); s++) {
                command="g++ -c "+sourceFiles[s]+".cpp -o "+sourceFiles[s]+".o ";
                for (unsigned int i = 0; i < options.size(); i++) {
                    command+="-"+options[i]+" ";
                }
                for (unsigned int i = 0; i < macros.size(); i++) {
                    command+="-D"+macros[i]+" ";
                }
                for (unsigned int i = 0; i < includeDirs.size(); i++) {
                    /*if (i == 0)
                        command += "-I";*/
                    command += "-I"+includeDirs[i]+" ";
                }
                command += " 2> "+funcName+".err";
                system(command.c_str());
                std::ifstream ifs(funcName+".err");
                std::string line;
                if (ifs) {
                    while (getline(ifs, line)) {
                        compileErrors+=line+"\n";
                    }
                    ifs.close();
                }
                std::cout<<"errors : "<<compileErrors<<std::endl;
            }
            for (unsigned int i = 0; i < libraryDirs.size(); i++) {
                if (i == 0)
                    command = "g++ ";
                command += "-L"+libraryDirs[i]+" ";
            }
            for (unsigned int i = 0; i < libraries.size(); i++) {
                command += "-l"+libraries[i]+" ";
            }
            //std::cout<<"command : "<<command<<std::endl;
            system(command.c_str());
            command = "g++ -shared -o "+funcName+".dll";
            for (unsigned int s = 0; s < sourceFiles.size(); s++) {
                command+=" "+sourceFiles[s]+".o ";
            }
            for (unsigned int i = 0; i < libraryDirs.size(); i++) {
                command += "-L"+libraryDirs[i]+" ";
            }
            for (unsigned int i = 0; i < libraries.size(); i++) {
                command += "-l"+libraries[i]+" ";
            }
            command += " 2> "+funcName+".err";

            std::cout<<"command : "<<command<<std::endl;
            system(command.c_str());
            std::ifstream ifs(funcName+".err");
            std::string line;
            if (ifs) {
                while (getline(ifs, line)) {
                    compileErrors+=line+"\n";
                }
                ifs.close();
            }
            /*std::string path = "./"+funcName+".so";
            flib = dlopen(path.c_str(), RTLD_LAZY);
            if (!flib) {
                throw Erreur(10, "Failed to open dynamic library!", 3);
            }*/
        }
        void RuntimeCompiler::addSourceFile(std::string sourceFile) {
            bool found = false;
            for (unsigned int i = 0; i < sourceFiles.size(); i++) {
                if (sourceFiles[i] == sourceFile) {
                    found = true;
                }
            }
            if (!found) {
                sourceFiles.push_back(sourceFile);
            }
        }
        std::string RuntimeCompiler::getCompileErrors() {
            return compileErrors;
        }
        void RuntimeCompiler::addMacro(std::string macro) {
            macros.push_back(macro);
        }
        void RuntimeCompiler::addOption(std::string option) {
            options.push_back(option);
        }
        void RuntimeCompiler::addIncludeDir(std::string includeDir) {
            includeDirs.push_back(includeDir);
        }
        void RuntimeCompiler::addLibraryDir(std::string libraryDir) {
            libraryDirs.push_back(libraryDir);
        }
        void RuntimeCompiler::addLibrary(std::string library) {
            libraries.push_back(library);
        }
        void RuntimeCompiler::addRuntimeFunction(std::string f) {
            functions.push_back(f);
        }
        RuntimeCompiler::~RuntimeCompiler() {
            if (isDllOpened)
                #if defined (ODFAEG_SYSTEM_LINUX)
                fclose(flib);
                #else if defined (ODFAEG_SYSTEM_WINDOWS)
                FreeLibrary(flib);
                #endif
        }
    }
}
