/***************************************************************************
 * @file GlDebug.cpp
 * @author dragonjoker59
 * @date 07/03/2014
 *
 * @brief Permet l'affichage des informations de debug provenant d'OpenGL
 *
 * @details Utilise les extensions GL_ARB_debug_output et GL_AMDX_debug_output si elles sont disponibles.
 *
 ***************************************************************************/

 #include "GlDebug.hpp"

#include <cstdint>
#include <iostream>

#if defined( _WIN32 )
#   include <Windows.h>
#else
#   include <GL/glx.h>
#endif

#include <SFML/OpenGL.hpp>

//*************************************************************************************************
namespace odfaeg {
    namespace graphic {
        namespace gl_api
        {
                /** Récupère une fonction dans la dll OpenGL actuellement chargée
                @param p_strName
                    Le nom de la fonction
                @param p_func
                    Reçoit la fonction
                @return
                    true si la fonction a été trouvée, false sinon
                */
                template< typename Func >
                bool GetFunction( std::string const & p_strName, Func & p_func )
                {
            #if defined( _WIN32 )
                    p_func = reinterpret_cast< Func >( wglGetProcAddress( p_strName.c_str() ) );
            #elif defined( __linux__ )
                    p_func = reinterpret_cast< Func >( glXGetProcAddress( (GLubyte const *)p_strName.c_str() ) );
            #endif
                    return p_func != NULL;
                }

            #define MAKE_GL_EXTENSION( x )	static const std::string x = "GL_"#x;

                MAKE_GL_EXTENSION( ARB_debug_output )
                MAKE_GL_EXTENSION( AMDX_debug_output )

                //! Dit que l'on veut les informations de debug en synchrone
                static const int iGL_DEBUG_OUTPUT_SYNCHRONOUS = 0x8242;
                static const int iGL_DEBUG_OUTPUT = 0x92E0;
                /** Les types d'informations
                */
                typedef enum eGL_DEBUG_TYPE : uint32_t
                {	eGL_DEBUG_TYPE_ERROR				= 0x824C
                ,	eGL_DEBUG_TYPE_DEPRECATED_BEHAVIOR	= 0x824D
                ,	eGL_DEBUG_TYPE_UNDEFINED_BEHAVIOR	= 0x824E
                ,	eGL_DEBUG_TYPE_PORTABILITY			= 0x824F
                ,	eGL_DEBUG_TYPE_PERFORMANCE			= 0x8250
                ,	eGL_DEBUG_TYPE_OTHER				= 0x8251
                }	eGL_DEBUG_TYPE;

                /** Les sources des informations
                */
                typedef enum eGL_DEBUG_SOURCE : uint32_t
                {	eGL_DEBUG_SOURCE_API                = 0x8246
                ,	eGL_DEBUG_SOURCE_WINDOW_SYSTEM      = 0x8247
                ,	eGL_DEBUG_SOURCE_SHADER_COMPILER    = 0x8248
                ,	eGL_DEBUG_SOURCE_THIRD_PARTY        = 0x8249
                ,	eGL_DEBUG_SOURCE_APPLICATION        = 0x824A
                ,	eGL_DEBUG_SOURCE_OTHER				= 0x824B
                }	eGL_DEBUG_SOURCE;

                /** Les catégories d'informations
                */
                typedef enum eGL_DEBUG_CATEGORY : uint32_t
                {	eGL_DEBUG_CATEGORY_API_ERROR			= 0x9149
                ,	eGL_DEBUG_CATEGORY_WINDOW_SYSTEM		= 0x914A
                ,	eGL_DEBUG_CATEGORY_DEPRECATION			= 0x914B
                ,	eGL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR	= 0x914C
                ,	eGL_DEBUG_CATEGORY_PERFORMANCE			= 0x914D
                ,	eGL_DEBUG_CATEGORY_SHADER_COMPILER		= 0x914E
                ,	eGL_DEBUG_CATEGORY_APPLICATION			= 0x914F
                ,	eGL_DEBUG_CATEGORY_OTHER				= 0x9150
                }	eGL_DEBUG_CATEGORY;

                /** Les importances des informations
                */
                typedef enum eGL_DEBUG_SEVERITY : uint32_t
                {	eGL_DEBUG_SEVERITY_HIGH		= 0x9146
                ,	eGL_DEBUG_SEVERITY_MEDIUM	= 0x9147
                ,	eGL_DEBUG_SEVERITY_LOW		= 0x9148
                }	eGL_DEBUG_SEVERITY;
            }
            //bool OpenGL::functionsInitialized = false;
        //*************************************************************************************************

        typedef void (CALLBACK * PFNGLDEBUGPROC)( uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int length, const char * message, void * userParam );
        typedef void (CALLBACK * PFNGLDEBUGAMDPROC)( uint32_t id, uint32_t category, uint32_t severity, int length, const char* message, void* userParam );
        typedef void (CALLBACK * PFNGLDEBUGMESSAGECALLBACK)( PFNGLDEBUGPROC callback, void * userParam );
        typedef void (CALLBACK * PFNGLDEBUGMESSAGECALLBACKAMD)( PFNGLDEBUGAMDPROC callback, void * userParam );


        PFNGLDEBUGMESSAGECALLBACK glDebugMessageCallback = NULL;
        PFNGLDEBUGMESSAGECALLBACKAMD glDebugMessageCallbackAMD = NULL;

