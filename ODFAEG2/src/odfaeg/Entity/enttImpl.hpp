#ifndef ODFAEG_ENTTIMPL_HPP
#define ODFAEG_ENTTIMPL_HPP
#include "entity.hpp"
namespace odfaeg {
	namespace entity {
		class EnttEntity {
		public:
			/*template <typename D, typename... Args>
		    static D* make_entity(Args... args);*/
			static void initEntity(Entity& entity);
		};
	}
}
#endif