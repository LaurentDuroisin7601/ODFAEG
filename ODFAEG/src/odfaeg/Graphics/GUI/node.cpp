#include "../../../../include/odfaeg/Graphics/GUI/node.hpp"
namespace odfaeg {
    namespace graphic {
        namespace gui {
            Node::Node(std::string name, LightComponent* component, math::Vec2f relNodePos, math::Vec2f relNodeSize, Node* parent)
            : name(name), component(component), relNodePos(relNodePos), relNodeSize(relNodeSize), parent(parent) {
                component->setRelPosition(relNodePos.x, relNodePos.y);
                component->setRelSize(relNodeSize.x, relNodeSize.y);
                if (parent != nullptr)
                    parent->addNode(this);
                nodeVisible = true;
            }
            bool Node::isNodeVisible() {
                return nodeVisible;
            }
            void Node::affiche() {
                std::cout<<component->getRelPosition().y<<std::endl;
                for (unsigned int i = 0; i < nodes.size(); i++)
                    nodes[i]->affiche();
            }
            Node* Node::findNode(LightComponent* component) {
                if (this->component == component) {
                    return this;
                }
                for (unsigned int i = 0; i < nodes.size(); i++) {
                    Node* node;
                    if((node = nodes[i]->findNode(component)) != nullptr)
                        return node;
                }
                return nullptr;
            }
            Node* Node::getRootNode() {
                if (parent != nullptr)
                    return parent->getRootNode();
                return this;
            }
            void Node::addNode(Node* node) {
                math::Vec2f nodeRelPos = component->getRelPosition();
                if (this != getRootNode())
                    nodeRelPos.y += component->getRelSize().y;
                findNodePos(this, nodeRelPos);
                node->component->setRelPosition(node->component->getRelPosition().x, nodeRelPos.y);
                getRootNode()->displaceNodes (node, nodeRelPos);
                std::unique_ptr<Node> ptr;
                ptr.reset(node);
                nodes.push_back(std::move(ptr));
                component->setAutoResized(true);
            }
            void Node::showNode(Node* node) {
                node->component->setVisible(true);
                node->component->setEventContextActivated(true);
                for (unsigned int i = 0; i < node->components.size(); i++) {
                    node->components[i]->setVisible(true);
                    node->components[i]->setEventContextActivated(true);
                }
                if (node->isNodeVisible())
                    node->showAllNodes();
            }
            void Node::findNodePos (Node* node, math::Vec2f& nodeRelPos) {
                for (unsigned int i = 0; i < nodes.size(); i++) {
                    nodeRelPos.y += nodes[i]->component->getRelSize().y;
                    nodes[i]->findNodePos(nodes[i].get(), nodeRelPos);
                }
            }
            void Node::showAllNodes() {
                for (unsigned int i = 0; i < nodes.size(); i++) {
                    showNode(nodes[i].get());
                }
                if (parent != nullptr) {
                    math::Vec2f nodeRelPos = component->getRelPosition();
                    getRootNode()->displaceNodes(this, nodeRelPos);
                }
                nodeVisible = true;
                component->setAutoResized(true);
            }
            void Node::hideNode(Node* node) {
                node->component->setVisible(false);
                node->component->setEventContextActivated(false);
                for (unsigned int i = 0; i < node->components.size(); i++) {
                    node->components[i]->setVisible(false);
                    node->components[i]->setEventContextActivated(false);
                }
                node->hideAllNodes();
            }
            void Node::displaceNodes(Node* selectedNode, math::Vec2f& nodeRelPos) {
                for (unsigned int i = 0; i < nodes.size(); i++) {
                    if (nodes[i].get() != selectedNode && nodes[i]->component->isVisible()
                        && nodes[i]->component->getRelPosition().y >= selectedNode->component->getRelPosition().y) {
                        nodeRelPos.y += nodes[i]->component->getRelSize().y;
                        nodes[i]->component->setRelPosition(nodes[i]->component->getRelPosition().x, nodeRelPos.y);
                        for (unsigned int j = 0; j < nodes[i]->components.size(); j++) {
                            nodes[i]->components[j]->setRelPosition(nodes[i]->components[j]->getRelPosition().x, nodeRelPos.y);
                        }
                    }
                    nodes[i]->displaceNodes(selectedNode,nodeRelPos);
                }
                component->setAutoResized(true);
            }
            void Node::hideAllNodes() {
                for (unsigned int i = 0; i < nodes.size(); i++) {
                    hideNode(nodes[i].get());
                }
                if (parent != nullptr) {
                    math::Vec2f nodeRelPos = component->getRelPosition();
                    getRootNode()->displaceNodes(this, nodeRelPos);
                }
                nodeVisible = false;
                component->setAutoResized(true);
            }
            void Node::addOtherComponent(LightComponent* component, math::Vec2f relSize) {
                float relXPos = this->component->getRelPosition().x + this->component->getRelSize().x;
                for (unsigned int i = 0; i < components.size(); i++) {
                    relXPos += components[i]->getRelSize().x;
                }
                component->setRelPosition(relXPos, this->component->getRelPosition().y);
                component->setRelSize(relSize.x, relSize.y);
                if (parent != nullptr)
                    parent->component->setAutoResized(true);
            }
            void Node::deleteAllNodes() {
                nodes.clear();
            }
            std::vector<Node*> Node::getNodes() {
                std::vector<Node*> nds;
                for (unsigned int i = 0; i < nodes.size(); i++) {
                    nds.push_back(nodes[i].get());
                }
                return nds;
            }
        }
    }
}
