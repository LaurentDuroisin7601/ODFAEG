#ifndef ODFAEG_ERREUR_HPP
#define ODFAEG_ERREUR_HPP
#include <exception>
#include <string>
#include "export.hpp"
/**
*\namespace odfaeg
* the namespace of the Opensource Development Framework Adapted for Every Games.
*/
namespace odfaeg {
    namespace core {
        /**
        * \file erreur.h
        * \class Erreur
        * \brief base class of all runtimes errors.
        * \author Duroisin.L
        * \version 1.0
        * \date 1/02/2014
        */
        class ODFAEG_CORE_API Erreur : public std::exception {
             public :
                /**
                * \fn Erreur (int numero, const std::string, int niveau)
                * \param int numero : the numero of the error.
                * \param const std::string& phrase : the message of the error.
                * \param int niveau : return the level of the error.
                */
                Erreur(int numero=0, const std::string &phrase="", int niveau=0) throw();
                /**
                * \fn virtual const char* what() const throw
                * \brief return the message of the error.
                * \return const char* : return the message of the error;
                */
                virtual const char* what() const throw();
                /**
                * \fn int getNiveau () const throw();
                * \return int  : return the level of the error.
                */
                int getNiveau() const throw();
                /**
                * \fn virtual ~Erreur() throw();
                * \brief destructor.
                */
                virtual ~Erreur() throw();
             private:
                int m_numero;  /**>number of the error*/             // Numéro de l'erreur.
                std::string m_phrase; /**>message of the error*/      // Description de l'erreur.
                int m_niveau; /**>level of the error.*/
         };
    }
}
 #endif
