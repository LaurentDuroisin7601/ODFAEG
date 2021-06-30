#ifndef ODFAEG_NODE_HPP
#define ODFAEG_NODE_HPP
#include "../lightComponent.h"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            class Node {
                public :
                Node (std::string name, LightComponent* component, math::Vec2f relNodePos, math::Vec2f relNodeSize, Node* parent = nullptr);
                void addNode (Node* component);
                std::vector<Node*> getNodes();
                Node* getRootNode();
                bool isNodeVisible();
                void showAllNodes();
                void hideAllNodes();
                void deleteAllNodes();
                Node* findNode (LightComponent* component);
                void addOtherComponent(LightComponent* component, math::Vec2f relSize);
                void affiche();
                private :
                bool nodeVisible;
                void hideNode(Node* node);
                void showNode (Node* node);
                void displaceNodes(Node* selectedNode, math::Vec2f& nodePos);
                void findNodePos(Node* selectedNode, math::Vec2f& nodePos);
                math::Vec2f relNodePos, relNodeSize;
                std::vector<std::unique_ptr<Node>> nodes;
                Node* parent;
                LightComponent* component;
                std::vector<LightComponent*> components;
                std::string name;
            };
        }
    }
}
#endif
