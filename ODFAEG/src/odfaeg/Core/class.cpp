#include "../../../include/odfaeg/Core/class.hpp"
#include <filesystem>
namespace odfaeg {
    namespace core {
        using namespace std;

        Class::Class(std::string name, std::string filePath) : name(name), filePath(filePath), implFilePath("") {

        }
        void Class::setName(std::string name) {
            this->name = name;
        }
        void Class::setFilePath(std::string filePath) {
            this->filePath = filePath;
        }
        void Class::setImplFilePath(std::string filePath) {
            implFilePath = filePath;
        }
        std::string Class::getQualifiedNamespace(CXCursor cursor) {
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

        std::string Class::normalize(const std::string& path) {
            return std::filesystem::canonical(path).string();
        }
        std::string Class::stripQuotes(std::string s) {
            if (!s.empty() && s.front() == '"')
                s.erase(0, 1);
            if (!s.empty() && s.back() == '"')
                s.pop_back();
            return s;
        }
        CXChildVisitResult Class::classesVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
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
            return CXChildVisit_Recurse; // continue à descendre dans l’AST
        }
        CXChildVisitResult Class::classVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
            CXCursorKind kind = clang_getCursorKind(cursor);
            CXString spelling = clang_getCursorSpelling(cursor);
            Context* ctx = static_cast<Context*>(client_data);
            if (kind == CXCursor_TranslationUnit || kind == CXCursor_Namespace) {
                return CXChildVisit_Recurse;
            }
            if (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl || kind == CXCursor_EnumDecl) {
                std::string ns = getQualifiedNamespace(cursor);
                if ((ctx->datas[0] == "" ||  ctx->datas[0] == ns) && ctx->datas[1] == clang_getCString(spelling)) {
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

                } else {
                    CXCursorKind kind = clang_getCursorKind(parent);
                    if ((kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl)
                        && ctx->cl.namespc == getQualifiedNamespace(parent)
                        && ctx->cl.getName() == clang_getCString(clang_getCursorSpelling(parent))) {

                        CXSourceLocation loc = clang_getCursorLocation(cursor);
                        CXFile file;
                        unsigned line, column, offset;
                        clang_getFileLocation(loc, &file, &line, &column, &offset);
                        CXString filename = clang_getFileName(file);
                        Class innerClass(ctx->cl.getName()+"::"+clang_getCString(spelling), clang_getCString(filename));
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
            } else if (kind == CXCursor_CXXBaseSpecifier) {
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
                    Class superClass(parts[parts.size()-1], clang_getCString(filename));
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
            } else if (kind == CXCursor_Constructor) {
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
            } else if (kind == CXCursor_CXXMethod) {
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
            } else if (kind == CXCursor_FieldDecl) {
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
        CXChildVisitResult Class::constructorVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
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
                    if (tok == name) break; // on s'arrête au nom
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
        CXChildVisitResult Class::memberFonctionVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
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
                    if (tok == name) break; // on s'arrête au nom
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
        std::vector<std::pair<std::string, std::string>> Class::getClassesFromMemory(std::vector<std::string> includePaths, std::string virtualFile, std::string virtualPath, std::string content, std::string nspc) {
            Context2 ctx;
            ctx.datas.push_back(nspc);
            std::string appiDir;
            if (virtualPath.find("C:\\") == std::string::npos)
                appiDir = virtualPath != "" ? getCurrentPath()+"\\"+virtualPath : getCurrentPath();
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
            unsaved.Length   = content.size();

            CXTranslationUnit tu = clang_parseTranslationUnit(
                index,
                virtualFile.c_str(),            // ton fichier source
                args.data(), args.size(),                 // options
                &unsaved, 1,              // pas de fichiers précompilés
                CXTranslationUnit_None
            );
            CXCursor rootCursor = clang_getTranslationUnitCursor(tu);
            clang_visitChildren(rootCursor, classesVisitor, &ctx);
            //Read header files.
            /*istringstream iss;
            iss.str(content);

            std::string line;
            //Read file's lines.
            while(getline(iss, line)) {
                //check if we find the c++ class keyword and the beginning of a class definition.
                if (line.find("class") != std::string::npos && line.find("{") != std::string::npos) {
                    //remove spaces at the beginning of the line.
                    while (line.size() > 0 && line.at(0) == ' ') {
                        line.erase(0, 1);
                    }
                    //split the std::string to get the class name.
                    std::vector<std::string> parts = split(line, " ");
                    //remove spaces before the class name.
                    while (parts[1].size() > 0 && parts[1].at(0) == ' ') {
                        parts[1].erase(0, 1);
                    }
                    //add the class name to the vector.
                    classes.push_back(parts[1]);
                }
            }*/
            return ctx.classes;
        }

        std::vector<std::pair<std::string, std::string>> Class::getClasses(std::vector<std::string> includePaths, std::string path, std::string nspc) {
            Context2 ctx;
            ctx.datas.push_back(nspc);
            std::string appiDir;
            if (path.find("C:\\") == std::string::npos)
                appiDir = path != "" ? getCurrentPath()+"\\"+path : getCurrentPath();
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
                    nullptr, 0,              // pas de fichiers précompilés
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

            /*std::string appiDir;
            if (path.find("C:\\") == std::string::npos)
                appiDir = path != "" ? getCurrentPath()+"\\"+path : getCurrentPath();
            else
                appiDir = path;
            std::vector<std::string> files;
            findFiles(".hpp .h", files, appiDir);
            for (unsigned int i = 0; i < files.size(); i++) {
                //Read header files.
                ifstream ifs(files[i]);

                if (ifs) {
                    std::string line;
                    //Read file's lines.
                    while(getline(ifs, line)) {
                        //check if we find the c++ class keyword and the beginning of a class definition.
                        if (line.find("class") != std::string::npos && line.find("{") != std::string::npos) {
                            //remove spaces at the beginning of the line.
                            while (line.size() > 0 && line.at(0) == ' ') {
                                line.erase(0, 1);
                            }
                            //split the std::string to get the class name.
                            std::vector<std::string> parts = split(line, " ");
                            //remove spaces before the class name.
                            while (parts[1].size() > 0 && parts[1].at(0) == ' ') {
                                parts[1].erase(0, 1);
                            }
                            //add the class name to the vector.
                            classes.push_back(parts[1]);
                        }
                    }
                }
            }*/
            return ctx.classes;
        }
        Class Class::getClassFromMemory(std::vector<std::string> includePaths, std::string virtualFile, std::string name, std::string content, std::string nspc) {

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
            unsaved.Length   = content.size();

            CXTranslationUnit tu = clang_parseTranslationUnit(
                index,
                virtualFile.c_str(),            // ton fichier source
                args.data(), args.size(),                 // options
                &unsaved, 1,              // pas de fichiers précompilés
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
            /*std::string headerFile;
            std::string namespc="";

            istringstream iss;
            iss.str(content);
            std::string line;
            //Read lines.

            while(getline(iss, line)) {
                //std::cout<<"get line"<<std::endl;
                //Ignore c++ comments.
                if (line.find("/*") != std::string::npos || line.find("/**") != std::string::npos) {
                    while(line.find("*//*") == std::string::npos && getline(iss, line)) {

                    }
                }
                //We must also ignore single line comments.
                if (line.find("//") == std::string::npos && line.find("*//*") == std::string::npos) {
                    //Remove template declaration.
                    if (line.find("template ") != std::string::npos) {
                        //check begin and end of template declaration and remove it.
                        int pos = line.find("template ");
                        int pos2 = line.find(">");
                        line.erase(pos, pos2 - pos+1);
                    }
                    //If the template declaration contains other template declarations, we remove them too.
                    if (line.find("<") != std::string::npos) {
                        //check begin and end of template declaration and remove it.
                        int pos = line.find("<");
                        int pos2 = line.find(">");
                        line.erase(pos, pos2 - pos+1);
                    }
                    //split string if the class inherits from base classes.
                    std::vector<std::string> parts = split(line, ":");
                    //if we find the class or struct keywork and the name of the c++ class match, we have found the class.
                    if (parts.size() > 0 && (parts[0].find("class ") != std::string::npos || parts[0].find("struct ") != std::string::npos) && parts[0].find(name) != std::string::npos) {

                        //Remove everything which is after class definition.
                        if (parts[0].find("{") != std::string::npos) {
                            int pos = parts[0].find("{");
                            parts[0].erase(pos);
                        }
                        //split the string.
                        std::vector<std::string> parts2 = split(parts[0], " ");
                        //remove space before and after class name.
                        while(parts2[parts2.size()-1].size() > 0 && parts2[parts2.size()-1].at(0) == ' ') {
                            parts2[parts2.size()-1].erase(0, 1);
                        }
                        while(parts2[parts2.size()-1].size() > 0 && parts2[parts2.size()-1].at(parts2[parts2.size()-1].size()-1) == ' ') {
                            parts2[parts2.size()-1].erase(parts2[parts2.size()-1].size()-1, 1);
                        }
                        //We have found the class name, we set the header file path and the boolean to stop the loop.
                        if (parts2[parts2.size()-1] == name) {
                            found = true;
                        }
                    }

                }
            }

            //If we have found the class we check informations about the class.
            if (found) {
                found = false;

                //check each namespaces englobing the class.
                while(content.find("namespace") != std::string::npos && !found) {
                    //Find the namespace pos.
                    unsigned int pos = content.find("namespace");
                    //Check the namespace name.
                    content = content.substr(pos+9, content.size()-pos-9);

                    while(content.size() > 0 && (content.at(0) == ' ' || content.at(0) == '\n')) {
                        content.erase(0, 1);
                    }
                    std::vector<std::string> parts = split(content, "{");
                    //We add :: for each sub namespaces found.

                    //we must check if the namespace is declared before the class.
                    pos = content.find("{");
                    int pos2 = findLastBracket(content, 0);

                    unsigned int pos3 = content.find(name);

                    //if there is no more namespace declaration after the class name we can check if the class is in the given namespace.
                    if (pos < pos3 && pos2 > pos3) {
                        if (namespc == "")
                            namespc += parts[0];
                        else
                            namespc += "::"+parts[0];
                    }
                    if (nspc != "") {
                        if (namespc == nspc) {
                            found = true;
                        }
                    } else {
                        found = false;
                    }
                }
                if (nspc == "")
                    found = true;
                //We have found the class in the specified namespace, we can get class's informations.
                if (found) {

                    Class cl(name, headerFile);
                    cl.setNamespace(namespc);

                    checkSuperClasses(content, cl);
                    //std::cout<<"sub classes checked"<<std::endl;
                    std::string innerClass = "";
                    std::string type = "";
                    int lvl = 0;
                    //At the first recursion the inner class name is empty, the recursion lvl is 0 and the class type is empty.
                    checkInnerClass(innerClass, type, content, lvl,  cl);
                    //std::cout<<"inner classes check"<<std::endl;
                    checkConstructors(content, cl);
                    //std::cout<<"constructor checked"<<std::endl;
                    checkMembersFunctions(content, cl);
                    //std::cout<<"member function checked"<<std::endl;
                    checkMembersVariables(content, cl);
                    //std::cout<<"member variable checked"<<std::endl;
                    return cl;
                }
            }
            Class cl("", "");*/
            return ctx.cl;
        }
        Class Class::getClass(std::vector<std::string> includePaths, std::string name, std::string path, std::string nspc) {
            Class cl("", "");
            Context ctx(cl);
            ctx.datas.push_back(nspc);
            ctx.datas.push_back(name);
            std::string appiDir;
            if (path.find("C:\\") == std::string::npos)
                appiDir = path != "" ? getCurrentPath()+"\\"+path : getCurrentPath();
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
                    nullptr, 0,              // pas de fichiers précompilés
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

            //If the path is not specified the folder to search c++ classes in is the project's directory.
            /*std::string appiDir;
            if (path.find("C:\\") == std::string::npos)
                appiDir = path != "" ? getCurrentPath()+"\\"+path : getCurrentPath();
            else
                appiDir = path;
            std::vector<std::string> files;
            //Find headers c++ files in the specified folder's path.
            findFiles(".hpp .h", files, appiDir);
            bool found=false;
            std::string fileContent;
            std::string headerFile;
            std::string namespc="";
            //browse every header files.
            for (unsigned int i = 0; i < files.size() && !found; i++) {
                //Read files.

                ifstream ifs(files[i]);
                if (ifs) {
                    std::string line;
                    fileContent="";
                    //Read lines.
                    while(getline(ifs, line)) {
                        //Ignore c++ comments.
                        if (line.find("/*") != std::string::npos || line.find("/**") != std::string::npos) {
                            while(line.find("*//*") == std::string::npos && getline(ifs, line)) {

                            }
                        }
                        //We must also ignore single line comments.
                        if (line.find("//") == std::string::npos && line.find("*//*") == std::string::npos) {
                            //Remove template declaration.
                            if (line.find("template ") != std::string::npos) {
                                //check begin and end of template declaration and remove it.
                                int pos = line.find("template ");
                                int pos2 = line.find(">");
                                line.erase(pos, pos2 - pos+1);
                            }
                            //If the template declaration contains other template declarations, we remove them too.
                            if (line.find("<") != std::string::npos) {
                                //check begin and end of template declaration and remove it.
                                int pos = line.find("<");
                                int pos2 = line.find(">");
                                line.erase(pos, pos2 - pos+1);
                            }
                            //split string if the class inherits from base classes.
                            std::vector<std::string> parts = split(line, ":");
                            //if we find the class or struct keywork and the name of the c++ class match, we have found the class.
                            if (parts.size() > 0 && (parts[0].find("class ") != std::string::npos || parts[0].find("struct ") != std::string::npos) && parts[0].find(name) != std::string::npos) {

                                //Remove everything which is after class definition.
                                if (parts[0].find("{") != std::string::npos) {
                                    int pos = parts[0].find("{");
                                    parts[0].erase(pos);
                                }
                                //split the string.
                                std::vector<std::string> parts2 = split(parts[0], " ");
                                //remove space before and after class name.
                                while(parts2[parts2.size()-1].size() > 0 && parts2[parts2.size()-1].at(0) == ' ') {
                                    parts2[parts2.size()-1].erase(0, 1);
                                }
                                while(parts2[parts2.size()-1].size() > 0 && parts2[parts2.size()-1].at(parts2[parts2.size()-1].size()-1) == ' ') {
                                    parts2[parts2.size()-1].erase(parts2[parts2.size()-1].size()-1, 1);
                                }
                                //We have found the class name, we set the header file path and the boolean to stop the loop.
                                if (parts2[parts2.size()-1] == name) {
                                    headerFile = files[i];
                                    found = true;
                                }
                            }
                            //put every lines of the file header file to a string.
                            fileContent += line+"\n";

                        }
                    }
                    ifs.close();

                }
            }

            //If we have found the class we check informations about the class.
            if (found) {
                found = false;
                //check each namespaces englobing the class.
                while(fileContent.find("namespace") != std::string::npos && !found) {
                    //Find the namespace pos.
                    unsigned int pos = fileContent.find("namespace");
                    //Check the namespace name.
                    fileContent = fileContent.substr(pos+9, fileContent.size()-pos-9);

                    while(fileContent.size() > 0 && (fileContent.at(0) == ' ' || fileContent.at(0) == '\n')) {
                        fileContent.erase(0, 1);
                    }
                    std::vector<std::string> parts = split(fileContent, "{");
                    //We add :: for each sub namespaces found.

                    //we must check if the namespace is declared before the class.
                    pos = fileContent.find("{");
                    int pos2 = findLastBracket(fileContent, 0);

                    unsigned int pos3 = fileContent.find(name);

                    //if there is no more namespace declaration after the class name we can check if the class is in the given namespace.
                    if (pos < pos3 && pos2 > pos3) {
                        if (namespc == "")
                            namespc += parts[0];
                        else
                            namespc += "::"+parts[0];
                    }
                    //if the class is contained in a namespace we must check if a class which the same name is not present in an another namespace.
                    if (nspc != "") {
                        if (namespc == nspc) {
                            found = true;
                        }
                    } else {
                        found = false;
                    }
                }
                if (nspc == "")
                    found = true;
                //We have found the class in the specified namespace, we can get class's informations.
                if (found) {

                    Class cl(name, headerFile);
                    cl.setNamespace(namespc);
                    checkSuperClasses(fileContent, cl);
                    std::string innerClass = "";
                    std::string type = "";
                    int lvl = 0;
                    //At the first recursion the inner class name is empty, the recursion lvl is 0 and the class type is empty.
                    checkInnerClass(innerClass, type, fileContent, lvl,  cl);
                    checkConstructors(fileContent, cl);
                    checkMembersFunctions(fileContent, cl);
                    checkMembersVariables(fileContent, cl);
                    return cl;
                }
            }
            Class cl("", "");*/
            return cl;
        }
        /*void Class::checkSuperClasses (std::string &fileContent, Class& cl) {


            bool found = false;
            std::string str = fileContent;
            int pos = fileContent.find("{");
            std::string names="";
            //erase everything which is before class definition.
            if (pos != std::string::npos){

                names = fileContent.substr(0, pos);
                fileContent = fileContent.substr(pos+1, fileContent.size() - pos-1);
            }
            if (names != "") {
                //if the base classes are in a nested name, we need to remove remove the :: before splitting with the :.
                std::vector<std::string> parts = split(names, "::");
                std::string str;
                for (unsigned int i = 0; i < parts.size(); i++) {
                    str += parts[i];
                    if (i != parts.size()-1)
                        str+=" ";
                }
                //So we can split to get base classes names specified after the :.
                parts = split(str, ":");
                if (parts.size() > 1) {
                    //split to get all base classes names.
                    parts = split(parts[1], ",");

                    for (unsigned int i = 0; i < parts.size(); i++) {
                        //split to extract nested name spacifiers.
                        std::vector<std::string> parts2 = split(parts[i], " ");
                        if (parts2.size() > 0) {
                            //Remove spaces.
                            while (parts2[parts2.size()-1].size() > 0 && parts2[parts2.size()-1].at(0) == ' ') {
                                parts2[parts2.size()-1] = parts2[parts2.size()-1].erase(0, 1);
                            }
                            while (parts2[parts2.size()-1].size() > 0 && parts2[parts2.size()-1].at(parts2[parts2.size()-1].size()-1) == ' ') {
                                parts2[parts2.size()-1] = parts2[parts2.size()-1].erase(parts2[parts2.size()-1].size()-1, 1);
                            }
                            //Remove template parameters if the base class is template.
                            int pos = parts2[parts2.size()-1].find("<");
                            if (pos != std::string::npos) {
                                parts2[parts2.size()-1].erase(pos);
                            }
                            //we must check if the base class name if different from the current class name.
                            if (cl.getName() != parts2[parts2.size()-1]) {
                                //Get informations about the base class.
                                Class cla = Class::getClass(parts2[parts2.size() - 1]);
                                //add the base class to the vector of base classes.
                                cl.addSuperClass(cla);
                            }
                        }

                    }
                }
            }

        }
        void Class::checkConstructors(std::string& fileContent, Class& cl) {

            if(fileContent.find("public") != std::string::npos) {
                int pos = fileContent.find("public");
                int pos2 = fileContent.find(":", pos);
                fileContent.erase(pos,pos2 - pos + 1);
                //std::cout<<"public file content : "<<fileContent<<std::endl;
            }
            if(fileContent.find("private") != std::string::npos) {
                int pos = fileContent.find("private");
                int pos2 = fileContent.find(":", pos);
                fileContent.erase(pos,pos2 - pos + 1);
                //std::cout<<"private file content : "<<fileContent<<std::endl;
            }
            if(fileContent.find("protected") != std::string::npos) {
                int pos = fileContent.find("protected");
                int pos2 = fileContent.find(":", pos);
                fileContent.erase(pos,pos2 - pos + 1);
                //std::cout<<"protected file content : "<<fileContent<<std::endl;
            }

            //The constructor definition can be at the same file than the constructor declaration, in this case, we need to split the string.
            while (fileContent.find("{") != std::string::npos) {
                //Count the number of sub blocks (introduced by if, while, etc...) and remove they definition.
                unsigned int pos = fileContent.find("{");
                unsigned int pos2 = findLastBracket(fileContent, 0);
                fileContent.erase(pos, pos2 - pos-1);
                fileContent.insert(pos-1, ";");
            }
            //We also need to erase the encapsulation keywords because we don't need it.


            //we also need to split when the constructor definition is not in the header.
            std::vector<std::string> parts2 = split(fileContent, ";");
            for (unsigned int j = 0; j < parts2.size(); j++) {
                std::string str;
                //erase constructor name to get constructor arguments.
                if(parts2[j].find("(") != std::string::npos) {
                    int pos = parts2[j].find("(");
                    str = parts2[j].substr(0, pos+1);
                    //Erase spaces before the \n.
                    while (str.size() > 1 && str.at(str.size()-2) == ' ') {
                        str = str.erase(str.size()-2, 1);

                    }
                }
                if (str == "") {
                    str = parts2[j];
                }
                //check the position of the constructor argument definition.
                int index = parts2[j].find("(");
                if (index != std::string::npos) {
                    //Remove the (.
                    std::string name = parts2[j].substr(0, index);
                    name.erase(0, 1);
                    //Remove spaces and \n at the beginning and at then end.
                    while (name.size() > 0 && name.at(0) == ' ' || name.at(0) == '\n') {
                        name = name.erase(0, 1);
                    }
                    while (name.size() > 0 && name.at(name.size()-1) == ' ' || name.at(name.size()-1) == '\n') {
                        name = name.erase(name.size()-1, 1);
                    }
                    //Check if this is a constructor, in this case the name is the same than the class name.
                    if (name.find(cl.getName()) == 0 && name == cl.getName()) {

                       int pos3 = parts2[j].find_last_of(":");
                       if (pos3 -1 >= 0 && parts2[j].at(pos3-1) != ':') {
                           parts2[j].erase(pos3);
                       }
                       int pos = parts2[j].find("(");
                       int pos2 = parts2[j].find_last_of(")");

                       if (pos != std::string::npos && pos2 != std::string::npos) {
                           //Get constructor arguments.
                           std::string types = parts2[j].substr(pos+1, pos2-pos-1);
                           std::string tmp = types;
                           while (types.find("=") != std::string::npos) {
                                unsigned int pos = types.find("=");
                                unsigned int pos2;
                                if (types.find("(") != std::string::npos) {
                                    pos2 = types.find(")");
                                    types.erase(pos, pos2 - pos + 1);
                                    tmp.erase(pos, pos2 - pos + 1);
                                } else {
                                    pos2 = tmp.find(",");
                                    tmp.erase(pos, pos2-pos+1);
                                    types.erase(pos, pos2 - pos);
                                }
                           }

                           std::vector<std::string> arguments = split(types, ",");
                           Constructor constructor(cl.getName());
                           for (unsigned int i = 0; i < arguments.size(); i++) {
                                //remove spaces and \n.
                                while(arguments[i].size() > 0 && arguments[i].at(0) == ' ' || arguments[i].at(0) == '\n') {
                                    arguments[i] = arguments[i].erase(0, 1);
                                }
                                //split the arguments.

                                std::vector<std::string> argTypeName = split(arguments[i], " ");
                                std::string fullTypeName="";
                                std::string argName = "";
                                //we need to check if the argument have a const qualifier.
                                if (argTypeName[0] == "const" || argTypeName[0] == "unsigned") {
                                    fullTypeName = argTypeName[0]+" "+argTypeName[1];
                                    argName = argTypeName[2];
                                } else {
                                    fullTypeName = argTypeName[0];
                                    argName = argTypeName[1];
                                }
                                //Add argument name and argument type.
                                if (fullTypeName != "") {
                                    constructor.addArgName(argName);
                                    constructor.addArgType(fullTypeName);
                                }
                           }
                           pos = fileContent.find(cl.getName());
                           //Remove constructor from the string we don't need it anymore we've got every informations needed.
                           if (pos != std::string::npos) {
                                pos2 = fileContent.find(";");
                                fileContent.erase(pos, pos2-pos+1);
                           }
                           //add the constructor to the class.
                           cl.addConstructor(constructor);
                       }
                    }
                }
            }
        }
        unsigned int Class::findLastBracket(std::string& fileContent, unsigned int nbBlocks) {
            unsigned int pos, pos2;
            do {
                pos = fileContent.find("{");
                pos2 = fileContent.find("}");
                if (pos != std::string::npos && pos2 != std::string::npos) {
                    if (pos < pos2) {
                        nbBlocks++;
                        fileContent.erase(pos, 1);
                    } else {
                        fileContent.erase(pos2, 1);
                        nbBlocks--;
                    }
                }
            } while (nbBlocks > 0 || pos == std::string::npos || pos2 == std::string::npos);
            return pos2;
        }
        void Class::checkMembersFunctions(std::string& fileContent, Class& cl) {
            //The function definition can be at the same file than the member function declaration, in this case, we need to split the string.

            while (fileContent.find("{") != std::string::npos) {
                //Count the number of sub blocks (introduced by if, while, etc...) and remove they definition.
                unsigned int pos = fileContent.find("{");
                unsigned int pos2 = findLastBracket(fileContent, 0);
                //std::cout<<"erase"<<std::endl;
                fileContent.erase(pos, pos2 - pos-1);
                //std::cout<<"erased"<<std::endl;
                fileContent.insert(pos-1, ";");
                //std::cout<<"inserted"<<std::endl;
            }
            while (fileContent.at(0) == ' ' || fileContent.at(0) == '\n') {
                fileContent = fileContent.erase(0, 1);

            }
            //we also need to split when the member function definition is not in the header.
            std::vector<std::string> parts2 = split(fileContent, ";");
            for (unsigned int j = 0; j < parts2.size(); j++) {
                std::string str;
                if(parts2[j].find("(") != std::string::npos) {
                    int pos = parts2[j].find("(");
                    str = parts2[j].substr(0, pos+1);
                    //std::cout<<"str sub"<<std::endl;
                    //Erase spaces before the \n.
                    while (str.size() > 1 && str.at(str.size()-2) == ' ') {
                        str = str.erase(str.size()-2, 1);

                    }
                    //std::cout<<"space removed"<<std::endl;
                }
                if (str == "") {
                    str = parts2[j];
                }

                //check the position of the function member arguments.
                int index = parts2[j].find("(");
                if (index != std::string::npos) {



                    //remove spaces and \n at the beginning and at the end.
                    while (parts2[j].size() > 0 && parts2[j].at(0) == ' ' || parts2[j].at(0) == '\n') {
                        parts2[j].erase(0, 1);
                    }
                    while (parts2[j].size() > 0 && parts2[j].at(parts2[j].size()-1) == ' ' || parts2[j].at(parts2[j].size()-1) == '\n') {
                        parts2[j].erase(parts2[j].size()-1, 1);
                    }
                    //split to get argument list.
                    //std::cout<<"name : "<<name<<std::endl;

                    std::vector<std::string> parts3 = split(parts2[j], "(");
                    parts3 = split(parts3[0], " ");
                    if (parts3.size() > 1) {
                        MemberFunction mf(parts3[0], parts3[1]);
                        //beginning and end of argument list.
                        int pos = parts2[j].find("(");
                        int pos2 = parts2[j].find_last_of(")");
                        if (pos != std::string::npos && pos2 != std::string::npos) {

                           std::string types = parts2[j].substr(pos+1, pos2-pos-1);
                           std::string tmp = types;
                           while (types.find("=") != std::string::npos) {
                                int pos = types.find("=");
                                int pos2;
                                if (types.find("(") != std::string::npos) {
                                    pos2 = types.find(")");
                                    types.erase(pos, pos2 - pos + 1);
                                    tmp.erase(pos, pos2 - pos + 1);
                                } else {
                                    pos2 = tmp.find(",");
                                    tmp.erase(pos, pos2-pos+1);
                                    types.erase(pos, pos2 - pos);
                                }
                           }
                           std::vector<std::string> arguments = split(types, ",");
                           for (unsigned int i = 0; i < arguments.size(); i++) {
                                while(arguments[i].size() > 0 && arguments[i].at(0) == ' ' || arguments[i].at(0) == '\n') {
                                    arguments[i] = arguments[i].erase(0, 1);
                                }

                                std::vector<std::string> argTypeName = split(arguments[i], " ");
                                std::string fullTypeName="";
                                std::string argName = "";

                                //check if there is a const qualifier.
                                if (argTypeName[0] == "const" || argTypeName[0] == "unsigned") {
                                    fullTypeName = argTypeName[0]+" "+argTypeName[1];
                                    argName = argTypeName[2];
                                } else {
                                    fullTypeName = argTypeName[0];
                                    argName = argTypeName[1];
                                }
                                if (fullTypeName != "") {
                                    mf.addArgType(fullTypeName);
                                    mf.addArgName(argName);
                                }
                           }
                           pos = fileContent.find(parts2[j]);
                           if (pos != std::string::npos) {
                               //Remove the member function from the string we don't need it anymore after extracting the informations.
                               if (fileContent.find(";") != std::string::npos) {
                                   pos2 = fileContent.find(";");
                               }
                               fileContent = fileContent.erase(pos, pos2-pos+1);

                           }
                           //Add the member function to the class.
                           cl.addMemberFunction(mf);
                        }
                    }
                }
            }
            //std::cout<<"after member file content : "<<fileContent<<std::endl;
        }
        void Class::checkInnerClass(std::string innerClass, std::string type, std::string& fileContent, int lvl, Class& cl) {
            //Remove template parameters and template template parameters.
            if (fileContent.find("template ") != std::string::npos) {
                int pos = fileContent.find("template ");
                int pos2 = fileContent.find(">");
                fileContent.erase(pos, pos2 - pos+1);
            }
            if (fileContent.find("<") != std::string::npos) {
                int pos = fileContent.find("<");
                int pos2 = fileContent.find(">");
                fileContent.erase(pos, pos2 - pos+1);
            }
            //if we find a subclass we extract informations, care full with the friend class declaration.
            if ((fileContent.find("class ") != std::string::npos || fileContent.find("enum ") != std::string::npos || fileContent.find("struct ") != std::string::npos)
                && fileContent.find("friend") == std::string::npos) {
                std::string type;
                //Check the sublclass type. (enum, calss or struct ?)
                if (fileContent.find("class ") != std::string::npos) {
                    //Remove the class keyword.
                    int pos = fileContent.find("class ");
                    fileContent = fileContent.substr(pos+5, fileContent.size()-pos-5);
                    type = "class";
                } else if (fileContent.find("enum ") != std::string::npos) {
                    //Remove the enum keyword.
                    int pos = fileContent.find("enum ");
                    fileContent = fileContent.substr(pos+4, fileContent.size()-pos-4);
                    type = "enum";
                } else if (fileContent.find("struct ") != std::string::npos) {
                    //Remove the struct keyword.
                    int pos = fileContent.find("struct ");
                    fileContent = fileContent.substr(pos+6, fileContent.size()-pos-6);
                    type = "struct";
                }
                //Remove spaces at the beginning.
                while(fileContent.size() > 0 && fileContent.at(0) == ' ') {
                   fileContent.erase(0, 1);
                }
                //Get the inner class name and check if the inner class have inner classes.
                std::vector<std::string> parts = split(fileContent, " ");
                if (lvl == 0) {
                    checkInnerClass(cl.getName()+"::"+parts[0], type, fileContent, lvl+1, cl);
                } else {
                    checkInnerClass(innerClass + "::"+parts[0], type, fileContent, lvl+1, cl);;
                }
                if (fileContent.find("};") != std::string::npos) {
                    lvl--;
                }
            }
            //If the recursion lvl is > than 0 this is an inner class and we need to add it.
            if (lvl > 0) {
                //Check inner class name in inner class full name.
                std::vector<std::string> parts = split(innerClass, "::");
                //start and end of inner class definition.
                int pos = fileContent.find(parts[parts.size()-1]);
                int pos2 = fileContent.find("};");
                //Add inner class and remove inner class declaration because we don't need it anymore.
                Class innerCl(innerClass, cl.getFilePath());
                fileContent = fileContent.erase(pos, pos2 - pos+2);
                cl.addInnerClass(innerCl);
            }
        }
        void Class::removeSpacesChars(std::string& str) {
            //remove spaces and \n at the beginning and at the end.
            while (str.size() > 0 && str.at(0) == ' ' || str.at(0) == '\n') {
                str = str.erase(0, 1);
            }
            while (str.size() > 0 && str.at(str.size()-1) == ' ' || str.at(str.size()-1) == '\n') {
                str = str.erase(str.size()-1, 1);
            }
        }
        void Class::checkMembersVariables(std::string& fileContent, Class& cl) {
            //std::cout<<"file content : "<<fileContent<<std::endl;
            std::vector<std::string> parts = split(fileContent, ";");

            for (unsigned int i = 0; i < parts[i].size(); i++) {
                removeSpacesChars(parts[i]);
                std::string fullTypeName="";
                if (parts[i].find(",") != std::string::npos) {
                    std::vector<std::string> argsComa = split(parts[i], ",");
                    for (unsigned int j = 0; j < argsComa.size(); j++) {
                        removeSpacesChars(argsComa[j]);
                        std::string argName = "";
                        if (argsComa[j].find("=") != std::string::npos) {
                            std::vector<std::string> argsEqual = split(argsComa[j], "=");
                            removeSpacesChars(argsEqual[0]);
                            std::vector<std::string> argTypeName = split(argsEqual[0], " ");
                            for (unsigned int k = 0; k < argTypeName.size(); k++) {
                                removeSpacesChars(argTypeName[k]);
                            }
                            if (j == 0) {
                                if (argTypeName[0] == "const" || argTypeName[0] == "unsigned") {
                                    fullTypeName = argTypeName[0]+" "+argTypeName[1];
                                    argName = argTypeName[2];
                                } else {
                                    fullTypeName = argTypeName[0];
                                    argName = argTypeName[1];
                                }
                            } else {
                                argName = argTypeName[0];
                            }
                        } else {
                            std::vector<std::string> argTypeName = split(argsComa[j], " ");
                            for (unsigned int k = 0; k < argTypeName.size(); k++) {
                                removeSpacesChars(argTypeName[k]);
                            }
                            if (j == 0) {
                                if (argTypeName[0] == "const" || argTypeName[0] == "unsigned") {
                                    fullTypeName = argTypeName[0]+" "+argTypeName[1];
                                    argName = argTypeName[2];
                                } else {
                                    fullTypeName = argTypeName[0];
                                    argName = argTypeName[1];
                                }
                            } else {
                                argName = argTypeName[0];
                            }
                        }
                        MemberVariable mb;
                        mb.setVarType(fullTypeName);
                        mb.setVarName(argName);
                        cl.addMemberVariable(mb);
                    }
                } else {
                    std::string argName = "";
                    if (parts[i].find("=") != std::string::npos) {
                        std::vector<std::string> argsEqual = split(parts[i], "=");
                        removeSpacesChars(argsEqual[0]);
                        std::vector<std::string> argTypeName = split(argsEqual[0], " ");
                        for (unsigned int k = 0; k < argTypeName.size(); k++) {
                            removeSpacesChars(argTypeName[k]);
                        }
                        //check if there is a const qualifier.
                        if (argTypeName[0] == "const" || argTypeName[0] == "unsigned") {
                            fullTypeName = argTypeName[0]+" "+argTypeName[1];
                            argName = argTypeName[2];
                        } else {
                            fullTypeName = argTypeName[0];
                            argName = argTypeName[1];
                        }
                    } else {
                        std::vector<std::string> argTypeName = split(parts[i], " ");
                        for (unsigned int k = 0; k < argTypeName.size(); k++) {
                            removeSpacesChars(argTypeName[k]);
                        }
                        //check if there is a const qualifier.
                        if (argTypeName[0] == "const" || argTypeName[0] == "unsigned") {
                            fullTypeName = argTypeName[0]+" "+argTypeName[1];
                            argName = argTypeName[2];
                        } else {
                            fullTypeName = argTypeName[0];
                            argName = argTypeName[1];
                        }
                    }
                    MemberVariable mb;
                    mb.setVarType(fullTypeName);
                    mb.setVarName(argName);
                    cl.addMemberVariable(mb);
                }
            }
        }*/
        void Class::addSuperClass(Class cl) {
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
        void Class::setNamespace(std::string namespc) {
            this->namespc = namespc;
        }
        void Class::addInnerClass(Class cl) {
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
        void Class::addConstructor(Constructor c) {
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
        void Class::addMemberFunction(MemberFunction mf) {
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
        void Class::addMemberVariable(MemberVariable mb) {
            memberVariables.push_back(mb);
        }
        std::string Class::getName() {
            return name;
        }
        std::string Class::getFilePath() {
            return filePath;
        }
        std::string Class::getImplFilePath() {
            return implFilePath;
        }
        std::vector<Constructor> Class::getConstructors() {
            return constructors;
        }
        std::vector<MemberFunction> Class::getMembersFunctions() {
            return memberFunctions;
        }
        std::vector<MemberVariable> Class::getMembersVariables() {
            return memberVariables;
        }
        std::string Class::getNamespace() {
            return namespc;
        }
        std::vector<Class> Class::getSuperClasses() {
            return superClasses;
        }
    }
}
