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