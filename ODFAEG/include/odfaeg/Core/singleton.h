#ifndef ODFAEG_SINGLETON_HPP
#define ODFAEG_SINGLETON_HPP
#include <mutex>
/**
 *\namespace odfaeg
 * the namespace of the Opensource Development Framework Adapted for Every Games.
 */
namespace odfaeg {
template <class T> class Singleton
{

public:
	static T& Instance();
protected:
	static T m_i;
private:
        T& operator= (const T&) = delete;
};
template <class T>  T& Singleton<T>::Instance()
{
    return m_i;
}
template <class T> T Singleton<T>::m_i=T();
static std::recursive_mutex rec_mutex;
}
#endif
