module;
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <clang-c/Index.h>
#include <vector>
#include <algorithm>
export module odfaeg.core.reflexibility;
import odfaeg.core.utilities;



export namespace odfaeg {
   namespace core {
        class Constructor {
        public:
           Constructor(std::string name) : name(name) {
            }
            void addArgType(std::string argType) {
                argsTypes.push_back(argType);
            }
            void addArgName(std::string argName) {
                argsNames.push_back(argName);
            }
            std::vector<std::string> getArgsNames() {
                return argsNames;
            }
            std::vector<std::string> getArgsTypes() {
                return argsTypes;
            }
            std::string getName() {
                return name;
            }
            CXTranslationUnit tu;
        private:
            std::string name; /**> constructor's name.*/
            std::vector<std::string> argsTypes; /**> constructor's arguments types.*/
            std::vector<std::string> argsNames; /**> constructor's arguments names.*/
        };
        class MemberFunction {
        public:
            MemberFunction(std::string returnType, std::string name) : m_name(name), m_returnType(returnType) {
            }
            void addArgType(std::string argType) {
                m_argsTypes.push_back(argType);
            }
            void addArgName(std::string argName) {
                m_argsNames.push_back(argName);
            }
            std::string getReturnType() {
                return m_returnType;
            }
            std::string getName() {
                return m_name;
            }
            std::vector<std::string> getArgsTypes() {
                return m_argsTypes;
            }
            std::vector<std::string> getArgsNames() {
                return m_argsNames;
            }
            CXTranslationUnit tu;
            std::pair<unsigned int, unsigned int> location;
            unsigned int offset;
        private:
            std::string m_name;
            std::string m_returnType;
            std::vector<std::string> m_argsTypes;
            std::vector<std::string> m_argsNames;
        };
        class MemberVariable {
        public:
            void setVarType(std::string varType) {
                this->varType = varType;
            }
            void setVarName(std::string varName) {
                this->varName = varName;
            }
            std::string getVarType() {
                return varType;
            }
            std::string getVarName() {
                return varName;
            }
            CXTranslationUnit tu;
        private:
            std::string varType;
            std::string varName;
        };
        class Class;
        struct Context {
            Context(Class& cl) : cl(cl) {
            }
            std::vector<std::string> datas;
            Class& cl;
            CXTranslationUnit tu;
        };
        struct Context2 {
            std::vector<std::string> datas;
            std::vector<std::pair<std::string, std::string>> classes;
        };
        /**
        * \file class.hpp
        * \class Class
        * \brief Search informations about a c++ class. (constructors, member's functions, sub classes)
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */       
        class Class {
        public:
            Class(std::string name, std::string filePath) : name(name), filePath(filePath), implFilePath("") {

            }
            void setName(std::string name) {
                this->name = name;
            }
            void setFilePath(std::string filePath) {
                this->filePath = filePath;
            }
            void setImplFilePath(std::string filePath) {
                implFilePath = filePath;
            }
            
            static std::vector<std::pair<std::string, std::string>> getClassesFromMemory(std::vector<std::string> includePaths, std::string virtualFile, std::string virtualPath, std::string content, std::string nspc) {
                Context2 ctx;
                ctx.datas.push_back(nspc);
                std::string appiDir;
                if (virtualPath.find("C:\\") == std::string::npos)
                    appiDir = virtualPath != "" ? getCurrentPath() + "\\" + virtualPath : getCurrentPath();
                else
                    appiDir = virtualPath;
                std::vector<std::string> hfiles;
                findFiles(".hpp .h", hfiles, appiDir);
                for (unsigned int i = 0; i < hfiles.size(); i++) {
                    std::replace(hfiles[i].begin(), hfiles[i].end(), '\\', '/');
                    ctx.datas.push_back(hfiles[i]);
                }
                CXIndex index = clang_createIndex(0, 0);
                std::vector<const char*> args;
                args.reserve(includePaths.size() * 2 + 1);
                std::vector<std::string> stableStrings;
                for (auto& path : includePaths) {
                    std::filesystem::path p = std::filesystem::canonical(stripQuotes(std::string(path)));
                    std::string canonical = p.string();
                    std::replace(canonical.begin(), canonical.end(), '\\', '/');
                    stableStrings.push_back(canonical);
                    args.push_back("-I");
                    args.push_back(stableStrings.back().c_str());
                }
                args.push_back("-std=c++20");
                CXUnsavedFile unsaved;
                unsaved.Filename = virtualFile.c_str();        // nom virtuel
                unsaved.Contents = content.c_str();        // contenu de ta TextArea
                unsaved.Length = content.size();

                CXTranslationUnit tu = clang_parseTranslationUnit(
                    index,
                    virtualFile.c_str(),            // ton fichier source
                    args.data(), args.size(),                 // options
                    &unsaved, 1,              // pas de fichiers pr�compil�s
                    CXTranslationUnit_None
                );
                CXCursor rootCursor = clang_getTranslationUnitCursor(tu);
                clang_visitChildren(rootCursor, classesVisitor, &ctx);                
                return ctx.classes;
            }

