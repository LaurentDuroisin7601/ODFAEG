#ifndef ODFAEG_RUNTIME_COMPILER_HPP
#define ODFAEG_RUNTIME_COMPILER_HPP
#include <vector>
#include <string>
#include <fstream>
#include "../config.hpp"
#if defined (ODFAEG_SYSTEM_LINUX)
#include <dlfcn.h>
#else if defined (ODFAEG_SYSTEM_WINDOWS)
#include <windows.h>
#endif
#include "erreur.h"
namespace odfaeg {
    namespace core {
        /**
        * \file runtimeCompiler.h
        * \class RuntimeCompiler
        * \brief This class compile c++ code at runtime and generate a shared library, and calls shared library's functions.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class RuntimeCompiler {
            public :
            /**
            * \fn RuntimeCompiler(std::string funcName)
            * \brief constructor.
            * \param std::string the name of the shared library.
            */
            RuntimeCompiler(std::string funcName);
            /**
            * \fn void compile()
            * \brief compile code of the c++ files.
            */
            void compile();
            /**
            * \fn R run(std::string fName, A... a)
            * \brief run the specified function with the specified args.
            * \param fName the name of the function.
            */
            template<typename R, typename... A>
            R run(std::string fName, A... a) {
                //Path of the dll.
                #if defined (ODFAEG_SYSTEM_LINUX)
                std::string path = "./"+funcName+".so";
                #else if defined (ODFAEG_SYSTEM_WINDOWS)
                std::string path = "./"+outputDir+"\\"+funcName+".dll";
                #endif
                #if defined (ODFAEG_SYSTEM_LINUX)
                 if (!isDllOpened) {
                    flib = reinterpret_cast<FILE*>(dlopen(path.c_str(), RTLD_LAZY));
                    if (!flib) {
                        throw Erreur(10, "Failed to open dynamic library!", 3);
                    }
                    isDllOpened = true;
                }
                typedef R(*func)(A...);
                func pfunc = (func) dlsym(flib, fName.c_str());
                #else if defined (ODFAEG_SYSTEM_WINDOWS)
                //if the shared lib is not opened, we open it.
                if (!isDllOpened) {
                    flib = LoadLibraryA(path.c_str());
                    if (!flib) {
                        throw Erreur(10, "Failed to open dynamic library!", 3);
                    }
                    isDllOpened = true;
                }
                //Get the pointer to the function of the shared lib.
                typedef R(*func)(A...);
                func pfunc = (func) GetProcAddress(flib, fName.c_str());
                #endif
                if (!pfunc) {
                    throw Erreur(10, "Failed to load the function!", 3);
                }
                //Call the function.
                return pfunc(a...);
            }
            /**\fn void addIncludeDir(std::string includeDir)
            *  \brief add an include directory.
            *  \param std::string the include directory to add.
            */
            void addIncludeDir(std::string includeDir);
            /**\fn addLibraryDir(std::string libraryDir)
            *  \brief add a library directory.
            *  \param std::string the library directory to add.
            */
            void addLibraryDir(std::string libraryDir);
            /**\fn addLibrary(std::string library)
            *  \brief add a library.
            *  \param std::string the library to add.
            */
            void addLibrary(std::string library);
            /**\fn addSourceFile(std::string sourceFile)
            *  \brief add a c++ source file.
            *  \param std::string the source file to add.
            */
            void addSourceFile(std::string sourceFile);
            /**\fn addOption(std::string option)
            *  \brief add a compiler option.
            *  \param std::string the compiler option to add.
            */
            void addOption(std::string option);
            /**\fn addMacro(std::string macro)
            *  \brief add a macro definition.
            *  \param std::string the macro definition to add.
            */
            void addMacro(std::string macro);
            /** \fn addRuntimeFunction (std::string f)
            *   \brief add a function to export to the shared lib.
            *   \param std::string the function's name.
            */
            void addRuntimeFunction (std::string f);
            /** \fn std::string getCompileErrors()
            *   \brief get the compilation errors.
            *   \return the compilation errors.
            */
            std::string getCompileErrors();
            /** \fn ~RuntimeCompiler()
            *   \brief destructor. (we need to free the shared library)
            */
            ~RuntimeCompiler();
            void setOutputDir(std::string outputDir);
            private :
            #if defined (ODFAEG_SYSTEM_LINUX)
            FILE* flib;
            #else if defined (ODFAEG_SYSTEM_WINDOWS)
            HMODULE flib; /** pointer to the shared library.*/
            #endif
            bool isDllOpened; /**> if the shared library is open.*/
            std::string funcName; /**> the name of the shared library.*/
            std::string compileErrors; /**> compilation errors.*/
            std::string outputDir;
            std::vector<std::string> sourceFiles; /**> the c++ source files to compiler.*/
            std::vector<std::string> includeDirs; /**> the include directories.*/
            std::vector<std::string> libraryDirs; /**> the library directories.*/
            std::vector<std::string> libraries; /**> the library files.*/
            std::vector<std::string> macros; /**> the macro definition.*/
            std::vector<std::string> options; /**> the compiler options.*/
            std::vector<std::string> functions; /**the functions to export to the shared lib*/
        };
    }
}
#endif
