#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <clang-c/Index.h>
#include <vector>
#include <algorithm>
namespace odfaeg {
   namespace core {
        class Constructor {
        public:
            Constructor(std::string name);
            void addArgType(std::string argType);
            void addArgName(std::string argName);
            std::vector<std::string> getArgsNames();
            std::vector<std::string> getArgsTypes();
            std::string getName();
            CXTranslationUnit tu;
        private:
            std::string name; /**> constructor's name.*/
            std::vector<std::string> argsTypes; /**> constructor's arguments types.*/
            std::vector<std::string> argsNames; /**> constructor's arguments names.*/
        };
        class MemberFunction {
        public:
            MemberFunction(std::string returnType, std::string name);
            void addArgType(std::string argType);
            void addArgName(std::string argName);
            std::string getReturnType();
            std::string getName();
            std::vector<std::string> getArgsTypes();
            std::vector<std::string> getArgsNames();
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
            void setVarType(std::string varType);
            void setVarName(std::string varName);
            std::string getVarType();
            std::string getVarName();
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
            Class(std::string name, std::string filePath);
            void setName(std::string name);
            void setFilePath(std::string filePath);
            void setImplFilePath(std::string filePath);
            
            static std::vector<std::pair<std::string, std::string>> getClassesFromMemory(std::vector<std::string> includePaths, std::string virtualFile, std::string virtualPath, std::string content, std::string nspc);

            static std::vector<std::pair<std::string, std::string>> getClasses(std::vector<std::string> includePaths, std::string path, std::string nspc);
            static Class getClassFromMemory(std::vector<std::string> includePaths, std::string virtualFile, std::string name, std::string content, std::string nspc);
            static Class getClass(std::vector<std::string> includePaths, std::string name, std::string path, std::string nspc);
            
            std::string getName();
            std::string getFilePath();
            std::string getImplFilePath();
            std::vector<Constructor> getConstructors();
            std::vector<MemberFunction> getMembersFunctions();
            std::vector<MemberVariable> getMembersVariables();
            std::string getNamespace();
            std::vector<Class> getSuperClasses();
            static std::string normalize(const std::string& path);
            static std::string stripQuotes(std::string s);
            private :
            static std::string getQualifiedNamespace(CXCursor cursor);

            
            static CXChildVisitResult classesVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
            static CXChildVisitResult classVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
            static CXChildVisitResult constructorVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
            static CXChildVisitResult memberFonctionVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
            void addSuperClass(Class cl);
            void setNamespace(std::string namespc);
            void addInnerClass(Class cl);
            void addConstructor(Constructor c);
            void addMemberFunction(MemberFunction mf);
            void addMemberVariable(MemberVariable mb);
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
#include "class.inl"