            static std::vector<std::pair<std::string, std::string>> getClasses(std::vector<std::string> includePaths, std::string path, std::string nspc) {
                Context2 ctx;
                ctx.datas.push_back(nspc);
                std::string appiDir;
                if (path.find("C:\\") == std::string::npos)
                    appiDir = path != "" ? getCurrentPath() + "\\" + path : getCurrentPath();
                else
                    appiDir = path;
                //std::cout<<"path : "<<appiDir<<std::endl;
                std::vector<std::string> files;
                findFiles(".cpp .c", files, appiDir);
                std::vector<std::string> hfiles;
                findFiles(".hpp .h", hfiles, appiDir);
                for (unsigned int i = 0; i < hfiles.size(); i++) {
                    std::replace(hfiles[i].begin(), hfiles[i].end(), '\\', '/');
                    ctx.datas.push_back(hfiles[i]);
                }
                for (unsigned int i = 0; i < files.size(); i++) {
                    //std::cout<<"get classes : "<<files[i]<<std::endl;
                    CXIndex index = clang_createIndex(0, 0);
                    std::vector<const char*> args;
                    args.reserve(includePaths.size() * 2 + 1);
                    // Ajouter les include dirs
                    std::vector<std::string> stableStrings;
                    for (auto& path : includePaths) {
                        std::filesystem::path p = std::filesystem::canonical(stripQuotes(std::string(path)));
                        std::string canonical = p.string();
                        std::replace(canonical.begin(), canonical.end(), '\\', '/');
                        stableStrings.push_back(canonical);
                        args.push_back("-I");
                        args.push_back(stableStrings.back().c_str());
                    }
                    args.push_back("-std=c++20");
                    std::replace(files[i].begin(), files[i].end(), '\\', '/');
                    //std::cout<<"file : "<<files[i]<<std::endl;
                    CXTranslationUnit tu = clang_parseTranslationUnit(
                        index,
                        files[i].c_str(),            // ton fichier source
                        args.data(), args.size(),                 // options
                        nullptr, 0,              // pas de fichiers pr�compil�s
                        CXTranslationUnit_None
                    );
                    /*unsigned diagCount = clang_getNumDiagnostics(tu);
                    for (unsigned i = 0; i < diagCount; i++) {
                        CXDiagnostic diag = clang_getDiagnostic(tu, i);
                        CXString msg = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());
                        std::cout << clang_getCString(msg) << std::endl; clang_disposeString(msg);
                        clang_disposeDiagnostic(diag);
                    }*/
                    CXCursor rootCursor = clang_getTranslationUnitCursor(tu);
                    clang_visitChildren(rootCursor, classesVisitor, &ctx);
                    clang_disposeTranslationUnit(tu);
                    clang_disposeIndex(index);
                }

                //We get the project directory, and concat it with the folder path.

                
                return ctx.classes;
            }
            static Class getClassFromMemory(std::vector<std::string> includePaths, std::string virtualFile, std::string name, std::string content, std::string nspc) {

                Class cl("", "");
                Context ctx(cl);
                ctx.datas.push_back(nspc);
                ctx.datas.push_back(name);
                CXIndex index = clang_createIndex(0, 0);
                std::vector<const char*> args;
                args.reserve(includePaths.size() * 2 + 1);
                // Ajouter les include dirs
                for (auto& path : includePaths) {
                    std::filesystem::path p = std::filesystem::canonical(stripQuotes(std::string(path)));
                    std::string canonical = p.string();
                    std::replace(canonical.begin(), canonical.end(), '\\', '/');
                    args.push_back("-I");
                    args.push_back(canonical.c_str());
                }
                args.push_back("-std=c++20");
                CXUnsavedFile unsaved;
                unsaved.Filename = virtualFile.c_str();        // nom virtuel
                unsaved.Contents = content.c_str();        // contenu de ta TextArea
                unsaved.Length = content.size();

                CXTranslationUnit tu = clang_parseTranslationUnit(
                    index,
                    virtualFile.c_str(),            // ton fichier source
                    args.data(), args.size(),                 // options
                    &unsaved, 1,              // pas de fichiers pr�compil�s
                    CXTranslationUnit_None
                );
                unsigned diagCount = clang_getNumDiagnostics(tu);
                for (unsigned i = 0; i < diagCount; i++) {
                    CXDiagnostic diag = clang_getDiagnostic(tu, i);
                    CXString msg = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());
                    std::cout << clang_getCString(msg) << std::endl; clang_disposeString(msg);
                    clang_disposeDiagnostic(diag);
                }
                ctx.tu = tu;
                CXCursor rootCursor = clang_getTranslationUnitCursor(tu);
                clang_visitChildren(rootCursor, classVisitor, &ctx);                
                return ctx.cl;
            }
            static Class getClass(std::vector<std::string> includePaths, std::string name, std::string path, std::string nspc) {
                Class cl("", "");
                Context ctx(cl);
                ctx.datas.push_back(nspc);
                ctx.datas.push_back(name);
                std::string appiDir;
                if (path.find("C:\\") == std::string::npos)
                    appiDir = path != "" ? getCurrentPath() + "\\" + path : getCurrentPath();
                else
                    appiDir = path;
                std::vector<std::string> files;
                //Find headers c++ files in the specified folder's path.
                findFiles(".cpp .c", files, appiDir);

                //browse every header files.
                for (unsigned int i = 0; i < files.size(); i++) {
                    //std::cout<<"get classes : "<<files[i]<<std::endl;
                    CXIndex index = clang_createIndex(0, 0);
                    std::vector<const char*> args;
                    args.reserve(includePaths.size() * 2 + 1);
                    // Ajouter les include dirs
                    std::vector<std::string> stableStrings;
                    for (auto& path : includePaths) {
                        std::filesystem::path p = std::filesystem::canonical(stripQuotes(std::string(path)));
                        std::string canonical = p.string();
                        std::replace(canonical.begin(), canonical.end(), '\\', '/');
                        stableStrings.push_back(canonical);
                        args.push_back("-I");
                        args.push_back(stableStrings.back().c_str());
                    }
                    args.push_back("-std=c++20");
                    std::replace(files[i].begin(), files[i].end(), '\\', '/');
                    CXTranslationUnit tu = clang_parseTranslationUnit(
                        index,
                        files[i].c_str(),            // ton fichier source
                        args.data(), args.size(),                 // options
                        nullptr, 0,              // pas de fichiers pr�compil�s
                        CXTranslationUnit_None
                    );
                    /*unsigned diagCount = clang_getNumDiagnostics(tu);
                    for (unsigned i = 0; i < diagCount; i++) {
                        CXDiagnostic diag = clang_getDiagnostic(tu, i);
                        CXString msg = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());
                        std::cout << clang_getCString(msg) << std::endl; clang_disposeString(msg);
                        clang_disposeDiagnostic(diag);
                    }*/
                    ctx.tu = tu;
                    CXCursor rootCursor = clang_getTranslationUnitCursor(tu);
                    clang_visitChildren(rootCursor, classVisitor, &ctx);
                    clang_disposeTranslationUnit(tu);
                    clang_disposeIndex(index);
                }                
                return cl;
            }           
            
