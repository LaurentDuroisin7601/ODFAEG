#ifndef ODFAEG_CLASS_HPP
#define ODFAEG_CLASS_HPP
#include <string>
#include "utilities.h"
#include "erreur.h"
#include <fstream>
#include <iostream>
#include <string>
#include "constructor.hpp"
#include "memberFunction.hpp"
#include "memberVariable.hpp"

namespace odfaeg {
    namespace core {
        /**
        * \file class.hpp
        * \class Class
        * \brief Search informations about a c++ class. (constructors, member's functions, sub classes)
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ODFAEG_CORE_API Class {
        public :
            /**\fn Class(std::string name, std::string filePath)
            *  \brief construct a class with the class name and the file path of it's header.
            *  \param std::string name : the name of the class.
            *  \param std::string filePath : the file path of the class's header.
            */
            Class(std::string name, std::string filePath);
            /**\fn std::vector<std::string> getClasses(std::string filePath)
            *  \brief get every classes names located int the specified folder path.
            *  \param the file path of the folder.
            *  \return the vector of the classes names.
            */
            static std::vector<std::pair<std::string, std::string>> getClasses(std::vector<std::string> includePaths, std::string filePath, std::string namespc="");
            static std::vector<std::pair<std::string, std::string>> getClassesFromMemory(std::vector<std::string> includePaths, std::string virtualFile, std::string virtualPath, std::string content, std::string namespc="");
            /**\fn Class getClass(std::string name, std::string nspc="", std::string path="");
            *  \brief get a class object which contains informations about a c++ class.
            *  \param std::string name : the name of the c++ class.
            *  \param std::string nspc : the namespace of the class. (empty by default)
            *  \param std::string path : the folder's path from where to seach the c++ class. (empty by default)
            *  \return Class : class object with informations about the c++ class.
            */
            static Class getClass(std::vector<std::string> includePaths,  std::string name, std::string path="", std::string nspc="");
            static Class getClassFromMemory(std::vector<std::string> includePaths, std::string virtualFile, std::string name, std::string content="", std::string namespc ="");
            void setName(std::string name);
            /**\fn std::string getName()
            *  \brief get the class's name.
            *  \return std::string : the class's name.
            */
            std::string getName();
            void setFilePath(std::string filePath);
            void setImplFilePath(std::string filePath);
            /**\fn std::string getFilePath()
            *  \brief return the file path of the class's header.
            *  \return std::string : the file path of the header's class.
            */
            std::string getFilePath();
            std::string getImplFilePath();
            /**\fn std::vector<Constructor> getConstructors()
            *  \brief get every constructors of the class.
            *  \return std::vector<Constructor> : the constructors of the class.
            */
            std::vector<Constructor> getConstructors();
            /**\fn std::vector<MemberFunction> getMembersFunctions()
            *  \brief get every member's function of the class.
            *  \return std::vector<MemberFunction> : the member's functions of the class.
            */
            std::vector<MemberFunction> getMembersFunctions();
            /**\fn std::vector<Class> getSuperClasses();
            *  \brief get every bases classes of the class.
            *  \return std::vector<Class> : the base classes of the class.
            */
            std::vector<MemberVariable> getMembersVariables();
            std::vector<Class> getSuperClasses();
            /**\fn std::string getNamespace();
            *  \brief get the namespace of the class.
            *  \return std::string : the namespace's name of the class.
            */
            std::string getNamespace();
        private :
            static std::string normalize(const std::string& path);
            static std::string getQualifiedNamespace(CXCursor cursor);
            static CXChildVisitResult memberFonctionVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
            static CXChildVisitResult constructorVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
            static CXChildVisitResult classesVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
            static CXChildVisitResult classVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
            //static unsigned int findLastBracket(std::string& fileContent, unsigned int nbBlocks);
            //static void removeSpacesChars(std::string& str);
            /** \fn void checkInnerClass(std::string innerClass, std::string type, std::string& fileContent, int lvl, Class& cl)
            *   \brief check every inner classes, structs or enums of the given c++ class.
            *   \param std::string innerClass : the name of the c++ class from which to check inner c++ classes, structs or enums.
            *   \param std::string type : the class type. (class, struct or enum)
            *   \param std::string& fileContent : the remaining file content.
            *   \param lvl : the recursion lvl.
            *   \param Class& cl : the englobing class.
            */
            //static void checkInnerClass(std::string innerClass, std::string type, std::string& fileContent, int lvl, Class& cl);
            /** \fn void checkConstructors(std::string& fileContent, Class& cl)
            *   \brief check every constructors of the given class.
            *   \param std::string& fileContent : the remaining file content.
            *   \param Class& cl : the class.
            */
            //static void checkConstructors(std::string& fileContent, Class& cl);
            /** \fn void checkMembersFunctions(std::string& fileContent, Class& cl);
            *   \brief check every functions of the given class.
            *   \param std::string& fileContent : the remaining file content.
            *   \param Class& cl : the class.
            */
            //static void checkMembersFunctions(std::string& fileContent, Class& cl);
            /** \fn void checkSuperClasses(std::string& fileContent, Class& cl);
            *   \brief check every bases classes of the given class.
            *   \param std::string& fileContent : the remaining file content.
            *   \param Class& cl : the class.
            */
            //static void checkSuperClasses(std::string& fileContent, Class& cl);
            /** \fn addInnerClass(Class innerClass);
            *   \brief add an inner class to the class.
            *   \param Class innerClass : the inner class to add.
            */
            //static void checkMembersVariables(std::string& fileContent, Class& cl);
            void addInnerClass(Class innerClass);
            /** \fn setNamespace(std::string namespc)
            *   \brief set the namespace name of the class.
            *   \param std::string namespc : the namespace name.
            */
            void setNamespace(std::string namespc);
            /** \fn addConstructor(Constructor constructor)
            *   \brief add a constructor.
            *   \param Constructor : the constructor.
            */
            void addConstructor(Constructor constructor);
            /** \fn addMemberFunction(MemberFunction mf)
            *   \brief add a member function.
            *   \param MemberFunction mf : the member's function.
            */
            void addMemberFunction(MemberFunction mf);
            /** \fn addSuperClass(Class cl)
            *   \brief add a base class.
            *   \param Class cl : the class.
            */
            void addMemberVariable(MemberVariable mv);
            void addSuperClass(Class cl);
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
    }
}
#endif // ODFAEG_CLASS_HPP
