module;
export module odfaeg.entity.impl;
import odfaeg.entity.entity;
namespace odfaeg {
	namespace entity {
		export class EnttEntity {
		public:
			/*template <typename D, typename... Args>
		    static D* make_entity(Args... args);*/
			static void initEntity(Entity& entity);
		};
	}
}
	