            std::string getName() {
                return name;
            }
            std::string getFilePath() {
                return filePath;
            }
            std::string getImplFilePath() {
                return implFilePath;
            }
            std::vector<Constructor> getConstructors() {
                return constructors;
            }
            std::vector<MemberFunction> getMembersFunctions() {
                return memberFunctions;
            }
            std::vector<MemberVariable> getMembersVariables() {
                return memberVariables;
            }
            std::string getNamespace() {
                return namespc;
            }
            std::vector<Class> getSuperClasses() {
                return superClasses;
            }
            private :
            static std::string getQualifiedNamespace(CXCursor cursor) {
                std::string ns;
                CXCursor parent = clang_getCursorLexicalParent(cursor);

                while (!clang_Cursor_isNull(parent) &&
                    clang_getCursorKind(parent) == CXCursor_Namespace) {

                    // Skip system namespaces
                    CXSourceLocation loc = clang_getCursorLocation(parent);
                    if (clang_Location_isInSystemHeader(loc)) {
                        parent = clang_getCursorLexicalParent(parent);
                        continue;
                    }

                    CXString spelling = clang_getCursorSpelling(parent);
                    std::string name = clang_getCString(spelling);
                    clang_disposeString(spelling);

                    // Skip inline/system/anonymous namespaces
                    if (name.empty() ||
                        name == "std" ||
                        name.rfind("__", 0) == 0 ||   // starts with "__"
                        name[0] == '_') {             // private/system
                        parent = clang_getCursorLexicalParent(parent);
                        continue;
                    }

                    ns = name + (ns.empty() ? "" : "::" + ns);
                    parent = clang_getCursorLexicalParent(parent);
                }

                return ns;
            }

