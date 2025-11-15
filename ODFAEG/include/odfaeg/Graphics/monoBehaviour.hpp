#ifndef MONO_BEHAVIOUR_HPP
#define MONO_BEHAVIOUR_HPP
namespace odfaeg {
    namespace graphic {
        class MonoBehaviour {
            public :
            virtual void init() = 0;
            virtual void update() = 0;
        };
    }
}
#endif // MONO_BEHAVIOUR_HPP
