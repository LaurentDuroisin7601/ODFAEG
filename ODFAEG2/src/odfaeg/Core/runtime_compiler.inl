namespace odfaeg {
    namespace core {
        RuntimeCompiler::RuntimeCompiler(std::string functionName) : funcName(functionName), isDllOpened(false) {
            
        }
        bool RuntimeCompiler::isFileModified(std::string file) { 
            auto ftime = std::filesystem::last_write_time(file);
            // Convertir en time_t pour affichage
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now()
            );
            std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
            std::string last_write_time(std::asctime(std::localtime(&cftime)));           
            for (unsigned int l = 0; l < fCacheLines.size(); l++) {
                std::vector<std::string> datas = split(fCacheLines[l], "*");
                if (datas.size() >= 2) {
                    if (file == datas[0]) {
                        std::string last_registered_write_time = datas[1];
                        if (last_registered_write_time != last_write_time) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }
        std::vector<std::string> RuntimeCompiler::checkModifiedFiles() {
            std::vector<std::string> modifiedFiles;            
            if (fCacheLines.empty()) {
                modifiedFiles = sourceFiles;
            } else {
                std::vector<std::string> files = sourceFiles;
                for (unsigned int s = 0; s < sourceFiles.size(); s++) {                    
                    std::vector<std::string> stableStrings;
                    std::vector<const char*> args;
                    args.reserve(includeDirs.size() * 2 + 1);
                    for (auto& path : includeDirs) {
                        std::filesystem::path p = std::filesystem::canonical(Class::stripQuotes(std::string(path)));
                        std::string canonical = p.string();
                        std::replace(canonical.begin(), canonical.end(), '\\', '/');
                        stableStrings.push_back(canonical);
                        args.push_back("-I");
                        args.push_back(stableStrings.back().c_str());
                    }                
                    args.push_back("-std=c++20");
                    std::replace(files[s].begin(), files[s].end(), '\\', '/');
                    //std::cout<<"file : "<<files[i]<<std::endl;
                    CXIndex index = clang_createIndex(0, 0);
                    CXTranslationUnit tu = clang_parseTranslationUnit(
                        index,
                        files[s].c_str(),            // ton fichier source
                        args.data(), args.size(),                 // options
                        nullptr, 0,              // pas de fichiers pr�compil�s
                        CXTranslationUnit_None
                    );
                    std::vector<std::string> includes;
                    clang_getInclusions(tu, inclusionVisitor, &includes);
                    for (auto& inc : includes) {
                        bool fileModified = isFileModified(inc);
                        if (fileModified) {
                            bool contains = false;
                            for (unsigned int m = 0; m < modifiedFiles.size(); m++) {
                                if (modifiedFiles[m] == inc) {
                                    contains = true;
                                    break;
                                }
                            }
                            if (!contains) {
                                modifiedFiles.push_back(inc);
                            }
                        }
                    }                    
                    for (unsigned int s = 0; s < sourceFiles.size(); s++) {
                        bool fileModified = isFileModified(sourceFiles[s]+".cpp"); 
                        bool contains = false;
                        std::string sourceFile = sourceFiles[s]+".cpp";
                        for (unsigned int m = 0; m < modifiedFiles.size(); m++) {                            
                            if (modifiedFiles[m] == sourceFile) {
                                contains = true;
                                break;
                            }
                        }
                        if (!contains) {
                            modifiedFiles.push_back(sourceFile);
                        }                    
                    }
                }
            }
            return modifiedFiles;
        }
        void RuntimeCompiler::inclusionVisitor(CXFile included_file,
                      CXSourceLocation* inclusion_stack,
                      unsigned include_len,
                      CXClientData client_data)
        {
            std::string filename (clang_getCString(clang_getFileName(included_file)));
            std::vector<std::string>* includes = static_cast<std::vector<std::string>*>(client_data);

            includes->push_back(filename);
        }
        void RuntimeCompiler::buildSharedLib() {
            if (isDllOpened) {
                #if defined (ODFAEG_SYSTEM_LINUX)
                dlclose(flib);
                #else if defined (ODFAEG_SYSTEM_WINDOWS)
                FreeLibrary(flib);
                #endif
                isDllOpened = false;
            }
            std::ofstream file(outputDir+"\\"+funcName+".DEF");
            file<<"LIBRARY \""+outputDir+"\\"+funcName+"\"\n";
            file<<"EXPORTS\n";
            for (unsigned int i = 0; i < functions.size(); i++) {
                file<<functions[i]+"\n";
            }
            file<<"SECTIONS\n";
            file<<".shared READ WRITE SHARED\n";
            file.close();
            std::string command;
            std::vector<std::string> sourceFiles = checkModifiedFiles();
            for (unsigned int s = 0; s < sourceFiles.size(); s++) {
                command="g++ -c \""+sourceFiles[s]+".cpp\" -o \""+sourceFiles[s]+".o\" ";
                for (unsigned int i = 0; i < options.size(); i++) {
                    command+="-"+options[i]+" ";
                }
                for (unsigned int i = 0; i < macros.size(); i++) {
                    command+="-D"+macros[i]+" ";
                }
                for (unsigned int i = 0; i < includeDirs.size(); i++) {
                    command += "-I"+includeDirs[i]+" ";
                }
                command += " 2> \""+sourceFiles[s]+".err\"";
                system(command.c_str());
            }
            /*for (unsigned int i = 0; i < libraryDirs.size(); i++) {
                if (i == 0)
                    command = "g++ ";
                command += "-L"+libraryDirs[i]+" ";
            }
            for (unsigned int i = 0; i < libraries.size(); i++) {
                command += "-l"+libraries[i]+" ";
            }
            ////////std::cout<<"command : "<<command<<std::endl;
            system(command.c_str());*/
            command = "g++ -shared -o "+outputDir+"\\"+funcName+".dll";
            for (unsigned int s = 0; s < sourceFiles.size(); s++) {
                command+=" \""+sourceFiles[s]+".o\" ";
            }
            for (unsigned int i = 0; i < libraryDirs.size(); i++) {
                command += "-L"+libraryDirs[i]+" ";
            }
            for (unsigned int i = 0; i < libraries.size(); i++) {
                command += "-l"+libraries[i]+" ";
            }
            command += " 2> "+outputDir+"\\"+funcName+"_linkage.err";

            system(command.c_str());
            compileErrors = "";            
            for (unsigned int s = 0; s < sourceFiles.size(); s++) {
                std::ifstream ifs(outputDir+"\\"+funcName+"_"+sourceFiles[s]+".err");
                std::string str;
                while (getline(ifs, str)) {
                    compileErrors += str;
                }
                ifs.close();
            }
            std::ifstream ifs(outputDir+"\\"+funcName+"_linkage.err");
            std::string str;
            while (getline(ifs, str)) {
                compileErrors += str;
            }
            ifs.close();
            /*std::string path = "./"+funcName+".so";
            flib = dlopen(path.c_str(), RTLD_LAZY);
            if (!flib) {
                throw Erreur(10, "Failed to open dynamic library!", 3);
            }*/
        }
        void RuntimeCompiler::buildExec() {
            std::string command;
            std::vector<std::string> sourceFiles = checkModifiedFiles();
            for (unsigned int s = 0; s < sourceFiles.size(); s++) {
                command="g++ -c \""+sourceFiles[s]+".cpp\" -o \""+sourceFiles[s]+".o\" ";
                for (unsigned int i = 0; i < options.size(); i++) {
                    command+="-"+options[i]+" ";
                }
                for (unsigned int i = 0; i < macros.size(); i++) {
                    command+="-D"+macros[i]+" ";
                }
                for (unsigned int i = 0; i < includeDirs.size(); i++) {
                    command += "-I"+includeDirs[i]+" ";
                }
                command += " 2> \""+sourceFiles[s]+".err\"";
            }
            /*for (unsigned int i = 0; i < libraryDirs.size(); i++) {
                if (i == 0)
                    command = "g++ ";
                command += "-L"+libraryDirs[i]+" ";
            }
            for (unsigned int i = 0; i < libraries.size(); i++) {
                command += "-l"+libraries[i]+" ";
            }
            ////////std::cout<<"command : "<<command<<std::endl;
            system(command.c_str());*/
            command = "g++ -o "+outputDir+"\\"+funcName+".exe";
            for (unsigned int s = 0; s < sourceFiles.size(); s++) {
                command+=" \""+sourceFiles[s]+".o\" ";
            }
            for (unsigned int i = 0; i < libraryDirs.size(); i++) {
                command += "-L"+libraryDirs[i]+" ";
            }
            for (unsigned int i = 0; i < libraries.size(); i++) {
                command += "-l"+libraries[i]+" ";
            }
            command += " 2> "+outputDir+"\\"+funcName+"_linkage.err";

            system(command.c_str());
            compileErrors = "";
            for (unsigned int s = 0; s < sourceFiles.size(); s++) {
                std::ifstream ifs(outputDir+"\\"+funcName+"_"+sourceFiles[s]+".err");
                std::string str;
                while (getline(ifs, str)) {
                    compileErrors += str;
                }
                ifs.close();
            }
            std::ifstream ifs(outputDir+"\\"+funcName+"_linkage.err");
            std::string str;
            while (getline(ifs, str)) {
                compileErrors += str;
            }
            ifs.close();
        }
        void RuntimeCompiler::exec() {
            std::string command = funcName+".exe";
            system(command.c_str());
        }
        std::string RuntimeCompiler::getErrors() {
            return compileErrors;
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
        void RuntimeCompiler::setOutputDir(std::string outputDir) {
            std::ifstream fRegisteredUpdTimes (outputDir+"\\"+"last_script_writes.txt");
            std::string line;
            if (fRegisteredUpdTimes) {
                while (getline(fRegisteredUpdTimes, line)) {
                    fCacheLines.push_back(line);
                }
            } 
            this->outputDir = outputDir;
        }
        std::vector<std::string> RuntimeCompiler::getIncludeDirs() {
            return includeDirs;
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