            static std::string normalize(const std::string& path) {
                return std::filesystem::canonical(path).string();
            }
            static std::string stripQuotes(std::string s) {
                if (!s.empty() && s.front() == '"')
                    s.erase(0, 1);
                if (!s.empty() && s.back() == '"')
                    s.pop_back();
                return s;
            }
            static CXChildVisitResult classesVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
                //std::cout<<"visit classes"<<std::endl;
                CXCursorKind kind = clang_getCursorKind(cursor);
                CXString spelling = clang_getCursorSpelling(cursor);
                //if (clang_getCString(spelling) == "Caracter")
                    //std::cout<<"spelling : "<<clang_getCString(spelling)<<std::endl;


                if (kind == CXCursor_ClassDecl) {
                    CXSourceLocation loc = clang_getCursorLocation(cursor);
                    CXFile file; clang_getSpellingLocation(loc, &file, nullptr, nullptr, nullptr);
                    CXString fname = clang_getFileName(file);
                    std::string filename = clang_getCString(fname);
                    clang_disposeString(fname);
                    Context2* ctx = static_cast<Context2*>(client_data);
                    bool isInOneHeaderFile = false;
                    for (unsigned int i = 1; i < ctx->datas.size(); i++) {
                        //std::cout<<"paths : "<<normalize(filename)<<std::endl<<normalize(ctx->datas[i])<<std::endl;
                        if (normalize(filename) == normalize(ctx->datas[i])) {
                            isInOneHeaderFile = true;
                        }
                    }
                    if (!isInOneHeaderFile || !clang_isCursorDefinition(cursor)) {
                        return CXChildVisit_Continue;
                    }
                    std::string ns = getQualifiedNamespace(cursor);
                    if (ctx->datas[0] == "" || ctx->datas[0] == ns) {
                        bool contains = false;
                        for (unsigned int i = 0; i < ctx->classes.size() && !contains; i++) {
                            if (ctx->classes[i].first == clang_getCString(spelling) && ctx->classes[i].second == ns)
                                contains = true;
                        }
                        if (!contains) {
                            //std::cout<<"filename : "<<filename<<std::endl<<"class : "<<clang_getCString(spelling)<<std::endl;
                            ctx->classes.push_back(std::make_pair(clang_getCString(spelling), ns));
                        }
                    }
                }
                clang_disposeString(spelling);
                return CXChildVisit_Recurse; // continue � descendre dans l�AST
            }
            static CXChildVisitResult classVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
                CXCursorKind kind = clang_getCursorKind(cursor);
                CXString spelling = clang_getCursorSpelling(cursor);
                Context* ctx = static_cast<Context*>(client_data);
                if (kind == CXCursor_TranslationUnit || kind == CXCursor_Namespace) {
                    return CXChildVisit_Recurse;
                }
                if (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl || kind == CXCursor_EnumDecl) {
                    std::string ns = getQualifiedNamespace(cursor);
                    if ((ctx->datas[0] == "" || ctx->datas[0] == ns) && ctx->datas[1] == clang_getCString(spelling)) {
                        //std::cout<<"infos : "<<ctx->datas[0]<<","<<ctx->datas[1]<<","<<clang_getCString(spelling)<<std::endl;

                        CXSourceLocation loc = clang_getCursorLocation(cursor);
                        CXFile file;
                        unsigned line, column, offset;
                        clang_getFileLocation(loc, &file, &line, &column, &offset);
                        CXString filename = clang_getFileName(file);
                        ctx->cl.setName(clang_getCString(spelling));
                        ctx->cl.setFilePath(clang_getCString(filename));
                        ctx->cl.namespc = ns;
                        ctx->datas[0] = ns;
                        clang_disposeString(filename);
                        clang_visitChildren(cursor, classVisitor, ctx);

                    }
                    else {
                        CXCursorKind kind = clang_getCursorKind(parent);
                        if ((kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl)
                            && ctx->cl.namespc == getQualifiedNamespace(parent)
                            && ctx->cl.getName() == clang_getCString(clang_getCursorSpelling(parent))) {

                            CXSourceLocation loc = clang_getCursorLocation(cursor);
                            CXFile file;
                            unsigned line, column, offset;
                            clang_getFileLocation(loc, &file, &line, &column, &offset);
                            CXString filename = clang_getFileName(file);
                            Class innerClass(ctx->cl.getName() + "::" + clang_getCString(spelling), clang_getCString(filename));
                            innerClass.namespc = ns;
                            ctx->cl.addInnerClass(innerClass);
                            Context innerCtx(ctx->cl.innerClasses.back());
                            innerCtx.tu = ctx->tu;
                            innerCtx.datas = ctx->datas;
                            clang_disposeString(filename);
                            clang_visitChildren(cursor, classVisitor, &innerCtx);
                        }
                        return CXChildVisit_Recurse;
                    }
                }
                else if (kind == CXCursor_CXXBaseSpecifier) {
                    CXCursorKind kind = clang_getCursorKind(parent);
                    //std::cout<<"ctx nsp : "<<ctx->cl.namespc<<std::endl<<"parent nsp : "<<getQualifiedNamespace(parent)<<std::endl<<"ctx name : "<<ctx->cl.getName()<<std::endl<<"parent name : "<<clang_getCString(clang_getCursorSpelling(parent))<<std::endl;
                    if ((kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl)
                        && ctx->cl.namespc == getQualifiedNamespace(parent)
                        && ctx->cl.getName() == clang_getCString(clang_getCursorSpelling(parent))) {
                        CXType baseType = clang_getCursorType(cursor);

                        CXString baseName = clang_getTypeSpelling(baseType);
                        CXCursor baseCursor = clang_getTypeDeclaration(baseType);
                        if (clang_Cursor_isNull(baseCursor))
                            return CXChildVisit_Continue;
                        std::string bname = clang_getCString(baseName);
                        std::vector<std::string> parts = split(bname, "::");

                        std::string ns = getQualifiedNamespace(baseCursor);
                        CXSourceLocation loc = clang_getCursorLocation(baseCursor);
                        CXFile file;
                        unsigned line, column, offset;
                        clang_getFileLocation(loc, &file, &line, &column, &offset);
                        CXString filename = clang_getFileName(file);
                        Class superClass(parts[parts.size() - 1], clang_getCString(filename));
                        superClass.namespc = ns;
                        ctx->cl.addSuperClass(superClass);
                        Context superCtx(ctx->cl.superClasses.back());
                        superCtx.tu = ctx->tu;
                        superCtx.datas = ctx->datas;
                        clang_disposeString(baseName);
                        clang_disposeString(filename);

                        clang_visitChildren(baseCursor, classVisitor, &superCtx);
                    }
                    return CXChildVisit_Continue;
                }
                else if (kind == CXCursor_Constructor) {
                    //std::cout<<"const decl"<<std::endl;
                    CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
                    CXString parentName = clang_getCursorSpelling(parentCursor);
                    std::string parentNs = getQualifiedNamespace(parentCursor);
                    if (ctx->cl.getName() == clang_getCString(parentName) && ctx->cl.namespc == parentNs) {
                        //std::cout<<"add constructor : "<<ctx->cl.getName()<<std::endl;
                        if (clang_isCursorDefinition(cursor)) {
                            Constructor c(clang_getCString(spelling));
                            c.tu = ctx->tu;
                            clang_visitChildren(cursor, constructorVisitor, &c);
                            ctx->cl.addConstructor(c);
                        }
                    }
                    clang_disposeString(parentName);
                }
                else if (kind == CXCursor_CXXMethod) {
                    //std::cout<<"method decl"<<std::endl;
                    CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
                    CXString parentName = clang_getCursorSpelling(parentCursor);
                    std::string parentNs = getQualifiedNamespace(parentCursor);
                    if (ctx->cl.getName() == clang_getCString(parentName) && ctx->cl.namespc == parentNs) {
                        if (clang_isCursorDefinition(cursor)) {
                            // Type de retour
                            CXType funcType = clang_getCursorType(cursor);
                            CXType returnType = clang_getResultType(funcType);

                            CXString typeStr = clang_getTypeSpelling(returnType);

                            MemberFunction m(clang_getCString(typeStr), clang_getCString(spelling));

                            CXSourceLocation loc = clang_getCursorLocation(cursor);
                            CXFile file;
                            unsigned line, col, offset;
                            clang_getFileLocation(loc, &file, &line, &col, &offset);

                            ctx->cl.setImplFilePath(clang_getCString(clang_getFileName(file)));
                            m.location = std::make_pair(line, col);
                            m.offset = offset;
                            m.tu = ctx->tu;


                            //std::cout<<"found function definition at : "<<m.location.first<<","<<m.location.second<<std::endl;
                            clang_visitChildren(cursor, memberFonctionVisitor, &m);
                            ctx->cl.addMemberFunction(m);

                            clang_disposeString(typeStr);
                        }
                    }
                    clang_disposeString(parentName);
                }
                else if (kind == CXCursor_FieldDecl) {
                    //std::cout<<"field decl"<<std::endl;
                    CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
                    CXString parentName = clang_getCursorSpelling(parentCursor);
                    std::string parentNs = getQualifiedNamespace(parentCursor);
                    if (ctx->cl.getName() == clang_getCString(parentName) && ctx->cl.namespc == parentNs) {
                        CXString spelling = clang_getCursorSpelling(cursor);
                        std::string name = clang_getCString(spelling);
                        CXType t = clang_getCursorType(cursor);
                        CXString ts = clang_getTypeSpelling(t);
                        std::string type = clang_getCString(ts);
                        clang_disposeString(spelling);

                        MemberVariable mv;
                        mv.setVarName(name);
                        mv.setVarType(type);
                        ctx->cl.addMemberVariable(mv);
                    }
                    clang_disposeString(parentName);
                }
                clang_disposeString(spelling);
                return CXChildVisit_Continue;
            }
            static CXChildVisitResult constructorVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
                if (clang_getCursorKind(cursor) == CXCursor_ParmDecl) {
                    Constructor* c = static_cast<Constructor*>(client_data);
                    CXString s = clang_getCursorSpelling(cursor);
                    std::string name = clang_getCString(s);
                    clang_disposeString(s);
                    CXSourceRange range = clang_getCursorExtent(cursor);
                    CXToken* tokens = nullptr;
                    unsigned numTokens = 0;
                    clang_tokenize(c->tu, range, &tokens, &numTokens);
                    std::string type;
                    for (unsigned i = 0; i < numTokens; ++i) {
                        CXString sp = clang_getTokenSpelling(c->tu, tokens[i]);
                        std::string tok = clang_getCString(sp);
                        clang_disposeString(sp);
                        if (tok == name) break; // on s'arr�te au nom
                        // gestion des espaces
                        if (!type.empty()) {
                            CXTokenKind kind = clang_getTokenKind(tokens[i]);
                            CXTokenKind prev = clang_getTokenKind(tokens[i - 1]);
                            bool needSpace = (prev == CXToken_Identifier && kind == CXToken_Identifier)
                                || (prev == CXToken_Identifier && kind == CXToken_Keyword)
                                || (prev == CXToken_Keyword && kind == CXToken_Identifier)
                                || (prev == CXToken_Keyword && kind == CXToken_Keyword);
                            if (kind == CXToken_Punctuation)
                                needSpace = false;
                            if (needSpace)
                                type += " ";
                        }
                        type += tok;
                    }
                    clang_disposeTokens(c->tu, tokens, numTokens);
                    c->addArgName(std::string(name));
                    c->addArgType(std::string(type));

                }
                return CXChildVisit_Recurse;

            }
            static CXChildVisitResult memberFonctionVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
                if (clang_getCursorKind(cursor) == CXCursor_ParmDecl) {
                    MemberFunction* m = static_cast<MemberFunction*>(client_data);
                    CXString s = clang_getCursorSpelling(cursor);
                    std::string name = clang_getCString(s);
                    clang_disposeString(s);
                    CXSourceRange range = clang_getCursorExtent(cursor);
                    CXToken* tokens = nullptr;
                    unsigned numTokens = 0;
                    clang_tokenize(m->tu, range, &tokens, &numTokens);
                    std::string type;
                    for (unsigned i = 0; i < numTokens; ++i) {
                        CXString sp = clang_getTokenSpelling(m->tu, tokens[i]);
                        std::string tok = clang_getCString(sp);
                        clang_disposeString(sp);
                        if (tok == name) break; // on s'arr�te au nom
                        // gestion des espaces
                        if (!type.empty()) {
                            CXTokenKind kind = clang_getTokenKind(tokens[i]);
                            CXTokenKind prev = clang_getTokenKind(tokens[i - 1]);
                            bool needSpace = (prev == CXToken_Identifier && kind == CXToken_Identifier)
                                || (prev == CXToken_Identifier && kind == CXToken_Keyword)
                                || (prev == CXToken_Keyword && kind == CXToken_Identifier)
                                || (prev == CXToken_Keyword && kind == CXToken_Keyword);
                            if (kind == CXToken_Punctuation)
                                needSpace = false;
                            if (needSpace)
                                type += " ";
                        }
                        type += tok;
                    }
                    clang_disposeTokens(m->tu, tokens, numTokens);

                    m->addArgName(std::string(name));
                    m->addArgType(std::string(type));

                }
                return CXChildVisit_Recurse;

            }
            void addSuperClass(Class cl) {
                bool contains = false;
                for (unsigned int i = 0; i < superClasses.size(); i++) {
                    if (cl.getName() == superClasses[i].getName() && cl.getNamespace() == superClasses[i].getNamespace()) {
                        contains = true;
                    }
                }
                if (!contains) {
                    superClasses.push_back(cl);
                }
            }
            void setNamespace(std::string namespc) {
                this->namespc = namespc;
            }
            void addInnerClass(Class cl) {
                bool contains = false;
                for (unsigned int i = 0; i < innerClasses.size(); i++) {
                    if (cl.getName() == innerClasses[i].getName() && cl.getNamespace() == innerClasses[i].getNamespace()) {
                        contains = true;
                    }
                }
                if (!contains) {
                    innerClasses.push_back(cl);
                }
            }
            void addConstructor(Constructor c) {
                bool contains = false;
                for (unsigned int i = 0; i < constructors.size() && !contains; i++) {
                    if (constructors[i].getName() == c.getName() && constructors[i].getArgsTypes().size() == c.getArgsTypes().size()) {
                        bool equals = true;
                        for (unsigned int j = 0; j < constructors[i].getArgsTypes().size() && equals; j++) {
                            if (constructors[i].getArgsTypes()[j] != c.getArgsTypes()[j]) {
                                equals = false;
                            }
                        }
                        if (equals) {
                            contains = true;
                        }
                    }
                }
                if (!contains) {
                    //std::cout<<"add constructor : "<<c.getName()<<std::endl;
                    constructors.push_back(c);
                }
            }
            void addMemberFunction(MemberFunction mf) {
                bool contains = false;
                for (unsigned int i = 0; i < memberFunctions.size() && !contains; i++) {
                    if (memberFunctions[i].getName() == mf.getName() && memberFunctions[i].getArgsTypes().size() == mf.getArgsTypes().size()) {
                        bool equals = true;
                        for (unsigned int j = 0; j < memberFunctions[i].getArgsTypes().size() && equals; j++) {
                            if (memberFunctions[i].getArgsTypes()[j] != mf.getArgsTypes()[j]) {
                                equals = false;
                            }
                        }
                        if (equals) {
                            contains = true;
                        }
                    }
                }
                if (!contains) {
                    memberFunctions.push_back(mf);
                }
            }
            void addMemberVariable(MemberVariable mb) {
                memberVariables.push_back(mb);
            }
            std::string name; /**> the name of the class.*/
            std::string filePath; /**> the file path of the class's header.*/
            std::string implFilePath;
            std::string namespc; /**> the namespace name.*/
            std::vector<Class> innerClasses; /**> the inner classes.*/
            std::vector<Class> superClasses; /**> the base classes.*/
            std::vector<Constructor> constructors; /** the constructors. */
            std::vector<MemberFunction> memberFunctions; /** the member functions. */
            std::vector<MemberVariable> memberVariables;
        };        
    }
}

