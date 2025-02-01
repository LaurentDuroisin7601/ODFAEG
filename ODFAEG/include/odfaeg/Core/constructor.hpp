#ifndef ODFAEG_CONSTRUCTOR_HPP
#define ODFAEG_CONSTRUCTOR_HPP
#include <string>
#include <vector>
namespace odfaeg {
    namespace core {
        /**
        * \file constructor.hpp
        * \class Constructor
        * \brief contains every informations about a c++ constructor.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class Constructor {
        public :
            /** \fn Constructor (std::string name)
            *   \brief construct constructor with the c++ constructor name.
            *   \param std::string : the c++ constructor name.
            */
            Constructor (std::string name);
            /** \fn addArgType(std::string argType)
            *   \brief add argument type info.
            *   \param std::string the argument type.
            */
            void addArgType(std::string argType);
            /** \fn addArgName(std::string argName)
            *   \brief add argument name info.
            *   \param std::string the argument name.
            */
            void addArgName(std::string argName);
            /** \fn std::vector<std::string> getArgsTypes()
            *   \brief get a list of each arg types.
            *   \return list of each arg type.
            */
            std::vector<std::string> getArgsTypes();
            /** \fn std::vector<std::string> getArgsNames()
            *   \brief get a list of each arg names.
            *   \return list of each arg names.
            */
            std::vector<std::string> getArgsNames();
            /** \fn std::string getName()
            *   \brief get the constructor's name.
            *   \return the constructor's name.
            */
            std::string getName();
        private:
            std::string name; /**> constructor's name.*/
            std::vector<std::string> argsTypes; /**> constructor's arguments types.*/
            std::vector<std::string> argsNames; /**> constructor's arguments names.*/
        };
    }
}
#endif // ODFAEG_CONSTRUCTOR_HPP
