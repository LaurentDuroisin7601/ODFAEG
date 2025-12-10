#ifndef ODFAEGCREATOR
#define ODFAEGCREATOR
#include "odfaeg/Network/application.h"
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
#include "odfaeg/Graphics/3D/model.hpp"
#include "rotationGismo.hpp"
#include "translationGismo.hpp"
#include "scaleGismo.hpp"
#include "odfaeg/Physics/orientedBoundingBox.h"

class ODFAEGCreator : public odfaeg::core::Application<ODFAEGCreator>,
                      public odfaeg::graphic::gui::MenuItemListener,
                      public odfaeg::graphic::gui::ActionListener {
    public :
    struct BlocInfo {
        std::multimap<unsigned int, std::pair<std::string, std::string>> blocInstances;
        unsigned int blocStart, blocEnd;
        std::vector<BlocInfo> subBlocs;
    };
    ODFAEGCreator (odfaeg::window::VideoMode vm, std::string title);
    void moveCursor(odfaeg::math::Vec2f mousePos);
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
    void displayChildren(odfaeg::graphic::gui::DropDownList* dp);
    void displayChildrenCV(odfaeg::graphic::gui::DropDownList* dp);
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
    void onWallTypeChanged(odfaeg::graphic::gui::DropDownList* dpWallType);
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
    void onMouseOnMenu(odfaeg::graphic::gui::MenuItem* menuItem);
    void onParentClickedCV(odfaeg::graphic::gui::Label* label);
    void onNameCVChanged(odfaeg::graphic::gui::TextArea* ta);
    void onSelectedParentCV (odfaeg::graphic::gui::DropDownList* dp);
    void onSelectedScriptChanged(odfaeg::graphic::gui::DropDownList* dp);
    std::map<std::string, std::vector<odfaeg::graphic::Entity*>>& getExternals();
    odfaeg::graphic::EntityFactory& getEntityFactory();
    void addId (unsigned int id);
    bool isAlreadyCalled(unsigned int id);
    void setIsCalled(unsigned int id);
    unsigned int getCurrentId();
    std::vector<odfaeg::physic::BoundingPolyhedron> getTmpBps();
    unsigned int findLine(std::string toFind, std::string content);
    void removeLine(unsigned int line, std::string& content);
    void onTextEntered(odfaeg::graphic::gui::TextArea* ta, char caracter);
    enum Fonts {
        Serif
    };
    private :
        std::pair<unsigned, unsigned> indexToLineColumn(const std::string& text, unsigned pos);
        /*bool isPrimitiveType(std::string type);
        bool isCharsOk(std::string strsearch, std::string str);
        void processInst(std::string inst, unsigned int currentPos, BlocInfo& bloc);
        void removeSpacesChars(std::string& str);
        void findComplVarsInBloc(std::vector<std::string>& instructions, BlocInfo& parentBloc, unsigned int& currentInst, unsigned int& currentPos);
        std::string getNamespaceIn(std::string content, unsigned int posInFile);
        std::string getHeaderContent(std::string content, unsigned int posInFile);
        void findLastBracket(std::string& fileContent, int nbBlocks, int& p);*/
        std::vector<std::string> checkCompletionNames(unsigned int posInFile);
        //void checkNamesToPropose(BlocInfo parentBloc, std::vector<std::string>& namesToPropose, std::string strsearch, unsigned int posInFile);
        void makeTransparent(odfaeg::graphic::Entity* entity);
        void convertSlash(std::string& path);
        odfaeg::math::Vec3f getGridCellPos(odfaeg::math::Vec3f pos);
        void addBoundingVolumes(odfaeg::physic::BoundingVolume* bv);
        float speed, sensivity;
        odfaeg::graphic::gui::CheckBox *cbIsPointer;
        odfaeg::graphic::gui::MenuBar* menuBar;
        odfaeg::graphic::gui::Menu *menu1, *menu2, *menu3, *menu4, *menu5, *menu6;
        odfaeg::graphic::gui::MenuItem *item11, *item12, *item13, *item14, *item15, *item16, *item17, *item18, *item19, *item21, *item22, *item23, *item31, *item32, *item33,
        *item34, *item35, *item36, *item37, *item38, *item39, *item310, *item311, *item312, *item41, *item42, *item43, *item44, *item45, *item46, *item47, *item51, *item52, *item61;
        odfaeg::core::ResourceCache<> cache;
        odfaeg::graphic::gui::FileDialog* fdTexturePath, *fdProjectPath, *fdImport3DModel;
        odfaeg::graphic::RenderWindow* wApplicationNew, *wNewMap, *wNewComponent, *wNewEntitiesUpdater, *wNewAnimUpdater, *wNewEmitter, *wNewParticleSystemUpdater, *wCreateNewWindow, *wCreateNewObject, *wModifyObject,
        *wGenerateTerrain, *wDeleteObject, *wGenerate3DTerrain, *wCreateScript;
        odfaeg::graphic::gui::TextArea *taScriptFileName, *ta, *taComponentExpression, *taComponentLayer, *taEntitiesUpdaterName, *taComponentName, *taAnimUpdaterName, *taPSName, *taEmissionRate,
        *taMinLifeTime, *taMaxLifeTime, *taRCPosX, *taRCPosY, *taRCPosZ, *taRCSizeX, *taRCSizeY, *taRCSizeZ, *taDeflX, *taDeflY, *taDeflZ, *taDeflAngle,
        *taRotMin, *taRotMax, *taTexIndexMin, *taTexIndexMax, *taScaleMinX, *taScaleMinY, *taScaleMinZ, *taScaleMaxX, *taScaleMaxY, *taScaleMaxZ, *taColor1, *taColor2,
        *taParticleSystemUpdaterName, *taTileWidth, *taTileHeight, *taZoneXPos, *taZoneYPos, *taZoneZPos, *taZoneWidth, *taZoneHeight, *taZoneDepth, *taFrameRate,
        *taTileWidth3D, *taTileDepth3D, *taZoneXPos3D, *taZoneYPos3D, *taZoneZPos3D, *taZoneWidth3D, *taZoneHeight3D, *taZoneDepth3D;
        odfaeg::graphic::gui::DropDownList *dpScriptType, *dpList, *dpSelectTexture, *dpMapTypeList, *dpComponentType, *dpSelectEm, *dpSelectComponent, *dpSelectParent, *dpSelectAU, *dpSelectPPType, *dpSelectPSU, *dpSelectClass, *dpSelectFunction, *dpSelectMClass,
        *dpSelectMFunction, *dpSelectRClass, *dpSelectPointerType, *dpSelectViewPerspective, *dpScriptBaseClass, *dpSelectWallType, *dpWallType, *dpSelectWallType3D, *dpChildren, *dpChildrenCV, *dpSelectParentCV, *dpSelectScript;
        odfaeg::graphic::gui::Label *lWidth, *lHeight, *lMapWidth, *lMapHeight, *lMapDepth, *lOrigX, *lOrigY, *lOrigZ, *lOrigin, *lParentCV;
        odfaeg::graphic::gui::TextArea *taWidth, *taHeight, *tScriptEdit, *taMapName, *taMapWidth, *taMapHeight, *taMapDepth, *taIntensity, *taQuality, *taWindowPos, *taWindowSize, *taWindowTitle, *taWindowName, *taObjectName, *taMObjectName, *taRObjectName, *taSelectExpression, *taNameCV;
        odfaeg::graphic::gui::Panel *pProjects, *pScriptsFiles, *pScriptsEdit, *pInfos, *pTransform, *pMaterial, *pShadows, *pCollisions, *pComponent, *pObjectsParameters, *pMObjectsParameters;
        std::string appliname, minAppliname;
        std::string applitype;
        std::string path, strsearch="";
        std::map<std::pair<std::string, std::string>, std::string> cppAppliContent;
        std::vector<std::string> openedProjects;
        std::vector<std::string> textPaths;
        std::vector<odfaeg::graphic::gui::TextArea*> tmpTextAreas;
        std::unique_ptr<odfaeg::graphic::gui::Node> rootNode, rootPropNode, rootMaterialNode, rootInfosNode, rootShadowsNode, rootCollisionNode, rootObjectParams, rootMObjectParams;
        odfaeg::graphic::gui::Node* rootScenesNode;
        odfaeg::graphic::CircleShape cursor;
        odfaeg::math::Vec3f guiSize, guiPos, mousePosition;
        bool isGuiShown, showGrid, alignToGrid, showRectSelect;
        std::vector<std::unique_ptr<odfaeg::graphic::Shape>> shapes;
        odfaeg::graphic::Entity* selectedObject;
        odfaeg::graphic::gui::TextArea *tTexId, *tPosX, *tPosY, *tPosZ, *tRColor, *tGColor, *tBColor, *tAColor, *tSizeW, *tSizeH, *tSizeD,
        *tMoveX, *tMoveY, *tMoveZ, *tScaleX, *tScaleY, *tScaleZ, *tRotAngle, *tTexCoordX, *tTexCoordY, *tTexCoordW, *tTexCoordH, *taName, *taChangeComponentExpression,
        *taXShadowCenter, *taYShadowCenter, *taZShadowCenter, *taXShadowScale, *taYShadowScale, *taZShadowScale, *taShadowRotAngle, *taBoundingBoxColX, *taBoundingBoxColY,
        *taBoundingBoxColZ, *taBoundingBoxColW, *taBoundingBoxColH, *taBoundingBoxColD, *taOriginX, *taOriginY, *taOriginZ,
        *taAmbientLightColor;
        odfaeg::graphic::gui::Label *lPosX, *lPosY, *lPosZ, *lPosition, *lColor, *lRColor,
        *lGColor, *lBColor, *lAColor, *lTexture, *lTexCoordX, *lTexCoordY, *lTexCoordW, *lTexCoordH, *lTexImage, *lParent, *lTexId;
        odfaeg::graphic::gui::TabPane* tabPane;
        odfaeg::graphic::gui::Button* bChooseText, *bAddTexRect, *bCreateComponent, *bCreateScene, *bCreateEntitiesUpdater, *bCreateAppli, *bCreateAnimUpdater, *bCreateEmitter, *bCreateParticleSystemUpdater, *bCreateWindow, *bCreateObject, *bModifyObject,
        *bGenerateTerrain, *bGenerate3DTerrain, *bRemoveObject, *bAddTileGround, *bAddWall, *bAddTileGround3D, *bAddWall3D, *bSetTexRect, *bAssignCollisionVolume, *bCreateScript;
        odfaeg::graphic::Shape* sTextRect;
        odfaeg::core::StateStack stateStack;
        ODFAEGCreatorStateExecutor se;
        std::vector<odfaeg::graphic::ConvexShape> cshapes;
        std::vector<odfaeg::graphic::ConvexShape> cellsPassableShapes;
        odfaeg::graphic::RectangleShape collisionBox;
        odfaeg::physic::BoundingVolume* selectedBoundingVolume;
        std::vector<odfaeg::physic::BoundingVolume*> boundingVolumes;
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
        bool isSelectingPolyhedron, isGeneratingTerrain, isGenerating3DTerrain, isSecondClick;
        std::vector<std::string> argsTps;
        std::vector<odfaeg::graphic::Tile*> ground;
        std::vector<odfaeg::graphic::g2d::Wall*> walls;
        std::vector<odfaeg::graphic::g3d::Wall*> walls3D;
        odfaeg::graphic::g3d::Model loader;
        RotationGuismo rotationGuismo;
        TranslationGuismo translationGuismo;
        ScaleGuismo scaleGuismo;
        odfaeg::math::Vec3f firstInters, prevObjectPos, prevObjectScale;
        bool isMovingXPos, isMovingYPos, isMovingZPos, isScalingX, isScalingY, isScalingZ;
        float prevAngle;
        odfaeg::graphic::View selectedComponentView;
        odfaeg::physic::OrientedBoundingBox selectionBox;
        std::vector<std::unique_ptr<odfaeg::graphic::World>> worlds;
        std::string selectedProject;
        std::string virtualFile;
        /*std::array<std::string, 24> primitiveTypes = {
            "bool",
            "char", "signed char", "unsigned char",
            "short", "unsigned short",
            "int", "unsigned int",
            "long", "unsigned long",
            "long long", "unsigned long long",
            "float", "double", "long double",
            "size_t", "int8_t", "uint8_t",
            "int16_t", "uint16_t", "int32_t",
            "uint32_t", "int64_t", "uint64_t"
        };*/
};

#endif

