/***************************************************************************
 * @file GlDebug.hpp
 * @author dragonjoker59
 * @date 07/03/2014
 *
 * @brief Permet l'affichage des informations de debug provenant d'OpenGL
 *
 * @details Utilise les extensions GL_ARB_debug_output et GL_AMDX_debug_output si elles sont disponibles.
 *
 ***************************************************************************/

#ifndef ___GL_Errors_H___
#define ___GL_Errors_H___

#include <string>

#if !defined( CALLBACK )
#	if defined( _WIN32 )
#		define CALLBACK __stdcall
#	else
#		define CALLBACK
#	endif
#endif
namespace odfaeg {
    namespace graphic {
        namespace OpenGL
        {
            /** Charge les fonctions de debug disponibles dans certaines extensions OpenGL.
            @remarks
                Les extensions concernées sont : GL_ARB_debug_output et GL_AMDX_debug_output
                Il faut que le contexte OpenGL ne soit pas actif
            */
            void LoadDebugFunctions();

            /** Affecte à la fonction de debug disponible sa callback d'affichage
            @remarks
                Il faut que le contexte OpenGL soit actif
            */
            void InitialiseDebugFunctions();

            /** Callback d'affichage d'une information de debug, avec l'extension ARB
            @param source
                La source de l'info
            @param type
                Le type de l'info
            @param id
                L'id de l'info
            @param severity
                L'importance de l'info
            @param length
                La longueur du message qui suit
            @param message
                Un message informatif
            @param userParam
                Paramètre donné par l'utilisateur
            */
            void CALLBACK CallbackDebugLog( uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int length, const char * message, void * userParam  );

            /** Callback d'affichage d'une information de debug, avec l'extension AMDX
            @param id
                L'id de l'info
            @param category
                La catégorie de l'info
            @param severity
                L'importance de l'info
            @param length
                La longueur du message qui suit
            @param message
                Un message informatif
            @param userParam
                Paramètre donné par l'utilisateur
            */
            void CALLBACK CallbackDebugLogAMD( uint32_t id, uint32_t category, uint32_t severity, int length, const char* message, void* userParam );
        }
    }
}

#endif //___GL_Errors_H___
