#include "factory.h"
namespace odfaeg {
    namespace core {
        template <class O, class K>
        std::map<K, O*> Factory<O, K>::m_map = std::map<K, O*> ();
        template <class O, class K>
        void Factory<O, K>::Register (K key, O* object) {
            if(m_map.find(key)==m_map.end())
            {
                   m_map[key]=object;
            }
        }

        template <class O, class K>
        O* Factory<O, K>::Create (const K& key) {
            O* tmp=0;
            typename std::map<K, O*>::iterator it=m_map.find(key);

            if(it!=m_map.end())
            {
                tmp=((*it).second)->Clone();
            }

            return tmp;
        }
    }
}

