#ifndef ODFAEGCREATOR
#define ODFAEGCREATOR
#include "odfaeg/Graphics/application.h"
#include "odfaeg/Core/stateStack.h"
#include "odfaeg/Graphics/GUI/menubar.hpp"
#include "odfaeg/Graphics/GUI/menuItem.hpp"
#include "odfaeg/Graphics/GUI/filedialog.hpp"
#include "odfaeg/Graphics/GUI/textArea.hpp"
#include "odfaeg/Graphics/GUI/button.hpp"
#include "odfaeg/Graphics/GUI/label.hpp"
#include "odfaeg/Graphics/GUI/dropDownList.hpp"
#include "odfaeg/Graphics/GUI/node.hpp"
#include "odfaeg/Graphics/GUI/tabPane.hpp"
#include "odfaeg/Graphics/GUI/checkBox.hpp"
#include "odfaeg/Graphics/circleShape.h"
#include "odfaeg/Graphics/rectangleShape.h"
#include "odfaeg/Graphics/sprite.h"
#include "odfaeg/Graphics/2D/decor.h"
#include "odfaeg/Graphics/2D/wall.h"
#include "odfaeg/Graphics/map.h"
#include "odfaegCreatorStateExecutor.hpp"
#include "odfaeg/Graphics/perPixelLinkedListRenderComponent.hpp"
#include "odfaeg/Graphics/shadowRenderComponent.hpp"
#include "odfaeg/Graphics/lightRenderComponent.hpp"
#include "odfaeg/Graphics/reflectRefractRenderComponent.hpp"
#include "odfaeg/Graphics/entitiesUpdater.h"
#include "rectangularSelection.hpp"
#include "odfaeg/Graphics/anim.h"
#include "odfaeg/Graphics/animationUpdater.h"
#include "odfaeg/Graphics/particleSystemUpdater.hpp"
#include "odfaeg/Graphics/tGround.h"
#include "odfaeg/Math/distributions.h"
#include "odfaeg/Core/class.hpp"
#include "odfaeg/Core/runtimeCompiler.hpp"
class ODFAEGCreator : public odfaeg::core::Application,
                      public odfaeg::graphic::gui::MenuItemListener,
                      public odfaeg::graphic::gui::ActionListener {
    public :
    ODFAEGCreator (sf::VideoMode vm, std::string title);
    void moveCursor(sf::Vector2f mousePos);
    void onLoad();
    void onInit ();
    void onRender(odfaeg::graphic::RenderComponentManager *cm);
    void onDisplay(odfaeg::graphic::RenderWindow* window);
    void onUpdate (odfaeg::graphic::RenderWindow*, odfaeg::window::IEvent& event);
    void onExec ();
    void processKeyHeldDown(odfaeg::window::IKeyboard::Key key);
    void actionPerformed(odfaeg::graphic::gui::Button* item);
    void actionPerformed(odfaeg::graphic::gui::MenuItem* item);
    void showProjectsFiles(odfaeg::graphic::gui::Label* label);
    void showSourcesFiles(odfaeg::graphic::gui::Label* label);
    void showHeadersFiles(odfaeg::graphic::gui::Label* label);
    void showScenes(odfaeg::graphic::gui::Label* label);
    void showScene(odfaeg::graphic::gui::Label* label);
    void showFileContent(odfaeg::graphic::gui::Label* lab);
    void displayInfos(odfaeg::graphic::Shape* shape);
    void displayExternalEntityInfo(odfaeg::graphic::Entity* entity);
    void displayBigtileInfos(odfaeg::graphic::Entity* bigTile);
    void displayEntityInfos(odfaeg::graphic::Entity* entity);
    void displayTransformInfos(odfaeg::graphic::Transformable* transf);
    void displayTileInfos(odfaeg::graphic::Entity* tile);
    void displayDecorInfos(odfaeg::graphic::Entity* decor);
    void displayWallInfos(odfaeg::graphic::Entity* wall);
    void displayAnimInfos(odfaeg::graphic::Entity* anim);
    void displayParticleSystemInfos(odfaeg::graphic::Entity* ps);
    void displayPonctualLightInfos(odfaeg::graphic::Entity* pl);
    void displayChildren(odfaeg::graphic::gui::Label* label);
    void onObjectPosChanged(odfaeg::graphic::gui::TextArea* ta);
    void onObjectSizeChanged(odfaeg::graphic::gui::TextArea* ta);
    void onObjectColorChanged(odfaeg::graphic::gui::TextArea* ta);
    void onSelectedTextureChanged(odfaeg::graphic::gui::DropDownList* dp);
    void onTexCoordsChanged(odfaeg::graphic::gui::TextArea* ta);
    void onSelectedEmChanged(odfaeg::graphic::gui::DropDownList* dp);
    void onAnimUpdaterChanged(odfaeg::graphic::gui::DropDownList* dp);
    void onObjectMoveChanged(odfaeg::graphic::gui::TextArea* ta);
    void onObjectScaleChanged(odfaeg::graphic::gui::TextArea* ta);
    void onObjectRotationChanged(odfaeg::graphic::gui::TextArea* ta);
   /* void addShape(odfaeg::graphic::Shape *shape);
    void addTile(odfaeg::graphic::Tile *tile);*/
    bool removeShape (unsigned int id);
    /*void updateScriptPos(odfaeg::graphic::Transformable* shape);
    void updateScriptColor(odfaeg::graphic::Transformable* shape);
    void updateScriptTextCoords(odfaeg::graphic::Transformable* shape);
    void updateScriptText(odfaeg::graphic::Shape* shape, const odfaeg::graphic::Texture* text);
    void updateScriptText(odfaeg::graphic::Tile* tile, const odfaeg::graphic::Texture* text);*/
    void onObjectNameChanged(odfaeg::graphic::gui::TextArea* ta);
    void onSelectedParentChanged(odfaeg::graphic::gui::DropDownList* dp);
    void onWallTypeChanged(odfaeg::graphic::gui::TextArea* taWallType);
    void onFrameRateChanged(odfaeg::graphic::gui::TextArea* taFRChanged);
    void onParentClicked(odfaeg::graphic::gui::Label* label);
    void onParticleSystemUpdaterChanged(odfaeg::graphic::gui::DropDownList* dp);
    void onComponentExpressionChanged(odfaeg::graphic::gui::TextArea* ta);
    void onSelectedComponentChanged(odfaeg::graphic::gui::DropDownList* dp);
    void onShadowCenterChanged(odfaeg::graphic::gui::TextArea* ta);
    void onShadowScaleChanged(odfaeg::graphic::gui::TextArea* ta);
    void onShadowRotAngleChanged(odfaeg::graphic::gui::TextArea* ta);
    void onCollisionBoundingBoxChanged(odfaeg::graphic::gui::TextArea* ta);
    void onSelectedClassChanged(odfaeg::graphic::gui::DropDownList* dp);
    void onSelectedFunctionChanged(odfaeg::graphic::gui::DropDownList* dp);
    void onDroppedDown(odfaeg::graphic::gui::DropDownList* dp);
    void addExternalEntity(odfaeg::graphic::Entity* entity, std::string type);
    void updateNb(std::string name, unsigned int nb);
    void onSelectPointerType(odfaeg::graphic::gui::DropDownList* dp);
    void onSelectedMClassChanged(odfaeg::graphic::gui::DropDownList *dp);
    void onSelectedMFunctionChanged(odfaeg::graphic::gui::DropDownList* dp);
    void onViewPerspectiveChanged(odfaeg::graphic::gui::DropDownList* dp);
    void onSelectedTextureDroppedDown(odfaeg::graphic::gui::DropDownList* dp);
    void onSelectedTextureNotDroppedDown(odfaeg::graphic::gui::DropDownList* dp);
    void onSelectedWallTypeDroppedDown(odfaeg::graphic::gui::DropDownList* dp);
    void onSelectedWallTypeNotDroppedDown(odfaeg::graphic::gui::DropDownList* dp);
    void onObjectOriginChanged(odfaeg::graphic::gui::TextArea* ta);
    std::map<std::string, std::vector<odfaeg::graphic::Entity*>>& getExternals();
    odfaeg::graphic::EntityFactory& getEntityFactory();
    void addId (unsigned int id);
    bool isAlreadyCalled(unsigned int id);
    void setIsCalled(unsigned int id);
    unsigned int getCurrentId();
    std::vector<odfaeg::physic::BoundingPolyhedron> getTmpBps();
    unsigned int findLine(std::string toFind, std::string content);
    void removeLine(unsigned int line, std::string& content);
    enum Fonts {
        Serif
    };
    private :
        void makeTransparent(odfaeg::graphic::Entity* entity);
        void convertSlash(std::string& path);
        odfaeg::math::Vec3f getGridCellPos(odfaeg::math::Vec3f pos);
        float speed, sensivity;
        odfaeg::graphic::gui::CheckBox *cbIsPointer, *cbIs3DTerrain;
        odfaeg::graphic::gui::MenuBar* menuBar;
        odfaeg::graphic::gui::Menu *menu1, *menu2, *menu3, *menu4, *menu5;
        odfaeg::graphic::gui::MenuItem *item11, *item12, *item13, *item14, *item15, *item16, *item17, *item18, *item19, *item21, *item22, *item23, *item31, *item32, *item33,
        *item34, *item35, *item36, *item37, *item38, *item39, *item310, *item311, *item41, *item42, *item43, *item44, *item45, *item46, *item51, *item52;
        odfaeg::core::ResourceCache<> cache;
        odfaeg::graphic::gui::FileDialog* fdTexturePath, *fdProjectPath;
        odfaeg::graphic::RenderWindow* wApplicationNew, *wNewMap, *wNewComponent, *wNewEntitiesUpdater, *wNewAnimUpdater, *wNewEmitter, *wNewParticleSystemUpdater, *wCreateNewWindow, *wCreateNewObject, *wModifyObject,
        *wGenerateTerrain, *wDeleteObject;
        odfaeg::graphic::gui::TextArea* ta, *taComponentExpression, *taComponentLayer, *taEntitiesUpdaterName, *taComponentName, *taAnimUpdaterName, *taPSName, *taEmissionRate,
        *taMinLifeTime, *taMaxLifeTime, *taRCPosX, *taRCPosY, *taRCPosZ, *taRCSizeX, *taRCSizeY, *taRCSizeZ, *taDeflX, *taDeflY, *taDeflZ, *taDeflAngle,
        *taRotMin, *taRotMax, *taTexIndexMin, *taTexIndexMax, *taScaleMinX, *taScaleMinY, *taScaleMinZ, *taScaleMaxX, *taScaleMaxY, *taScaleMaxZ, *taColor1, *taColor2,
        *taParticleSystemUpdaterName, *taTileWidth, *taTileHeight, *taZoneXPos, *taZoneYPos, *taZoneZPos, *taZoneWidth, *taZoneHeight, *taZoneDepth;
        odfaeg::graphic::gui::DropDownList* dpList, *dpSelectTexture, *dpMapTypeList, *dpComponentType, *dpSelectEm, *dpSelectComponent, *dpSelectParent, *dpSelectAU, *dpSelectPPType, *dpSelectPSU, *dpSelectClass, *dpSelectFunction, *dpSelectMClass,
        *dpSelectMFunction, *dpSelectRClass, *dpSelectPointerType, *dpSelectViewPerspective, *dpScriptBaseClass, *dpSelectWallType;
        odfaeg::graphic::gui::Label *lWidth, *lHeight, *lMapWidth, *lMapHeight, *lOrigX, *lOrigY, *lOrigZ, *lOrigin;
        odfaeg::graphic::gui::TextArea *taWidth, *taHeight, *tScriptEdit, *taMapName, *taMapWidth, *taMapHeight, *taWallType, *taIntensity, *taQuality, *taWindowPos, *taWindowSize, *taWindowTitle, *taWindowName, *taObjectName, *taMObjectName, *taRObjectName, *taSelectExpression;
        odfaeg::graphic::gui::Panel *pProjects, *pScriptsFiles, *pScriptsEdit, *pInfos, *pTransform, *pMaterial, *pShadows, *pCollisions, *pComponent, *pObjectsParameters, *pMObjectsParameters;
        std::string appliname, minAppliname;
        std::string applitype;
        std::string path;
        std::map<std::string, std::string> cppAppliContent;
        std::vector<std::string> openedProjects;
        std::vector<std::string> textPaths;
        std::vector<odfaeg::graphic::gui::TextArea*> tmpTextAreas;
        std::unique_ptr<odfaeg::graphic::gui::Node> rootNode, rootScenesNode, rootPropNode, rootMaterialNode, rootInfosNode, rootShadowsNode, rootCollisionNode, rootObjectParams, rootMObjectParams;
        odfaeg::graphic::CircleShape cursor;
        odfaeg::math::Vec3f guiSize, guiPos, mousePosition;
        bool isGuiShown, showGrid, alignToGrid, showRectSelect;
        std::vector<std::unique_ptr<odfaeg::graphic::Shape>> shapes;
        odfaeg::graphic::Transformable* selectedObject;
        odfaeg::graphic::gui::TextArea *tTexId, *tPosX, *tPosY, *tPosZ, *tRColor, *tGColor, *tBColor, *tAColor, *tSizeW, *tSizeH, *tSizeD,
        *tMoveX, *tMoveY, *tMoveZ, *tScaleX, *tScaleY, *tScaleZ, *tRotAngle, *tTexCoordX, *tTexCoordY, *tTexCoordW, *tTexCoordH, *taName, *taChangeComponentExpression,
        *taXShadowCenter, *taYShadowCenter, *taZShadowCenter, *taXShadowScale, *taYShadowScale, *taZShadowScale, *taShadowRotAngle, *taBoundingBoxColX, *taBoundingBoxColY,
        *taBoundingBoxColZ, *taBoundingBoxColW, *taBoundingBoxColH, *taBoundingBoxColD, *taOriginX, *taOriginY, *taOriginZ;
        odfaeg::graphic::gui::Label *lPosX, *lPosY, *lPosZ, *lPosition, *lColor, *lRColor,
        *lGColor, *lBColor, *lAColor, *lTexture, *lTexCoordX, *lTexCoordY, *lTexCoordW, *lTexCoordH, *lTexImage, *lParent, *lTexId;
        odfaeg::graphic::gui::TabPane* tabPane;
        odfaeg::graphic::gui::Button* bChooseText, *bAddTexRect, *bCreateComponent, *bCreateScene, *bCreateEntitiesUpdater, *bCreateAppli, *bCreateAnimUpdater, *bCreateEmitter, *bCreateParticleSystemUpdater, *bCreateWindow, *bCreateObject, *bModifyObject,
        *bGenerateTerrain, *bRemoveObject, *bAddTileGround, *bAddWall, *bSetTexRect;
        odfaeg::graphic::Shape* sTextRect;
        odfaeg::core::StateStack stateStack;
        ODFAEGCreatorStateExecutor se;
        std::vector<odfaeg::graphic::ConvexShape> cshapes;
        std::vector<odfaeg::graphic::ConvexShape> cellsPassableShapes;
        std::vector<odfaeg::graphic::RectangleShape> collisionsBox;
        odfaeg::graphic::Scene* theMap;
        int gridWidth, gridHeight, oldX, oldY;
        RectangularSelection rectSelect;
        std::vector<std::string> emitterParams;
        std::vector<std::string> affectorParams;
        odfaeg::math::Vec3f viewPos;
        odfaeg::core::RuntimeCompiler rtc;
        std::map<std::string, unsigned int> nbs;
        std::vector<odfaeg::graphic::Entity*> selectionBorders;
        std::map<std::string, std::vector<odfaeg::graphic::Entity*>> externals;
        std::map<std::string, std::vector<odfaeg::graphic::Entity*>> toAdd;
        std::string pluginSourceCode;
        odfaeg::graphic::EntityFactory factory;
        unsigned int currentId, currentBp;
        std::map<unsigned int, bool> callIds;
        std::vector<odfaeg::physic::BoundingSphere> selectBpPoints;
        std::vector<odfaeg::physic::BoundingPolyhedron> tmpBps;
        odfaeg::graphic::VertexArray bpLines;
        odfaeg::graphic::VertexArray bpPoints;
        bool isSelectingPolyhedron, isGeneratingTerrain, isSecondClick;
        std::vector<std::string> argsTps;
        std::vector<odfaeg::graphic::Tile*> ground;
        std::vector<odfaeg::graphic::g2d::Wall*> walls;
};
#endif