        void OpenGL::LoadDebugFunctions()
        {
            //if (!OpenGL::functionsInitialized) {
                //std::cout<<"load debug functions!"<<std::endl;
                std::string l_strExtensions = (char const *)glGetString( GL_EXTENSIONS );

                if( l_strExtensions.find( gl_api::ARB_debug_output ) != std::string::npos )
                {
                    std::cout<<"load debug functions"<<std::endl;
                    if( !gl_api::GetFunction( "glDebugMessageCallback", glDebugMessageCallback ) )
                    {
                        if( !gl_api::GetFunction( "glDebugMessageCallbackARB", glDebugMessageCallback ) )
                        {
                            std::cout << "Unable to retrieve function glDebugMessageCallback" << std::endl;
                        }
                    }
                }
                else if( l_strExtensions.find( gl_api::AMDX_debug_output ) != std::string::npos )
                {
                    if( !gl_api::GetFunction( "glDebugMessageCallbackAMD", glDebugMessageCallbackAMD ) )
                    {
                        std::cout << "Unable to retrieve function glDebugMessageCallbackAMD" << std::endl;
                    }
                }
            //}
        }

        void OpenGL::InitialiseDebugFunctions()
        {
            //if (!OpenGL::functionsInitialized) {
                //std::cout<<"initialize debug functions!"<<std::endl;
                if( glDebugMessageCallback )
                {
                    std::cout<<"init debug functions"<<std::endl;
                    glDebugMessageCallback( PFNGLDEBUGPROC( &CallbackDebugLog ), NULL );
                    glEnable( gl_api::iGL_DEBUG_OUTPUT_SYNCHRONOUS );
                }
                else if( glDebugMessageCallbackAMD )
                {
                    //std::cout<<"init debug functions AMD"<<std::endl;
                     glDebugMessageCallbackAMD( PFNGLDEBUGAMDPROC( &CallbackDebugLogAMD ), NULL );
                     glEnable(gl_api::iGL_DEBUG_OUTPUT);
                     glEnable(gl_api::iGL_DEBUG_OUTPUT_SYNCHRONOUS);

                }
            //}
            //OpenGL::functionsInitialized = true;
        }

        void OpenGL::CallbackDebugLog( uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int length, const char * message, void * userParam  )
        {
            std::cout << "OpenGL Debug - ";

            switch( source )
            {
            case gl_api::eGL_DEBUG_SOURCE_API:				std::cout << "Source:OpenGL\t";			    break;
            case gl_api::eGL_DEBUG_SOURCE_WINDOW_SYSTEM:	std::cout << "Source:Windows\t";			break;
            case gl_api::eGL_DEBUG_SOURCE_SHADER_COMPILER:	std::cout << "Source:Shader compiler\t";	break;
            case gl_api::eGL_DEBUG_SOURCE_THIRD_PARTY:		std::cout << "Source:Third party\t";		break;
            case gl_api::eGL_DEBUG_SOURCE_APPLICATION:		std::cout << "Source:Application\t";		break;
            case gl_api::eGL_DEBUG_SOURCE_OTHER:			std::cout << "Source:Other\t";				break;
            }

            switch( type )
            {
            case gl_api::eGL_DEBUG_TYPE_ERROR:					std::cout << "Type:Error\t";				break;
            case gl_api::eGL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	std::cout << "Type:Deprecated behavior\t";	break;
            case gl_api::eGL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:		std::cout << "Type:Undefined behavior\t";	break;
            case gl_api::eGL_DEBUG_TYPE_PORTABILITY:			std::cout << "Type:Portability\t";			break;
            case gl_api::eGL_DEBUG_TYPE_PERFORMANCE:			std::cout << "Type:Performance\t";			break;
            case gl_api::eGL_DEBUG_TYPE_OTHER:					std::cout << "Type:Other\t";				break;
            }

            std::cout << "ID:" << id << "\t";

            switch( severity )
            {
            case gl_api::eGL_DEBUG_SEVERITY_HIGH:	std::cout << "Severity:High\t";	    break;
            case gl_api::eGL_DEBUG_SEVERITY_MEDIUM:	std::cout << "Severity:Medium\t";	break;
            case gl_api::eGL_DEBUG_SEVERITY_LOW:	std::cout << "Severity:Low\t";  	break;
            }

            std::cout << "Message:" << message << std::endl;
        }

        void OpenGL::CallbackDebugLogAMD( uint32_t id, uint32_t category, uint32_t severity, int length, const char* message, void* userParam )
        {
            std::cout << "OpenGL Debug - ";

            switch( category )
            {
            case gl_api::eGL_DEBUG_CATEGORY_API_ERROR:			std::cout << "Category:OpenGL\t";				break;
            case gl_api::eGL_DEBUG_CATEGORY_WINDOW_SYSTEM:		std::cout << "Category:Windows\t";				break;
            case gl_api::eGL_DEBUG_CATEGORY_DEPRECATION:		std::cout << "Category:Deprecated behavior\t";	break;
            case gl_api::eGL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR:	std::cout << "Category:Undefined behavior\t";	break;
            case gl_api::eGL_DEBUG_CATEGORY_PERFORMANCE:		std::cout << "Category:Performance\t";			break;
            case gl_api::eGL_DEBUG_CATEGORY_SHADER_COMPILER:	std::cout << "Category:Shader compiler\t";		break;
            case gl_api::eGL_DEBUG_CATEGORY_APPLICATION:		std::cout << "Category:Application\t";			break;
            case gl_api::eGL_DEBUG_CATEGORY_OTHER:				std::cout << "Category:Other\t";				break;
            }

            std::cout << "ID:" << id << "\t";

            switch( severity )
            {
            case gl_api::eGL_DEBUG_SEVERITY_HIGH:	std::cout << "Severity:High\t";	break;
            case gl_api::eGL_DEBUG_SEVERITY_MEDIUM:	std::cout << "Severity:Medium\t";	break;
            case gl_api::eGL_DEBUG_SEVERITY_LOW:	std::cout << "Severity:Low\t"; 	break;
            }

            std::cout << "Message:" << message << std::endl;
        }
    }
}

//*************************************************************************************************
