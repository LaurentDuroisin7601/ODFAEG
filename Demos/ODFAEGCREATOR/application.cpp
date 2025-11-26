

#include "application.hpp"
#include "odfaeg/Window/action.h"
#include "odfaeg/Window/command.h"
#include <sys/stat.h>
#include <stdlib.h>
#include "odfaeg/Core/utilities.h"
#include <queue>

using namespace odfaeg::core;
using namespace odfaeg::math;
using namespace odfaeg::graphic;
using namespace odfaeg::graphic::gui;
using namespace odfaeg::graphic::g2d;
using namespace odfaeg::physic;
using namespace odfaeg::window;
ODFAEGCreator::ODFAEGCreator(VideoMode vm, std::string title) :
Application (vm, title, Style::Resize|Style::Close, ContextSettings(0, 8, 4, 4, 6)), loader(getDevice()), isGuiShown (false), cursor(10), se(this), rtc("create"), rotationGuismo(), translationGuismo(10), scaleGuismo(10) {
    //Command::sname = "modified by process";

    isSelectingPolyhedron = false;
    isSecondClick = false;
    isGeneratingTerrain = false;
    isGenerating3DTerrain = false;
    dpSelectTexture = nullptr;
    dpSelectEm = nullptr;
    sTextRect = nullptr;
    dpSelectPSU = nullptr;
    showGrid = false;
    alignToGrid = false;
    showRectSelect = false;
    gridWidth = 100;
    gridHeight = 50;
    appliname = "";
    currentId = 0;
    currentBp = 0;
    fdTexturePath = nullptr;
    fdProjectPath = nullptr;
    bAddTexRect = nullptr;
    bSetTexRect = nullptr;
    selectedBoundingVolume = nullptr;
    selectedObject = nullptr;
    viewPos = getRenderWindow().getView().getPosition();
    rtc.addOption("std=c++20");
    rtc.addMacro("ODFAEG_STATIC");
    rtc.addIncludeDir("\"C:\\Program Files (x86)\\ODFAEG\\include\"");
    rtc.addIncludeDir("..\\..\\ODFAEG-master2\\ODFAEG\\extlibs\\headers");
    rtc.addLibraryDir("\"C:\\Program Files (x86)\\ODFAEG\\lib\"");
    rtc.addLibraryDir("..\\..\\ODFAEG-master2\\ODFAEG\\extlibs\\libs-mingw\\x64");
    rtc.addLibrary("odfaeg-network-s-d");
	rtc.addLibrary("odfaeg-audio-s-d");
	rtc.addLibrary("odfaeg-graphics-s-d");
	rtc.addLibrary("odfaeg-physics-s-d");
	rtc.addLibrary("odfaeg-core-s-d");
	rtc.addLibrary("odfaeg-window-s-d");
	rtc.addLibrary("odfaeg-math-s-d");
	rtc.addLibrary("assimp");
	rtc.addLibrary("FLAC");
	rtc.addLibrary("vorbis");
	rtc.addLibrary("vorbisenc");
	rtc.addLibrary("vorbisfile");
	rtc.addLibrary("glfw3.dll");
	rtc.addLibrary("SDL3.dll");
	rtc.addLibrary("shaderc_shared.dll");
    rtc.addLibrary("jpeg");
    rtc.addLibrary("ogg");
	rtc.addLibrary("crypto.dll");
	rtc.addLibrary("ssl.dll");
	rtc.addLibrary("freetype");
	rtc.addLibrary("glew32");
	rtc.addLibrary("opengl32");
	rtc.addLibrary("vulkan-1.dll");
	rtc.addLibrary("openal32");
	rtc.addLibrary("ws2_32");
	rtc.addLibrary("gdi32");
	rtc.addLibrary("dl.dll");
	rtc.addLibrary("sndfile.dll");
	rtc.addRuntimeFunction("createObject");
	rtc.addRuntimeFunction("readObjects");
	rtc.addSourceFile("../../ODFAEG-master/Demos/ODFAEGCREATOR/application");
    rtc.addSourceFile("../../ODFAEG-master/Demos/ODFAEGCREATOR/odfaegCreatorStateExecutor");
    rtc.addSourceFile("../../ODFAEG-master/Demos/ODFAEGCREATOR/rectangularSelection");
    rtc.addSourceFile("../../ODFAEG-master/Demos/ODFAEGCREATOR/rotationGismo");
    rtc.addSourceFile("../../ODFAEG-master/Demos/ODFAEGCREATOR/scaleGuismo");
    rtc.addSourceFile("../../ODFAEG-master/Demos/ODFAEGCREATOR/translationGismo");
	speed = 10.f;
    sensivity = 0.5f;
    /*oldX = IMouse::getPosition(getRenderWindow()).x();
    oldY = IMouse::getPosition(getRenderWindow()).y();*/
    pluginSourceCode = "";


	getRenderWindow().setKeyRepeatEnabled(false);
	prevAngle = 0;
	isMovingXPos = isMovingYPos = isMovingZPos = isScalingX = isScalingY = isScalingZ = false;

	EXPORT_CLASS_GUID(BoundingVolumeBoundingBox, BoundingVolume, BoundingBox)
    EXPORT_CLASS_GUID_(EntityTile, Entity, Tile,
                       (EntityFactory&),
                       (std::ref(factory)))
    EXPORT_CLASS_GUID_(EntityBigTile, Entity, BigTile,
                       (EntityFactory&), (std::ref(factory)))
    EXPORT_CLASS_GUID_(EntityWall, Entity, g2d::Wall,
                       (EntityFactory&),
                       (std::ref(factory)))
    EXPORT_CLASS_GUID_(EntityDecor, Entity, g2d::Decor,
                       (EntityFactory&),
                       (std::ref(factory)))
    EXPORT_CLASS_GUID_(EntityAnimation, Entity, Anim,
                       (EntityFactory&),
                       (std::ref(factory)))
    EXPORT_CLASS_GUID_(EntityMesh, Entity, Mesh,
                       (EntityFactory&),
                       (std::ref(factory)))
    EXPORT_CLASS_GUID_(EntityParticleSystem, Entity, ParticleSystem,
                       (Device&)(EntityFactory&),
                       (std::ref(getDevice()))(std::ref(factory)))
    EXPORT_CLASS_GUID(ShapeRectangleShape, Shape, RectangleShape)
}
void ODFAEGCreator::onLoad() {
    std::tuple<std::reference_wrapper<Device>> rArgs = std::make_tuple(std::ref(getDevice()));
    FontManager<Fonts> fm;
    try {
        fm.fromFileWithAlias("fonts/FreeSerif.ttf", Serif, rArgs);
    }
    catch (Erreur& erreur) {
        std::cerr << erreur.what() << std::endl;
    }
    TextureManager<> tm;
    cache.addResourceManager(fm, "FontManager");
    cache.addResourceManager(tm, "TextureManager");
}
void ODFAEGCreator::onInit() {
    FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
    menuBar = new MenuBar(getRenderWindow());
    getRenderComponentManager().addComponent(menuBar);
    try {
        menu1 = new Menu(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "File");
        getRenderComponentManager().addComponent(menu1);
        menu2 = new Menu(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Compile");
        getRenderComponentManager().addComponent(menu2);
        menu3 = new Menu(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Add");
        getRenderComponentManager().addComponent(menu3);
        menu4 = new Menu(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Edition");
        getRenderComponentManager().addComponent(menu4);
        menu5 = new Menu(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "object plugin");
        getRenderComponentManager().addComponent(menu5);
        menu6 = new Menu(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "new collision volume");
        getRenderComponentManager().addComponent(menu6);


        menuBar->addMenu(menu1);
        menuBar->addMenu(menu2);
        menuBar->addMenu(menu3);
        menuBar->addMenu(menu4);
        menuBar->addMenu(menu5);
        menuBar->addMenu(menu6);

        Action amom(Action::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
        item11 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "New application");
        item11->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item11);
        item12 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "New scene");
        item12->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item12);
        item13 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "New component");
        item13->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item13);
        item14 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "New entities updater");
        item14->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item14);
        item15 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Open application");
        item15->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item15);
        item16 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "New anim updater");
        item16->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item16);
        item17 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Save project");
        item17->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item17);
        item18 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "New particle system updater");
        item18->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item18);
        item19 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "New script");
        item19->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item19);

        menu1->addMenuItem(item11);
        menu1->addMenuItem(item12);
        menu1->addMenuItem(item13);
        menu1->addMenuItem(item14);
        menu1->addMenuItem(item15);
        menu1->addMenuItem(item16);
        menu1->addMenuItem(item17);
        menu1->addMenuItem(item18);
        menu1->addMenuItem(item19);

        item21 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Build");
        item21->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item21);
        item22 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Run");
        item22->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item22);
        item23 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Build and run");
        item23->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item23);
        menu2->addMenuItem(item21);
        menu2->addMenuItem(item22);
        menu2->addMenuItem(item23);
        item31 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Rectangle shape");
        item31->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item31);
        item32 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Circle Shape");
        getRenderComponentManager().addComponent(item32);
        item33 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Sprite");
        getRenderComponentManager().addComponent(item33);
        item34 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Tile");
        getRenderComponentManager().addComponent(item34);
        item34->addMenuItemListener(this);
        item35 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Decor");
        getRenderComponentManager().addComponent(item35);
        item35->addMenuItemListener(this);
        item36 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Wall");
        getRenderComponentManager().addComponent(item36);
        item36->addMenuItemListener(this);
        item37 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Animation");
        getRenderComponentManager().addComponent(item37);
        item37->addMenuItemListener(this);
        item38 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Particle System");
        item38->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item38);
        item39 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Emitter");
        item39->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item39);
        item310 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Affector");
        item310->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item310);
        item311 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Ponctual Light");
        item311->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item311);
        item312 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "3D Model");
        item312->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item312);
        menu3->addMenuItem(item31);
        menu3->addMenuItem(item32);
        menu3->addMenuItem(item33);
        menu3->addMenuItem(item34);
        menu3->addMenuItem(item35);
        menu3->addMenuItem(item36);
        menu3->addMenuItem(item37);
        menu3->addMenuItem(item38);
        menu3->addMenuItem(item39);
        menu3->addMenuItem(item310);
        menu3->addMenuItem(item311);
        menu3->addMenuItem(item312);
        item41 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Undo");
        item41->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item41);
        item42 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Redo");
        item42->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item42);
        item43 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Show grid");
        item43->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item43);
        item44 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Align to grid");
        item44->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item44);
        item45 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Rect selection");
        item45->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item45);
        item46 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Generate terrain");
        item46->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item46);
        item47 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Generate 3D terrain");
        item47->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item47);

        menu4->addMenuItem(item41);
        menu4->addMenuItem(item42);
        menu4->addMenuItem(item43);
        menu4->addMenuItem(item44);
        menu4->addMenuItem(item45);
        menu4->addMenuItem(item46);
        menu4->addMenuItem(item47);

        item51 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "New object");
        Command cmdmom(amom, FastDelegate<bool>(&MenuItem::isMouseOnMenu, item51), FastDelegate<void>(&ODFAEGCreator::onMouseOnMenu, this, item51));
        item51->getListener().connect("A"+item51->getText(), cmdmom);
        item51->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item51);
        item52 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "Modify object");
        item52->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item52);
        menu5->addMenuItem(item51);
        menu5->addMenuItem(item52);

        item61 = new MenuItem(getRenderWindow(), fm.getResourceByAlias(Fonts::Serif), "New bounding box");
        item61->addMenuItemListener(this);
        getRenderComponentManager().addComponent(item61);
        menu6->addMenuItem(item61);

        Action a1(Action::EVENT_TYPE::KEY_HELD_DOWN, IKeyboard::Key::Z);
        Action a2(Action::EVENT_TYPE::KEY_HELD_DOWN, IKeyboard::Key::Q);
        Action a3(Action::EVENT_TYPE::KEY_HELD_DOWN, IKeyboard::Key::S);
        Action a4(Action::EVENT_TYPE::KEY_HELD_DOWN, IKeyboard::Key::D);
        Action combined = a1 || a2 || a3 || a4;
        Command moveAction(combined, FastDelegate<void>(&ODFAEGCreator::processKeyHeldDown, this, IKeyboard::Unknown));
        getListener().connect("MoveAction", moveAction);
        fdTexturePath = new FileDialog(getDevice(),Vec3f(0, 0, 0), Vec3f(800, 600, 0), fm.getResourceByAlias(Fonts::Serif));
        fdTexturePath->setVisible(false);
        fdTexturePath->setEventContextActivated(false);
        addWindow(&fdTexturePath->getWindow(), false);
        getRenderComponentManager().addComponent(fdTexturePath);
        fdProjectPath = new FileDialog(getDevice(), Vec3f(0, 0, 0), Vec3f(800, 600, 0), fm.getResourceByAlias(Fonts::Serif));
        fdProjectPath->setVisible(false);
        fdProjectPath->setEventContextActivated(false);
        fdProjectPath->setName("fd open project");
        addWindow(&fdProjectPath->getWindow(), false);
        getRenderComponentManager().addComponent(fdProjectPath);
        fdImport3DModel = new FileDialog(getDevice(), Vec3f(0, 0, 0), Vec3f(800, 600, 0), fm.getResourceByAlias(Fonts::Serif));
        fdImport3DModel->setVisible(false);
        fdImport3DModel->setEventContextActivated(false);
        addWindow(&fdImport3DModel->getWindow(), false);
        getRenderComponentManager().addComponent(fdImport3DModel);
        wApplicationNew = new RenderWindow(VideoMode(400, 300), "Create ODFAEG Application", getDevice(), Style::Default, ContextSettings(0, 0, 0, 3, 0));
        //New application.
        Label* label = new Label(*wApplicationNew, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Application name : ", 15);
        getRenderComponentManager().addComponent(label);
        ta = new TextArea(Vec3f(200, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wApplicationNew);
        getRenderComponentManager().addComponent(ta);
        Label* label2 = new Label(*wApplicationNew, Vec3f(0, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Application type : ", 15);
        getRenderComponentManager().addComponent(label2);
        dpList = new DropDownList(*wApplicationNew, Vec3f(200, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Normal", 15);
        dpList->addItem("Client", 15);
        dpList->addItem("Server", 15);
        getRenderComponentManager().addComponent(dpList);
        lWidth = new Label(*wApplicationNew, Vec3f(0, 100, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Width : ", 15);
        lHeight = new Label(*wApplicationNew, Vec3f(200, 100, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Height : ", 15);
        getRenderComponentManager().addComponent(lWidth);
        getRenderComponentManager().addComponent(lHeight);
        taWidth = new TextArea(Vec3f(50, 100, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wApplicationNew);
        taHeight = new TextArea(Vec3f(250, 100, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wApplicationNew);
        getRenderComponentManager().addComponent(taWidth);
        getRenderComponentManager().addComponent(taHeight);
        bCreateAppli = new Button(Vec3f(0, 200, 0), Vec3f(400, 100, 0), fm.getResourceByAlias(Fonts::Serif), "Create", 15, *wApplicationNew);
        bCreateAppli->addActionListener(this);
        getRenderComponentManager().addComponent(bCreateAppli);
        addWindow(wApplicationNew);
        wApplicationNew->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wApplicationNew);
        //Create map window.
        wNewMap = new RenderWindow(VideoMode(400, 400), "Create new scene", getDevice(),Style::Default, ContextSettings(0, 0, 0, 3, 0));
        Label* labMapName = new Label(*wNewMap, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Map name : ", 15);
        getRenderComponentManager().addComponent(labMapName);
        taMapName = new TextArea(Vec3f(200, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewMap);
        getRenderComponentManager().addComponent(taMapName);
        Label* labMapType = new Label(*wNewMap, Vec3f(0, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Application type : ", 15);
        getRenderComponentManager().addComponent(labMapType);
        dpMapTypeList = new DropDownList(*wNewMap, Vec3f(200, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Normal", 15);
        dpMapTypeList->addItem("2D iso", 15);
        getRenderComponentManager().addComponent(dpMapTypeList);
        lMapWidth = new Label(*wNewMap, Vec3f(0, 100, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "case width : ", 15);
        lMapHeight = new Label(*wNewMap, Vec3f(200, 100, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "case height : ", 15);
        lMapDepth = new Label(*wNewMap, Vec3f(0, 200, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "case depth : ", 15);

        getRenderComponentManager().addComponent(lMapWidth);
        getRenderComponentManager().addComponent(lMapHeight);
        getRenderComponentManager().addComponent(lMapDepth);
        taMapWidth = new TextArea(Vec3f(80, 100, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "100", *wNewMap);
        taMapHeight = new TextArea(Vec3f(280, 100, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "50", *wNewMap);
        taMapDepth = new TextArea(Vec3f(80, 200, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "0", *wNewMap);
        getRenderComponentManager().addComponent(taMapWidth);
        getRenderComponentManager().addComponent(taMapHeight);
        getRenderComponentManager().addComponent(taMapDepth);
        bCreateScene = new Button(Vec3f(0, 300, 0), Vec3f(400, 100, 0), fm.getResourceByAlias(Fonts::Serif), "Create scene", 15, *wNewMap);
        bCreateScene->addActionListener(this);
        getRenderComponentManager().addComponent(bCreateScene);
        addWindow(wNewMap);
        wNewMap->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wNewMap);
        //Create component.
        wNewComponent = new RenderWindow(VideoMode(1000, 300), "Create new render component", getDevice(), Style::Default, ContextSettings(0, 0, 0, 3, 0));
        Label* labComponentExpression = new Label(*wNewComponent, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "entity's type(s) : ", 15);
        getRenderComponentManager().addComponent(labComponentExpression);
        taComponentExpression = new TextArea(Vec3f(200, 0, 0), Vec3f(800, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewComponent);
        getRenderComponentManager().addComponent(taComponentExpression);
        Label* lComponentLayer = new Label(*wNewComponent, Vec3f(0, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Layer number : ", 15);
        getRenderComponentManager().addComponent(lComponentLayer);
        taComponentLayer = new TextArea(Vec3f(200, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "0", *wNewComponent);
        getRenderComponentManager().addComponent(taComponentLayer);
        Label* lComponentType = new Label(*wNewComponent, Vec3f(0, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "component type : ", 15);
        getRenderComponentManager().addComponent(lComponentType);
        dpComponentType = new DropDownList(*wNewComponent, Vec3f(200, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "LinkedList", 15);
        dpComponentType->addItem("Shadow", 15);
        dpComponentType->addItem("Light", 15);
        dpComponentType->addItem("Refraction", 15);
        getRenderComponentManager().addComponent(dpComponentType);
        Label* lComponentName = new Label(*wNewComponent, Vec3f(0, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Name : ", 15);
        getRenderComponentManager().addComponent(lComponentName);
        taComponentName = new TextArea(Vec3f(200, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewComponent);
        getRenderComponentManager().addComponent(taComponentName);
        bCreateComponent = new Button(Vec3f(0, 200, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Create component", 15, *wNewComponent);
        bCreateComponent->addActionListener(this);
        getRenderComponentManager().addComponent(bCreateComponent);
        addWindow(wNewComponent);
        wNewComponent->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wNewComponent);
        //Create entities updater.
        wNewEntitiesUpdater = new RenderWindow(VideoMode(400, 300), "Create new entities updater", getDevice(), Style::Default, ContextSettings(0, 0, 0, 3, 0));
        Label* lEntitiesUpdaterName = new Label(*wNewEntitiesUpdater, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "entities updater name : ", 15);
        getRenderComponentManager().addComponent(lEntitiesUpdaterName);
        taEntitiesUpdaterName = new TextArea(Vec3f(200, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEntitiesUpdater);
        getRenderComponentManager().addComponent(taEntitiesUpdaterName);
        bCreateEntitiesUpdater = new Button(Vec3f(200, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Create entities updater", 15, *wNewEntitiesUpdater);
        bCreateEntitiesUpdater->addActionListener(this);
        getRenderComponentManager().addComponent(bCreateEntitiesUpdater);
        addWindow(wNewEntitiesUpdater);
        wNewEntitiesUpdater->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wNewEntitiesUpdater);
        //Create animation updater.
        wNewAnimUpdater = new RenderWindow(VideoMode(400, 300), "Create new anim updater", getDevice(), Style::Default, ContextSettings(0, 0, 0, 3, 0));
        Label* lAnimUpdaterName = new Label(*wNewAnimUpdater, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "animation updater name : ", 15);
        getRenderComponentManager().addComponent(lAnimUpdaterName);
        taAnimUpdaterName = new TextArea(Vec3f(200, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewAnimUpdater);
        getRenderComponentManager().addComponent(taAnimUpdaterName);
        bCreateAnimUpdater = new Button(Vec3f(200, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Create anim updater", 15, *wNewAnimUpdater);
        bCreateAnimUpdater->addActionListener(this);
        getRenderComponentManager().addComponent(bCreateAnimUpdater);
        addWindow(wNewAnimUpdater);
        wNewAnimUpdater->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wNewAnimUpdater);
        //Create particle system updater.
        wNewParticleSystemUpdater = new RenderWindow(VideoMode(400, 300), "Create new particle system updater", getDevice(), Style::Default, ContextSettings(0, 0, 0, 3, 0));
        Label* lPartSysUpdName = new Label(*wNewParticleSystemUpdater, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "particle system updater name : ", 15);
        getRenderComponentManager().addComponent(lPartSysUpdName);
        taParticleSystemUpdaterName = new TextArea(Vec3f(200, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewParticleSystemUpdater);
        getRenderComponentManager().addComponent(taParticleSystemUpdaterName);
        bCreateParticleSystemUpdater = new Button(Vec3f(200, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Create ps updater", 15, *wNewParticleSystemUpdater);
        bCreateParticleSystemUpdater->addActionListener(this);
        getRenderComponentManager().addComponent(bCreateParticleSystemUpdater);
        addWindow(wNewParticleSystemUpdater);
        wNewParticleSystemUpdater->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wNewParticleSystemUpdater);
        //Create emitter for particle systems.
        wNewEmitter = new RenderWindow(VideoMode(800, 800), "Create new emitter", getDevice(), Style::Default, ContextSettings(0, 0, 0, 3, 0));
        //Particle system name.
        Label* lParticleSystemName = new Label(*wNewEmitter, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "particle system name : ", 15);
        getRenderComponentManager().addComponent(lParticleSystemName);
        taPSName = new TextArea(Vec3f(200, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taPSName);
        //emission rate.
        Label* lEmmisionRate = new Label(*wNewEmitter, Vec3f(400, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "emission rate : ", 15);
        getRenderComponentManager().addComponent(lEmmisionRate);
        taEmissionRate = new TextArea(Vec3f(600, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taEmissionRate);
        //Life time.
        Label* lMinLifeTime = new Label(*wNewEmitter, Vec3f(0, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "min life time : ", 15);
        getRenderComponentManager().addComponent(lMinLifeTime);
        taMinLifeTime = new TextArea(Vec3f(200, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taMinLifeTime);
        Label* lMaxLifeTime = new Label(*wNewEmitter, Vec3f(400, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "max life time : ", 15);
        getRenderComponentManager().addComponent(lMaxLifeTime);
        taMaxLifeTime = new TextArea(Vec3f(600, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taMaxLifeTime);
        //Particle position type.
        Label* ppType = new Label(*wNewEmitter, Vec3f(0, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "particle pos type : ", 15);
        getRenderComponentManager().addComponent(ppType);
        dpSelectPPType = new DropDownList(*wNewEmitter, Vec3f(200, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Rect", 15);
        dpSelectPPType->addItem("Circle", 15);
        getRenderComponentManager().addComponent(dpSelectPPType);
        //Particle position rect x.
        Label* lrectPosX = new Label(*wNewEmitter, Vec3f(400, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "x pos : ", 15);
        getRenderComponentManager().addComponent(lrectPosX);
        taRCPosX = new TextArea(Vec3f(600, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taRCPosX);
        Label* lrectPosY = new Label(*wNewEmitter, Vec3f(0, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "y pos : ", 15);
        getRenderComponentManager().addComponent(lrectPosY);
        taRCPosY = new TextArea(Vec3f(200, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taRCPosY);
        Label* lrectPosZ = new Label(*wNewEmitter, Vec3f(400, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "z pos : ", 15);
        getRenderComponentManager().addComponent(lrectPosZ);
        taRCPosZ = new TextArea(Vec3f(600, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taRCPosZ);
        //Particle position rect half size.
        Label* lrectHSX = new Label(*wNewEmitter, Vec3f(0, 200, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "half size x : ", 15);
        getRenderComponentManager().addComponent(lrectHSX);
        taRCSizeX = new TextArea(Vec3f(200, 200, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taRCSizeX);
        Label* lrectHSY = new Label(*wNewEmitter, Vec3f(400, 200, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "half size y : ", 15);
        getRenderComponentManager().addComponent(lrectHSY);
        taRCSizeY = new TextArea(Vec3f(600, 200, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taRCSizeY);
        Label* lrectHSZ = new Label(*wNewEmitter, Vec3f(0, 250, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "half size z : ", 15);
        getRenderComponentManager().addComponent(lrectHSZ);
        taRCSizeZ = new TextArea(Vec3f(200, 250, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taRCSizeZ);
        //Deflect direction.
        Label* ldeflDirX = new Label(*wNewEmitter, Vec3f(400, 250, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "defl dir x : ", 15);
        getRenderComponentManager().addComponent(ldeflDirX);
        taDeflX = new TextArea(Vec3f(600, 250, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taDeflX);
        Label* ldeflDirY = new Label(*wNewEmitter, Vec3f(0, 300, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "defl dir y : ", 15);
        getRenderComponentManager().addComponent(ldeflDirY);
        taDeflY = new TextArea(Vec3f(200, 300, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taDeflY);
        Label* ldeflDirZ = new Label(*wNewEmitter, Vec3f(400, 300, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "defl dir z : ", 15);
        getRenderComponentManager().addComponent(ldeflDirZ);
        taDeflZ = new TextArea(Vec3f(600, 300, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taDeflZ);
        //Deflect angle.
        Label* ldeflAngle = new Label(*wNewEmitter, Vec3f(0, 350, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "defl angle : ", 15);
        getRenderComponentManager().addComponent(ldeflAngle);
        taDeflAngle = new TextArea(Vec3f(200, 350, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taDeflAngle);
        //Rotation.
        Label* lRotMin = new Label(*wNewEmitter, Vec3f(400, 350, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "min rotation angle : ", 15);
        getRenderComponentManager().addComponent(lRotMin);
        taRotMin = new TextArea(Vec3f(600, 350, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taRotMin);
        Label* lRotMax = new Label(*wNewEmitter, Vec3f(0, 400, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "max rotation angle : ", 15);
        getRenderComponentManager().addComponent(lRotMax);
        taRotMax = new TextArea(Vec3f(200, 400, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taRotMax);
        //Texture index.
        Label* lTexMin = new Label(*wNewEmitter, Vec3f(400, 400, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "min texture index : ", 15);
        getRenderComponentManager().addComponent(lTexMin);
        taTexIndexMin = new TextArea(Vec3f(600, 400, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taTexIndexMin);
        Label* lTexMax = new Label(*wNewEmitter, Vec3f(0, 450, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "max texture index : ", 15);
        getRenderComponentManager().addComponent(lTexMax);
        taTexIndexMax = new TextArea(Vec3f(200, 450, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taTexIndexMax);
        //Min scale.
        Label* lminScaleX = new Label(*wNewEmitter, Vec3f(400, 450, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "min scale x : ", 15);
        getRenderComponentManager().addComponent(lminScaleX);
        taScaleMinX = new TextArea(Vec3f(600, 450, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taScaleMinX);
        Label* lminScaleY = new Label(*wNewEmitter, Vec3f(0, 500, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "min scale y : ", 15);
        getRenderComponentManager().addComponent(lminScaleY);
        taScaleMinY = new TextArea(Vec3f(200, 500, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taScaleMinY);
        Label* lminScaleZ = new Label(*wNewEmitter, Vec3f(400, 500, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "min scale z : ", 15);
        getRenderComponentManager().addComponent(lminScaleZ);
        taScaleMinZ = new TextArea(Vec3f(600, 500, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taScaleMinZ);
        //Max scale.
        Label* lmaxScaleX = new Label(*wNewEmitter, Vec3f(0, 550, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "max scale x : ", 15);
        getRenderComponentManager().addComponent(lmaxScaleX);
        taScaleMaxX = new TextArea(Vec3f(200, 550, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taScaleMaxX);
        Label* lmaxScaleY = new Label(*wNewEmitter, Vec3f(400, 550, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "max scale y : ", 15);
        getRenderComponentManager().addComponent(lmaxScaleY);
        taScaleMaxY = new TextArea(Vec3f(600, 550, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taScaleMaxY);
        Label* lmaxScaleZ = new Label(*wNewEmitter, Vec3f(0, 600, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "max scale z : ", 15);
        getRenderComponentManager().addComponent(lmaxScaleZ);
        taScaleMaxZ = new TextArea(Vec3f(200, 600, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taScaleMaxZ);
        //Color
        Label* lcolor1 = new Label(*wNewEmitter, Vec3f(400, 600, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "color 1 (r;g;b;a) ", 15);
        getRenderComponentManager().addComponent(lcolor1);
        taColor1 = new TextArea(Vec3f(600, 600, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taColor1);
        Label* lcolor2 = new Label(*wNewEmitter, Vec3f(0, 650, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "color 2 (r;g;b;a) : ", 15);
        getRenderComponentManager().addComponent(lcolor2);
        taColor2 = new TextArea(Vec3f(200, 650, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wNewEmitter);
        getRenderComponentManager().addComponent(taColor2);
        //Button.
        bCreateEmitter = new Button(Vec3f(400, 650, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Create emitter", 15, *wNewEmitter);
        bCreateEmitter->addActionListener(this);
        getRenderComponentManager().addComponent(bCreateEmitter);
        wNewEmitter->setVisible(false);
        addWindow(wNewEmitter);
        getRenderComponentManager().setEventContextActivated(false, *wNewEmitter);
        //Create new window.
        wCreateNewWindow = new RenderWindow(VideoMode(400, 300), "Create new window", getDevice(), Style::Default, ContextSettings(0, 0, 4, 3, 0));
        //Title.
        Label* lWindowTitle = new Label(*wCreateNewWindow, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Title : ", 15);
        getRenderComponentManager().addComponent(lWindowTitle);
        taWindowTitle = new TextArea(Vec3f(200, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wCreateNewWindow);
        getRenderComponentManager().addComponent(taWindowTitle);
        //Position.
        Label* lWindowPosition = new Label(*wCreateNewWindow, Vec3f(0, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Position (x;y)", 15);
        getRenderComponentManager().addComponent(lWindowPosition);
        taWindowPos = new TextArea(Vec3f(200, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wCreateNewWindow);
        getRenderComponentManager().addComponent(taWindowPos);
        //Size.
        Label* lWindowSize = new Label(*wCreateNewWindow, Vec3f(0, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Size (x;y)", 15);
        getRenderComponentManager().addComponent(lWindowSize);
        taWindowSize = new TextArea(Vec3f(200, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wCreateNewWindow);
        getRenderComponentManager().addComponent(taWindowSize);
        //Name.
        Label* lWindowName = new Label(*wCreateNewWindow, Vec3f(0, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Name : ", 15);
        getRenderComponentManager().addComponent(lWindowName);
        taWindowName = new TextArea(Vec3f(200, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wCreateNewWindow);
        getRenderComponentManager().addComponent(taWindowName);
        //Button.
        bCreateWindow = new Button(Vec3f(0, 200, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Create window", 15, *wCreateNewWindow);
        bCreateWindow->addActionListener(this);
        getRenderComponentManager().addComponent(bCreateWindow);
        wCreateNewWindow->setVisible(false);
        addWindow(wCreateNewWindow);
        getRenderComponentManager().setEventContextActivated(false, *wCreateNewWindow);
        //Create new object.
        wCreateNewObject = new RenderWindow(VideoMode(400, 1000), "Create new object", getDevice(), Style::Default, ContextSettings(0, 0, 4, 3, 0));
        Label* objectName = new Label(*wCreateNewObject, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Name : ", 15);
        getRenderComponentManager().addComponent(objectName);
        taObjectName = new TextArea(Vec3f(200, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wCreateNewObject);
        getRenderComponentManager().addComponent(taObjectName);
        Label* selectClass = new Label(*wCreateNewObject, Vec3f(0, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Class : ", 15);
        getRenderComponentManager().addComponent(selectClass);
        dpSelectClass = new DropDownList(*wCreateNewObject, Vec3f(200, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Select class", 15);
        dpSelectClass->setPriority(-3);
        getRenderComponentManager().addComponent(dpSelectClass);
        Command cmdSelectClass(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectClass), FastDelegate<void>(&ODFAEGCreator::onSelectedClassChanged, this, dpSelectClass));
        dpSelectClass->getListener().connect("CLASSCHANGED", cmdSelectClass);
        Command cmdSelectClassDroppedDown(FastDelegate<bool>(&DropDownList::isDroppedDown, dpSelectClass), FastDelegate<void>(&ODFAEGCreator::onDroppedDown, this, dpSelectClass));
        dpSelectClass->getListener().connect("SELECTCLASSDROPPEDDOWN", cmdSelectClassDroppedDown);
        Label* lSelectFunction = new Label(*wCreateNewObject, Vec3f(0, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Function : ", 15);
        getRenderComponentManager().addComponent(lSelectFunction);
        dpSelectFunction = new DropDownList(*wCreateNewObject, Vec3f(200, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Select function : ", 15);
        dpSelectFunction->setPriority(-2);
        getRenderComponentManager().addComponent(dpSelectFunction);
        Command cmdSelectFunction(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectFunction), FastDelegate<void>(&ODFAEGCreator::onSelectedFunctionChanged, this, dpSelectFunction));
        dpSelectFunction->getListener().connect("FUNCTIONCHANGED", cmdSelectFunction);
        Command cmdSelectFunctionDroppedDown(FastDelegate<bool>(&DropDownList::isDroppedDown, dpSelectFunction), FastDelegate<void>(&ODFAEGCreator::onDroppedDown, this, dpSelectFunction));
        dpSelectFunction->getListener().connect("SELECTFUNCTIONDROPPEDDOWN", cmdSelectFunctionDroppedDown);
        dpSelectFunction->setName("FUNCTION");
        Label* lSelectPointerType = new Label(*wCreateNewObject, Vec3f(0, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Pointer type : ", 15);
        getRenderComponentManager().addComponent(lSelectPointerType);
        dpSelectPointerType = new DropDownList(*wCreateNewObject, Vec3f(200, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "No pointer : ", 15);
        getRenderComponentManager().addComponent(dpSelectPointerType);
        Command cmdPointerTypeDroppedDown(FastDelegate<bool>(&DropDownList::isDroppedDown, dpSelectPointerType), FastDelegate<void>(&ODFAEGCreator::onDroppedDown, this, dpSelectPointerType));
        dpSelectPointerType->getListener().connect("POINTERTYPEDROPPEDDOWN", cmdPointerTypeDroppedDown);
        Command cmdPointerTypeValuechanged(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectPointerType), FastDelegate<void>(&ODFAEGCreator::onSelectPointerType, this, dpSelectPointerType));
        dpSelectPointerType->getListener().connect("POINTERTYPEVALUECHANGED", cmdPointerTypeValuechanged);
        bCreateObject = new Button(Vec3f(200, 200, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Apply", 15, *wCreateNewObject);
        bCreateObject->addActionListener(this);
        getRenderComponentManager().addComponent(bCreateObject);
        pObjectsParameters = new Panel(*wCreateNewObject, Vec3f(0, 200, 0), Vec3f(400, 800, 0));
        getRenderComponentManager().addComponent(pObjectsParameters);
        rootObjectParams = std::make_unique<Node>("object params", pObjectsParameters, Vec2f(0.f, 250.f / 1000.f), Vec2f(1.f, 750.f / 1000.f));
        wCreateNewObject->setVisible(false);
        addWindow(wCreateNewObject);
        getRenderComponentManager().setEventContextActivated(false, *wCreateNewObject);
        //Modify object.
        wModifyObject = new RenderWindow(VideoMode(400, 1000), "Modify object", getDevice(), Style::Default, ContextSettings(0, 0, 4, 3, 0));
        Label* lMObjectName = new Label(*wModifyObject, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Object name : ", 15);
        getRenderComponentManager().addComponent(lMObjectName);
        taMObjectName = new TextArea(Vec3f(200, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wModifyObject);
        getRenderComponentManager().addComponent(taMObjectName);
        Label* lMIsPointer = new Label(*wModifyObject, Vec3f(0, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Is pointer ? : ", 15);
        getRenderComponentManager().addComponent(lMIsPointer);
        cbIsPointer = new CheckBox(*wModifyObject, Vec3f(200, 65, 0), Vec3f(10, 10, 0));
        getRenderComponentManager().addComponent(cbIsPointer);
        Label* lSelectMClass = new Label(*wModifyObject, Vec3f(0, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Class : ", 15);
        getRenderComponentManager().addComponent(lSelectMClass);
        dpSelectMClass = new DropDownList(*wModifyObject, Vec3f(200, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Select class : ", 15);
        getRenderComponentManager().addComponent(dpSelectMClass);
        Command cmdSelectMClass(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectMClass), FastDelegate<void>(&ODFAEGCreator::onSelectedMClassChanged, this, dpSelectMClass));
        dpSelectMClass->getListener().connect("MCLASSCHANGED", cmdSelectMClass);
        Command cmdSelectMClassDroppedDown(FastDelegate<bool>(&DropDownList::isDroppedDown, dpSelectMClass), FastDelegate<void>(&ODFAEGCreator::onDroppedDown, this, dpSelectMClass));
        dpSelectMClass->getListener().connect("SELECTMCLASSDROPPEDDOWN", cmdSelectMClassDroppedDown);
        Label* lSelectMFunction = new Label(*wModifyObject, Vec3f(0, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Function : ", 15);
        getRenderComponentManager().addComponent(lSelectMFunction);
        dpSelectMFunction = new DropDownList(*wModifyObject, Vec3f(200, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Select function : ", 15);
        dpSelectMFunction->setPriority(-2);
        getRenderComponentManager().addComponent(dpSelectMFunction);
        Command cmdSelectMFunction(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectMFunction), FastDelegate<void>(&ODFAEGCreator::onSelectedMFunctionChanged, this, dpSelectMFunction));
        dpSelectMFunction->getListener().connect("MFUNCTIONCHANGED", cmdSelectMFunction);
        Command cmdSelectMFunctionDroppedDown(FastDelegate<bool>(&DropDownList::isDroppedDown, dpSelectMFunction), FastDelegate<void>(&ODFAEGCreator::onDroppedDown, this, dpSelectMFunction));
        dpSelectMFunction->getListener().connect("SELECTMFUNCTIONDROPPEDDOWN", cmdSelectMFunctionDroppedDown);
        bModifyObject = new Button(Vec3f(200, 200, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Apply", 15, *wModifyObject);
        bModifyObject->addActionListener(this);
        getRenderComponentManager().addComponent(bModifyObject);
        pMObjectsParameters = new Panel(*wModifyObject, Vec3f(0, 250, 0), Vec3f(400, 800, 0));
        getRenderComponentManager().addComponent(pMObjectsParameters);
        rootMObjectParams = std::make_unique<Node>("m object params", pMObjectsParameters, Vec2f(0.f, 250.f / 1000.f), Vec2f(1.f, 750.f / 1000.f));
        wModifyObject->setVisible(false);
        addWindow(wModifyObject);
        getRenderComponentManager().setEventContextActivated(false, *wModifyObject);
        //Generate terrain.
        wGenerateTerrain = new RenderWindow(VideoMode(400, 800), "Generate terrain", getDevice(), Style::Default, ContextSettings(0, 0, 4, 3, 0));
        Label* lTileWidth = new Label(*wGenerateTerrain, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Tile width : ", 15);
        getRenderComponentManager().addComponent(lTileWidth);
        taTileWidth = new TextArea(Vec3f(200, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerateTerrain);
        getRenderComponentManager().addComponent(taTileWidth);

        Label* lTileHeight = new Label(*wGenerateTerrain, Vec3f(0, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Tile height : ", 15);
        getRenderComponentManager().addComponent(lTileHeight);
        taTileHeight = new TextArea(Vec3f(200, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerateTerrain);
        getRenderComponentManager().addComponent(taTileHeight);

        Label* lZoneXPos = new Label(*wGenerateTerrain, Vec3f(0, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Zone X pos : ", 15);
        getRenderComponentManager().addComponent(lZoneXPos);
        taZoneXPos = new TextArea(Vec3f(200, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerateTerrain);
        getRenderComponentManager().addComponent(taZoneXPos);

        Label* lZoneYPos = new Label(*wGenerateTerrain, Vec3f(0, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Zone y pos : ", 15);
        getRenderComponentManager().addComponent(lZoneYPos);
        taZoneYPos = new TextArea(Vec3f(200, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerateTerrain);
        getRenderComponentManager().addComponent(taZoneYPos);

        Label* lZoneZPos = new Label(*wGenerateTerrain, Vec3f(0, 200, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Zone z pos : ", 15);
        getRenderComponentManager().addComponent(lZoneZPos);
        taZoneZPos = new TextArea(Vec3f(200, 200, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerateTerrain);
        getRenderComponentManager().addComponent(taZoneZPos);

        Label* lZoneWidth = new Label(*wGenerateTerrain, Vec3f(0, 250, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Zone width : ", 15);
        getRenderComponentManager().addComponent(lZoneWidth);
        taZoneWidth = new TextArea(Vec3f(200, 250, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerateTerrain);
        getRenderComponentManager().addComponent(taZoneWidth);

        Label* lZoneHeight = new Label(*wGenerateTerrain, Vec3f(0, 300, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Zone height : ", 15);
        getRenderComponentManager().addComponent(lZoneHeight);
        taZoneHeight = new TextArea(Vec3f(200, 300, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerateTerrain);
        getRenderComponentManager().addComponent(taZoneHeight);

        Label* lZoneDepth = new Label(*wGenerateTerrain, Vec3f(0, 350, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Zone depth : ", 15);
        getRenderComponentManager().addComponent(lZoneDepth);
        taZoneDepth = new TextArea(Vec3f(200, 350, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerateTerrain);
        getRenderComponentManager().addComponent(taZoneDepth);

        bAddTileGround = new Button(Vec3f(0, 450, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Add tile ground", 15, *wGenerateTerrain);
        bAddTileGround->addActionListener(this);
        getRenderComponentManager().addComponent(bAddTileGround);

        dpSelectWallType = new DropDownList(*wGenerateTerrain, Vec3f(0, 500, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "top bottom", 15);
        dpSelectWallType->addItem("right left", 15);
        dpSelectWallType->addItem("bottom right", 15);
        dpSelectWallType->addItem("top left", 15);
        dpSelectWallType->addItem("top right", 15);
        dpSelectWallType->addItem("bottom left", 15);
        Command cmdSelectWallTypeDroppeddown (FastDelegate<bool>(&DropDownList::isDroppedDown, dpSelectWallType), FastDelegate<void>(&ODFAEGCreator::onSelectedWallTypeDroppedDown, this, dpSelectWallType));
        getRenderComponentManager().addComponent(dpSelectWallType);
        dpSelectWallType->getListener().connect("selectWallTypeDroppedDown", cmdSelectWallTypeDroppeddown);
        Command cmdSelectWallTypeNotDroppedDown(FastDelegate<bool>(&DropDownList::isNotDroppedDown, dpSelectWallType), FastDelegate<void>(&ODFAEGCreator::onSelectedWallTypeNotDroppedDown, this, dpSelectWallType));
        dpSelectWallType->getListener().connect("selectWallTypeNotDroppedDown", cmdSelectWallTypeNotDroppedDown);

        bAddWall = new Button(Vec3f(200, 500, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Add wall", 15, *wGenerateTerrain);
        bAddWall->addActionListener(this);
        getRenderComponentManager().addComponent(bAddWall);


        bGenerateTerrain = new Button(Vec3f(0, 550, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Generate terrain", 15, *wGenerateTerrain);
        bGenerateTerrain->addActionListener(this);
        getRenderComponentManager().addComponent(bGenerateTerrain);

        wGenerateTerrain->setVisible(false);
        addWindow(wGenerateTerrain);
        getRenderComponentManager().setEventContextActivated(false, *wGenerateTerrain);

        wGenerate3DTerrain = new RenderWindow(VideoMode(400, 800), "Generate 3D terrain", getDevice(), Style::Default, ContextSettings(0, 0, 4, 3, 0));
        Label* lTileWidth3D = new Label(*wGenerate3DTerrain, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Tile width : ", 15);
        getRenderComponentManager().addComponent(lTileWidth3D);
        taTileWidth3D = new TextArea(Vec3f(200, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerate3DTerrain);
        getRenderComponentManager().addComponent(taTileWidth3D);

        Label* lTileHeight3D = new Label(*wGenerate3DTerrain, Vec3f(0, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Tile height : ", 15);
        getRenderComponentManager().addComponent(lTileHeight3D);
        taTileDepth3D = new TextArea(Vec3f(200, 50, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerate3DTerrain);
        getRenderComponentManager().addComponent(taTileDepth3D);

        Label* lZoneXPos3D = new Label(*wGenerate3DTerrain, Vec3f(0, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Zone X pos : ", 15);
        getRenderComponentManager().addComponent(lZoneXPos3D);
        taZoneXPos3D = new TextArea(Vec3f(200, 100, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerate3DTerrain);
        getRenderComponentManager().addComponent(taZoneXPos3D);

        Label* lZoneYPos3D = new Label(*wGenerate3DTerrain, Vec3f(0, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Zone y pos : ", 15);
        getRenderComponentManager().addComponent(lZoneYPos3D);
        taZoneYPos3D = new TextArea(Vec3f(200, 150, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerate3DTerrain);
        getRenderComponentManager().addComponent(taZoneYPos3D);

        Label* lZoneZPos3D = new Label(*wGenerate3DTerrain, Vec3f(0, 200, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Zone z pos : ", 15);
        getRenderComponentManager().addComponent(lZoneZPos3D);
        taZoneZPos3D = new TextArea(Vec3f(200, 200, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerate3DTerrain);
        getRenderComponentManager().addComponent(taZoneZPos3D);

        Label* lZoneWidth3D = new Label(*wGenerate3DTerrain, Vec3f(0, 250, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Zone width : ", 15);
        getRenderComponentManager().addComponent(lZoneWidth3D);
        taZoneWidth3D = new TextArea(Vec3f(200, 250, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerate3DTerrain);
        getRenderComponentManager().addComponent(taZoneWidth3D);

        Label* lZoneHeight3D = new Label(*wGenerate3DTerrain, Vec3f(0, 300, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Zone height : ", 15);
        getRenderComponentManager().addComponent(lZoneHeight3D);
        taZoneHeight3D = new TextArea(Vec3f(200, 300, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerate3DTerrain);
        getRenderComponentManager().addComponent(taZoneHeight3D);

        Label* lZoneDepth3D = new Label(*wGenerate3DTerrain, Vec3f(0, 350, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Zone depth : ", 15);
        getRenderComponentManager().addComponent(lZoneDepth3D);
        taZoneDepth3D = new TextArea(Vec3f(200, 350, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wGenerate3DTerrain);
        getRenderComponentManager().addComponent(taZoneDepth3D);

        bAddTileGround3D = new Button(Vec3f(0, 450, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Add tile ground", 15, *wGenerate3DTerrain);
        bAddTileGround3D->addActionListener(this);
        getRenderComponentManager().addComponent(bAddTileGround3D);

        dpSelectWallType3D = new DropDownList(*wGenerate3DTerrain, Vec3f(0, 500, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "top bottom", 15);
        dpSelectWallType3D->addItem("right left", 15);
        dpSelectWallType3D->addItem("bottom right", 15);
        dpSelectWallType3D->addItem("top left", 15);
        dpSelectWallType3D->addItem("top right", 15);
        dpSelectWallType3D->addItem("bottom left", 15);
        Command cmdSelectWallTypeDroppeddown3D (FastDelegate<bool>(&DropDownList::isDroppedDown, dpSelectWallType3D), FastDelegate<void>(&ODFAEGCreator::onSelectedWallTypeDroppedDown, this, dpSelectWallType3D));
        getRenderComponentManager().addComponent(dpSelectWallType3D);
        dpSelectWallType->getListener().connect("selectWallTypeDroppedDown3D", cmdSelectWallTypeDroppeddown3D);
        Command cmdSelectWallTypeNotDroppedDown3D(FastDelegate<bool>(&DropDownList::isNotDroppedDown, dpSelectWallType3D), FastDelegate<void>(&ODFAEGCreator::onSelectedWallTypeNotDroppedDown, this, dpSelectWallType3D));
        dpSelectWallType->getListener().connect("selectWallTypeNotDroppedDown3D", cmdSelectWallTypeNotDroppedDown3D);

        bAddWall3D = new Button(Vec3f(200, 500, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Add wall", 15, *wGenerate3DTerrain);
        bAddWall3D->addActionListener(this);
        getRenderComponentManager().addComponent(bAddWall3D);

        bGenerate3DTerrain = new Button(Vec3f(0, 550, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Generate terrain", 15, *wGenerate3DTerrain);
        bGenerate3DTerrain->addActionListener(this);
        getRenderComponentManager().addComponent(bGenerate3DTerrain);

        wGenerate3DTerrain->setVisible(false);
        addWindow(wGenerate3DTerrain);
        getRenderComponentManager().setEventContextActivated(false, *wGenerate3DTerrain);

        //Create scripts window.
        wCreateScript = new RenderWindow(VideoMode(400, 800), "Create scripts", getDevice(), Style::Default, ContextSettings(0, 0, 4, 3, 0));
        Label* lScriptFileName = new Label(*wCreateScript, Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Script file name : ", 15);
        getRenderComponentManager().addComponent(lScriptFileName);
        taScriptFileName = new TextArea(Vec3f(200, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "", *wCreateScript);
        getRenderComponentManager().addComponent(taScriptFileName);
        bCreateScript = new Button(Vec3f(0, 50, 0), Vec3f(400, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Create script file", 15, *wCreateScript);
        bCreateScript->addActionListener(this);
        getRenderComponentManager().addComponent(bCreateScript);
        wCreateScript->setVisible(false);
        addWindow(wCreateScript);
        getRenderComponentManager().setEventContextActivated(false, *wCreateScript);

        //Create panel for project files.
        pProjects = new Panel(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 700, 0), 0);
        rootNode = std::make_unique<Node>("projects", pProjects, Vec2f(0.f, 0.015f), Vec2f(1.f / 6.f, 1.f));
        pProjects->setBorderColor(Color(128, 128, 128));
        pProjects->setBackgroundColor(Color::White);
        pProjects->setBorderThickness(5);
        unsigned int i = 0;


        //system("PAUSE");
        getRenderComponentManager().addComponent(pProjects);
        pScriptsEdit = new Panel(getRenderWindow(), Vec3f(200, 10, 0), Vec3f(800, 700, 0));
        pScriptsEdit->setRelPosition(1.f / 6.f, 0.015f);
        pScriptsEdit->setRelSize(0.60f, 0.75f);
        pScriptsEdit->setBorderColor(Color(128, 128, 128));
        pScriptsEdit->setBackgroundColor(Color::White);
        pScriptsEdit->setBorderThickness(5);
        pScriptsEdit->setName("PSCRIPTEDIT");
        getRenderComponentManager().addComponent(pScriptsEdit);
        pScriptsFiles = new Panel(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 700, 0), 0);
        pScriptsFiles->setBorderColor(Color(128, 128, 128));
        pScriptsFiles->setBackgroundColor(Color::White);
        pScriptsFiles->setBorderThickness(5);
        pScriptsFiles->setRelPosition(5.f / 6.5f, 0.015f);
        pScriptsFiles->setRelSize(1.5f / 6.f, 1.f);
        pScriptsFiles->setName("PSCRIPTFILES");
        getRenderComponentManager().addComponent(pScriptsFiles);
        pComponent = new Panel(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(500, 200, 0), 0);
        pComponent->setBorderColor(Color(128, 128, 128));
        pComponent->setBackgroundColor(Color::White);
        pComponent->setBorderThickness(5);
        pComponent->setRelPosition(1.f / 6.f, 0.75f);
        pComponent->setRelSize(1.f - 1.f / 6.f - 1.39f / 6.f, 0.25f);

        getRenderComponentManager().addComponent(pComponent);
        dpSelectComponent = new DropDownList(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "MAIN WINDOW", 15);
        dpSelectComponent->setRelPosition(0, 0);
        dpSelectComponent->setRelSize(0.3, 0.1);
        dpSelectComponent->setParent(pComponent);
        pComponent->addChild(dpSelectComponent);
        Command cmdSelectComponent(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectComponent), FastDelegate<void>(&ODFAEGCreator::onSelectedComponentChanged, this, dpSelectComponent));
        dpSelectComponent->getListener().connect("SELECTCOMPONENT", cmdSelectComponent);
        taChangeComponentExpression = new TextArea(Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "change component expression", getRenderWindow());
        taChangeComponentExpression->setTextSize(15);
        taChangeComponentExpression->setRelPosition(0.3, 0);
        taChangeComponentExpression->setRelSize(0.7, 0.1);
        taChangeComponentExpression->setParent(pComponent);
        pComponent->addChild(taChangeComponentExpression);
        Label* ltaSelectExp = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Selectable entities types : ", 15);
        ltaSelectExp->setRelPosition(0, 0.1);
        ltaSelectExp->setRelSize(0.3, 0.1);
        ltaSelectExp->setParent(pComponent);
        pComponent->addChild(ltaSelectExp);
        taSelectExpression = new TextArea(Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "*", getRenderWindow());
        taSelectExpression->setTextSize(15);
        taSelectExpression->setRelPosition(0.3, 0.1);
        taSelectExpression->setRelSize(0.7, 0.1);
        taSelectExpression->setParent(pComponent);
        pComponent->addChild(taSelectExpression);
        Command cmdCEChanged(FastDelegate<bool>(&TextArea::isTextChanged, taChangeComponentExpression), FastDelegate<void>(&ODFAEGCreator::onComponentExpressionChanged, this, taChangeComponentExpression));
        taChangeComponentExpression->getListener().connect("CECHANGED", cmdCEChanged);
        Label* lViewPerspective = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "View perspective : ", 15);
        lViewPerspective->setRelPosition(0, 0.2);
        lViewPerspective->setRelSize(0.3, 0.2);
        lViewPerspective->setParent(pComponent);
        pComponent->addChild(lViewPerspective);
        dpSelectViewPerspective = new DropDownList(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Ortho 2D", 15);
        dpSelectViewPerspective->addItem("Persp 3D", 15);
        dpSelectViewPerspective->setRelPosition(0.3, 0.2);
        dpSelectViewPerspective->setRelSize(0.3, 0.2);
        dpSelectViewPerspective->setParent(pComponent);
        Command cmdSelectViewPerspective(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectViewPerspective), FastDelegate<void>(&ODFAEGCreator::onViewPerspectiveChanged, this, dpSelectViewPerspective));
        dpSelectViewPerspective->getListener().connect("VIEWPERSPECTIVECHANGED", cmdSelectViewPerspective);
        pComponent->addChild(dpSelectViewPerspective);
        //Ambient light.
        Label* lAmbientLight = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Ambient light color (r;g;b) : ", 15);
        lAmbientLight->setRelPosition(0, 0.4);
        lAmbientLight->setRelSize(0.2, 0.2);
        lAmbientLight->setParent(pComponent);
        pComponent->addChild(lAmbientLight);


        tabPane = new TabPane(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 700, 0));
        tabPane->setRelPosition(0, 0);
        tabPane->setRelSize(1, 1);
        tabPane->setParent(pScriptsFiles);
        pScriptsFiles->addChild(tabPane);
        pScriptsFiles->setBackgroundColor(Color::Black);
        pInfos = new Panel(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 700, 0), 0);
        pInfos->setBackgroundColor(Color::White);
        pInfos->setName("PINFOS");
        //pInfos->setScissorEnabled(false);
        rootInfosNode = std::make_unique<Node>("Infos", pInfos, Vec2f(0.f, 0.05f), Vec2f(1.f, 1.f - 0.05f));
        tabPane->addTab(pInfos, "Informations", *fm.getResourceByAlias(Fonts::Serif));
        pTransform = new Panel(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 700, 0), 0);
        pTransform->setBackgroundColor(Color::White);
        //pTransform->setScissorEnabled(false);
        rootPropNode = std::make_unique<Node>("Properties", pTransform, Vec2f(0.f, 0.05f), Vec2f(1.f, 1.f - 0.05f));
        tabPane->addTab(pTransform, "Transform", *fm.getResourceByAlias(Fonts::Serif));
        pMaterial = new Panel(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 700, 0), 0);
        pMaterial->setBackgroundColor(Color::White);
        pMaterial->setMoveComponents(false);
        //pMaterial->setScissorEnabled(false);
        rootMaterialNode = std::make_unique<Node>("Material", pMaterial, Vec2f(0.f, 0.05f), Vec2f(1.f - 0.05f, 1.f - 0.06f));
        tabPane->addTab(pMaterial, "Material", *fm.getResourceByAlias(Fonts::Serif));
        pShadows = new Panel(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 700, 0), 0);
        pShadows->setBackgroundColor(Color::White);
        //pShadows->setScissorEnabled(false);
        rootShadowsNode = std::make_unique<Node>("Shadows", pShadows, Vec2f(0.f, 0.05f), Vec2f(1.f, 1.f - 0.05f));
        tabPane->addTab(pShadows, "Shadow", *fm.getResourceByAlias(Fonts::Serif));
        pCollisions = new Panel(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 700, 0), 0);
        pCollisions->setBackgroundColor(Color::White);
        rootCollisionNode = std::make_unique<Node>("Collisions", pCollisions, Vec2f(0.f, 0.05f), Vec2f(1.f, 1.f - 0.05f));
        tabPane->addTab(pCollisions, "Collisions", *fm.getResourceByAlias(Fonts::Serif));
        tScriptEdit = new TextArea(Vec3f(200, 20, 0), Vec3f(790, 650, 0), fm.getResourceByAlias(Fonts::Serif), "", getRenderWindow());
        Action a5 (Action::TEXT_ENTERED);
        Command cmd5(a5, FastDelegate<bool>(&TextArea::hasFocus, tScriptEdit), FastDelegate<void>(&ODFAEGCreator::onTextEntered, this, 'a'));
        getListener().connect("CONTEXTENTERED", cmd5);
        tScriptEdit->setParent(pScriptsEdit);
        tScriptEdit->setRelPosition(0.f, 0.f);
        tScriptEdit->setRelSize(0.9f, 0.9f);
        pScriptsEdit->addChild(tScriptEdit);
        guiSize.x() = getRenderWindow().getSize().x() - pProjects->getSize().x() - pScriptsFiles->getSize().x();
        guiSize.y() = getRenderWindow().getSize().y() - menuBar->getSize().y();
        guiPos.x() = pProjects->getSize().x();
        guiPos.y() = menuBar->getSize().y();
        cursor.setOutlineColor(Color::Red);
        cursor.setOutlineThickness(5);
        cursor.setFillColor(Color::Transparent);
        Action moveCursorAction(Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
        Command moveCursorCommand(moveCursorAction, FastDelegate<void>(&ODFAEGCreator::moveCursor, this, Vec2f(-1, -1)));
        getListener().connect("MoveCursor", moveCursorCommand);
        createWindowsDescriptorsAndPipelines();
    }
    catch (Erreur& erreur) {
        std::cerr << erreur.what() << std::endl;
    }

    //std::cout<<"on init"<<std::endl;
}
/*void ODFAEGCreator::updateScriptText(Shape* shape, const Texture* text) {
    std::map<std::string, std::string>::iterator it;
    it = cppAppliContent.find(minAppliname+".cpp");
    if (it != cppAppliContent.end()) {
        std::string& content = it->second;
        TextureManager<>& tm = cache.resourceManager<Texture, std::string>("TextureManager");
        std::string relativePath = tm.getPathByResource(text);
        unsigned int id = tm.getIdByResource(text);
        if(content.find("text"+conversionUIntString(id)) == std::string::npos) {
            unsigned int pos = content.find("TextureManager<>& tm = cache.resourceManager<Texture, std::string>(\"TextureManager\");");
            std::string subs = content.substr(pos);
            pos += subs.find_first_of('\n') + 1;
            content.insert(pos, "const Texture* text"+conversionUIntString(id)+" = tm.getResourceByAlias(\"+realtivePath+\");\n");
        }
        if (content.find("shape"+conversionUIntString(shape->getId())+"->setTexture") == std::string::npos) {
            unsigned int pos = content.find("shape"+conversionUIntString(shape->getId()));
            std::string subs = content.substr(pos);
            pos += subs.find_first_of('\n') + 1;
            content.insert(pos,"shape"+conversionUIntString(shape->getId())+"->setTexture(text"+conversionUIntString(id)+");\n");
        } else {
            unsigned int pos = content.find("shape"+conversionUIntString(shape->getId())+"->setTexture");
            std::string subs = content.substr(pos);
            unsigned int endpos = subs.find_first_of('\n') + pos + 1;
            content.erase(pos, endpos - pos);
            content.insert(pos,"shape"+conversionUIntString(shape->getId())+"->setTexture(text"+conversionUIntString(id)+");\n");
        }
    }
}
void ODFAEGCreator::updateScriptText(Tile* tile, const Texture* text) {
    std::map<std::string, std::string>::iterator it;
    it = cppAppliContent.find(minAppliname+".cpp");
    if (it != cppAppliContent.end()) {
        std::string& content = it->second;
        TextureManager<>& tm = cache.resourceManager<Texture, std::string>("TextureManager");
        std::string relativePath = tm.getPathByResource(text);
        unsigned int id = tm.getIdByResource(text);
        if(content.find("text"+conversionUIntString(id)) == std::string::npos) {
            unsigned int pos = content.find("TextureManager<>& tm = cache.resourceManager<Texture, std::string>(\"TextureManager\");");
            if (pos != std::string::npos) {
                std::string subs = content.substr(pos);
                pos += subs.find_first_of('\n') + 1;
                content.insert(pos, "const Texture* text"+conversionUIntString(id)+" = tm.getResourceByAlias(\"+realtivePath+\");\n");
            }
        }
        if (content.find("tile"+conversionUIntString(tile->getId())+"->getFace(0)->getMaterial()") == std::string::npos) {
            unsigned int pos = content.find("tile"+conversionUIntString(tile->getId()));
            if (pos != std::string::npos) {
                std::string subs = content.substr(pos);
                pos += subs.find_first_of('\n') + 1;
                content.insert(pos,"tile"+conversionUIntString(tile->getId())+"->getFace(0)->getMaterial().clearTextures();\n"+
                               "tile->getFace(0)->getMaterial().addTexture(text"+conversionUIntString(id)+", IntRect(0, 0,"+
                               conversionIntString(text->getSize().x())+","+conversionIntString(text->getSize().y())+"));\n");
            }
        } else {
            unsigned int pos = content.find("tile"+conversionUIntString(tile->getId())+"->getFace(0).getMaterial().addTexture");
            if (pos != std::string::npos) {
                std::string subs = content.substr(pos);
                unsigned int endpos = subs.find_first_of('\n') + pos + 1;
                content.erase(pos, endpos - pos);
                content.insert(pos,"tile->getFace(0)->getMaterial().addTexture(text"+conversionUIntString(id)+", IntRect(0, 0,"+
                               conversionIntString(text->getSize().x())+","+conversionIntString(text->getSize().y())+"));\n");
            }
        }
    }
}*/
void ODFAEGCreator::onRender(RenderComponentManager *cm) {

}
void ODFAEGCreator::onDisplay(RenderWindow* window) {
    if (window == &getRenderWindow()) {
        for (unsigned int i = 0; i < shapes.size(); i++)
            window->draw(*shapes[i]);
        View defaultView = window->getDefaultView();
        window->setView(selectedComponentView);


        if (isGuiShown) {
            //std::cout<<"get visible tiles : "<<std::endl;
           /* for (unsigned int i = 0; i < selectionBorders.size(); i++) {
                delete selectionBorders[i];
            }
            selectionBorders.clear();*/
            //std::vector<Transformable*> entities = rectSelect.getItems();
            //std::cout<<"create borders"<<std::endl;
            /*for (unsigned int i = 0; i < entities.size(); i++) {
                RectangleShape rect(entities[i]->getSize());
                rect.setPosition(entities[i]->getPosition());
                rect.setFillColor(Color::Transparent);
                rect.setOutlineThickness(5);
                rect.setOutlineColor(Color::Cyan);
                window->draw(rect);*/
                /*if (dynamic_cast<Entity*>(entities[i])) {
                    Entity* border = dynamic_cast<Entity*>(entities[i])->clone();
                    for (unsigned int f = 0; f < border->getNbFaces(); f++) {
                        if (border->getFace(f)->getMaterial().getTexture() != nullptr) {
                            border->getFace(f)->getMaterial().clearTextures();
                            border->getFace(f)->getMaterial().addTexture(nullptr, IntRect(0, 0, 0, 0));
                        }
                        //std::cout<<"get va"<<std::endl;
                        VertexArray& va = border->getFace(f)->getVertexArray();
                        //std::cout<<"change color"<<std::endl;
                        for (unsigned int j = 0; j < va.getVertexCount(); j++) {

                            va[j].color = Color::Cyan;
                        }
                        //std::cout<<"color changed"<<std::endl;
                    }
                    border->setOrigin(border->getSize() * 0.5f);
                    border->setScale(Vec3f(1.1f, 1.1f, 1.1f));
                    selectionBorders.push_back(border);
                }*/
            //}
            //std::cout<<"tiles size : "<<tiles.size()<<std::endl;
            /*glCheck(glEnable(GL_STENCIL_TEST));
            glCheck(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilMask(0xFF);
            glDisable(GL_ALPHA_TEST);
            //std::cout<<"draw transparent entities"<<std::endl;
            for (unsigned int i = 0; i < entities.size(); i++) {
                //std::cout<<"dynamic cast test : "<<entities[i]<<std::endl;
                if (dynamic_cast<Entity*>(entities[i])) {
                    //std::cout<<"dynamic cast : "<<entities[i]<<std::endl;
                    Entity* entity = dynamic_cast<Entity*>(entities[i])->clone();
                    //std::cout<<"make transparent"<<std::endl;
                    makeTransparent(entity);
                    window->draw(*entity);
                    delete entity;
                }
            }
            //std::cout<<"draw borders : "<<selectionBorders.size()<<std::endl;
            glCheck(glStencilFunc(GL_NOTEQUAL, 1, 0xFF));
            glCheck(glStencilMask(0x00));
            for (unsigned int i = 0; i < selectionBorders.size(); i++) {
                window->draw(*selectionBorders[i]);
            }
            //std::cout<<"borders drawn"<<std::endl;
            glCheck(glDisable(GL_STENCIL_TEST));
            glEnable(GL_ALPHA_TEST);*/
            if (tabPane->getSelectedTab() == "Collisions") {
                //window->setView(defaultView);
                BoundingBox view = selectedComponentView.getViewVolume();
                /*Vec3f delta = defaultView.getPosition()-selectedComponentView.getPosition();
                int moveX = (int) delta.x() / (int) (gridWidth) * (int) (gridWidth);
                int moveY = (int) delta.y() / (int) (gridHeight) * (int) (gridHeight);
                if (delta.x() < 0)
                    moveX-=gridWidth;
                if (delta.y() < 0)
                    moveY-=gridHeight;*/
                int x = view.getPosition().x();
                int y = view.getPosition().y();
                int endX = view.getPosition().x() + view.getWidth();
                int endY = view.getPosition().y() + view.getHeight();
                for (int i = x; i <= endX; i+=gridWidth*0.5f) {
                    for (int j = y; j <= endY; j+=gridHeight*0.5f) {
                        Vec3f point(i, j, 0);
                        CellMap* cellMap = getWorld()->getGridCellAt(point);
                        if(cellMap != nullptr) {
                            FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
                            Text text;
                            text.setString(conversionIntString(cellMap->getCoords().x())+","+conversionIntString(cellMap->getCoords().y()));
                            text.setFont(*fm.getResourceByAlias(Fonts::Serif));
                            text.setColor(Color::Red);
                            ConvexShape cellPassable(4);
                            BoundingPolyhedron* bp = cellMap->getCellVolume();
                            std::vector<Vec3f> points = bp->getPoints();
                            //std::cout<<"0 : "<<points[0]<<"1 : "<<points[1]<<"2 : "<<points[2]<<"3 : "<<points[5];
                            cellPassable.setPoint(0, Vec3f(points[0].x(), points[0].y(), points[0].z()));
                            cellPassable.setPoint(1, Vec3f(points[1].x(), points[1].y(), points[1].z()));
                            cellPassable.setPoint(2, Vec3f(points[2].x(), points[2].y(), points[2].z()));
                            cellPassable.setPoint(3, Vec3f(points[3].x(), points[3].y(), points[3].z()));
                            text.setPosition(Vec3f(points[0].x(), points[0].y(), points[0].z()));
                            cellPassable.setOutlineColor(Color::Red);
                            cellPassable.setOutlineThickness(1);
                            if (cellMap->isPassable()) {
                                cellPassable.setFillColor(Color(0, 255, 0, 100));
                            } else {
                                cellPassable.setFillColor(Color(255, 0, 0, 100));
                            }
                            //cellPassable.move(Vec3f(moveX, moveY, 0));
                            window->draw(cellPassable);
                            window->draw(text);
                        }
                    }
                }

                //std::cout<<"draw collision rect!"<<std::endl;
                window->draw(collisionBox);
            }
            window->draw(rotationGuismo);
            window->draw(translationGuismo);
            window->draw(scaleGuismo);
            if (showGrid) {
                BoundingBox view = selectedComponentView.getViewVolume();
                cshapes.clear();

                int x = (int) view.getPosition().x() / gridWidth * gridWidth;
                int y = (int) view.getPosition().y() / gridHeight * gridHeight;
                int endX = (int) (view.getPosition().x() + view.getWidth()) / gridWidth * gridWidth;
                int endY = (int) (view.getPosition().y() + view.getHeight()) / gridHeight * gridHeight;
                for (int i = x; i < endX; i+=gridWidth) {
                    for (int j = y; j < endY; j+=gridHeight) {
                        ConvexShape cshape(4);
                        cshape.setFillColor(Color::Transparent);
                        cshape.setOutlineColor(Color(75, 75, 75));
                        cshape.setOutlineThickness(1.f);
                        Vec3f points[4];
                        points[0] = Vec2f(0, 0);
                        points[1] = Vec2f(gridWidth, 0);
                        points[2] = Vec2f(gridWidth, gridHeight);
                        points[3] = Vec2f(0, gridHeight);
                        for (unsigned int i = 0; i < 8; i++) {

                            if (getWorld()->getCurrentSceneManager() != nullptr) {
                                points[i] = getWorld()->getCurrentSceneManager()->getBaseChangementMatrix().changeOfBase(points[i]);
                            }
                            /*if (i < 4)
                                //std::cout<<"point "<<i<<" : "<<v[i]<<std::endl;*/
                        }
                        for (unsigned int n = 0; n < 4; n++) {
                            points[n] += Vec3f(i, j, 0);
                            cshape.setPoint(n, Vec3f(points[n].x(), points[n].y(), 0));
                        }
                        cshapes.push_back(cshape);
                    }
                }
                for (unsigned int  i = 0; i < cshapes.size(); i++)
                    window->draw(cshapes[i]);
            }
            //std::cout<<"selected tab : "<<tabPane->getSelectedTab()<<std::endl;

        }
        window->setView(defaultView);
        window->draw(cursor);
        if (isSelectingPolyhedron) {
            window->draw(bpPoints);
            window->draw(bpLines);
        }
        if (showRectSelect) {
            window->draw(rectSelect);
        }

    }
}
Vec3f ODFAEGCreator::getGridCellPos(Vec3f pos) {
    BaseChangementMatrix bcm = getWorld()->getBaseChangementMatrix();
    Vec3f c = bcm.unchangeOfBase(pos);
    Vec3f v1, p;
    p.x() = (int) c.x() / gridWidth;
    p.y() = (int) c.y() / gridHeight;
    if (c.x() < 0)
        p.x()--;
    if (c.y() < 0)
        p.y()--;
    v1.x() = p.x() * gridWidth;
    v1.y() = p.y() * gridHeight;
    Vec3f ve1(v1.x(), v1.y(), 0);
    Vec3f ve2(v1.x() + gridWidth, v1.y(), 0);
    Vec3f ve3(v1.x() + gridWidth, v1.y() + gridHeight, 0);
    Vec3f ve4(v1.x(), v1.y() + gridHeight, 0);
    Vec3f p1 = bcm.changeOfBase(ve1);
    Vec3f p2 = bcm.changeOfBase(ve2);
    Vec3f p3 = bcm.changeOfBase(ve3);
    Vec3f p4 = bcm.changeOfBase(ve4);
    std::vector<Vec3f> points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    std::array<std::array<float, 2>, 3> extends = Computer::getExtends(points);
    return Vec3f (extends[0][0], extends[1][0], extends[2][0]);
}
void ODFAEGCreator::onUpdate(RenderWindow* window, IEvent& event) {
    if (&getRenderWindow() == window && event.type == IEvent::TEXT_INPUT_EVENT) {
        getListener().setCommandSlotParams("CONTEXTENTERED", this, static_cast<char>(event.text.unicode));
    }
    if(&getRenderWindow() == window && event.type == IEvent::MOUSE_WHEEL_EVENT && event.mouseWheel.type == IEvent::MOUSE_WHEEL_MOVED && isGuiShown) {
        if (event.mouseWheel.direction > 0) {
            if (selectedObject != nullptr) {
                selectedObject->getRootEntity()->changeHeights(selectionBox, -1.f);
            }
        } else {
            if (selectedObject != nullptr) {
                selectedObject->getRootEntity()->changeHeights(selectionBox, 1.f);
            }
        }
    }
    if(&getRenderWindow() == window && event.type == IEvent::KEYBOARD_EVENT && event.keyboard.type == IEvent::KEY_EVENT_PRESSED && isGuiShown && event.keyboard.control && event.keyboard.code == IKeyboard::R) {
        if (selectedObject != nullptr) {

            rotationGuismo.setCenterSize(selectedObject->getPosition()+selectedObject->getSize()*0.5f, selectedObject->getSize());
            rotationGuismo.setVisible(true);
            translationGuismo.setVisible(false);
            scaleGuismo.setVisible(false);
        }
    }
    if(&getRenderWindow() == window && event.type == IEvent::KEYBOARD_EVENT && event.keyboard.type == IEvent::KEY_EVENT_PRESSED && isGuiShown && event.keyboard.control && event.keyboard.code == IKeyboard::M) {
        if (selectedObject != nullptr) {

            translationGuismo.setCenterSize(selectedObject->getPosition()+selectedObject->getSize()*0.5f, selectedObject->getSize());
            translationGuismo.setVisible(true);
            rotationGuismo.setVisible(false);
            scaleGuismo.setVisible(false);
        }
    }
    if(&getRenderWindow() == window && event.type == IEvent::KEYBOARD_EVENT && event.keyboard.type == IEvent::KEY_EVENT_PRESSED && isGuiShown && event.keyboard.control &&  event.keyboard.code == IKeyboard::S) {
        if (selectedObject != nullptr) {

            scaleGuismo.setCenterSize(selectedObject->getPosition()+selectedObject->getSize()*0.5f, selectedObject->getSize());
            scaleGuismo.setVisible(true);
            rotationGuismo.setVisible(false);
            translationGuismo.setVisible(false);
        }
    }
    if (&getRenderWindow() == window && event.type == IEvent::KEYBOARD_EVENT && event.keyboard.type == IEvent::KEY_EVENT_PRESSED && event.keyboard.code == IKeyboard::Delete) {

        rootPropNode->deleteAllNodes();
        rootMaterialNode->deleteAllNodes();
        rootInfosNode->deleteAllNodes();
        rootShadowsNode->deleteAllNodes();
        rootCollisionNode->deleteAllNodes();
        pTransform->removeAll();
        pMaterial->removeAll();
        pMaterial->removeSprites();
        pInfos->removeAll();
        pShadows->removeAll();
        pCollisions->removeAll();
        pInfos->getListener().removeCommand("CMDPARENTCLICKED");
        pInfos->getListener().removeCommand("DisplayChildren");
        pCollisions->getListener().removeCommand("CMDCVPARENTCLICKED");
        pCollisions->getListener().removeCommand("CMDCHILDRENCV");
            //std::cout<<"detach children selected object : "<<std::endl;


        for (unsigned int i = 0; i < selectedObject->getChildren().size(); i++) {
            selectedObject->getChildren()[i]->setParent(nullptr);
        }
        selectedObject->detachChildren();
        //std::cout<<"children detached"<<std::endl;
        getWorld()->removeEntity(selectedObject);
        //std::cout<<"selected object removed: "<<dynamic_cast<Entity*>(selectedObject)->getType()<<std::endl;
        if (selectedObject->getParent() != nullptr) {
            //std::cout<<"selected object detach child"<<std::endl;
            selectedObject->getParent()->detachChild(selectedObject);
        }
        //std::cout<<"selected object child detached"<<std::endl;
        if (selectedObject->isAnimated()) {
            static_cast<AnimUpdater*>(getWorld()->getTimer(selectedObject->getAnimUpdater()))->removeAnim(selectedObject);
        }
        delete selectedObject;

            //std::cout<<"selected object deleted"<<std::endl;


        std::vector<Entity*> objects = rectSelect.getItems();
        for (unsigned int i = 1; i < objects.size(); i++) {
            //std::cout<<"detach children : "<<i<<std::endl;
            for (unsigned int j = 0; j < objects[i]->getChildren().size(); j++) {
                objects[i]->getChildren()[j]->setParent(nullptr);
            }
            //std::cout<<"remove entity : "<<dynamic_cast<Entity*>(objects[i])->getType()<<std::endl;
            objects[i]->detachChildren();
            //std::cout<<"children detached"<<std::endl;
            getWorld()->removeEntity(objects[i]);
            //std::cout<<"entity removed : "<<objects[i]<<std::endl;
            if (objects[i]->getParent() != nullptr) {
                //std::cout<<"detach child"<<std::endl;
                objects[i]->getParent()->detachChild(objects[i]);
            }
            //std::cout<<"child detached"<<std::endl;
            //std::cout<<"delete selected object : "<<dynamic_cast<Entity*>(objects[i])->getType()<<std::endl;
            if (objects[i]->isAnimated()) {
                static_cast<AnimUpdater*>(getWorld()->getTimer(dynamic_cast<Entity*>(objects[i])->getAnimUpdater()))->removeAnim(dynamic_cast<Entity*>(objects[i]));
            }
            delete objects[i];

        }
        rectSelect.getItems().clear();
        selectedBoundingVolume = nullptr;
        /*for (unsigned int i = 0; i < selectionBorders.size(); i++) {
            delete selectionBorders[i];
        }
        selectionBorders.clear();*/

    }
    if (&getRenderWindow() == window && event.type == IEvent::KEYBOARD_EVENT && event.keyboard.type == IEvent::KEY_EVENT_PRESSED) {
        getListener().setCommandSlotParams("MoveAction", this, static_cast<IKeyboard::Key>(event.keyboard.code));
        if (event.keyboard.control && event.keyboard.code == IKeyboard::V) {
            if (selectedObject != nullptr) {
                Vec3f position;
                if (dpSelectComponent->getSelectedItem() == "MAIN WINDOW") {
                    position = getRenderWindow().mapPixelToCoords(Vec3f(cursor.getPosition().x(), getRenderWindow().getSize().y() - cursor.getPosition().y(), 0))+getRenderWindow().getView().getSize()*0.5f;
                } else {
                    for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
                        if (getRenderComponentManager().getRenderComponent(i) != nullptr && getRenderComponentManager().getRenderComponent(i)->getName() == dpSelectComponent->getSelectedItem()) {
                            position = getRenderComponentManager().getRenderComponent(i)->getFrameBuffer()->mapPixelToCoords(Vec3f(cursor.getPosition().x(), getRenderWindow().getSize().y() - cursor.getPosition().y(), 0))+getRenderWindow().getView().getSize()*0.5f;
                        }
                    }
                }

                selectedObject->setSelected(false);
                Entity* entity = selectedObject->clone();


                //std::cout<<"position : "<<position<<std::endl;
                entity->setPosition(position);

                entity->getTransform().update();
                getWorld()->addEntity(entity);
                selectedObject = entity;
                Entity* selectedEntity = dynamic_cast<Entity*>(selectedObject);
                selectedBoundingVolume = selectedEntity->getCollisionVolume();
                if (selectedEntity->getType() == "E_TILE") {
                    displayTileInfos(selectedEntity);
                } else if (selectedEntity->getType() == "E_BIGTILE") {
                    displayBigtileInfos(selectedEntity);
                } else if (selectedEntity->getType() == "E_WALL") {
                    displayWallInfos(selectedEntity);
                } else if (selectedEntity->getType() == "E_DECOR") {
                    displayDecorInfos(selectedEntity);
                } else if (selectedEntity->getType() == "E_ANIMATION") {
                    displayAnimInfos(selectedEntity);
                } else if (selectedEntity->getType() == "E_PARTICLES") {
                    displayParticleSystemInfos(selectedEntity);
                } else if (selectedEntity->getType() == "E_PONCTUAL_LIGHT") {
                    displayPonctualLightInfos(selectedEntity);
                } else {
                    displayExternalEntityInfo(selectedEntity);
                    std::map<std::string, std::vector<Entity*>>::iterator it = externals.find(selectedEntity->getClassName());
                    it->second.push_back(selectedEntity);
                }

            }

        }
        if (event.keyboard.control && event.keyboard.code == IKeyboard::A) {
            if (getWorld()->getCurrentSceneManager() != nullptr) {
                for (unsigned int i = 0; i < rectSelect.getItems().size(); i++) {
                    rectSelect.getItems()[i]->setSelected(false);
                }
                rectSelect.getItems().clear();
                //std::cout<<"select get visible entities"<<std::endl;
                std::vector<Entity*> entities = getWorld()->getEntities(taSelectExpression->getText());                ;
                for (unsigned int i = 0; i < entities.size(); i++) {
                    //std::cout<<"type : "<<entities[i]->getType()<<std::endl<<"select pos : "<<rectSelect.getSelectionRect().getPosition()<<"select size : "<<rectSelect.getSelectionRect().getSize()<<"globalbounds pos : "<<entities[i]->getGlobalBounds().getPosition()<<"globalbounds size : "<<entities[i]->getGlobalBounds().getSize()<<std::endl;
                    //if (dynamic_cast<Tile*>(entities[i])) {
                        //std::cout<<"add tile : "<<i<<std::endl;

                        entities[i]->setSelected(true);
                        rectSelect.addItem(entities[i]);
                        //std::cout<<"border added"<<std::endl;
                    //}
                }
                if (rectSelect.getItems().size() > 0)
                    selectedObject = rectSelect.getItems()[0];
                Entity* selectedEntity = dynamic_cast<Entity*>(selectedObject);
                selectedBoundingVolume = selectedEntity->getCollisionVolume();
                selectedEntity->setSelected(true);
                if (selectedEntity->getType() == "E_TILE") {
                    displayTileInfos(selectedEntity);
                } else if (selectedEntity->getType() == "E_BIGTILE") {
                    displayBigtileInfos(selectedEntity);
                } else if (selectedEntity->getType() == "E_WALL") {
                    displayWallInfos(selectedEntity);
                } else if (selectedEntity->getType() == "E_DECOR") {
                    displayDecorInfos(selectedEntity);
                } else if (selectedEntity->getType() == "E_ANIMATION") {
                    displayAnimInfos(selectedEntity);
                } else if (selectedEntity->getType() == "E_PARTICLES") {
                    displayParticleSystemInfos(selectedEntity);
                } else if (selectedEntity->getType() == "E_PONCTUAL_LIGHT") {
                    displayPonctualLightInfos(selectedEntity);
                } else {
                    displayExternalEntityInfo(selectedEntity);
                }
            }
        }
    }
    if (event.keyboard.control && event.keyboard.code == IKeyboard::D) {
        if (getWorld()->getCurrentSceneManager() != nullptr) {
            for (unsigned int i = 0; i < rectSelect.getItems().size(); i++) {
                rectSelect.getItems()[i]->setSelected(false);
            }
            rectSelect.getItems().clear();
            selectedBoundingVolume = nullptr;
            selectedObject = nullptr;
        }
    }
    if (&getRenderWindow() == window && event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
        std::ifstream fexist(appliname+"\\sourceCode.cpp");
        if (fexist && openedProjects.size() > 0) {
            std::ofstream file(appliname+"\\sourceCode.cpp");
            file<<pluginSourceCode;
        }
        stop();
    }
    if (window != &getRenderWindow() && event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {
        window->setVisible(false);
        setEventContextActivated(true);
    }
    if (&getRenderWindow() == window && event.type == IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == IEvent::BUTTON_EVENT_PRESSED && event.mouseButton.button == IMouse::Left) {
        Vec2f mousePos (event.mouseButton.x, event.mouseButton.y);
        getListener().setCommandSlotParams("MoveCursor", this, mousePos);
    }
    if (&getRenderWindow() == window && event.type == IEvent::MOUSE_MOTION_EVENT && IMouse::isButtonPressed(IMouse::Left)) {
        Vec2f mousePos (event.mouseMotion.x, event.mouseMotion.y);
        Vec3f halfWSize(getRenderWindow().getView().getSize().x() * 0.5f, getRenderWindow().getView().getSize().y() * 0.5f, 0);
        Vec3f ext(mousePos.x() - getRenderWindow().getView().getSize().x() * 0.5f, mousePos.y() - getRenderWindow().getView().getSize().y() * 0.5f, 0);
        Vec3f orig (mousePos.x() - getRenderWindow().getView().getSize().x() * 0.5f, mousePos.y() - getRenderWindow().getView().getSize().y() * 0.5f, 1);
        if (dpSelectComponent->getSelectedItem() == "MAIN WINDOW") {
            orig = getRenderWindow().mapPixelToCoords(Vec3f(orig.x(), getRenderWindow().getSize().y()-orig.y(), orig.z()))+halfWSize;
            ext = getRenderWindow().mapPixelToCoords(Vec3f(ext.x(), getRenderWindow().getSize().y()-ext.y(), ext.z()))+halfWSize;
        } else {
            for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
                if (getRenderComponentManager().getRenderComponent(i) != nullptr && getRenderComponentManager().getRenderComponent(i)->getName() == dpSelectComponent->getSelectedItem()) {
                    if(getRenderComponentManager().getRenderComponent(i)->getFrameBuffer()->getView().isOrtho()) {
                        orig = getRenderComponentManager().getRenderComponent(i)->getFrameBuffer()->mapPixelToCoords(Vec3f(orig.x(), getRenderWindow().getSize().y() - orig.y(), orig.z()))+halfWSize;
                        ext = getRenderComponentManager().getRenderComponent(i)->getFrameBuffer()->mapPixelToCoords(Vec3f(ext.x(), getRenderWindow().getSize().y() - ext.y(), ext.z()))+halfWSize;
                    } else {
                        orig = getRenderComponentManager().getRenderComponent(i)->getFrameBuffer()->mapPixelToCoords(Vec3f(0, 0, orig.z()));
                        ext += halfWSize;
                        ext = getRenderComponentManager().getRenderComponent(i)->getFrameBuffer()->mapPixelToCoords(Vec3f(ext.x(), getRenderWindow().getSize().y() - ext.y(), ext.z()));
                    }
                }
            }
        }
        Ray ray(orig, ext);
        Vec3f _near;
        if (rotationGuismo.intersectsWhere(_near, ray)) {
            Vec3f secondInters = _near;
            secondInters = rotationGuismo.getCenter() - secondInters;
            float angle = firstInters.getAngleBetween(secondInters, -Vec3f::zAxis);

            rotationGuismo.setAngle(firstInters, Math::toDegrees(angle));
            getWorld()->rotateEntity(selectedObject, Math::toDegrees(angle));
            prevAngle = angle;
        }
        if (isMovingXPos && translationGuismo.intersectsXArrow(ray, _near)) {
            float mx = _near.x() - prevObjectPos.x();
            translationGuismo.move(Vec3f(mx, 0, 0));
            getWorld()->moveEntity(selectedObject, mx, 0, 0);
            prevObjectPos.x() = _near.x();
        }
        if (isMovingYPos && translationGuismo.intersectsYArrow(ray, _near)) {
            float my = _near.y() - prevObjectPos.y();
            translationGuismo.move(Vec3f(0, my, 0));
            getWorld()->moveEntity(selectedObject, my, 0, 0);
            prevObjectPos.y() = _near.y();
        }
        if (isMovingZPos && translationGuismo.intersectsZArrow(ray, _near)) {
            float mz = _near.z() - prevObjectPos.z();
            translationGuismo.move(Vec3f(0, 0, mz));
            getWorld()->moveEntity(selectedObject, 0, 0, mz);
            prevObjectPos.z() = _near.z();
        }
        if (isScalingX && scaleGuismo.intersectsXRect(ray, _near)) {
            float sx = _near.x() / prevObjectScale.x();
            getWorld()->scaleEntity(selectedObject, sx, 1, 1);
            scaleGuismo.setCenterSize(selectedObject->getPosition()+selectedObject->getSize()*0.5f, selectedObject->getSize());
            prevObjectScale.x() = _near.x();
        }
        if (isScalingY && scaleGuismo.intersectsYRect(ray, _near)) {
            float sy = _near.y() / prevObjectScale.y();
            getWorld()->scaleEntity(selectedObject, 1, sy, 1);
            scaleGuismo.setCenterSize(selectedObject->getPosition()+selectedObject->getSize()*0.5f, selectedObject->getSize());
            prevObjectScale.y() = _near.y();
        }
        if (isScalingZ && scaleGuismo.intersectsZRect(ray, _near)) {
            float sz = _near.z() / prevObjectScale.z();
            getWorld()->scaleEntity(selectedObject, 1, 1, sz);
            scaleGuismo.setCenterSize(selectedObject->getPosition()+selectedObject->getSize()*0.5f, selectedObject->getSize());
            prevObjectScale.z() = _near.z();
        }
    }
    if (&getRenderWindow() == window && event.type == IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == IEvent::BUTTON_EVENT_PRESSED && event.mouseButton.button == IMouse::Left) {
        if (tabPane->getSelectedTab() == "Collisions") {
            Vec2f mousePos (event.mouseButton.x, event.mouseButton.y);
            Vec3f pos(mousePos.x(), mousePos.y(), 0);
            if (dpSelectComponent->getSelectedItem() == "MAIN WINDOW") {
                pos = getRenderWindow().mapPixelToCoords(Vec3f(mousePos.x(), getRenderWindow().getSize().y()-mousePos.y(), 0))+getRenderWindow().getView().getSize()*0.5f;
            } else {
                for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
                    if (getRenderComponentManager().getRenderComponent(i) != nullptr && getRenderComponentManager().getRenderComponent(i)->getName() == dpSelectComponent->getSelectedItem()) {
                        pos = getRenderComponentManager().getRenderComponent(i)->getFrameBuffer()->mapPixelToCoords(Vec3f(cursor.getPosition().x(), getRenderWindow().getSize().y() - cursor.getPosition().y(), 0))+getRenderWindow().getView().getSize()*0.5f;
                    }
                }
            }
            /*bool isOk;
            do {
                pos = getRenderWindow().mapPixelToCoords(Vec3f(mousePos.x(), getRenderWindow().getSize().y()-mousePos.y(), 0));
                Vec3f delta = viewPos-getRenderWindow().getView().getPosition();
                pos += delta;
                isOk = true;
                if ((int) delta.x() / gridWidth != 0 || delta.x() < 0) {
                    viewPos.x() = (int) getRenderWindow().getView().getPosition().x() / gridWidth * gridWidth;
                    if (getRenderWindow().getView().getPosition().x() > viewPos.x())
                        viewPos.x() += gridWidth;
                    isOk = false;
                }

                if ((int) delta.y() / gridHeight != 0 || delta.y() < 0) {
                    viewPos.y() = (int) getRenderWindow().getView().getPosition().y() / gridHeight * gridHeight;
                    if (getRenderWindow().getView().getPosition().y() > viewPos.y())
                        viewPos.y() += gridHeight;
                    isOk = false;
                }
            } while (!isOk);*/
            CellMap* cell = getWorld()->getGridCellAt(pos);
            //std::cout<<"pos : "<<getRenderWindow().getView().getPosition()<<"view pos : "<<viewPos<<std::endl;
            if (cell != nullptr) {
                BoundingPolyhedron* bp = cell->getCellVolume();
                //std::cout<<"center cell : "<<cell->getCenter()<<std::endl;
                if (cell->isPassable()) {
                    cell->setPassable(false);
                } else {
                    cell->setPassable(true);
                }
            }
        } else {
            Vec2f mousePos (event.mouseButton.x, event.mouseButton.y);
            Vec3f halfWSize(getRenderWindow().getView().getSize().x() * 0.5f, getRenderWindow().getView().getSize().y() * 0.5f, 0);
            Vec3f ext(mousePos.x()-getRenderWindow().getView().getSize().x() * 0.5f, mousePos.y() - getRenderWindow().getView().getSize().y() * 0.5f, 0);
            Vec3f orig = Vec3f(mousePos.x()-getRenderWindow().getView().getSize().x() * 0.5f, mousePos.y() - getRenderWindow().getView().getSize().y() * 0.5f, 1);
            //std::cout<<"ray window coords : "<<orig<<ext<<std::endl;
            if (dpSelectComponent->getSelectedItem() == "MAIN WINDOW") {
                orig = getRenderWindow().mapPixelToCoords(Vec3f(orig.x(), getRenderWindow().getSize().y()-orig.y(), orig.z()))+halfWSize;
                ext = getRenderWindow().mapPixelToCoords(Vec3f(ext.x(), getRenderWindow().getSize().y()-ext.y(), ext.z()))+halfWSize;
            } else {
                for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
                    if (getRenderComponentManager().getRenderComponent(i) != nullptr && getRenderComponentManager().getRenderComponent(i)->getName() == dpSelectComponent->getSelectedItem()) {
                        if(getRenderComponentManager().getRenderComponent(i)->getFrameBuffer()->getView().isOrtho()) {
                            orig = getRenderComponentManager().getRenderComponent(i)->getFrameBuffer()->mapPixelToCoords(Vec3f(orig.x(), getRenderWindow().getSize().y() - orig.y(), orig.z()))+halfWSize;
                            ext = getRenderComponentManager().getRenderComponent(i)->getFrameBuffer()->mapPixelToCoords(Vec3f(ext.x(), getRenderWindow().getSize().y() - ext.y(), ext.z()))+halfWSize;
                        } else {
                            orig = getRenderComponentManager().getRenderComponent(i)->getFrameBuffer()->mapPixelToCoords(Vec3f(0, 0, orig.z()));
                            ext += halfWSize;
                            ext = getRenderComponentManager().getRenderComponent(i)->getFrameBuffer()->mapPixelToCoords(Vec3f(ext.x(), getRenderWindow().getSize().y() - ext.y(), ext.z()));
                        }
                    }
                }
            }
            Ray ray(orig, ext);
            //std::cout<<"ray : "<<ray.getOrig()<<ray.getExt()<<std::endl;
            Vec3f _near;
            if (rotationGuismo.intersectsWhere(_near, ray)) {
                firstInters = _near;
                firstInters = rotationGuismo.getCenter() - firstInters;
                std::cout<<"first inters : "<<firstInters<<"center : "<<rotationGuismo.getCenter()<<std::endl;
            }
            if (translationGuismo.intersectsXArrow(ray, prevObjectPos)) {
                isMovingXPos = true;
            }
            if (translationGuismo.intersectsYArrow(ray, prevObjectPos)) {
                isMovingYPos = true;
            }
            if (translationGuismo.intersectsZArrow(ray, prevObjectPos)) {
                isMovingZPos = true;
            }
            if (scaleGuismo.intersectsXRect(ray, prevObjectScale)) {
                isScalingX = true;
            }
            if (scaleGuismo.intersectsYRect(ray, prevObjectScale)) {
                isScalingY = true;
            }
            if (scaleGuismo.intersectsZRect(ray, prevObjectScale)) {
                isScalingZ = true;
            }
        }
    }
    if (&getRenderWindow() == window && event.type == IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == IEvent::BUTTON_EVENT_PRESSED && event.mouseButton.button == IMouse::Right) {
        Vec2f mousePos (event.mouseButton.x, event.mouseButton.y);
        if (showRectSelect && !pScriptsFiles->isPointInside(Vec3f(mousePos.x(), mousePos.y(), 0))) {
            if (alignToGrid) {
                if (getWorld()->getCurrentSceneManager() == nullptr) {
                    int x = mousePos.x() - getRenderWindow().getSize().x() * 0.5f;
                    int y = mousePos.y() - getRenderWindow().getSize().y() * 0.5f;
                    Vec3f coordsCaseP = getWorld()->getCoordinatesAt(Vec3f(x, y, 0));
                    Vec3f p = getWorld()->getBaseChangementMatrix().unchangeOfBase(Vec3f(x, y, 0));

                    Vec3f v1;
                    v1.x() = (int) p.x() / gridWidth;
                    v1.y() =  (int) p.y() / gridHeight;
                    if (p.x() < 0)
                        v1.x()--;
                    if (p.y() < 0)
                        v1.y()--;
                    v1.x() *= gridWidth;
                    v1.y() *= gridHeight;
                    v1 = getWorld()->getBaseChangementMatrix().changeOfBase(v1);
                    mousePosition = Vec3f(v1.x(), v1.y(), 0);
                } else {
                    int x = mousePos.x() - getRenderWindow().getSize().x() * 0.5f;
                    int y = mousePos.y() - getRenderWindow().getSize().y() * 0.5f;
                    mousePosition = getGridCellPos(Vec3f(x, y, 0));
                }
            } else {
                int x = mousePos.x()-getRenderWindow().getSize().x() * 0.5f;
                int y = mousePos.y()-getRenderWindow().getSize().y() * 0.5f;
                mousePosition = Vec3f(x, y, 0);
            }
        } else if (pScriptsFiles->isPointInside(Vec3f(mousePos.x(), mousePos.y(), 0))) {
            if (alignToGrid) {

                int x = ((int) mousePos.x() / gridWidth * gridWidth);
                int y = ((int) mousePos.y() / gridHeight * gridHeight);
                int deltaX = 0, deltaX2 = - ((int) pMaterial->getDeltas().x() % gridWidth);
                while (deltaX < pScriptsFiles->getPosition().x()-gridWidth) {
                    deltaX += gridWidth;
                }
                while (deltaX2 < mousePos.x()-gridWidth) {
                    deltaX2 += gridWidth;
                }
                deltaX = pScriptsFiles->getPosition().x() - deltaX + (deltaX2 - x);
                int deltaY = 0, deltaY2 = - ((int) pMaterial->getDeltas().y() % gridHeight);
                while (deltaY < bChooseText->getPosition().y() + bChooseText->getSize().y()+10-gridHeight) {
                    deltaY += gridHeight;
                }
                while (deltaY2 < mousePos.y()-gridHeight) {
                    deltaY2 += gridHeight;
                }
                //std::cout<<"sprite pos, old delta Y, pmat delta y : "<<(bChooseText->getPosition().y() + bChooseText->getSize().y()+10)<<","<<deltaY<<","<<pMaterial->getDeltas().y()<<std::endl;
                deltaY = (bChooseText->getPosition().y() + bChooseText->getSize().y()+10) - deltaY + (deltaY2 - y);

                mousePosition = Vec3f(x+deltaX, y+deltaY, 0);
            } else {
                int x = mousePos.x();
                int y = mousePos.y();
                mousePosition = Vec3f(x, y, 0);
            }
        }
    }
    if (&getRenderWindow() == window && event.type == IEvent::MOUSE_MOTION_EVENT && IMouse::isButtonPressed(IMouse::Right)) {
        Vec2f mousePos (event.mouseMotion.x, event.mouseMotion.y);
        if (showRectSelect && !pScriptsFiles->isPointInside(mousePosition)) {
            if (alignToGrid) {
                if (getWorld()->getCurrentSceneManager() == nullptr) {
                    int x = mousePos.x() - getRenderWindow().getSize().x() * 0.5f;
                    int y = mousePos.y() - getRenderWindow().getSize().y() * 0.5f;
                    Vec3f coordsCaseP = getWorld()->getCoordinatesAt(Vec3f(x, y, 0));
                    Vec3f p = getWorld()->getBaseChangementMatrix().unchangeOfBase(Vec3f(x, y, 0));

                    Vec3f v1;
                    v1.x() = (int) p.x() / gridWidth;
                    v1.y() =  (int) p.y() / gridHeight;
                    if (p.x() < 0)
                        v1.x()--;
                    if (p.y() < 0)
                        v1.y()--;
                    v1.x() *= gridWidth;
                    v1.y() *= gridHeight;
                    v1 = getWorld()->getBaseChangementMatrix().changeOfBase(v1);
                    int width = v1.x() - mousePosition.x();
                    int height = v1.y() - mousePosition.y();
                    if (width > 0 && height > 0)
                        rectSelect.setRect(mousePosition.x(), mousePosition.y(), 0, width, height, getRenderWindow().getView().getDepth());
                } else {
                    int x = mousePos.x() - getRenderWindow().getSize().x() * 0.5f;
                    int y = mousePos.y() - getRenderWindow().getSize().y() * 0.5f;
                    Vec3f pos = getGridCellPos(Vec3f(x, y, 0));
                    int width = pos.x() - mousePosition.x();
                    int height = pos.y() - mousePosition.y();
                    if (width > 0 && height > 0)
                        rectSelect.setRect(mousePosition.x(), mousePosition.y(), 0, width, height, getRenderWindow().getView().getDepth());
                }

            } else {
                int x = mousePos.x()-getRenderWindow().getSize().x() * 0.5f;
                int y = mousePos.y()-getRenderWindow().getSize().y() * 0.5f;
                int width = x - mousePosition.x();
                int height = y - mousePosition.y();
                if (width > 0 && height > 0)
                    rectSelect.setRect(mousePosition.x(), mousePosition.y(), 0,  width, height, getRenderWindow().getView().getDepth());
            }
            //std::cout<<"rect size : "<<rectSelect.getSelectionRect().getSize()<<std::endl;
        } else if (pScriptsFiles->isPointInside(mousePosition)) {
            if (alignToGrid) {
                int x = ((int) mousePos.x() / gridWidth * gridWidth);
                int y = ((int) mousePos.y() / gridHeight * gridHeight);
                int deltaX = 0, deltaX2 = - ((int) pMaterial->getDeltas().x() % gridWidth);
                while (deltaX < pScriptsFiles->getPosition().x()-gridWidth) {
                    deltaX += gridWidth;
                }
                while (deltaX2 < mousePos.x()-gridWidth) {
                    deltaX2 += gridWidth;
                }
                deltaX = pScriptsFiles->getPosition().x() - deltaX + (deltaX2 - x);
                int deltaY = 0, deltaY2 = - ((int) pMaterial->getDeltas().y() % gridHeight);
                while (deltaY < bChooseText->getPosition().y() + bChooseText->getSize().y()+10-gridHeight) {
                    deltaY += gridHeight;
                }
                while (deltaY2 < mousePos.y()-gridHeight) {
                    deltaY2 += gridHeight;
                }
                //std::cout<<"sprite pos, old delta Y, pmat delta y : "<<(bChooseText->getPosition().y() + bChooseText->getSize().y()+10)<<","<<deltaY<<","<<pMaterial->getDeltas().y()<<std::endl;
                deltaY = (bChooseText->getPosition().y() + bChooseText->getSize().y()+10) - deltaY + (deltaY2 - y);

                //std::cout<<"mousePos, y, deltaY, y + delta: "<<mousePos.y()<<","<<y<<","<<deltaY<<","<<(y + deltaY)<<std::endl;

                int width = x + deltaX - mousePosition.x();
                int height = y + deltaY - mousePosition.y();
                if (width > 0 && height > 0 && sTextRect != nullptr) {
                    sTextRect->setPosition(Vec3f(mousePosition.x(), mousePosition.y(), 0));
                    sTextRect->setSize(Vec3f(width, height, 0));
                    //std::cout<<"sTextRect : "<<sTextRect->getSize()<<std::endl;
                }
            } else {
                int x = mousePos.x();
                int y = mousePos.y();
                int width = x - mousePosition.x();
                int height = y - mousePosition.y();
                if (width > 0 && height > 0 && sTextRect != nullptr) {
                    sTextRect->setPosition(Vec3f(mousePosition.x(), mousePosition.y(), 0));
                    sTextRect->setSize(Vec3f(width, height, 0));
                }
            }
        }
        if (!showRectSelect) {
            int relX = (event.mouseMotion.x - oldX) * sensivity;
            int relY = (event.mouseMotion.y - oldY) * sensivity;
            //Rotate the view, (Polar coordinates) but you can also use the lookAt function to look at a point.
            /*View view = getRenderWindow().getView();
            if (!view.isOrtho()) {
                int teta = view.getTeta() - relY;
                int phi = view.getPhi() - relX;
                view.rotate(teta, phi);
                getRenderWindow().setView(view);
            }*/
            for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
                if (getRenderComponentManager().getRenderComponent(i) != nullptr) {
                    View view = getRenderComponentManager().getRenderComponent(i)->getView();
                    if (!view.isOrtho()) {
                        int teta = view.getTeta() - relY;
                        int phi = view.getPhi() - relX;
                        view.rotate(teta, phi);
                        getRenderComponentManager().getRenderComponent(i)->setView(view);
                    }
                    if (getRenderComponentManager().getRenderComponent(i)->getName() == taComponentName->getText()) {
                        selectedComponentView = view;
                    }
                }
            }
        }
    }
    if (&getRenderWindow() == window && event.type == IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == IEvent::BUTTON_EVENT_RELEASED && event.mouseButton.button == IMouse::Right) {
        if (showRectSelect && !pScriptsFiles->isPointInside(mousePosition)) {
            for (unsigned int i = 0; i < rectSelect.getItems().size(); i++) {
               rectSelect.getItems()[i]->setSelected(false);
            }

            rectSelect.getItems().clear();
            BoundingBox box = rectSelect.getSelectionRect();
            std::vector<Vec3f> bbVerts = box.getVertices();
            std::array<Vec3f, 8> obbVerts;
            if (dpSelectComponent->getSelectedItem() == "MAIN WINDOW") {
                for (unsigned int v = 0; v < 8; v++) {
                    if (v < 4)
                        obbVerts[v] = getRenderWindow().mapPixelToCoords(Vec3f(bbVerts[v].x(), getRenderWindow().getSize().y()-bbVerts[v].y(), 1))+Vec3f(getRenderWindow().getView().getSize().x()*0.5f, getRenderWindow().getView().getSize().y()*0.5f, getRenderWindow().getView().getSize().z()*0.5f);
                    else
                        obbVerts[v] = getRenderWindow().mapPixelToCoords(Vec3f(bbVerts[v].x(), getRenderWindow().getSize().y()-bbVerts[v].y(), 0))+Vec3f(getRenderWindow().getView().getSize().x()*0.5f, getRenderWindow().getView().getSize().y()*0.5f, getRenderWindow().getView().getSize().z()*0.5f);
                }
            } else {
                std::string name = dpSelectComponent->getSelectedItem();
                std::vector<Component*> components = getRenderComponentManager().getRenderComponents();
                for (unsigned int i = 0; i < components.size(); i++) {
                    if (name == components[i]->getName() && dynamic_cast<HeavyComponent*>(components[i])) {
                        if(components[i]->getFrameBuffer()->getView().isOrtho()) {
                            for (unsigned int v = 0; v < 8; v++) {
                                if (v < 4)
                                    obbVerts[v] = components[i]->getFrameBuffer()->mapPixelToCoords(Vec3f(bbVerts[v].x(), getRenderWindow().getSize().y()-bbVerts[v].y(), 1))+Vec3f(getRenderWindow().getView().getSize().x()*0.5f, getRenderWindow().getView().getSize().y()*0.5f, getRenderWindow().getView().getSize().z()*0.5f);
                                else
                                    obbVerts[v] = components[i]->getFrameBuffer()->mapPixelToCoords(Vec3f(bbVerts[v].x(), getRenderWindow().getSize().y()-bbVerts[v].y(), 0))+Vec3f(getRenderWindow().getView().getSize().x()*0.5f, getRenderWindow().getView().getSize().y()*0.5f, getRenderWindow().getView().getSize().z()*0.5f);


                            }

                        } else {
                            for (unsigned int v = 0; v < 8; v++) {
                                //std::cout<<"bb verts : "<<bbVerts[v]<<std::endl;
                                bbVerts[v] += getRenderWindow().getView().getSize()*0.5f;
                                if (v < 4)
                                    obbVerts[v] = components[i]->getFrameBuffer()->mapPixelToCoords(Vec3f(bbVerts[v].x(), getRenderWindow().getSize().y()-bbVerts[v].y(), 1));
                                else
                                    obbVerts[v] = components[i]->getFrameBuffer()->mapPixelToCoords(Vec3f(bbVerts[v].x(), getRenderWindow().getSize().y()-bbVerts[v].y(), 0));
                                //std::cout<<"obb verts : "<<obbVerts[v]<<std::endl;
                            }
                            //system("PAUSE");

                        }
                    }
                }
            }
            selectionBox = OrientedBoundingBox(obbVerts[0], obbVerts[1], obbVerts[2], obbVerts[3], obbVerts[4], obbVerts[5], obbVerts[6], obbVerts[7]);
            if (getWorld()->getCurrentSceneManager() != nullptr) {
                //std::cout<<"select get visible entities"<<std::endl;
                //std::cout<<"get entities"<<std::endl;
                std::vector<Entity*> entities = getWorld()->getEntities(taSelectExpression->getText());
                for (unsigned int i = 0; i < entities.size(); i++) {
                    //std::cout<<"type : "<<entities[i]->getType()<<std::endl<<"select pos : "<<rectSelect.getSelectionRect().getPosition()<<"select size : "<<rectSelect.getSelectionRect().getSize()<<"globalbounds pos : "<<entities[i]->getGlobalBounds().getPosition()<<"globalbounds size : "<<entities[i]->getGlobalBounds().getSize()<<std::endl;
                    //std::cout<<rectSelect.getSelectionRect().getPosition()<<rectSelect.getSelectionRect().getSize()<<entities[i]->getGlobalBounds().getPosition()<<entities[i]->getGlobalBounds().getSize();

                    //system("PAUSE");
                    /*std::vector<Vec3f> vertices = entities[i]->getGlobalBounds().getVertices();
                    for (unsigned int i = 0; i < vertices.size(); i++) {
                        std::cout<<"vert : "<<vertices[i]<<std::endl;
                    }
                    system("PAUSE");*/
                    CollisionResultSet::Info infos;
                    if (selectionBox.intersects(entities[i]->getGlobalBounds(), infos)) {
                            std::cout<<"add to selection"<<std::endl;
                        //if (dynamic_cast<Tile*>(entities[i])) {

                            /*for (unsigned int v = 0; v < 8; v++) {
                                std::cout<<"obb vertex : "<<v<<" : "<<selectionBox.getVertices()[v]<<std::endl;
                            }*/


                            rectSelect.addItem(entities[i]);
                            entities[i]->setSelected(true);
                            //std::cout<<"border added"<<std::endl;
                        //}
                    }
                }
                //std::cout<<"visible entities selected"<<std::endl;
            }

            if (rectSelect.getItems().size() > 0)
                selectedObject = rectSelect.getItems()[0];
            if (selectedObject != nullptr) {
                selectedBoundingVolume = selectedObject->getCollisionVolume();
                if (selectedObject->getType() == "E_TILE") {
                    displayTileInfos(selectedObject);
                } else if (selectedObject->getType() == "E_BIGTILE") {
                    displayBigtileInfos(selectedObject);
                } else if (selectedObject->getType() == "E_WALL") {
                    displayWallInfos(selectedObject);
                } else if (selectedObject->getType() == "E_DECOR") {
                    displayDecorInfos(selectedObject);
                } else if (selectedObject->getType() == "E_ANIMATION") {
                    displayAnimInfos(selectedObject);
                } else if (selectedObject->getType() == "E_PARTICLES") {
                    displayParticleSystemInfos(selectedObject);
                } else if (selectedObject->getType() == "E_PONCTUAL_LIGHT") {
                    displayPonctualLightInfos(selectedObject);
                } else {
                    displayExternalEntityInfo(selectedObject);
                }
            }
            if (tabPane->getSelectedTab() == "Collisions") {
                BoundingBox selectRect = rectSelect.getSelectionRect();
                taBoundingBoxColX->setText(conversionFloatString(selectRect.getPosition().x()));
                taBoundingBoxColY->setText(conversionFloatString(selectRect.getPosition().y()));
                taBoundingBoxColZ->setText(conversionFloatString(selectRect.getPosition().z()));
                taBoundingBoxColW->setText(conversionFloatString(selectRect.getSize().x()));
                taBoundingBoxColH->setText(conversionFloatString(selectRect.getSize().y()));
                taBoundingBoxColZ->setText(conversionFloatString(selectRect.getSize().z()));
            }
        }
        if (pScriptsFiles->isPointInside(mousePosition) && sTextRect != nullptr) {
            //std::cout<<"deltas : "<<pMaterial->getDeltas()<<std::endl;
            int x = mousePosition.x() - pScriptsFiles->getPosition().x()+pMaterial->getDeltas().x();
            int y = mousePosition.y() - (bChooseText->getPosition().y() + bChooseText->getSize().y()+10)+pMaterial->getDeltas().y();

            //std::cout<<"mouse pos : "<<mousePosition.x()<<" x : "<<x<<" delta : "<<pMaterial->getDeltas().x()<<std::endl;
            //std::cout<<pMaterial->getDeltas().y()<<std::endl;
            tTexCoordX->setText(conversionIntString(x));
            tTexCoordY->setText(conversionIntString(y));
            tTexCoordW->setText(conversionIntString(sTextRect->getSize().x()));
            tTexCoordH->setText(conversionIntString(sTextRect->getSize().y()));
        }
    }
    if (&getRenderWindow() == window && event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_RESIZED) {
        getRenderWindow().getDefaultView().reset(BoundingBox(0, 0, getRenderWindow().getDefaultView().getPosition().z(), event.window.data1, event.window.data2, getRenderWindow().getDefaultView().getDepth()));
        getRenderWindow().getDefaultView().setPerspective(-event.window.data1 * 0.5f, event.window.data1 * 0.5f, -event.window.data2 * 0.5f, event.window.data2 * 0.5f, getRenderWindow().getDefaultView().getViewport().getPosition().z(), getRenderWindow().getDefaultView().getViewport().getSize().z());
        View view = getRenderWindow().getView();
        view.reset(BoundingBox(0, 0, getRenderWindow().getView().getPosition().z(), event.window.data1, event.window.data2, getRenderWindow().getView().getDepth()));
        view.setPerspective(-event.window.data1 * 0.5f, event.window.data1 * 0.5f, -event.window.data2 * 0.5f, event.window.data2 * 0.5f, getRenderWindow().getView().getViewport().getPosition().z(), getRenderWindow().getView().getViewport().getSize().z());
        getRenderWindow().setView(view);
    }
    if (&getRenderWindow() == window && isSelectingPolyhedron && event.type == IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == IEvent::BUTTON_EVENT_RELEASED && event.mouseButton.button == IMouse::Left) {
        if (isSecondClick) {
            Vec2f mousePos (event.mouseButton.x, event.mouseButton.y);
            int x = IMouse::getPosition(getRenderWindow()).x();
            int y = IMouse::getPosition(getRenderWindow()).y();
            bpPoints.append(Vertex(Vec3f(x - getRenderWindow().getSize().x()*0.5f, y-getRenderWindow().getSize().y()*0.5f, getRenderWindow().getDefaultView().getPosition().z()), Color::Red));
            bpLines.setPrimitiveType(LinesStrip);
            if (bpPoints.getVertexCount() == 2) {
                bpLines.append(Vertex(Vec3f(bpPoints[0].position.x(), bpPoints[0].position.y(), bpPoints[0].position.z()), Color::Red));
                bpLines.append(Vertex(Vec3f(bpPoints[1].position.x(), bpPoints[1].position.y(), bpPoints[1].position.z()), Color::Red));

            } else if (bpPoints.getVertexCount() > 2) {
                unsigned int last = bpPoints.getVertexCount() - 1;
                bpLines.append(Vertex(Vec3f(bpPoints[last].position.x(), bpPoints[last].position.y(), bpPoints[last].position.z()), Color::Red));
            }
            BoundingSphere bs (Vec3f(x, y, 0), 5);
            selectBpPoints.push_back(bs);
            CollisionResultSet::Info info;
            if (selectBpPoints.size() > 2 && selectBpPoints[0].intersects(selectBpPoints[selectBpPoints.size()-1], info)) {
                BoundingPolyhedron bp;
                for (unsigned int i = 0; i < selectBpPoints.size()-2; i++) {
                    bp.addTriangle(getRenderWindow().mapPixelToCoords(Vec3f(selectBpPoints[0].getCenter().x(), getRenderWindow().getSize().y() - selectBpPoints[0].getCenter().y(), selectBpPoints[0].getCenter().z())),
                                   getRenderWindow().mapPixelToCoords(Vec3f(selectBpPoints[i+1].getCenter().x(), getRenderWindow().getSize().y() - selectBpPoints[i+1].getCenter().y(), selectBpPoints[i+1].getCenter().z())),
                                   getRenderWindow().mapPixelToCoords(Vec3f(selectBpPoints[i+2].getCenter().x(), getRenderWindow().getSize().y() - selectBpPoints[i+2].getCenter().y(), selectBpPoints[i+2].getCenter().z())));
                }
                tmpBps.push_back(bp);
                isSelectingPolyhedron = false;
                isSecondClick = false;
                wCreateNewObject->setVisible(true);
                getRenderComponentManager().setEventContextActivated(true, *wCreateNewObject);
                tScriptEdit->setEventContextActivated(false);
                bpLines.clear();
                bpPoints.clear();
                selectBpPoints.clear();
            }
        }
        isSecondClick = true;
    }
    if (&getRenderWindow() == window && event.type == IEvent::MOUSE_BUTTON_EVENT && event.mouseButton.type == IEvent::BUTTON_EVENT_RELEASED && event.mouseButton.button == IMouse::Left) {
        isMovingXPos = isMovingYPos = isMovingZPos = isScalingX = isScalingY = isScalingZ = false;
    }


}
void ODFAEGCreator::onExec() {
    if (IKeyboard::isKeyPressed(IKeyboard::Right)) {
        /*View view = getRenderWindow().getView();
        if (static_cast<Scene*>(getWorld()->getCurrentSceneManager())->getCellDepth() == 0) {

            if (alignToGrid)
                view.move(gridWidth, 0, 0);
            else
                view.move(speed * getClock("LoopTime").getElapsedTime().asSeconds(), 0, 0);
            getRenderWindow().setView(view);
        }*/
        for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
            if (getRenderComponentManager().getRenderComponent(i) != nullptr) {
                View cpntView = getRenderComponentManager().getRenderComponent(i)->getView();
                if (cpntView.isOrtho()) {
                    if (alignToGrid)
                        cpntView.move(gridWidth, 0, 0);
                    else
                        cpntView.move(speed * getClock("LoopTime").getElapsedTime().asSeconds(), 0, 0);
                    getRenderComponentManager().getRenderComponent(i)->setView(cpntView);
                } else {
                    if (alignToGrid)
                        cpntView.move(cpntView.getLeft(), gridWidth);
                    else
                        cpntView.move(cpntView.getLeft(), speed * getClock("LoopTime").getElapsedTime().asSeconds());
                    getRenderComponentManager().getRenderComponent(i)->setView(cpntView);
                }
                if (getRenderComponentManager().getRenderComponent(i)->getName() == taComponentName->getText()) {
                    selectedComponentView = cpntView;
                }
            }
        }
    }
    if (IKeyboard::isKeyPressed(IKeyboard::Left)) {
        /*View view = getRenderWindow().getView();
        if (static_cast<Scene*>(getWorld()->getCurrentSceneManager())->getCellDepth() == 0) {

            if (alignToGrid)
                view.move(-gridWidth, 0, 0);
            else
                view.move(-speed * getClock("LoopTime").getElapsedTime().asSeconds(), 0, 0);
            getRenderWindow().setView(view);
        }*/
        for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
            if (getRenderComponentManager().getRenderComponent(i) != nullptr) {
                View cpntView = getRenderComponentManager().getRenderComponent(i)->getView();
                if (cpntView.isOrtho()) {
                    if (alignToGrid)
                        cpntView.move(-gridWidth, 0, 0);
                    else
                        cpntView.move(-speed * getClock("LoopTime").getElapsedTime().asSeconds(), 0, 0);
                    getRenderComponentManager().getRenderComponent(i)->setView(cpntView);
                } else {
                    if (alignToGrid)
                        cpntView.move(cpntView.getLeft(), -gridWidth);
                    else
                        cpntView.move(cpntView.getLeft(), -speed * getClock("LoopTime").getElapsedTime().asSeconds());
                    getRenderComponentManager().getRenderComponent(i)->setView(cpntView);
                }
                if (getRenderComponentManager().getRenderComponent(i)->getName() == taComponentName->getText()) {
                    selectedComponentView = cpntView;
                }
            }
        }
    }
    if (IKeyboard::isKeyPressed(IKeyboard::Up)) {
        /*View view = getRenderWindow().getView();
        if (static_cast<Scene*>(getWorld()->getCurrentSceneManager())->getCellDepth() == 0) {

            if (alignToGrid)
                view.move(0, -gridHeight, 0);
            else
                view.move(0, -speed * getClock("LoopTime").getElapsedTime().asSeconds(), 0);
            getRenderWindow().setView(view);
        }*/
        for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
            if (getRenderComponentManager().getRenderComponent(i) != nullptr) {
                View cpntView = getRenderComponentManager().getRenderComponent(i)->getView();
                if (cpntView.isOrtho()) {
                    if (alignToGrid)
                        cpntView.move(0, -gridHeight, 0);
                    else
                        cpntView.move(0, -speed * getClock("LoopTime").getElapsedTime().asSeconds(), 0);
                    getRenderComponentManager().getRenderComponent(i)->setView(cpntView);
                } else {
                    if (alignToGrid)
                        cpntView.move(cpntView.getForward(), -gridHeight);
                    else
                        cpntView.move(cpntView.getForward(), -speed * getClock("LoopTime").getElapsedTime().asSeconds());
                    getRenderComponentManager().getRenderComponent(i)->setView(cpntView);
                }
                if (getRenderComponentManager().getRenderComponent(i)->getName() == taComponentName->getText()) {
                    selectedComponentView = cpntView;
                }
            }
        }
    }
    if (IKeyboard::isKeyPressed(IKeyboard::Down)) {
        /*View view = getRenderWindow().getView();
        if (static_cast<Scene*>(getWorld()->getCurrentSceneManager())->getCellDepth() == 0) {

            if (alignToGrid)
                view.move(0, gridHeight, 0);
            else
                view.move(0, speed * getClock("LoopTime").getElapsedTime().asSeconds(), 0);
            getRenderWindow().setView(view);
        }*/
        for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
            if (getRenderComponentManager().getRenderComponent(i) != nullptr) {
                View cpntView = getRenderComponentManager().getRenderComponent(i)->getView();
                if (cpntView.isOrtho()) {
                    if (alignToGrid)
                        cpntView.move(0, gridHeight, 0);
                    else
                        cpntView.move(0, speed * getClock("LoopTime").getElapsedTime().asSeconds(), 0);
                    getRenderComponentManager().getRenderComponent(i)->setView(cpntView);
                } else {
                    if (alignToGrid)
                        cpntView.move(cpntView.getForward(), gridHeight);
                    else
                        cpntView.move(cpntView.getForward(), speed * getClock("LoopTime").getElapsedTime().asSeconds());
                    getRenderComponentManager().getRenderComponent(i)->setView(cpntView);
                }
                if (getRenderComponentManager().getRenderComponent(i)->getName() == taComponentName->getText()) {
                    selectedComponentView = cpntView;
                }
            }
        }
    }
    for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
        if (getRenderComponentManager().getRenderComponent(i) != nullptr && getRenderComponentManager().getRenderComponent(i)->getName() == dpSelectComponent->getSelectedItem()) {
            selectedComponentView = getRenderComponentManager().getRenderComponent(i)->getFrameBuffer()->getView();
        }
    }
    std::map<std::string, std::vector<Entity*>>::iterator it;
    std::map<std::string, std::vector<Entity*>>::iterator it2;
    for (it = toAdd.begin(); it != toAdd.end(); it++) {
        it2 = externals.find(it->first);
        if (it2 == externals.end()) {
            externals.insert(std::make_pair(it->first, std::vector<Entity*>()));
            it2 = externals.find(it->first);
        }
        for (unsigned int i = 0; i < it->second.size(); i++) {
            bool contains = false;
            for (unsigned int j = 0; j < it2->second.size() && !contains; j++) {
                if (it2->second[j]->getExternalObjectName() == it->second[i]->getExternalObjectName()) {
                    contains = true;
                }
            }
            if (!contains) {
                //std::cout<<"clone entity"<<std::endl;
                Entity* entity = it->second[i]->clone();
                //std::cout<<"nb entities type : "<<Entity::getNbEntitiesTypes()<<std::endl;
                /*for (unsigned int j = 0; j < entity->getChildren().size(); j++) {
                    std::cout<<"parent : "<<entity->getChildren()[j]->getParent()<<","<<entity<<std::endl;
                }*/
                //std::cout<<"sname from process : "<<Command::sname<<std::endl;

                entity->setExternal(true);
                getWorld()->addEntity(entity);
                std::vector<Entity*> entities=getWorld()->getChildrenEntities(entity->getType());
                TextureManager<>& tm = cache.resourceManager<Texture, std::string>("TextureManager");
                for (unsigned int e = 0; e < entities.size(); e++) {

                    for (unsigned int f = 0; f < entities[e]->getNbFaces(); f++) {
                        Face* face = entities[e]->getFace(f);
                        std::string alias = face->getMaterial().getTexId();
                        //std::cout<<"alias : "<<alias<<std::endl;
                        if (alias != "") {
                            face->getMaterial().clearTextures();
                            face->getMaterial().addTexture(tm.getResourceByAlias(alias), face->getMaterial().getTexRect());
                            face->getMaterial().setTexId(alias);
                        } else {
                            face->getMaterial().clearTextures();
                            face->getMaterial().addTexture(nullptr, IntRect(0, 0, 0, 0));
                        }
                    }
                }
                it2->second.push_back(entity);
                if (i == it->second.size()-1) {
                    selectedObject=entity;
                    displayTransformInfos(entity);
                    displayEntityInfos(entity);
                }
            }
            delete it->second[i];
        }
    }
    toAdd.clear();
    std::string path = (fdTexturePath != nullptr) ? fdTexturePath->getPathChosen() : "";
    if (path != "") {
        unsigned int lastSlash;
        #if defined(ODFAEG_SYSTEM_LINUX)
        lastSlash = path.find_last_of("/");
        #else if defined (ODFAEG_SYSTEM_WINDOWS)
        lastSlash = path.find_last_of("\\");
        #endif // if
        std::string ImgName = path.substr(lastSlash+1);
        if (tTexId->getText() == "")
            dpSelectTexture->addItem(ImgName,15);
        else
            dpSelectTexture->addItem(tTexId->getText(),15);
        fdTexturePath->setVisible(false);
        fdTexturePath->setEventContextActivated(false);
        TextureManager<>& tm = cache.resourceManager<Texture, std::string>("TextureManager");
        if (!tm.exists(path)) {
            std::tuple<std::reference_wrapper<Device>> rArgs = std::make_tuple(std::ref(getDevice()));
            if (tTexId->getText() == "") {
                tm.fromFileWithAlias(path, ImgName, rArgs);
                textPaths.push_back(ImgName);
            } else {
                tm.fromFileWithAlias(path, tTexId->getText(), rArgs);
                textPaths.push_back(tTexId->getText());
            }

            /*std::map<std::string, std::string>::iterator it;
            it = cppAppliContent.find(minAppliname+".cpp");
            if (it != cppAppliContent.end()) {
                std::string& content = it->second;
                unsigned int pos = content.find("TextureManager<>& tm");
                std::string subs = content.substr(pos);
                pos += subs.find_first_of('\n') + 1;
                content.insert(pos, "    tm.fromFileWithAlias("+path+","+ImgName+");");
            }*/
            fdTexturePath->setEventContextActivated(false);
            tScriptEdit->setEventContextActivated(true);
            for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
                if(getRenderComponentManager().getRenderComponent(i) != nullptr) {
                       //std::cout<<"load texture indexes"<<std::endl;
                       getRenderComponentManager().getRenderComponent(i)->loadTextureIndexes();
                }
            }
        }
    }
    std::string projectPath = (fdProjectPath != nullptr) ? fdProjectPath->getPathChosen() : "";

    if (projectPath != "" && projectPath.find(".poc") != std::string::npos) {
        std::cout<<"open project"<<std::endl;
        FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
        std::ifstream applis(projectPath);
        std::string line;
        bool opened = false;
        if (applis) {
            if(getline(applis, line)) {
                for (unsigned int i = 0; i < openedProjects.size() && !opened; i++) {
                    if (openedProjects[i] == line) {
                        opened = true;
                    }
                }
                if (!opened) {
                    Label* lab = new Label(getRenderWindow(),Vec3f(0,0,0),Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),line, 15);
                    Node* node = new Node("test",lab,Vec2f(0, 0),Vec2f(1.f, 0.025f),rootNode.get());
                    lab->setParent(pProjects);
                    lab->setForegroundColor(Color::Red);
                    lab->setBackgroundColor(Color::White);
                    pProjects->addChild(lab);
                    Action a(Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
                    Command cmd(a, FastDelegate<bool>(&Label::isMouseInside, lab), FastDelegate<void>(&ODFAEGCreator::showProjectsFiles, this, lab));
                    lab->getListener().connect("SHOWPFILES", cmd);
                    Label* labScene = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 35, 0), fm.getResourceByAlias(Fonts::Serif), "Scenes", 15);
                    labScene->setBackgroundColor(Color::White);
                    Node* rSceneNode = new Node ("Scenes", labScene, Vec2f(0.f, 0.f), Vec2f(1.f, 0.025f), node);
                    rootScenesNode = rSceneNode;
                    labScene->setForegroundColor(Color::Red);
                    labScene->setParent(pProjects);
                    pProjects->addChild(labScene);
                    Action a2(Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);

                    FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
                    std::vector<LightComponent*> children = pProjects->getChildren();
                    Label* lHeaders = new Label(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(200,17,0),fm.getResourceByAlias(Fonts::Serif),"headers", 15);
                    Label* lSources = new Label(getRenderWindow(),Vec3f(0, 0,0),Vec3f(200,17,0),fm.getResourceByAlias(Fonts::Serif),"sources", 15);
                    lHeaders->setBackgroundColor(Color::White);
                    lSources->setBackgroundColor(Color::White);
                    lHeaders->setParent(pProjects);
                    Node* hNode = new Node ("headers", lHeaders, Vec2f(0, 0), Vec2f(1.f, 0.025f), node);
                    pProjects->addChild(lHeaders);
                    lSources->setParent(pProjects);
                    Node* sNode = new Node("sources",lSources,Vec2f(0, 0), Vec2f(1.f, 0.025f), node);
                    pProjects->addChild(lSources);
                    lHeaders->setForegroundColor(Color::Green);
                    lSources->setForegroundColor(Color::Green);
                    pProjects->setAutoResized(true);
                    Action a3(Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
                    Command cmd1(a3, FastDelegate<bool>(&Label::isMouseInside, lHeaders), FastDelegate<void>(&ODFAEGCreator::showHeadersFiles, this, lHeaders));
                    lHeaders->getListener().connect("SHOWHFILES", cmd1);
                    Command cmd2(a3, FastDelegate<bool>(&Label::isMouseInside, lSources), FastDelegate<void>(&ODFAEGCreator::showSourcesFiles, this, lSources));
                    lSources->getListener().connect("SHOWSFILES", cmd2);
                    //std::cout<<"label : "<<lab<<std::endl;


                    appliname = line;
                    Command cmd3(a2, FastDelegate<bool>(&Label::isMouseInside, lab), FastDelegate<void>(&ODFAEGCreator::showScenes, this, labScene));
                    lab->getListener().connect("SHOWSCENES"+appliname, cmd3);
                    std::unique_ptr<World> world = std::make_unique<World>();
                    world->projectName = appliname;
                    worlds.push_back(std::move(world));
                    setCurrentWorld(worlds.back().get());
                    openedProjects.push_back(line);
                }
            }
        }
        if (!opened) {
            rtc.setOutputDir(appliname);
            const char* lcars = appliname.c_str();
            char* ucars = new char[appliname.size()];
            for (unsigned int i = 0; i < appliname.length(); i++) {
                ucars[i] = std::tolower(lcars[i]);
            }
            minAppliname = std::string(ucars, appliname.length());
            if (appliname != "") {
                while(getline(applis, line)) {
                    #if defined (ODFAEG_SYSTEM_LINUX)
                    std::string path = fdProjectPath->getAppiDir() + "/" + appliname + "/" + line;
                    #else if defined (ODFAEG_SYSTEM_WINDOWS
                    std::string path = fdProjectPath->getAppiDir() + "\\" + appliname + "\\" + line;
                    #endif // if
                    std::ifstream source (path);
                    std::string fileContent;
                    std::string line2;
                    while(getline(source, line2)) {
                        fileContent += line2+"\n";
                    }
                    source.close();
                    cppAppliContent.insert(std::make_pair(std::make_pair(appliname, line), fileContent));
                }
            }
            applis.close();
            //Load textures.
            std::cout<<"load textures"<<std::endl;
            std::ifstream ifs(appliname+"\\"+"textures.oc");
            TextureManager<>& tm = cache.resourceManager<Texture, std::string>("TextureManager");
            if (ifs) {
                //std::cout<<"textures : "<<std::endl;
                ITextArchive ia(ifs);
                std::vector<std::string> paths;
                ia(paths);
                ia(textPaths);
                for (unsigned int i = 0; i < paths.size(); i++) {
                    std::tuple<std::reference_wrapper<Device>> rArgs = std::make_tuple(std::ref(getDevice()));
                    //std::cout<<"load texture : "<<paths[i]<<std::endl;
                    //std::cout<<"alias : "<<ImgName<<std::endl;
                    tm.fromFileWithAlias(paths[i], textPaths[i], rArgs);
                }
                ifs.close();
            }

            std::ifstream ifs2(appliname+"\\"+"scenes.oc");
            std::cout<<"load scenes"<<std::endl;
            if (ifs2) {
                ITextArchive ia2(ifs2);
                std::vector<Scene*> maps;
                ia2(maps);

                //std::cout<<"is 2D iso matrix ? "<<maps[0]->getBaseChangementMatrix().isIso2DMatrix()<<std::endl;
                for (unsigned int i = 0; i < maps.size(); i++) {
                    //std::cout<<"add map "<<maps[i]->getName()<<std::endl;
                    maps[i]->setRenderComponentManager(&getRenderComponentManager());
                    getWorld()->addSceneManager(maps[i]);
                    Label* lab = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 35, 0), fm.getResourceByAlias(Fonts::Serif), maps[i]->getName(), 15);
                    lab->setBackgroundColor(Color::White);
                    Node* node = new Node(maps[i]->getName(), lab, Vec2f(0.f, 0.f), Vec2f(1.f, 0.025f), rootScenesNode);
                    lab->setForegroundColor(Color::Blue);
                    lab->setParent(pProjects);
                    pProjects->addChild(lab);

                    Action a(Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
                    Command cmd(a, FastDelegate<bool>(&Label::isMouseInside, lab), FastDelegate<void>(&ODFAEGCreator::showScene, this, lab));
                    lab->getListener().connect("SHOWSCENE"+maps[i]->getName(), cmd);

                    getWorld()->setCurrentSceneManager(maps[i]->getName());
                }
                ifs2.close();
            }
            if (getWorld()->getCurrentSceneManager() != nullptr) {
                cshapes.clear();
                for (int i = 0; i < getRenderWindow().getSize().x(); i+=gridWidth*0.5) {
                    for (int j = 0; j < getRenderWindow().getSize().y(); j+=gridHeight*0.5) {
                        Vec3f coordsCaseP = getWorld()->getCoordinatesAt(Vec3f(i, j, 0));
                        Vec3f p = getWorld()->getBaseChangementMatrix().unchangeOfBase(Vec3f(i, j, 0));

                        Vec3f v1;
                        v1.x() = (int) p.x() / gridWidth;
                        v1.y() =  (int) p.y() / gridHeight;
                        if (p.x() < 0)
                            v1.x()--;
                        if (p.y() < 0)
                            v1.y()--;
                        v1.x() *= gridWidth;
                        v1.y() *= gridHeight;
                        Vec3f v[4];
                        v[0] = Vec3f (v1.x(), v1.y(), v1.z());
                        v[1] = Vec3f (v1.x() + gridWidth, v1.y(), v1.z());
                        v[2] = Vec3f (v1.x() + gridWidth, v1.y() + gridHeight, v1.z());
                        v[3] = Vec3f (v1.x(), v1.y() + gridHeight, v1.z());

                        for (unsigned int i = 0; i < 4; i++) {
                            v[i] = getWorld()->getBaseChangementMatrix().changeOfBase(v[i]);
                        }

                        ConvexShape cshape(4);
                        cshape.setFillColor(Color::Transparent);
                        cshape.setOutlineColor(Color(75, 75, 75));
                        cshape.setOutlineThickness(1.f);
                        for (unsigned int n = 0; n < 4; n++) {
                            //std::cout<<"point : "<<v[n]<<std::endl;
                            cshape.setPoint(n, Vec3f(v[n].x(), v[n].y(), 0));
                        }
                        cshapes.push_back(cshape);
                    }
                }
            }

            std::vector<Entity*> entities=getWorld()->getEntities("*");

            for (unsigned int i = 0; i < entities.size(); i++) {
                if (entities[i]->getCollisionVolume() != nullptr)
                    addBoundingVolumes(entities[i]->getCollisionVolume());
                for (unsigned int f = 0; f < entities[i]->getNbFaces(); f++) {
                    Face* face = entities[i]->getFace(f);
                    std::string alias = face->getMaterial().getTexId();


                    if (alias != "") {
                        if (entities[i]->getType() == "E_PARTICLES") {
                            static_cast<ParticleSystem*>(entities[i])->setTexture(*tm.getResourceByAlias(alias));
                        }
                        face->getMaterial().clearTextures();
                        face->getMaterial().addTexture(tm.getResourceByAlias(alias), face->getMaterial().getTexRect());
                        face->getMaterial().setTexId(alias);
                    } else {
                        face->getMaterial().clearTextures();
                        face->getMaterial().addTexture(nullptr, IntRect(0, 0, 0, 0));
                    }
                }
            }
            std::cout<<"load timers"<<std::endl;
            std::ifstream ifs4(appliname+"\\"+"timers.oc");
            unsigned int size;
            if (ifs4) {
                ITextArchive ia4(ifs4);
                ia4(size);
                for (unsigned int i  = 0; i < size; i++) {

                    std::string name;
                    std::string type;
                    ia4(name);
                    ia4(type);
                    if (type == "AnimationUpdater") {
                        std::cout<<"add anim updater"<<std::endl;
                        AnimUpdater* au = new AnimUpdater();
                        au->setName(name);
                        std::vector<Entity*> anims = getWorld()->getRootEntities("E_ANIMATION");
                        for (unsigned int a = 0; a < anims.size(); a++) {
                            if (anims[a]->getAnimUpdater() == name) {
                                au->addAnim(anims[a]);
                            }
                        }
                        getWorld()->addTimer(au);
                    }
                }
                ifs4.close();
            }
            std::cout<<"load components"<<std::endl;
            std::ifstream ifs5(appliname+"\\"+"components.oc");
            if (ifs5) {
                ITextArchive ia5(ifs5);
                ia5(size);
                for (unsigned int i = 0; i < size; i++) {
                    std::string name;
                    std::string type;
                    ia5(name);
                    ia5(type);
                    if (type == "PerPixelLinkedList") {
                        //std::cout<<"load components"<<std::endl;
                        int layer;
                        ia5(layer);
                        std::string expression;
                        ia5(expression);
                        //std::cout<<"layer : "<<layer<<std::endl;
                        PerPixelLinkedListRenderComponent* ppll = new PerPixelLinkedListRenderComponent(getRenderWindow(),layer,expression,ContextSettings(24, 8, 4, 4, 6));
                        ppll->setName(name);
                        getRenderComponentManager().addComponent(ppll);
                        dpSelectComponent->addItem(name, 15);
                        dpSelectComponent->setSelectedItem(name);
                        selectedComponentView = ppll->getFrameBuffer()->getView();
                        std::cout<<"set text : "<<expression<<std::endl;
                        taChangeComponentExpression->setText(expression);
                    }
                    if (type == "Shadow") {
                        //std::cout<<"load components"<<std::endl;
                        int layer;
                        ia5(layer);
                        std::string expression;
                        ia5(expression);
                        //std::cout<<"layer : "<<layer<<std::endl;
                        ShadowRenderComponent* ppll = new ShadowRenderComponent(getRenderWindow(),layer,expression,ContextSettings(0, 0, 4, 4, 6));
                        ppll->setName(name);
                        getRenderComponentManager().addComponent(ppll);
                        dpSelectComponent->addItem(name, 15);
                        dpSelectComponent->setSelectedItem(name);
                        selectedComponentView = ppll->getFrameBuffer()->getView();
                        taChangeComponentExpression->setText(expression);
                    }
                    if (type == "ReflectRefract") {
                        //std::cout<<"load components"<<std::endl;
                        int layer;
                        ia5(layer);
                        std::string expression;
                        ia5(expression);
                        //std::cout<<"layer : "<<layer<<std::endl;
                        ReflectRefractRenderComponent* ppll = new ReflectRefractRenderComponent(getRenderWindow(),layer,expression,ContextSettings(0, 0, 4, 4, 6));
                        ppll->setName(name);
                        getRenderComponentManager().addComponent(ppll);
                        dpSelectComponent->addItem(name, 15);
                        dpSelectComponent->setSelectedItem(name);
                        selectedComponentView = ppll->getFrameBuffer()->getView();
                        taChangeComponentExpression->setText(expression);
                    }
                    if (type == "Light") {
                        //std::cout<<"load components"<<std::endl;
                        int layer;
                        ia5(layer);
                        std::string expression;
                        ia5(expression);
                        //std::cout<<"layer : "<<layer<<std::endl;
                        LightRenderComponent* ppll = new LightRenderComponent(getRenderWindow(),layer,expression,ContextSettings(0, 0, 4, 4, 6));
                        ppll->setName(name);
                        getRenderComponentManager().addComponent(ppll);
                        dpSelectComponent->addItem(name, 15);
                        dpSelectComponent->setSelectedItem(name);
                        selectedComponentView = ppll->getFrameBuffer()->getView();
                        taChangeComponentExpression->setText(expression);
                    }
                }
                getRenderComponentManager().recreateDescriptorsAndPipelines();
                ifs5.close();
            }
            std::cout<<"load workers"<<std::endl;
            std::ifstream ifs6(appliname+"\\"+"workers.oc");
            if (ifs6) {
                ITextArchive ia6(ifs6);
                ia6(size);
                for (unsigned int i = 0; i < size; i++) {
                    std::string name;
                    std::string type;
                    ia6(name);
                    ia6(type);

                    std::cout<<"name : "<<name<<"type : "<<type<<std::endl;
                    if (type == "EntityUpdater") {
                        //std::cout<<"load entities updater"<<std::endl;
                        EntitiesUpdater* eu = new EntitiesUpdater(factory, *getWorld());
                        eu->setName(name);
                        getWorld()->addWorker(eu);
                        //std::cout<<"entities updater added"<<std::endl;
                    }
                    if (type == "ParticleSystemUpdater") {
                        //std::cout<<"add particle systme updater"<<std::endl;
                        ParticleSystemUpdater* psu = new ParticleSystemUpdater();
                        psu->setName(name);
                        std::vector<Entity*> ps = getWorld()->getEntities("E_PARTICLES");
                        for (unsigned int p = 0; p < ps.size(); p++) {

                            if (ps[p]->getPsUpdater() == name) {
                                psu->addParticleSystem(ps[p]);
                            }

                        }
                        getWorld()->addTimer(psu);
                    }
                }
                ifs6.close();
            }
            std::ifstream ifs7(appliname+"\\"+"emitters.oc");
            if (ifs7) {
                ITextArchive ia7(ifs7);
                std::vector<std::string> parameters;
                ia7(parameters);
                int i = 0;
                while (i < parameters.size()) {
                    std::string psName = parameters[i];
                    emitterParams.push_back(parameters[i]);
                    //std::cout<<"i : "<<i<<std::endl<<"size : "<<parameters.size()<<std::endl<<"psname : "<<parameters[i]<<std::endl;
                    i++;
                    UniversalEmitter emitter;
                    emitter.setEmissionRate(conversionStringFloat(parameters[i]));
                    emitterParams.push_back(parameters[i]);
                    //std::cout<<"i : "<<i<<std::endl<<"size : "<<parameters.size()<<std::endl<<"psname : "<<parameters[i]<<std::endl;
                    i++;
                    emitter.setParticleLifetime(Distributions::uniform(seconds(conversionStringFloat(parameters[i])), seconds(conversionStringFloat(parameters[i+1]))));
                    emitterParams.push_back(parameters[i]);
                    emitterParams.push_back(parameters[i+1]);
                    //std::cout<<"i : "<<i<<std::endl<<"size : "<<parameters.size()<<std::endl<<"psname : "<<parameters[i]<<","<<parameters[i+1]<<std::endl;
                    i+=2;
                    std::string type = parameters[i];
                    emitterParams.push_back(parameters[i]);
                    //std::cout<<"i : "<<i<<std::endl<<"size : "<<parameters.size()<<std::endl<<"psname : "<<parameters[i]<<std::endl;
                    i++;
                    if (type == "Rect") {
                        emitter.setParticlePosition(Distributions::rect(Vec3f(conversionStringFloat(parameters[i]), conversionStringFloat(parameters[i+1]), conversionStringFloat(parameters[i+2])),
                                                                       Vec3f(conversionStringFloat(parameters[i+3]), conversionStringFloat(parameters[i+4]), conversionStringFloat(parameters[i+5]))));
                        //std::cout<<"i : "<<i<<std::endl<<"size : "<<parameters.size()<<std::endl<<"psname : "<<parameters[i]<<","<<parameters[i+1]<<","<<parameters[i+2]<<","<<parameters[i+2]<<","<<parameters[i+3]<<","<<parameters[i+4]<<","<<parameters[i+5]<<std::endl;
                        emitterParams.push_back(parameters[i]);
                        emitterParams.push_back(parameters[i+1]);
                        emitterParams.push_back(parameters[i+2]);
                        emitterParams.push_back(parameters[i+3]);
                        emitterParams.push_back(parameters[i+4]);
                        emitterParams.push_back(parameters[i+5]);
                        i += 6;
                    } else {
                        emitter.setParticlePosition(Distributions::circle(Vec3f(conversionStringFloat(parameters[i]), conversionStringFloat(parameters[i+1]), conversionStringFloat(parameters[i+2])),
                                                                         conversionStringFloat(parameters[i+3])));
                        //std::cout<<"i : "<<i<<std::endl<<"size : "<<parameters.size()<<std::endl<<"psname : "<<parameters[i]<<","<<parameters[i+1]<<","<<parameters[i+2]<<","<<parameters[i+2]<<std::endl;
                        emitterParams.push_back(parameters[i]);
                        emitterParams.push_back(parameters[i+1]);
                        emitterParams.push_back(parameters[i+2]);
                        emitterParams.push_back(parameters[i+3]);
                        i += 4;
                    }
                    emitter.setParticleVelocity(Distributions::deflect(Vec3f(conversionStringFloat(parameters[i]), conversionStringFloat(parameters[i+1]), conversionStringFloat(parameters[i+2])), conversionStringFloat(parameters[i+3])));
                    //std::cout<<"i : "<<i<<std::endl<<"size : "<<parameters.size()<<std::endl<<"psname : "<<parameters[i]<<","<<parameters[i+1]<<","<<parameters[i+2]<<","<<parameters[i+3]<<std::endl;
                    emitterParams.push_back(parameters[i]);
                    emitterParams.push_back(parameters[i+1]);
                    emitterParams.push_back(parameters[i+2]);
                    emitterParams.push_back(parameters[i+3]);
                    i += 4;
                    emitter.setParticleRotation(Distributions::uniform(conversionStringFloat(parameters[i]), conversionStringFloat(parameters[i+1])));
                    //std::cout<<"i : "<<i<<std::endl<<"size : "<<parameters.size()<<std::endl<<"psname : "<<parameters[i]<<","<<parameters[i+1]<<std::endl;
                    emitterParams.push_back(parameters[i]);
                    emitterParams.push_back(parameters[i+1]);
                    i += 2;
                    emitter.setParticleTextureIndex(Distributions::uniformui(conversionStringInt(parameters[i]),conversionStringInt(parameters[i+1])));
                    //std::cout<<"i : "<<i<<std::endl<<"size : "<<parameters.size()<<std::endl<<"psname : "<<parameters[i]<<","<<parameters[i+1]<<std::endl;
                    emitterParams.push_back(parameters[i]);
                    emitterParams.push_back(parameters[i+1]);
                    i += 2;
                    emitter.setParticleScale(Distributions::rect(Vec3f(conversionStringFloat(parameters[i]), conversionStringFloat(parameters[i+1]), conversionStringFloat(parameters[i+2])),
                                                                       Vec3f(conversionStringFloat(parameters[i+3]), conversionStringFloat(parameters[i+4]), conversionStringFloat(parameters[i+5]))));
                    //std::cout<<"i : "<<i<<std::endl<<"size : "<<parameters.size()<<std::endl<<"psname : "<<parameters[i]<<","<<parameters[i+1]<<","<<parameters[i+2]<<","<<parameters[i+2]<<","<<parameters[i+3]<<","<<parameters[i+4]<<","<<parameters[i+5]<<std::endl;
                    emitterParams.push_back(parameters[i]);
                    emitterParams.push_back(parameters[i+1]);
                    emitterParams.push_back(parameters[i+2]);
                    emitterParams.push_back(parameters[i+3]);
                    emitterParams.push_back(parameters[i+4]);
                    emitterParams.push_back(parameters[i+5]);
                    i += 6;
                    std::vector<std::string> color1 = split(parameters[i], ";");
                    std::vector<std::string> color2 = split(parameters[i+1], ";");
                    emitterParams.push_back(parameters[i]);
                    emitterParams.push_back(parameters[i+1]);
                    Vec4f c1(conversionStringInt(color1[0]), conversionStringInt(color1[1]), conversionStringInt(color1[2]), conversionStringInt(color1[3]));
                    //std::cout<<"i : "<<i<<std::endl<<"size : "<<parameters.size()<<std::endl<<"psname : "<<parameters[i]<<","<<parameters[i+1]<<","<<parameters[i+2]<<","<<parameters[i+3]<<std::endl;

                    Vec4f c2(conversionStringInt(color2[0]), conversionStringInt(color2[1]), conversionStringInt(color2[2]), conversionStringInt(color2[3]));
                    emitter.setParticleColor(Distributions::color(c1, c2));
                    //std::cout<<"i : "<<i<<std::endl<<"size : "<<parameters.size()<<std::endl<<"psname : "<<parameters[i]<<","<<parameters[i+1]<<","<<parameters[i+2]<<","<<parameters[i+3]<<std::endl;
                    i += 2;
                    Entity* ps = getWorld()->getEntity(psName);
                    if (dynamic_cast<ParticleSystem*>(ps)) {
                        static_cast<ParticleSystem*>(ps)->addEmitter(emitter);
                    }
                }
            }
            if (isGenerating3DTerrain) {
                std::string path = (fdImport3DModel != nullptr) ? fdImport3DModel->getPathChosen() : "";
                if (path != "") {
                    if (isGenerating3DTerrain) {
                        Entity* mesh = loader.loadModel(path, factory);
                        if (dpSelectWallType->getSelectedItem() == "top bottom") {
                            std::cout<<"top bottom"<<std::endl;
                            walls3D[g3d::Wall::TOP_BOTTOM]->setMesh(mesh);
                        }
                        else if (dpSelectWallType->getSelectedItem() == "right left") {
                            std::cout<<"right left"<<std::endl;
                            walls3D[g3d::Wall::RIGHT_LEFT]->setMesh(mesh);
                        } else if (dpSelectWallType->getSelectedItem() == "bottom right") {
                            std::cout<<"bottom right"<<std::endl;
                            walls3D[g3d::Wall::BOTTOM_RIGHT]->setMesh(mesh);
                        } else if (dpSelectWallType->getSelectedItem() == "top left") {
                            std::cout<<"top left"<<std::endl;
                            walls3D[g3d::Wall::TOP_LEFT]->setMesh(mesh);
                        } else if (dpSelectWallType->getSelectedItem() == "top right") {
                            std::cout<<"top right"<<std::endl;
                            walls3D[g3d::Wall::TOP_RIGHT]->setMesh(mesh);
                        } else if (dpSelectWallType->getSelectedItem() == "bottom left") {
                            std::cout<<"bottom left"<<std::endl;
                            walls3D[g3d::Wall::BOTTOM_LEFT]->setMesh(mesh);
                        }
                        g3d::Wall* wall = new g3d::Wall(factory);
                        wall->setMesh(mesh);
                        walls3D.push_back(wall);
                        fdImport3DModel->setVisible(false);
                        isGenerating3DTerrain = false;
                        wGenerate3DTerrain->setVisible(true);
                        getRenderComponentManager().setEventContextActivated(true, *wGenerate3DTerrain);
                        getRenderComponentManager().setEventContextActivated(false, getRenderWindow());
                    }
                }
            }
            std::vector<std::string> classes = Class::getClasses(appliname+"\\Scripts");
            for (unsigned int i = 0; i < classes.size(); i++) {
                Class cl = Class::getClass(classes[i]);
                if (cl.getNamespace() == "") {
                    dpSelectClass->addItem(classes[i], 15);
                    dpSelectMClass->addItem(classes[i], 15);
                } else {
                    dpSelectClass->addItem(cl.getNamespace()+"::"+classes[i], 15);
                    dpSelectMClass->addItem(classes[i], 15);
                }
            }
            std::string startDir = getCurrentPath()+"\\"+appliname+"\\Scripts";
            std::vector<std::string> scriptSourceFiles;
            findFiles(".cpp", scriptSourceFiles, startDir);
            for (unsigned int i = 0; i < scriptSourceFiles.size(); i++) {
                int pos = scriptSourceFiles[i].find(".cpp");
                std::string path = scriptSourceFiles[i];
                path.erase(pos);
                rtc.addSourceFile(path);
                std::ifstream source (scriptSourceFiles[i]);
                std::string fileContent;
                std::string line;
                while(getline(source, line)) {
                    fileContent += line+"\n";
                }
                source.close();
                pos = scriptSourceFiles[i].find_last_of("\\");
                scriptSourceFiles[i].erase(0, pos+1);
                cppAppliContent.insert(std::make_pair(std::make_pair(appliname, scriptSourceFiles[i]), fileContent));
            }
            scriptSourceFiles.clear();
            findFiles(".hpp", scriptSourceFiles, startDir);
            for (unsigned int i = 0; i < scriptSourceFiles.size(); i++) {
                std::ifstream source (scriptSourceFiles[i]);
                std::string fileContent;
                std::string line;
                while(getline(source, line)) {
                    fileContent += line+"\n";
                }
                source.close();
                int pos = scriptSourceFiles[i].find_last_of("\\");
                scriptSourceFiles[i].erase(0, pos+1);
                cppAppliContent.insert(std::make_pair(std::make_pair(appliname, scriptSourceFiles[i]), fileContent));
            }


            std::ifstream file(appliname+"\\sourceCode.cpp");
            if (file) {
                std::string line;
                while (getline(file, line)) {
                    //std::cout<<"add line : "<<line<<std::endl;
                    pluginSourceCode += line + "\n";
                }
                if (pluginSourceCode != "") {
                    //std::cout<<"compile!"<<std::endl;
                    rtc.addSourceFile(appliname+"\\sourceCode");
                    rtc.compile();
                    rtc.run<void>("readObjects", this);
                }
            }
            std::ifstream file9(appliname+"\\otherData.oc");
            if (file9) {
                ITextArchive ia(file9);
                ia(callIds);
                ia(currentId);
                ia(tmpBps);
                ia(currentBp);
            }
        }
        fdProjectPath->setVisible(false);
        fdProjectPath->setEventContextActivated(false);
        tScriptEdit->setEventContextActivated(true);
        /*Entity* model = loader.loadModel("tilesets\\mesh_puddingmill\\puddingmill.obj", factory);
            //model->setPosition(position);
        std::vector<Entity*> entities = getWorld()->getEntities("E_BIGTILE");
        if (entities.size() > 0) {
            Entity* heightMap = entities[0]->getRootEntity();
            float height;
            model->move(Vec3f(0, 0, -10));
            bool isOnHeightMap = heightMap->getHeight(Vec2f(model->getPosition().x(), model->getPosition().z()), height);

            model->move(Vec3f(0, height, 0));
        }
        getWorld()->addEntity(model);*/
    }
    std::string model3DPath = (fdImport3DModel != nullptr) ? fdImport3DModel->getPathChosen() : "";
    if (model3DPath != "") {
        Entity* model = loader.loadModel(model3DPath, factory);
        for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
            if (getRenderComponentManager().getRenderComponent(i) != nullptr) {
                getRenderComponentManager().getRenderComponent(i)->loadTextureIndexes();
            }
        }
        if (selectedComponentView.isOrtho()) {
            Vec3f position = getRenderWindow().mapPixelToCoords(Vec3f(cursor.getPosition().x(), cursor.getPosition().y(), 1), selectedComponentView)+getRenderWindow().getView().getSize() * 0.5f;
            model->setPosition(position);
        } else {
            Vec3f cursorPos = cursor.getPosition() + getRenderWindow().getView().getSize() * 0.5f;
            Vec3f position = getRenderWindow().mapPixelToCoords(Vec3f(cursorPos.x(), cursorPos.y(), 1), selectedComponentView);
            //std::cout<<"position : "<<position<<std::endl;
            model->setPosition(position);
            std::vector<Entity*> entities = getWorld()->getEntities("E_BIGTILE");
            if (entities.size() > 0) {
                Entity* heightMap = entities[0]->getRootEntity();
                float height;
                bool isOnHeightMap = heightMap->getHeight(Vec2f(model->getPosition().x(), model->getPosition().z()), height);
                model->setPosition(Vec3f(model->getPosition().x(), height, model->getPosition().z()));
            }
            selectedObject = model;
            model->setSelected(true);
        }
        getWorld()->addEntity(model);
        fdImport3DModel->setVisible(false);
        fdImport3DModel->setEventContextActivated(false);
    }
    if (getWorld() != nullptr) {
        getWorld()->update();
    }
    oldX = IMouse::getPosition(getRenderWindow()).x();
    oldY = IMouse::getPosition(getRenderWindow()).y();
}
void ODFAEGCreator::showScenes(Label* label) {
    Node* node = rootNode->findNode(label);
    if (node->getNodes().size() > 0 && node->isNodeVisible()) {
        node->hideAllNodes();
    } else if (node->getNodes().size() > 0 && !node->isNodeVisible()) {
        node->showAllNodes();
    }
    rootScenesNode= node;
}
void ODFAEGCreator::showScene(Label* label) {
    Node* node = rootNode->findNode(label);
    Node* parent = node->getParent()->getParent();
    for (unsigned int i = 0; i < worlds.size(); i++) {
        if (worlds[i]->projectName == static_cast<Label*>(parent->getComponent())->getText()) {
            setCurrentWorld(worlds[i].get());
        }
    }
    getWorld()->setCurrentSceneManager(label->getText());
    isGuiShown = true;
    pScriptsEdit->setVisible(false);
    rootScenesNode = node->getParent();
}
void ODFAEGCreator::showProjectsFiles(Label* label) {
    isGuiShown = false;
    pScriptsEdit->setVisible(true);
    Node* node = rootNode->findNode(label);
    if (!node->isNodeVisible()) {
        node->showAllNodes();
    } else {
        node->hideAllNodes();
    }
    for (unsigned int i = 0; i < worlds.size(); i++) {
        if (worlds[i]->projectName == label->getText()) {
            setCurrentWorld(worlds[i].get());
        }
    }
}
void ODFAEGCreator::showHeadersFiles(Label* label) {
    //std::cout<<"show header files"<<std::endl;
    Node* node = rootNode->findNode(label);
    if (node->getNodes().size() == 0) {
        FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
        std::vector<std::string> files;
        std::string cpath = getCurrentPath();
        #if defined (ODFAEG_SYSTEM_LINUX)
        findFiles("hpp", files, cpath+"/"+appliname);
        #else if defined (ODFAEG_SYSTEM_WINDOWS)
        findFiles("hpp", files, cpath+"\\"+appliname);
        #endif // if
        for (unsigned int i = 0; i < files.size(); i++) {
            int pos = files[i].find_last_of("\\");
            std::string path = files[i].substr(pos+1, files[i].size()-pos-1);
            Label* lab = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif), path, 15);
            lab->setParent(pProjects);
            lab->setBackgroundColor(Color::White);
            lab->setForegroundColor(Color::Yellow);
            Node* lnode = new Node("header files", lab, Vec2f(0, 0),Vec2f(1,0.025f),node);
            pProjects->addChild(lab);
            Action a(Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
            Command cmd(a, FastDelegate<bool>(&Label::isMouseInside, lab), FastDelegate<void>(&ODFAEGCreator::showFileContent, this, lab));
            lab->getListener().connect("SHOWHFILECONTENT"+lab->getText(), cmd);
        }
    } else if (!node->isNodeVisible()) {
        node->showAllNodes();
    } else {
        node->hideAllNodes();
    }
}
void ODFAEGCreator::showSourcesFiles(Label* label) {
     Node* node = rootNode->findNode(label);
     if (node->getNodes().size() == 0) {
        FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
        std::vector<std::string> files;
        std::string cpath = getCurrentPath();
        #if defined (ODFAEG_SYSTEM_LINUX)
        findFiles("cpp", files, cpath+"/"+appliname);
        #else if defined (ODFAEG_SYSTEM_WINDOWS)
        findFiles("cpp", files, cpath+"\\"+appliname);
        #endif // if
        for (unsigned int i = 0; i < files.size(); i++) {
            int pos = files[i].find_last_of("\\");
            std::string path = files[i].substr(pos+1, files[i].size()-pos-1);
            Label* lab = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif), path, 15);
            Node* lNode = new Node("source files", lab, Vec2f(0, 0), Vec2f(1.f, 0.025f), node);
            lab->setBackgroundColor(Color::White);
            lab->setForegroundColor(Color::Yellow);
            lab->setParent(pProjects);
            pProjects->addChild(lab);
            Action a(Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
            Command cmd(a, FastDelegate<bool>(&Label::isMouseInside, lab), FastDelegate<void>(&ODFAEGCreator::showFileContent, this, lab));
            lab->getListener().connect("SHOWHFILECONTENT"+lab->getText(), cmd);
        }
    } else if (!node->isNodeVisible()){
        node->showAllNodes();
    } else {
        node->hideAllNodes();
    }
}
void ODFAEGCreator::showFileContent(Label* lab) {
    Node* node = rootNode->findNode(lab);
    Node* parent = node->getParent()->getParent();
    for (unsigned int i = 0; i < worlds.size(); i++) {
        if (worlds[i]->projectName == static_cast<Label*>(parent->getComponent())->getText()) {
            setCurrentWorld(worlds[i].get());
        }
    }
    isGuiShown = false;
    pScriptsEdit->setVisible(true);
    std::map<std::pair<std::string, std::string>, std::string>::iterator it;
    it = cppAppliContent.find(std::make_pair(getWorld()->projectName, lab->getText()));
    if (it != cppAppliContent.end()) {
        tScriptEdit->setTextSize(20);
        tScriptEdit->setText(it->second);
        Vec3f textSize = tScriptEdit->getTextSize();
        if (textSize.x() > tScriptEdit->getSize().x())
            tScriptEdit->setSize(Vec3f(textSize.x(), tScriptEdit->getSize().y(), tScriptEdit->getSize().z()));
        if (textSize.y() > tScriptEdit->getSize().y())
            tScriptEdit->setSize(Vec3f(tScriptEdit->getSize().x(), textSize.y(), tScriptEdit->getSize().z()));
        pScriptsEdit->updateScrolls();
    }
}
void ODFAEGCreator::processKeyHeldDown (IKeyboard::Key key) {

}
void ODFAEGCreator::actionPerformed(Button* button) {
    if (button == bAssignCollisionVolume) {
        if (selectedObject != nullptr) {
            selectedObject->setCollisionVolume(selectedBoundingVolume);
        }
    }
    if (button == bAddTileGround) {
        isGeneratingTerrain = true;
        wGenerateTerrain->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wGenerateTerrain);
        getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        Tile* tile = factory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(120, 60, 0),IntRect(0, 0, 100, 50), factory);
        selectedObject = tile;
        ground.push_back(tile);
        displayTileInfos(tile);
    }
    if (button == bAddTileGround3D) {
        isGenerating3DTerrain = true;
        wGenerate3DTerrain->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wGenerate3DTerrain);
        getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        Tile* tile = factory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(50, 0, 50),IntRect(0, 0, 50, 50), factory);
        selectedObject = tile;
        ground.push_back(tile);
        displayTileInfos(tile);
    }
    if (button == bAddWall3D) {
        isGeneratingTerrain = true;
        wGenerateTerrain->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wGenerateTerrain);
        getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        if (dpSelectWallType->getSelectedItem() == "top bottom") {
            std::cout<<"top bottom"<<std::endl;
            g3d::Wall* wall = factory.make_entity<g3d::Wall>(factory);
            walls3D[g3d::Wall::TOP_BOTTOM] = wall;
        }
        else if (dpSelectWallType->getSelectedItem() == "right left") {
            std::cout<<"right left"<<std::endl;
            g3d::Wall* wall = factory.make_entity<g3d::Wall>(factory);
            walls3D[g3d::Wall::RIGHT_LEFT] = wall;
        } else if (dpSelectWallType->getSelectedItem() == "bottom right") {
            std::cout<<"bottom right"<<std::endl;
            g3d::Wall* wall = factory.make_entity<g3d::Wall>(factory);
            walls3D[g3d::Wall::BOTTOM_RIGHT] = wall;
        } else if (dpSelectWallType->getSelectedItem() == "top left") {
            std::cout<<"top left"<<std::endl;
            g3d::Wall* wall = factory.make_entity<g3d::Wall>(factory);
            walls3D[g3d::Wall::TOP_LEFT] = wall;
        } else if (dpSelectWallType->getSelectedItem() == "top right") {
            std::cout<<"top right"<<std::endl;
            g3d::Wall* wall = factory.make_entity<g3d::Wall>(factory);
            walls3D[g3d::Wall::TOP_RIGHT] = wall;
        } else if (dpSelectWallType->getSelectedItem() == "bottom left") {
            std::cout<<"bottom left"<<std::endl;
            g3d::Wall* wall = factory.make_entity<g3d::Wall>(factory);
            walls3D[g3d::Wall::BOTTOM_LEFT] = wall;
        }
    }
    if (button == bAddWall) {
        isGeneratingTerrain = true;
        wGenerateTerrain->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wGenerate3DTerrain);
        getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
        if (dpSelectWallType->getSelectedItem() == "top bottom") {
            std::cout<<"top bottom"<<std::endl;
            Wall* wall = factory.make_entity<g2d::Wall>(factory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(100, 100, 0), IntRect(100, 0, 100, 100), factory),g2d::Wall::TOP_BOTTOM,&g2d::AmbientLight::getAmbientLight(), factory);
            walls[g2d::Wall::TOP_BOTTOM] = wall;
            selectedObject = wall->getChildren()[0];
            displayTileInfos(wall->getChildren()[0]);
        }
        else if (dpSelectWallType->getSelectedItem() == "right left") {
            std::cout<<"right left"<<std::endl;
            Wall* wall = factory.make_entity<g2d::Wall>(factory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(100, 100, 0), IntRect(100, 100, 100, 100), factory),g2d::Wall::RIGHT_LEFT,&g2d::AmbientLight::getAmbientLight(), factory);
            walls[g2d::Wall::RIGHT_LEFT] = wall;
            selectedObject = wall->getChildren()[0];
            displayTileInfos(wall->getChildren()[0]);
        } else if (dpSelectWallType->getSelectedItem() == "bottom right") {
            std::cout<<"bottom right"<<std::endl;
            Wall* wall = factory.make_entity<g2d::Wall>(factory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(100, 100, 0), IntRect(100, 200, 100, 100), factory),g2d::Wall::BOTTOM_RIGHT,&g2d::AmbientLight::getAmbientLight(), factory);
            walls[g2d::Wall::BOTTOM_RIGHT] = wall;
            selectedObject = wall->getChildren()[0];
            displayTileInfos(wall->getChildren()[0]);
        } else if (dpSelectWallType->getSelectedItem() == "top left") {
            std::cout<<"top left"<<std::endl;
            Wall* wall = factory.make_entity<g2d::Wall>(factory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(100, 100, 0), IntRect(100, 300, 100, 100), factory),g2d::Wall::TOP_LEFT,&g2d::AmbientLight::getAmbientLight(), factory);
            walls[g2d::Wall::TOP_LEFT] = wall;
            selectedObject = wall->getChildren()[0];
            displayTileInfos(wall->getChildren()[0]);
        } else if (dpSelectWallType->getSelectedItem() == "top right") {
            std::cout<<"top right"<<std::endl;
            Wall* wall = factory.make_entity<g2d::Wall>(factory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(100, 100, 0), IntRect(100, 400, 100, 100), factory),g2d::Wall::TOP_RIGHT,&g2d::AmbientLight::getAmbientLight(), factory);
            walls[g2d::Wall::TOP_RIGHT] = wall;
            selectedObject = wall->getChildren()[0];
            displayTileInfos(wall->getChildren()[0]);
        } else if (dpSelectWallType->getSelectedItem() == "bottom left") {
            std::cout<<"bottom left"<<std::endl;
            Wall* wall = factory.make_entity<g2d::Wall>(factory.make_entity<Tile>(nullptr, Vec3f(0, 0, 0), Vec3f(100, 100, 0), IntRect(100, 500, 100, 100), factory),g2d::Wall::BOTTOM_LEFT,&g2d::AmbientLight::getAmbientLight(), factory);
            walls[g2d::Wall::BOTTOM_LEFT] = wall;
            selectedObject = wall->getChildren()[0];
            displayTileInfos(wall->getChildren()[0]);
        }
        fdImport3DModel->setVisible(true);
    }
    if (button == bSetTexRect) {
        if (isGeneratingTerrain) {
            isGeneratingTerrain = false;
            wGenerateTerrain->setVisible(true);
            getRenderComponentManager().setEventContextActivated(true, *wGenerateTerrain);
            getRenderComponentManager().setEventContextActivated(false, getRenderWindow());
        } else if (isGenerating3DTerrain) {
            isGenerating3DTerrain = false;
            wGenerate3DTerrain->setVisible(true);
            getRenderComponentManager().setEventContextActivated(true, *wGenerate3DTerrain);
            getRenderComponentManager().setEventContextActivated(false, getRenderWindow());
        }
    }
    if (button->getText() == "Select Area") {
        isSelectingPolyhedron = true;
        wCreateNewObject->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wCreateNewObject);
        tScriptEdit->setEventContextActivated(true);
    }
    if (button->getText() == "Create") {
        appliname = ta->getText();
        applitype = dpList->getSelectedItem();
        #if defined (ODFAEG_SYSTEM_LINUX)
        path = fdProjectPath->getAppiDir() + "/" + appliname;
        #else if defined (ODFAEG_SYSTEM_WINDOWS)
        path = fdProjectPath->getAppiDir() + "\\" + appliname;
        #endif // if

        const char* lcars = appliname.c_str();
        char* ucars = new char[appliname.size()];
        for (unsigned int i = 0; i < appliname.length(); i++) {
            ucars[i] = std::toupper(lcars[i]);
        }
        std::string majAppliname (ucars, appliname.length());
        delete ucars;
        ucars = new char[appliname.size()];
        for (unsigned int i = 0; i < appliname.length(); i++) {
            ucars[i] = std::tolower(lcars[i]);
        }
        minAppliname = std::string(ucars, appliname.length());
        if(_mkdir(path.c_str()/*, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH*/) == -1) {
            std::cerr<<"Failed to create application directory!";
            //std::cerr << "Error: " << strerror(errno);
        }
        std::ofstream applis(appliname+"\\"+appliname+".poc");
        applis<<appliname<<std::endl;
        FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
        wApplicationNew->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wApplicationNew);
        tScriptEdit->setEventContextActivated(true);
        Label* lab = new Label(getRenderWindow(),Vec3f(0,0,0),Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),appliname, 15);
        Node* node = new Node("test",lab,Vec2f(0, 0),Vec2f(1.f, 0.025f),rootNode.get());
        lab->setParent(pProjects);
        lab->setForegroundColor(Color::Red);
        lab->setBackgroundColor(Color::White);
        pProjects->addChild(lab);
        Action a(Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
        Command cmd(a, FastDelegate<bool>(&Label::isMouseInside, lab), FastDelegate<void>(&ODFAEGCreator::showProjectsFiles, this, lab));
        Label* labScene = new Label(getRenderWindow(),Vec3f(0,0,0),Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Scenes", 15);
        Node* rSceneNode = new Node("test",labScene,Vec2f(0, 0),Vec2f(1.f, 0.025f),rootNode.get());
        rootScenesNode = rSceneNode;
        labScene->getListener().connect("SHOWPFILES", cmd);
        labScene->setForegroundColor(Color::Red);
        labScene->setParent(pProjects);
        pProjects->addChild(labScene);
        Action a2(Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
        //std::cout<<"label : "<<lab<<std::endl;

        Command cmd2(a, FastDelegate<bool>(&Label::isMouseInside, labScene), FastDelegate<void>(&ODFAEGCreator::showScenes, this, labScene));
        labScene->getListener().connect("SHOWSCENES"+appliname, cmd2);
        if (applitype == "Normal") {
            #if defined (ODFAEG_SYSTEM_LINUX)
            std::ofstream header(path+"/"+minAppliname+".hpp");
            #else if defined (ODFAEG_SYSTEM_WINDOWS)
            std::ofstream header(path+"\\"+minAppliname+".hpp");
            #endif // if
            std::ostringstream oss;
            oss<<"#ifndef "<<majAppliname<<"_HPP"<<std::endl;
            oss<<"#define "<<majAppliname<<"_HPP"<<std::endl;
            oss<<"#include \"odfaeg/Core/application.h\""<<std::endl;
            oss<<"class "<<appliname<<" : public odfaeg::core::Application<"+appliname+"> {"<<std::endl;
            oss<<"   public : "<<std::endl;
            oss<<"   "<<appliname<<"(VideoMode vm, std::string title);"<<std::endl;
            oss<<"   void onLoad();"<<std::endl;
            oss<<"   void onInit();"<<std::endl;
            oss<<"   void onRender(odfaeg::graphic::RenderComponentManager* cm);"<<std::endl;
            oss<<"   void onDisplay(odfaeg::graphic::RenderWindow* window);"<<std::endl;
            oss<<"   void onUpdate (odfaeg::graphic::RenderWindow*, odfaeg::window::IEvent& event);"<<std::endl;
            oss<<"   void onExec ();"<<std::endl;
            oss<<"   private : "<<std::endl;
            oss<<"   std::vector<std::unique_ptr<Drawable>> drawables;"<<std::endl;
            oss<<"   ResourceCache<> cache;"<<std::endl;
            oss<<"};"<<std::endl;
            oss<<"#endif"<<std::endl;
            header<<oss.str();
            header.close();
            cppAppliContent.insert(std::make_pair(std::make_pair(appliname, minAppliname+".hpp"), oss.str()));
            oss.str("");
            oss.clear();
            #if defined (ODFAEG_SYSTEM_LINUX)
            std::ofstream source(path+"/"+minAppliname+".cpp");
            #else if defined (ODFAEG_SYSTEM_WINDOWS)
            std::ofstream source(path+"\\"+minAppliname+".cpp");
            #endif // if
            oss<<"#include \""+minAppliname+".hpp\""<<std::endl;
            oss<<"using namespace odfaeg::graphic;"<<std::endl;
            oss<<"using namespace odfaeg::math;"<<std::endl;
            oss<<"using namespace odfaeg::window;"<<std::endl;
            oss<<appliname<<"::"<<appliname<<"(VideoMode vm, std::string title) : "<<std::endl;
            oss<<"Application (vm, title, Style::Resize|Style::Close, ContextSettings(0, 0, 0, 3, 0)) {"<<std::endl;
            oss<<"    EXPORT_CLASS_GUID(BoundingVolumeBoundingBox, odfaeg::physic::BoundingVolume, odfaeg::physic::BoundingBox)"<<std::endl;
            oss<<"    EXPORT_CLASS_GUID_(EntityTile, odfaeg::graphic::Entity, odfaeg::graphic::Tile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))"<<std::endl;;
            oss<<"    EXPORT_CLASS_GUID_(EntityBigTile, odfaeg::graphic::Entity, odfaeg::graphic::BigTile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))"<<std::endl;;
            oss<<"    EXPORT_CLASS_GUID_(EntityWall, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Wall, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))"<<std::endl;;
            oss<<"    EXPORT_CLASS_GUID_(EntityDecor, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Decor, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))"<<std::endl;;
            oss<<"    EXPORT_CLASS_GUID_(EntityAnimation, odfaeg::graphic::Entity, odfaeg::graphic::Anim, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))"<<std::endl;;
            oss<<"    EXPORT_CLASS_GUID_(EntityMesh, odfaeg::graphic::Entity, odfaeg::graphic::Mesh, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))"<<std::endl;;
            oss<<"    EXPORT_CLASS_GUID_(EntityParticleSystem, odfaeg::graphic::Entity, odfaeg::physic::ParticleSystem, (odfaeg::window::Device&)(odfaeg::graphic::EntityFactory&), (std::ref(c->getDevice()))(std::ref(c->getEntityFactory())))"<<std::endl;
            oss<<"    std::ifstream ifs("+appliname+"\\scenes.oc);"<<std::endl;
            oss<<"    if (ifs2) {"<<std::endl;
            oss<<"        ITextArchive ia2(ifs2);"<<std::endl;
            oss<<"        std::vector<Scene*> maps;"<<std::endl;
            oss<<"        ia2(maps);"<<std::endl;
            oss<<"        for (unsigned int i = 0; i < maps.size(); i++) "<<std::endl;
            oss<<"             getWorld()->addSceneManager(maps[i]);"<<std::endl;
            oss<<"    }"<<std::endl;
            oss<<"    std::vector<Entity*> entities = getWorld()->getRootEntities(\"*\");"<<std::endl;
            oss<<"    for (unsigned int i = 0; i < entities.size(); i++) {"<<std::endl;
            oss<<"    }"<<std::endl;
            oss<<"}"<<std::endl;
            oss<<"void "<<appliname<<"::onLoad() {"<<std::endl;
            oss<<"}"<<std::endl;
            oss<<"void "<<appliname<<"::onInit() {"<<std::endl;
            oss<<"    std::vector<Entity*> entities = getWorld()->getRootEntities(\"*\");"<<std::endl;
            oss<<"    for (unsigned int i = 0; i < entities.size(); i++) {"<<std::endl;
            oss<<"         entities[i]->onInit();"<<std::endl;
            oss<<"    }"<<std::endl;
            oss<<"}"<<std::endl;
            oss<<"void "<<appliname<<"::onRender(RenderComponentManager *cm) {"<<std::endl;
            oss<<"}"<<std::endl;
            oss<<"void "<<appliname<<"::onDisplay(RenderWindow* window) {"<<std::endl;
            oss<<"}"<<std::endl;
            oss<<"void "<<appliname<<"::onUpdate (RenderWindow* window, IEvent& event) {"<<std::endl;
            oss<<"    std::vector<Entity*> entities = getWorld()->getRootEntities(\"*\");"<<std::endl;
            oss<<"    for (unsigned int i = 0; i < entities.size(); i++) {"<<std::endl;
            oss<<"         entities[i]->onUpdate();"<<std::endl;
            oss<<"    }"<<std::endl;
            oss<<"    if (&getRenderWindow() == window && event.type == IEvent::WINDOW_EVENT && event.window.type == IEvent::WINDOW_EVENT_CLOSED) {"<<std::endl;
            oss<<"      stop();"<<std::endl;
            oss<<"    }"<<std::endl;
            oss<<"}"<<std::endl;
            oss<<"void "<<appliname<<"::onExec () {"<<std::endl;
            oss<<"}"<<std::endl;
            source<<oss.str();
            source.close();
            cppAppliContent.insert(std::make_pair(std::make_pair(appliname, minAppliname+".cpp"), oss.str()));
            oss.str("");
            oss.clear();
            std::string width = taWidth->getText();
            std::string height = taHeight->getText();
            #if defined (ODFAEG_SYSTEM_LINUX)
            std::ofstream main(path+"/main.cpp");
            #else if defined (ODFAEG_SYSTEM_WINDOWS)
            std::ofstream main(path+"\\main.cpp");
            #endif // if
            oss<<"#include \""<<minAppliname<<".hpp\""<<std::endl;
            oss<<"int main(int argc, char* argv[]) {"<<std::endl;
            oss<<"    "<<appliname<<" app(VideoMode("<<width<<","<<height<<"),\""<<appliname<<"\");"<<std::endl;
            oss<<"    return app.exec();"<<std::endl;
            oss<<"}"<<std::endl;
            main<<oss.str();
            main.close();
            cppAppliContent.insert(std::make_pair(std::make_pair(appliname, "main.cpp"), oss.str()));
            applis<<minAppliname+".hpp"<<std::endl;
            applis<<minAppliname+".cpp"<<std::endl;
            applis<<"main.cpp"<<std::endl;
            applis.close();
            std::unique_ptr<World> world = std::make_unique<World>();
            world->projectName = appliname;
            worlds.push_back(std::move(world));
            setCurrentWorld(worlds.back().get());
        }
    }
    if (button->getText() == "New texture") {
        fdTexturePath->setVisible(true);
        fdTexturePath->setEventContextActivated(true);
        tScriptEdit->setEventContextActivated(false);
    }
    if (button->getText() == "Create scene") {
        wNewMap->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wNewMap);
        tScriptEdit->setEventContextActivated(true);
        BaseChangementMatrix bcm;
        if (dpMapTypeList->getSelectedItem() == "2D iso") {
            bcm.set2DIsoMatrix();
        }
        theMap = new Scene(&getRenderComponentManager(), taMapName->getText(), conversionStringInt(taMapWidth->getText()), conversionStringInt(taMapHeight->getText()), conversionStringInt(taMapDepth->getText()));
        theMap->setBaseChangementMatrix(bcm);
        getWorld()->addSceneManager(theMap);
        getWorld()->setCurrentSceneManager(taMapName->getText());
        FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
        Label* lab = new Label(getRenderWindow(),Vec3f(0,0,0),Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),taMapName->getText(), 15);
        Node* node = new Node(taMapName->getText(),lab,Vec2f(0, 0),Vec2f(1.f, 0.025f),rootScenesNode);
        lab->setParent(pProjects);
        lab->setForegroundColor(Color::Blue);
        lab->setBackgroundColor(Color::White);
        pProjects->addChild(lab);
        Action a(Action::EVENT_TYPE::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
        Command cmd(a, FastDelegate<bool>(&Label::isMouseInside, lab), FastDelegate<void>(&ODFAEGCreator::showScene, this, lab));
        lab->getListener().connect("SHOWPFILES", cmd);
        cshapes.clear();
        for (int i = 0; i < getRenderWindow().getSize().x(); i+=100) {
            for (int j = 0; j < getRenderWindow().getSize().y(); j+=50) {
                ConvexShape cshape(4);
                cshape.setFillColor(Color::Transparent);
                cshape.setOutlineColor(Color(75, 75, 75));
                cshape.setOutlineThickness(1.f);
                Vec2f points[4];
                points[0] = Vec2f(0, 0);
                points[1] = Vec2f(100, 0);
                points[2] = Vec2f(100, 50);
                points[3] = Vec2f(0, 50);
                for (unsigned int n = 0; n < 4; n++) {
                    points[n] = bcm.changeOfBase(points[n]);
                    points[n] += Vec2f(i, j);
                    cshape.setPoint(n, Vec3f(points[n].x(), points[n].y(), 0));
                }
                cshapes.push_back(cshape);
            }
        }
        if (dpSelectEm != nullptr) {
            dpSelectEm->addItem(taMapName->getText(), 15);
        }
    }
    if (button->getText() == "Create component") {
        wNewComponent->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wNewComponent);
        tScriptEdit->setEventContextActivated(true);
        if (dpComponentType->getSelectedItem() == "LinkedList") {
            PerPixelLinkedListRenderComponent* ppll = new PerPixelLinkedListRenderComponent(getRenderWindow(),conversionStringInt(taComponentLayer->getText()),taComponentExpression->getText(),ContextSettings(24, 8, 4, 4, 6));
            getRenderComponentManager().addComponent(ppll);
            ppll->setName(taComponentName->getText());
            dpSelectComponent->addItem(taComponentName->getText(), 15);
            dpSelectComponent->setSelectedItem(taComponentName->getText());
        }
        if (dpComponentType->getSelectedItem() == "Shadow") {
            ShadowRenderComponent* src = new ShadowRenderComponent(getRenderWindow(),conversionStringInt(taComponentLayer->getText()),taComponentExpression->getText(),ContextSettings(0, 0, 4, 4, 6));
            getRenderComponentManager().addComponent(src);
            src->setName(taComponentName->getText());
            dpSelectComponent->addItem(taComponentName->getText(), 15);
            dpSelectComponent->setSelectedItem(taComponentName->getText());
        }
        if (dpComponentType->getSelectedItem() == "Light") {
            LightRenderComponent* lrc = new LightRenderComponent(getRenderWindow(),conversionStringInt(taComponentLayer->getText()),taComponentExpression->getText(),ContextSettings(0, 0, 4, 4, 6));
            getRenderComponentManager().addComponent(lrc);
            lrc->setName(taComponentName->getText());
            dpSelectComponent->addItem(taComponentName->getText(), 15);
            dpSelectComponent->setSelectedItem(taComponentName->getText());
        }
        if (dpComponentType->getSelectedItem() == "Refraction") {
            ReflectRefractRenderComponent* rrrc = new ReflectRefractRenderComponent(getRenderWindow(),conversionStringInt(taComponentLayer->getText()),taComponentExpression->getText(),ContextSettings(0, 0, 4, 4, 6));
            getRenderComponentManager().addComponent(rrrc);
            rrrc->setName(taComponentName->getText());
            dpSelectComponent->addItem(taComponentName->getText(), 15);
            dpSelectComponent->setSelectedItem(taComponentName->getText());
        }
        for (unsigned int i = 0; i < getRenderComponentManager().getComponents().size(); i++) {
            if (getRenderComponentManager().getComponents()[i]->getName() == dpSelectComponent->getSelectedItem()) {
                selectedComponentView = getRenderComponentManager().getComponents()[i]->getFrameBuffer()->getView();
            }
        }
        getRenderComponentManager().recreateDescriptorsAndPipelines();
    }
    if(button==bCreateEntitiesUpdater) {
        std::string name = taEntitiesUpdaterName->getText();
        EntitiesUpdater* eu = new EntitiesUpdater(factory, *getWorld());
        eu->setName(name);
        getWorld()->addWorker(eu);
        wNewEntitiesUpdater->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wNewEntitiesUpdater);
        tScriptEdit->setEventContextActivated(true);
    }
    if(button==bCreateAnimUpdater) {
        std::string name = taAnimUpdaterName->getText();
        AnimUpdater* au = new AnimUpdater();
        au->setName(name);
        getWorld()->addTimer(au);
        wNewAnimUpdater->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wNewAnimUpdater);
        tScriptEdit->setEventContextActivated(true);
        if (dpSelectAU != nullptr) {
            dpSelectAU->addItem(name, 15);
        }
    }
    if (button==bCreateEmitter) {
        std::string psName = taPSName->getText();
        emitterParams.push_back(taPSName->getText());
        UniversalEmitter emitter;
        emitter.setEmissionRate(conversionStringFloat(taEmissionRate->getText()));
        emitterParams.push_back(taEmissionRate->getText());
        emitter.setParticleLifetime(Distributions::uniform(seconds(conversionStringFloat(taMinLifeTime->getText())), seconds(conversionStringFloat(taMaxLifeTime->getText()))));
        emitterParams.push_back(taMinLifeTime->getText());
        emitterParams.push_back(taMaxLifeTime->getText());
        if (dpSelectPPType->getSelectedItem() == "Rect") {
            emitter.setParticlePosition(Distributions::rect(Vec3f(conversionStringFloat(taRCPosX->getText()), conversionStringFloat(taRCPosY->getText()), conversionStringFloat(taRCPosZ->getText())),
                                                           Vec3f(conversionStringFloat(taRCSizeX->getText()), conversionStringFloat(taRCSizeY->getText()), conversionStringFloat(taRCSizeZ->getText()))));
            emitterParams.push_back("Rect");
            emitterParams.push_back(taRCPosX->getText());
            emitterParams.push_back(taRCPosY->getText());
            emitterParams.push_back(taRCPosZ->getText());
            emitterParams.push_back(taRCSizeX->getText());
            emitterParams.push_back(taRCSizeY->getText());
            emitterParams.push_back(taRCSizeZ->getText());
        } else {
            emitter.setParticlePosition(Distributions::circle(Vec3f(conversionStringFloat(taRCPosX->getText()), conversionStringFloat(taRCPosY->getText()), conversionStringFloat(taRCPosZ->getText())),
                                                             conversionStringFloat(taRCSizeX->getText())));
            emitterParams.push_back("Circle");
            emitterParams.push_back(taRCPosX->getText());
            emitterParams.push_back(taRCPosY->getText());
            emitterParams.push_back(taRCPosZ->getText());
            emitterParams.push_back(taRCSizeX->getText());
        }
        emitter.setParticleVelocity(Distributions::deflect(Vec3f(conversionStringFloat(taDeflX->getText()), conversionStringFloat(taDeflY->getText()), conversionStringFloat(taDeflZ->getText())), conversionStringFloat(taDeflAngle->getText())));
        emitterParams.push_back(taDeflX->getText());
        emitterParams.push_back(taDeflY->getText());
        emitterParams.push_back(taDeflZ->getText());
        emitterParams.push_back(taDeflAngle->getText());
        emitter.setParticleRotation(Distributions::uniform(conversionStringFloat(taRotMin->getText()), conversionStringFloat(taRotMax->getText())));
        emitterParams.push_back(taRotMin->getText());
        emitterParams.push_back(taRotMax->getText());
        emitter.setParticleTextureIndex(Distributions::uniformui(conversionStringInt(taTexIndexMin->getText()),conversionStringInt(taTexIndexMax->getText())));
        emitterParams.push_back(taTexIndexMin->getText());
        emitterParams.push_back(taTexIndexMax->getText());
        emitter.setParticleScale(Distributions::rect(Vec3f(conversionStringFloat(taScaleMinX->getText()), conversionStringFloat(taScaleMinY->getText()), conversionStringFloat(taScaleMinZ->getText())),
                                                           Vec3f(conversionStringFloat(taScaleMaxX->getText()), conversionStringFloat(taScaleMaxY->getText()), conversionStringFloat(taScaleMaxZ->getText()))));
        emitterParams.push_back(taScaleMinX->getText());
        emitterParams.push_back(taScaleMinY->getText());
        emitterParams.push_back(taScaleMinZ->getText());
        emitterParams.push_back(taScaleMaxX->getText());
        emitterParams.push_back(taScaleMaxY->getText());
        emitterParams.push_back(taScaleMaxZ->getText());
        std::vector<std::string> color1 = split(taColor1->getText(), ";");
        std::vector<std::string> color2 = split(taColor2->getText(), ";");
        Vec4f c1(conversionStringInt(color1[0]), conversionStringInt(color1[1]), conversionStringInt(color1[2]), conversionStringInt(color1[3]));
        Vec4f c2(conversionStringInt(color2[0]), conversionStringInt(color2[1]), conversionStringInt(color2[2]), conversionStringInt(color2[3]));
        emitter.setParticleColor(Distributions::color(c1, c2));
        emitterParams.push_back(taColor1->getText());
        emitterParams.push_back(taColor2->getText());
        Entity* ps = getWorld()->getEntity(psName);
        if (dynamic_cast<ParticleSystem*>(ps)) {
            std::cout<<"add emitter to particles"<<std::endl;
            static_cast<ParticleSystem*>(ps)->addEmitter(emitter);
        }
        wNewEmitter->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wNewEmitter);
        tScriptEdit->setEventContextActivated(true);
    }
    if (button == bCreateParticleSystemUpdater) {
        std::string name = taParticleSystemUpdaterName->getText();
        ParticleSystemUpdater* psu = new ParticleSystemUpdater();
        psu->setName(name);
        psu->setInterval(seconds(0.01f));
        getWorld()->addTimer(psu);
        wNewParticleSystemUpdater->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wNewParticleSystemUpdater);
        tScriptEdit->setEventContextActivated(true);
        if (dpSelectPSU != nullptr) {
            dpSelectPSU->addItem(name, 15);
        }
    }
    if (button == bAddTexRect) {
        IntRect rect;
        rect.left = conversionStringInt(tTexCoordX->getText());
        rect.top = conversionStringInt(tTexCoordY->getText());
        rect.width = conversionStringInt(tTexCoordW->getText());
        rect.height = conversionStringInt(tTexCoordH->getText());
        if (dynamic_cast<ParticleSystem*>(selectedObject)) {
            static_cast<ParticleSystem*>(selectedObject)->addTextureRect(rect);
        }
    }
    if (button == bCreateObject) {
        std::vector<std::string> parts = split(dpSelectClass->getSelectedItem(), "::");
        Class cl = Class::getClass(parts[parts.size()-1]);
        std::string headerFile = cl.getFilePath();
        /*std::cout<<"header file : "<<headerFile<<std::endl;
        system("PAUSE");*/
        convertSlash(headerFile);
        //std::cout<<"header file : "<<headerFile<<std::endl;
        std::ifstream ifs(appliname+"\\sourceCode.cpp");
        std::string sourceCode="";
        if (ifs) {
            //fileExist = true;
            std::string line;
            while (getline(ifs, line)) {
                sourceCode += line+"\n";
            }
            ifs.close();
        }
        if (sourceCode == "") {
            sourceCode += "#include <fstream>\n";
            sourceCode += "#include <iostream>\n";
            sourceCode += "#include <vector>\n";
            sourceCode += "#include <string>\n";
            sourceCode += "#include \"odfaeg/Core/archive.h\"\n";
            sourceCode += "#include \"odfaeg/Core/class.hpp\"\n";
            sourceCode += "#include \"../../../ODFAEG-master2/Demos/ODFAEGCREATOR/application.hpp\"\n";
            sourceCode += "extern \"C\" {\n";
            sourceCode += "    void createObject(ODFAEGCreator* c, bool save);\n";
            sourceCode += "    void readObjects (ODFAEGCreator* c);\n";
            sourceCode += "}\n";
            sourceCode += "void createObject(ODFAEGCreator *c, bool save) {\n";
            sourceCode += "    EXPORT_CLASS_GUID(BoundingVolumeBoundingBox, odfaeg::physic::BoundingVolume, odfaeg::physic::BoundingBox)\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityTile, odfaeg::graphic::Entity, odfaeg::graphic::Tile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityBigTile, odfaeg::graphic::Entity, odfaeg::graphic::BigTile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityWall, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Wall, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityDecor, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Decor, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityAnimation, odfaeg::graphic::Entity, odfaeg::graphic::Anim, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityMesh, odfaeg::graphic::Entity, odfaeg::graphic::Mesh, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityParticleSystem, odfaeg::graphic::Entity, odfaeg::physic::ParticleSystem, (odfaeg::window::Device&)(odfaeg::graphic::EntityFactory&), (std::ref(c->getDevice()))(std::ref(c->getEntityFactory())))\n";
            sourceCode += "    odfaeg::core::Application::app = c;\n";
            sourceCode += "    if(save) {\n";
            sourceCode += "    }\n";
            sourceCode += "}\n";
            sourceCode += "void readObjects (ODFAEGCreator *c) {\n";
            sourceCode += "    EXPORT_CLASS_GUID(BoundingVolumeBoundingBox, odfaeg::physic::BoundingVolume, odfaeg::physic::BoundingBox)\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityTile, odfaeg::graphic::Entity, odfaeg::graphic::Tile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityBigTile, odfaeg::graphic::Entity, odfaeg::graphic::BigTile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityWall, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Wall, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityDecor, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Decor, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityAnimation, odfaeg::graphic::Entity, odfaeg::graphic::Anim, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityMesh, odfaeg::graphic::Entity, odfaeg::graphic::Mesh, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            sourceCode += "    EXPORT_CLASS_GUID_(EntityParticleSystem, odfaeg::graphic::Entity, odfaeg::physic::ParticleSystem, (odfaeg::window::Device&)(odfaeg::graphic::EntityFactory&), (std::ref(c->getDevice()))(std::ref(c->getEntityFactory())))\n";
            sourceCode += "    odfaeg::core::Application::app = c;\n";
            sourceCode += "}\n";
        }
        std::queue<Class> q;
        for (auto& sc : cl.getSuperClasses()) {
            q.push(sc);
        }
        bool found = false;
        while (!q.empty() && !found) {
            Class current = q.front();
            q.pop();
            if (current.getName() == "Entity") {
                found = true;
            }
            if (!found) {
                // Ajouter ses super-classes dans la file
                for (auto& sc : current.getSuperClasses()) {
                    q.push(sc);
                }
            }
        }
        std::string toInsert = "";
        int pos;
        std::string toFind;
        //std::cout<<"source file : "<<sourceFile<<std::endl;
        if (sourceCode.find("#include \""+headerFile+"\"") == std::string::npos) {
            sourceCode.insert(0, "#include \""+headerFile+"\"\n");

            if (sourceCode.find("}") != std::string::npos) {
                pos = sourceCode.find("}")+2;
                if (dpSelectPointerType->getSelectedItem() == "No pointer") {
                    if (cl.getNamespace() != "") {
                        toInsert = "std::vector<"+cl.getNamespace()+"::"+cl.getName()+"> v"+cl.getName()+";\n";
                    } else {
                        toInsert = "std::vector<"+cl.getName()+"> v"+cl.getName()+";\n";
                    }
                } else {
                    toInsert = "std::vector<"+dpSelectPointerType->getSelectedItem()+"*> v"+cl.getName()+";\n";
                }
                sourceCode.insert(pos, toInsert);
            }
            toInsert = "";
            toFind = "void createObject(ODFAEGCreator *c, bool save) {\n";
            toFind += "    EXPORT_CLASS_GUID(BoundingVolumeBoundingBox, odfaeg::physic::BoundingVolume, odfaeg::physic::BoundingBox)\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityTile, odfaeg::graphic::Entity, odfaeg::graphic::Tile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityBigTile, odfaeg::graphic::Entity, odfaeg::graphic::BigTile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityWall, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Wall, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityDecor, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Decor, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityAnimation, odfaeg::graphic::Entity, odfaeg::graphic::Anim, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityMesh, odfaeg::graphic::Entity, odfaeg::graphic::Mesh, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityParticleSystem, odfaeg::graphic::Entity, odfaeg::physic::ParticleSystem, (odfaeg::window::Device&)(odfaeg::graphic::EntityFactory&), (std::ref(c->getDevice()))(std::ref(c->getEntityFactory())))\n";
            toFind += "    odfaeg::core::Application::app = c;\n";
            pos = sourceCode.find(toFind)+toFind.size();
            if (dpSelectPointerType->getSelectedItem() != "No pointer" && dpSelectPointerType->getSelectedItem() != dpSelectClass->getSelectedItem()) {
                std::vector<std::string> parts = split(dpSelectPointerType->getSelectedItem(), "::");
                std::string name = parts[parts.size()-1];
                toInsert += "   EXPORT_CLASS_GUID_("+name+cl.getName()+","+dpSelectPointerType->getSelectedItem()+","+dpSelectClass->getSelectedItem()+", (odfaeg::graphic::EntityFactory&), (c->getEntityFactory()))\n";

                if (found) {
                    toInsert += "       std::map<std::string, std::vector<odfaeg::graphic::Entity*>>::iterator it"+cl.getName()+" = c->getExternals().find(\""+cl.getName()+"\");\n";
                } else {
                    toInsert += "   v"+cl.getName()+".clear();\n";
                }
            } else {
                toInsert += "   v"+cl.getName()+".clear();\n";

            }
            sourceCode.insert(pos, toInsert);
            toFind = "if(save) {\n";
            toInsert = "";
            toInsert += "       std::ofstream of"+cl.getName()+" (\""+cl.getName()+".oc\");\n";
            toInsert += "       odfaeg::core::OTextArchive oa"+cl.getName()+" (of"+cl.getName()+");\n";
            if (found) {
                toInsert += "       oa"+cl.getName()+"(it"+cl.getName()+"->second);\n";
            } else {
                toInsert += "       oa"+cl.getName()+"(v"+cl.getName()+");\n";
            }
            pos = sourceCode.find(toFind) + toFind.size();
            sourceCode.insert(pos, toInsert);
            toInsert = "";
            toFind = "void readObjects (ODFAEGCreator *c) {\n";
            toFind += "    EXPORT_CLASS_GUID(BoundingVolumeBoundingBox, odfaeg::physic::BoundingVolume, odfaeg::physic::BoundingBox)\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityTile, odfaeg::graphic::Entity, odfaeg::graphic::Tile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityBigTile, odfaeg::graphic::Entity, odfaeg::graphic::BigTile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityWall, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Wall, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityDecor, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Decor, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityAnimation, odfaeg::graphic::Entity, odfaeg::graphic::Anim, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityMesh, odfaeg::graphic::Entity, odfaeg::graphic::Mesh, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityParticleSystem, odfaeg::graphic::Entity, odfaeg::physic::ParticleSystem, (odfaeg::window::Device&)(odfaeg::graphic::EntityFactory&), (std::ref(c->getDevice()))(std::ref(c->getEntityFactory())))\n";
            toFind += "    odfaeg::core::Application::app = c;\n";
            pos = sourceCode.find(toFind)+toFind.size();
            if (dpSelectPointerType->getSelectedItem() != "No pointer" && dpSelectPointerType->getSelectedItem() != dpSelectClass->getSelectedItem()) {
                std::vector<std::string> parts = split(dpSelectPointerType->getSelectedItem(), "::");
                std::string name = parts[parts.size()-1];
                toInsert += "   EXPORT_CLASS_GUID_("+name+cl.getName()+","+dpSelectPointerType->getSelectedItem()+","+dpSelectClass->getSelectedItem()+", (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
                std::map<std::pair<std::string, std::string>, std::string>::iterator it;
                it = cppAppliContent.find(std::make_pair(getWorld()->projectName, minAppliname+".cpp"));
                if (it != cppAppliContent.end()) {
                    std::string toInsert2 = toInsert;
                    toInsert2 += "   std::vector<"+dpSelectPointerType->getSelectedItem()+"*> v"+cl.getName()+";\n";
                    toInsert2 += "   std::ifstream if"+cl.getName()+" (\""+cl.getName()+".oc\");\n";
                    toInsert2 += "   odfaeg::core::ITextArchive ia"+cl.getName()+" (if"+cl.getName()+");\n";
                    toInsert2 += "   ia"+cl.getName()+"(v"+cl.getName()+");\n";
                    toInsert2 += "   for (unsigned int i = 0; i < v"+cl.getName()+".size(); i++) { \n";
                    toInsert2 += "      getWorld()->setCurrentSceneManager(v"+cl.getName()+"[i]->currentScene);\n";
                    toInsert2 += "      getWorld()->addEntity(v"+cl.getName()+"[i]);\n";
                    toInsert2 += "   }\n";
                    toFind = "";
                    toFind +="    if (ifs2) {\n";
                    toFind +="        ITextArchive ia2(ifs2);\n";
                    toFind +="        std::vector<Scene*> maps;\n";
                    toFind +="        ia2(maps);\n";
                    toFind +="        for (unsigned int i = 0; i < maps.size(); i++) \n";
                    toFind +="             getWorld()->addSceneManager(maps[i]);\n";
                    toFind +="    }\n";
                    int pos2 = it->second.find(toFind)+toFind.size();
                    it->second.insert(pos2, toInsert2);
                }
            }
            toInsert += "   std::ifstream if"+cl.getName()+" (\""+cl.getName()+".oc\");\n";
            toInsert += "   odfaeg::core::ITextArchive ia"+cl.getName()+" (if"+cl.getName()+");\n";
            toInsert += "   ia"+cl.getName()+"(v"+cl.getName()+");\n";

            toFind = "";
            toFind += "    EXPORT_CLASS_GUID(BoundingVolumeBoundingBox, odfaeg::physic::BoundingVolume, odfaeg::physic::BoundingBox)\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityTile, odfaeg::graphic::Entity, odfaeg::graphic::Tile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityBigTile, odfaeg::graphic::Entity, odfaeg::graphic::BigTile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityWall, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Wall, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityDecor, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Decor, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityAnimation, odfaeg::graphic::Entity, odfaeg::graphic::Anim, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityMesh, odfaeg::graphic::Entity, odfaeg::graphic::Mesh, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
            toFind += "    EXPORT_CLASS_GUID_(EntityParticleSystem, odfaeg::graphic::Entity, odfaeg::physic::ParticleSystem, (odfaeg::window::Device&)(odfaeg::graphic::EntityFactory&), (std::ref(c->getDevice()))(std::ref(c->getEntityFactory())))\n";


            if (found) {
                //toInsert += "   c->getExternals().insert(std::make_pair(\""+cl.getName()+"\",v"+cl.getName()+"));\n";
                toInsert += "   for (unsigned int i = 0; i < v"+cl.getName()+".size(); i++)\n";
                toInsert += "       c->addExternalEntity(v"+cl.getName()+"[i],\""+cl.getName()+"\");\n";
                //toInsert += "   c->updateNb(\""+cl.getName()+"\",v"+cl.getName()+".size());\n";
            }
            sourceCode.insert(pos, toInsert);
        }
        toFind = "void createObject(ODFAEGCreator *c, bool save) {\n";
        toFind += "    EXPORT_CLASS_GUID(BoundingVolumeBoundingBox, odfaeg::physic::BoundingVolume, odfaeg::physic::BoundingBox)\n";
        toFind += "    EXPORT_CLASS_GUID_(EntityTile, odfaeg::graphic::Entity, odfaeg::graphic::Tile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
        toFind += "    EXPORT_CLASS_GUID_(EntityBigTile, odfaeg::graphic::Entity, odfaeg::graphic::BigTile, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
        toFind += "    EXPORT_CLASS_GUID_(EntityWall, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Wall, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
        toFind += "    EXPORT_CLASS_GUID_(EntityDecor, odfaeg::graphic::Entity, odfaeg::graphic::g2d::Decor, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
        toFind += "    EXPORT_CLASS_GUID_(EntityAnimation, odfaeg::graphic::Entity, odfaeg::graphic::Anim, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
        toFind += "    EXPORT_CLASS_GUID_(EntityMesh, odfaeg::graphic::Entity, odfaeg::graphic::Mesh, (odfaeg::graphic::EntityFactory&), (std::ref(c->getEntityFactory())))\n";
        toFind += "    EXPORT_CLASS_GUID_(EntityParticleSystem, odfaeg::graphic::Entity, odfaeg::physic::ParticleSystem, (odfaeg::window::Device&)(odfaeg::graphic::EntityFactory&), (std::ref(c->getDevice()))(std::ref(c->getEntityFactory())))\n";
        toFind += "    odfaeg::core::Application::app = c;\n";
        pos = sourceCode.find(toFind)+toFind.size();

        std::vector<std::string> argValues;
        unsigned int j = 0;
        for (unsigned int i = 0; i < argsTps.size(); i++) {
            if (argsTps[i] == "odfaeg::physic::BoundingPolyhedron") {
                argValues.push_back("c->getTmpBps()["+conversionIntString(currentBp)+"];\n");
                currentBp++;
            } else {
                argValues.push_back(tmpTextAreas[j]->getText());
                j++;
            }
        }
        if(sourceCode.find("if(save) {\n") != std::string::npos) {
            int pos = sourceCode.find("if(save) {\n")-3;


            std::string args;
            for (unsigned int j = 0; j < argValues.size(); j++) {
                args += argValues[j];
                if (j != argValues.size()-1) {
                    args += ",";
                }
            }
            std::string toInsert = "";
            if (found) {
                /*int nb = 0;
                std::map<std::string, unsigned int>::iterator it = nbs.find(cl.getName());
                if (it != nbs.end()) {
                    nb = it->second;
                } else {
                    updateNb(cl.getName(), nb);
                    it = nbs.find(cl.getName());
                }
                toInsert += "   if(it"+cl.getName()+"->second.size() == "+conversionIntString(nb)+") {\n";*/
                toInsert += "   "+dpSelectPointerType->getSelectedItem()+" *"+taObjectName->getText()+" = c->getEntityFactory().make_entity<"+dpSelectClass->getSelectedItem()+"> ("+args+");\n";
                toInsert += "   "+taObjectName->getText()+"->setExternalObjectName(\""+taObjectName->getText()+"\");\n";
                //toInsert += "       it"+cl.getName()+"->second.push_back("+taObjectName->getText()+");\n";
                toInsert += "   c->addExternalEntity("+taObjectName->getText()+",\""+cl.getName()+"\");\n";
                /*toInsert += "   }\n";
                it->second += 1;*/
            } else {
                if (dpSelectPointerType->getSelectedItem() == "No pointer") {
                    if (cl.getNamespace() != "") {
                        toInsert += "   "+cl.getNamespace()+"::"+cl.getName()+" "+taObjectName->getText()+" = "+cl.getNamespace()+"::"+cl.getName()+" ("+args+");\n";
                    } else {
                        toInsert += "   "+cl.getName()+" "+taObjectName->getText()+" ("+args+");\n";
                    }
                } else {
                    toInsert += "   "+dpSelectPointerType->getSelectedItem()+" *"+taObjectName->getText()+" = new "+dpSelectClass->getSelectedItem()+" ("+args+");\n";
                }
                toInsert += "   v"+cl.getName()+".push_back("+taObjectName->getText()+");\n";
            }
            sourceCode.insert(pos, toInsert);
        }

        std::ofstream file(appliname+"\\sourceCode.cpp");
        file<<sourceCode;
        file.close();

        rtc.addSourceFile(appliname+"\\sourceCode");
        rtc.compile();
        std::string errors = rtc.getCompileErrors();
        //std::cout<<"errors : "<<rtc.getCompileErrors();
        rtc.run<void>("createObject", this, false);
        wCreateNewObject->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wCreateNewObject);
        setEventContextActivated(true);
    }
    if (button == bModifyObject) {
        std::vector<std::string> parts = split(dpSelectMClass->getSelectedItem(), "::");
        Class cl = Class::getClass(parts[parts.size()-1]);
        std::vector<odfaeg::core::Class> superClasses = cl.getSuperClasses();
        std::queue<Class> q;
        for (auto& sc : cl.getSuperClasses()) {
            q.push(sc);
        }
        bool found = false;
        while (!q.empty() && !found) {
            Class current = q.front();
            q.pop();
            if (current.getName() == "Entity") {
                found = true;
            }
            if (!found) {
                // Ajouter ses super-classes dans la file
                for (auto& sc : current.getSuperClasses()) {
                    q.push(sc);
                }
            }
        }
        std::string sourceCode="", toInsert = "";
        std::vector<std::string> argValues;
        for (unsigned int i = 0; i < tmpTextAreas.size(); i++) {
            argValues.push_back(tmpTextAreas[i]->getText());
        }
        std::ifstream ifs(appliname+"\\sourceCode.cpp");
        bool fileExist = false;
        if (ifs) {
            fileExist = true;
            std::string line;
            while (getline(ifs, line)) {
                sourceCode += line+"\n";
            }
            ifs.close();
        }
        if(sourceCode.find("if(save) {\n") != std::string::npos) {
            std::string str("if(save) {\n");
            int pos = sourceCode.find(str)-1;
            std::string args;
            for (unsigned int j = 0; j < argValues.size(); j++) {
                args += argValues[j];
                if (j != argValues.size()-1) {
                    args += ",";
                }
            }
            if (found) {
                std::vector<std::string> parts2 = split(dpSelectMFunction->getSelectedItem(), "(");
                parts2 = split(parts2[0], " ");
                std::string name;
                if (cl.getNamespace() != "") {
                    name = cl.getNamespace()+"::"+cl.getName();
                } else {
                    name = cl.getName();
                }
                toInsert += "   if(c->getCurrentId() == "+conversionIntString(currentId)+") {\n";
                toInsert += "       for(unsigned int i = 0; i < it"+cl.getName()+"->second.size(); i++) {\n";
                toInsert += "           if(it"+cl.getName()+"->second[i]->getExternalObjectName() == \""+taMObjectName->getText()+"\") {\n";
                toInsert += "               static_cast<"+name+"*>(it"+cl.getName()+"->second[i])->"+parts2[1]+"("+args+");\n";
                toInsert += "           }\n";
                toInsert += "       }\n";
                toInsert += "   }\n";
                sourceCode.insert(pos, toInsert);

            } else {
                std::string toFind = "v"+cl.getName()+".push_back("+taMObjectName->getText()+");\n";
                pos = sourceCode.find(toFind) - 1;
                std::vector<std::string> parts2 = split(dpSelectMFunction->getSelectedItem(), "(");
                parts2 = split(parts2[0], " ");
                if (cbIsPointer->isChecked()) {
                    toInsert += "   "+taMObjectName->getText()+"->"+parts2[1]+"("+args+");\n";
                } else {
                    toInsert += "   "+taMObjectName->getText()+"."+parts2[1]+"("+args+");\n";
                }
                sourceCode.insert(pos, toInsert);
            }
        }
        std::ofstream file(appliname+"\\sourceCode.cpp");
        file<<sourceCode;
        file.close();
        rtc.addSourceFile(appliname+"\\sourceCode");
        rtc.compile();
        std::string errors = rtc.getCompileErrors();
        //std::cout<<"errors : "<<rtc.getCompileErrors();
        rtc.run<void>("createObject", this, false);
        currentId++;
        wModifyObject->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wModifyObject);
        tScriptEdit->setEventContextActivated(true);
    }
    if (button == bRemoveObject) {
        std::vector<std::string> parts = split(dpSelectRClass->getSelectedItem(), "::");
        Class cl = Class::getClass(parts[parts.size()-1]);
        std::vector<odfaeg::core::Class> superClasses = cl.getSuperClasses();
        std::queue<Class> q;
        for (auto& sc : cl.getSuperClasses()) {
            q.push(sc);
        }
        bool found = false;
        while (!q.empty() && !found) {
            Class current = q.front();
            q.pop();
            if (current.getName() == "Entity") {
                found = true;
            }
            if (!found) {
                // Ajouter ses super-classes dans la file
                for (auto& sc : current.getSuperClasses()) {
                    q.push(sc);
                }
            }
        }
        std::ifstream ifs("sourceCode.cpp");
        std::string sourceCode="";
        bool fileExist = false;
        if (ifs) {
            fileExist = true;
            std::string line;
            while (getline(ifs, line)) {
                sourceCode += line+"\n";
            }
            ifs.close();
        }
        if (fileExist) {

            if (found) {
                std::map<std::string, std::vector<Entity*>>::iterator it = externals.find(cl.getName());
                if (it != externals.end()) {
                    std::vector<Entity*>::iterator it2;
                    for (it2 = it->second.begin(); it2 != it->second.end();) {
                        if ((*it2)->getExternalObjectName() == taRObjectName->getText()) {
                            getWorld()->deleteEntity(*it2);
                            it2 = it->second.erase(it2);
                        } else {
                            it2++;
                        }
                    }
                }

            }
            std::string toFind=taRObjectName->getText();
            while (sourceCode.find(toFind) != std::string::npos) {
                int line = findLine(toFind, sourceCode);
                removeLine(line, sourceCode);
            }
            std::ofstream file(appliname+"\\sourceCode.cpp");
            file<<sourceCode;
            file.close();
            rtc.addSourceFile(appliname+"\\sourceCode");
            rtc.compile();
            std::string errors = rtc.getCompileErrors();
            //std::cout<<"errors : "<<rtc.getCompileErrors();
            rtc.run<void>("createObject", this, false);
        }
        wDeleteObject->setVisible(false);
        getRenderComponentManager().setEventContextActivated(false, *wDeleteObject);
        tScriptEdit->setEventContextActivated(true);
    }
    if (button == bGenerateTerrain) {
        wGenerateTerrain->setVisible(false);
        getWorld()->generate_map(ground, walls, Vec2f(conversionStringInt(taTileWidth->getText()), conversionStringInt(taTileHeight->getText())), BoundingBox(conversionStringInt(taZoneXPos->getText()), conversionStringInt(taZoneYPos->getText()),conversionStringInt(taZoneZPos->getText()),
                                                                                                                                                            conversionStringInt(taZoneWidth->getText()), conversionStringInt(taZoneHeight->getText()),conversionStringInt(taZoneDepth->getText())), factory);

        getRenderComponentManager().setEventContextActivated(false, *wGenerateTerrain);
        getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
    }
    if (button == bGenerate3DTerrain) {
        wGenerate3DTerrain->setVisible(false);
        std::cout<<"ground : "<<ground.size()<<std::endl;
        std::cout<<"tile size : "<<conversionStringInt(taTileWidth3D->getText())<<","<<conversionStringInt(taZoneHeight3D->getText())<<std::endl;
        std::cout<<"generate 3D terrain bx : "<<conversionStringInt(taZoneXPos3D->getText())<<","<<conversionStringInt(taZoneYPos3D->getText())<<","<<conversionStringInt(taZoneZPos3D->getText())
        <<","<<conversionStringInt(taZoneWidth3D->getText())<<","<<conversionStringInt(taZoneHeight3D->getText())<<","<<conversionStringInt(taZoneDepth3D->getText())<<std::endl;
        getWorld()->generate_3d_map(ground, walls3D, Vec2f(conversionStringInt(taTileWidth3D->getText()), conversionStringInt(taTileDepth3D->getText())), BoundingBox(conversionStringInt(taZoneXPos3D->getText()), conversionStringInt(taZoneYPos3D->getText()),conversionStringInt(taZoneZPos3D->getText()),
                                                                                                                              conversionStringInt(taZoneWidth3D->getText()), conversionStringInt(taZoneHeight3D->getText()),conversionStringInt(taZoneDepth3D->getText())), factory);
        getRenderComponentManager().setEventContextActivated(false, *wGenerate3DTerrain);
        getRenderComponentManager().setEventContextActivated(true, getRenderWindow());
    }
    if (button == bCreateScript) {
        cppAppliContent.insert(std::make_pair(std::make_pair(appliname, taScriptFileName->getText()), ""));
    }
}
void ODFAEGCreator::updateNb(std::string name, unsigned int nb) {
    nbs.insert(std::make_pair(name, nb));
}
void ODFAEGCreator::addExternalEntity(Entity* entity, std::string type) {
    /*Command::sname = "EXTERNAL";
    name = "EXTERNAL";*/
    /*entity->setExternal(true);
    selectedObject=entity;
    displayTransformInfos(entity);
    displayEntityInfos(entity);
    getWorld()->addEntity(entity);*/
    std::map<std::string, std::vector<Entity*>>::iterator it = toAdd.find(type);
    if (it == toAdd.end()) {
        Vec3f position = getRenderWindow().mapPixelToCoords(Vec3f(cursor.getPosition().x(), getRenderWindow().getSize().y() - cursor.getPosition().y(), 0))+getRenderWindow().getView().getSize()*0.5f;
        entity->setPosition(position);
        entity->setClassName(type);
        entity->currentScene = getWorld()->getCurrentSceneManager()->getName();
        toAdd.insert(std::make_pair(type, std::vector<Entity*>()));
        it = toAdd.find(type);
        it->second.push_back(entity);
        if (entity->getCollisionVolume() != nullptr)
            addBoundingVolumes(entity->getCollisionVolume());
    }
}
std::map<std::string, std::vector<Entity*>>& ODFAEGCreator::getExternals() {
    return externals;
}
void ODFAEGCreator::actionPerformed(MenuItem* item) {
    if (item->getText() == "New application") {
        std::cout<<"new application"<<std::endl;
        wApplicationNew->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wApplicationNew);
        tScriptEdit->setEventContextActivated(false);
    }
    if (item->getText() == "Build") {
        std::string command = "clang++-3.6 -Wall -std=c++14 -g "+appliname+"/"+"*.cpp -o "+appliname+"/"+minAppliname+".out "
        "/usr/local/lib/libodfaeg-network-s-d.a /usr/local/lib/libodfaeg-audio-s-d.a /usr/local/lib/libodfaeg-graphics-s-d.a "
        "/usr/local/lib/libodfaeg-physics-s-d.a /usr/local/lib/libodfaeg-math-s-d.a /usr/local/lib/libodfaeg-core-s-d.a "
        "/usr/local/lib/libsfml-audio.so /usr/local/lib/libsfml-network.so /usr/local/lib/libsfml-graphics.so /usr/local/lib/libsfml-window.so "
        "/usr/local/lib/libsfml-system.so /usr/lib/x86_64-linux-gnu/libfreetype.so /usr/lib/x86_64-linux-gnu/libpthread.so /usr/lib/x86_64-linux-gnu/libGLEW.so "
        "-lGL -lssl -lcrypto /usr/lib/x86_64-linux-gnu/libSDL2.so 2> errors.err";
        std::system(command.c_str());
    }
    if (item->getText() == "Run") {
        std::string command = std::string("./"+appliname+"/"+minAppliname+".out");
        std::system(command.c_str());
    }
    if (item->getText() == "Build and run") {
        std::string command = "clang++-3.6 -Wall -std=c++14 -g "+appliname+"/"+"*.cpp -o "+appliname+"/"+minAppliname+".out "
        "/usr/local/lib/libodfaeg-network-s-d.a /usr/local/lib/libodfaeg-audio-s-d.a /usr/local/lib/libodfaeg-graphics-s-d.a "
        "/usr/local/lib/libodfaeg-physics-s-d.a /usr/local/lib/libodfaeg-math-s-d.a /usr/local/lib/libodfaeg-core-s-d.a "
        "/usr/local/lib/libsfml-audio.so /usr/local/lib/libsfml-network.so /usr/local/lib/libsfml-graphics.so /usr/local/lib/libsfml-window.so "
        "/usr/local/lib/libsfml-system.so /usr/lib/x86_64-linux-gnu/libfreetype.so /usr/lib/x86_64-linux-gnu/libpthread.so /usr/lib/x86_64-linux-gnu/libGLEW.so "
        "-lGL -lssl -lcrypto /usr/lib/x86_64-linux-gnu/libSDL2.so /usr/lib/x86_64-linux-gnu/libgmp.so 2> errors.err";
        std::system(command.c_str());
        command = std::string("./"+appliname+"/"+minAppliname+".out");
        std::system(command.c_str());
    }

    if (item->getText() == "Tile") {
        if (appliname != "" && getWorld()->getCurrentSceneManager() != nullptr) {
            if (!showRectSelect) {
                Vec3f position = getRenderWindow().mapPixelToCoords(Vec3f(cursor.getPosition().x(), getRenderWindow().getSize().y() - cursor.getPosition().y(), 0))+getRenderWindow().getView().getSize()*0.5f;
                Tile* tile = factory.make_entity<Tile>(nullptr, position,Vec3f(gridWidth, gridHeight, 0), IntRect(0, 0, gridWidth, gridHeight), factory);
                selectedObject = tile;
                //std::cout << "wall center : " << tile->getCenter()<< std::endl;
                displayTileInfos(tile);
                getWorld()->addEntity(tile);
                //tile->setName("WALL");
                //addTile(tile);
            } else {
                BoundingBox rect = rectSelect.getSelectionRect();
                Vec3f savedPos = rectSelect.getSelectionRect().getPosition();
                Vec3f pos = rectSelect.getSelectionRect().getPosition();
                pos = getRenderWindow().mapPixelToCoords(Vec3f(pos.x(), getRenderWindow().getSize().y() - pos.y(), 0))+getRenderWindow().getView().getSize()*0.5f;
                rectSelect.setRect(pos.x(), pos.y(), pos.z(), rectSelect.getSelectionRect().getSize().x(),rectSelect.getSelectionRect().getSize().y(), rectSelect.getSelectionRect().getSize().z());
                //In 2D iso the tiles are in a staggered arrangement so we need to shift the x position every two times in the loop.
                if (getWorld()->getBaseChangementMatrix().isIso2DMatrix()) {
                    int i = 0;
                    int x = rect.getPosition().x();
                    int y = rect.getPosition().y();
                    int endX = rect.getPosition().x() + rect.getSize().x();
                    int endY = rect.getPosition().y() + rect.getSize().y();
                    while (y <= endY) {
                        int offsetX;
                        if (i%2==0)
                            offsetX = 0;
                        else
                            offsetX = gridWidth*0.5f;
                        x = rect.getPosition().x()+offsetX;
                        while (x <= endX) {
                            Vec3f position = getGridCellPos(Vec3f(x, y, 0));
                            Tile* tile = factory.make_entity<Tile>(nullptr,position,Vec3f(gridWidth, gridHeight, 0),IntRect(0, 0, gridWidth, gridHeight), factory);
                            rectSelect.addItem(tile);
                            if (rectSelect.getItems().size() == 1) {
                                selectedObject = tile;
                                displayTileInfos(tile);
                            }
                            getWorld()->addEntity(tile);
                            x += gridWidth;
                        }
                        y += gridHeight*0.5f;
                        i++;
                    }
                } else {
                    for (int x = rect.getPosition().x(); x < rect.getPosition().x() + rect.getSize().x(); x+=gridWidth) {
                        for (int y = rect.getPosition().y(); y <  rect.getPosition().y() + rect.getSize().y(); y+=gridHeight) {
                            Vec3f position = getGridCellPos(Vec3f(x, y, 0));
                            Tile* tile = factory.make_entity<Tile>(nullptr,position,Vec3f(gridWidth, gridHeight, 0),IntRect(0, 0, gridWidth, gridHeight), factory);
                            rectSelect.addItem(tile);
                            if (rectSelect.getItems().size() == 1) {
                                selectedObject = tile;
                                displayTileInfos(tile);
                            }
                            getWorld()->addEntity(tile);
                        }
                    }
                }
                rectSelect.setRect(savedPos.x(), savedPos.y(), savedPos.z(), rectSelect.getSelectionRect().getSize().x(),rectSelect.getSelectionRect().getSize().y(), rectSelect.getSelectionRect().getSize().z());
            }
        }
    }
    if(item->getText() == "Decor") {
        if (appliname != "" && getWorld()->getCurrentSceneManager() != nullptr) {
            if (!showRectSelect) {
                Decor* decor = factory.make_entity<Decor>(factory);
                decor->setName("HOUSE");
                Vec3f position = getRenderWindow().mapPixelToCoords(Vec3f(cursor.getPosition().x(), getRenderWindow().getSize().y() - cursor.getPosition().y(), 0))+getRenderWindow().getView().getSize()*0.5f;
                decor->setPosition(position);
                selectedObject = decor;
                displayDecorInfos(decor);
                getWorld()->addEntity(decor);
                //addDecor(decor);
            } else {
                BoundingBox rect = rectSelect.getSelectionRect();
                Vec3f savedPos = rectSelect.getSelectionRect().getPosition();
                Vec3f pos = rectSelect.getSelectionRect().getPosition();
                pos = getRenderWindow().mapPixelToCoords(Vec3f(pos.x(), getRenderWindow().getSize().y() - pos.y(), 0))+getRenderWindow().getView().getSize()*0.5f;
                rectSelect.setRect(pos.x(), pos.y(), pos.z(), rectSelect.getSelectionRect().getSize().x(),rectSelect.getSelectionRect().getSize().y(), rectSelect.getSelectionRect().getSize().z());
                for (int x = rect.getPosition().x(); x < rect.getPosition().x() + rect.getSize().x()-gridWidth; x+=gridWidth) {
                    for (int y = rect.getPosition().y(); y <  rect.getPosition().y() + rect.getSize().y()-gridHeight; y+=gridHeight) {
                        Decor* decor = factory.make_entity<Decor>(factory);
                        rectSelect.addItem(decor);
                        if (rectSelect.getItems().size() == 1) {
                            selectedObject = decor;
                            displayDecorInfos(decor);
                        }
                        getWorld()->addEntity(decor);
                    }
                }
                rectSelect.setRect(savedPos.x(), savedPos.y(), savedPos.z(), rectSelect.getSelectionRect().getSize().x(),rectSelect.getSelectionRect().getSize().y(), rectSelect.getSelectionRect().getSize().z());
            }
        }
    }
    if (item->getText() == "Wall") {
         if (appliname != "" && getWorld()->getCurrentSceneManager() != nullptr) {
            if (!showRectSelect) {
                Wall* wall = factory.make_entity<Wall>(factory);
                Vec3f position = getRenderWindow().mapPixelToCoords(Vec3f(cursor.getPosition().x(), getRenderWindow().getSize().y() - cursor.getPosition().y(), 0))+getRenderWindow().getView().getSize()*0.5f;
                wall->setPosition(position);
                selectedObject = wall;
                displayWallInfos(wall);
                getWorld()->addEntity(wall);
                //addDecor(decor);
            } else {
                BoundingBox rect = rectSelect.getSelectionRect();
                Vec3f savedPos = rectSelect.getSelectionRect().getPosition();
                Vec3f pos = rectSelect.getSelectionRect().getPosition();
                pos = getRenderWindow().mapPixelToCoords(Vec3f(pos.x(), getRenderWindow().getSize().y() - pos.y(), 0))+getRenderWindow().getView().getSize()*0.5f;
                rectSelect.setRect(pos.x(), pos.y(), pos.z(), rectSelect.getSelectionRect().getSize().x(),rectSelect.getSelectionRect().getSize().y(), rectSelect.getSelectionRect().getSize().z());
                for (int x = rect.getPosition().x(); x < rect.getPosition().x() + rect.getSize().x()-gridWidth; x+=gridWidth) {
                    for (int y = rect.getPosition().y(); y <  rect.getPosition().y() + rect.getSize().y()-gridHeight; y+=gridHeight) {
                        Wall* wall = factory.make_entity<Wall>(factory);
                        rectSelect.addItem(wall);
                        if (rectSelect.getItems().size() == 1) {
                            selectedObject = wall;
                            displayWallInfos(wall);
                        }
                        getWorld()->addEntity(wall);
                    }
                }
                rectSelect.setRect(savedPos.x(), savedPos.y(), savedPos.z(), rectSelect.getSelectionRect().getSize().x(),rectSelect.getSelectionRect().getSize().y(), rectSelect.getSelectionRect().getSize().z());
            }
        }
    }
    if (item->getText() == "Animation") {
        if (appliname != "" && getWorld()->getCurrentSceneManager() != nullptr) {
            if (!showRectSelect) {
                Anim* anim = factory.make_entity<Anim>(factory);
                Vec3f position = getRenderWindow().mapPixelToCoords(Vec3f(cursor.getPosition().x(), getRenderWindow().getSize().y() - cursor.getPosition().y(), 0))+getRenderWindow().getView().getSize()*0.5f;
                anim->setPosition(position);
                selectedObject = anim;
                displayAnimInfos(anim);
                getWorld()->addEntity(anim);
                //addDecor(decor);
            } else {
                BoundingBox rect = rectSelect.getSelectionRect();
                Vec3f savedPos = rectSelect.getSelectionRect().getPosition();
                Vec3f pos = rectSelect.getSelectionRect().getPosition();
                pos = getRenderWindow().mapPixelToCoords(Vec3f(pos.x(), getRenderWindow().getSize().y() - pos.y(), 0))+getRenderWindow().getView().getSize()*0.5f;
                rectSelect.setRect(pos.x(), pos.y(), pos.z(), rectSelect.getSelectionRect().getSize().x(),rectSelect.getSelectionRect().getSize().y(), rectSelect.getSelectionRect().getSize().z());
                for (int x = rect.getPosition().x(); x < rect.getPosition().x() + rect.getSize().x()-gridWidth; x+=gridWidth) {
                    for (int y = rect.getPosition().y(); y <  rect.getPosition().y() + rect.getSize().y()-gridHeight; y+=gridHeight) {
                        Anim* anim = factory.make_entity<Anim>(factory);
                        rectSelect.addItem(anim);
                        if (rectSelect.getItems().size() == 1) {
                            selectedObject = anim;
                            displayAnimInfos(anim);
                        }
                        getWorld()->addEntity(anim);
                    }
                }
                rectSelect.setRect(savedPos.x(), savedPos.y(), savedPos.z(), rectSelect.getSelectionRect().getSize().x(),rectSelect.getSelectionRect().getSize().y(), rectSelect.getSelectionRect().getSize().z());
            }
        }
    }
    if (item->getText() == "Particle System") {
        if (appliname != "" && getWorld()->getCurrentSceneManager() != nullptr) {
            if (!showRectSelect) {
                ParticleSystem* ps = factory.make_entity<ParticleSystem>(getDevice(), Vec3f(0, 0, 0),Vec3f(100, 100, 0), factory);
                selectedObject = ps;
                displayParticleSystemInfos(ps);
                getWorld()->addEntity(ps);
                //addDecor(decor);
            }
        }
    }
    if (item->getText() == "Emitter") {
        wNewEmitter->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wNewEmitter);
        tScriptEdit->setEventContextActivated(false);
    }
    if (item->getText() == "Affector") {
        //wNewAffector->setVisbile(true);
    }
    if (item->getText() == "Ponctual Light") {
        if (appliname != "" && getWorld()->getCurrentSceneManager() != nullptr) {
            if (!showRectSelect) {
                PonctualLight* pl = factory.make_entity<PonctualLight>(Vec3f(-50, 420, 420), 100, 50, 0, 255, Color::Yellow, 16, factory);
                selectedObject = pl;
                displayPonctualLightInfos(pl);
                getWorld()->addEntity(pl);
            }
        }
    }
    if (item->getText() == "Undo") {
        stateStack.undo();
    }
    if (item->getText() == "Redo") {
        stateStack.redo();
    }
    if (item == item43) {
        showGrid = !showGrid;
        if (showGrid) {
            item43->setText("v Show grid");
        } else {
            item43->setText("Show grid");
        }
    }
    if (item == item44) {
        alignToGrid = !alignToGrid;
        if (alignToGrid) {
            item44->setText("v Align to grid");
        } else {
            item44->setText("Align to grid");
        }
    }
    if (item == item45) {
        showRectSelect = !showRectSelect;
        if (showRectSelect) {
            item45->setText("v Rect selection");
        } else {
            item45->setText("Rect selection");
        }
    }
    if (item->getText() == "New scene") {
        wNewMap->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wNewMap);
        setEventContextActivated(false);
    }
    if (item->getText() == "New component") {
        wNewComponent->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wNewComponent);
        tScriptEdit->setEventContextActivated(false);
    }
    if (item->getText() == "New entities updater") {
        wNewEntitiesUpdater->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wNewEntitiesUpdater);
        tScriptEdit->setEventContextActivated(false);
    }
    if (item->getText() == "3D Model") {
        fdImport3DModel->setVisible(true);
        fdImport3DModel->setEventContextActivated(true);
    }
    if (item == item15) {
        fdProjectPath->setVisible(true);
        fdProjectPath->setEventContextActivated(true);
        tScriptEdit->setEventContextActivated(false);
    }
    if (item == item16) {
        wNewAnimUpdater->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wNewAnimUpdater);
        tScriptEdit->setEventContextActivated(false);
    }
    if (item == item17) {
        //Save textures.
        TextureManager<>& tm = cache.resourceManager<Texture, std::string>("TextureManager");
        std::vector<std::string> paths = tm.getPaths();
        std::ofstream file(appliname+"\\"+"textures.oc");
        OTextArchive oa(file);
        oa(paths);
        oa(textPaths);
        file.close();
        //Save entities.
        /*std::vector<Entity*> entities = World::getEntities("*");
        std::ofstream file2(appliname+"\\"+"entities.oc");
        OTextArchive oa2(file2);
        oa2(entities);
        file.close();*/
        //Save components.
        std::ofstream file3(appliname+"\\"+"components.oc");
        OTextArchive oa3(file3);
        std::vector<Component*> components = getRenderComponentManager().getRenderComponents();
        unsigned int size = components.size();
        oa3(size);
        for (unsigned int i = 0; i < components.size(); i++) {
            std::string name = components[i]->getName();
            std::string componentType="";
            int layer=0;
            std::string expression="";
            if (dynamic_cast<PerPixelLinkedListRenderComponent*>(components[i])) {
                componentType = "PerPixelLinkedList";
                layer = static_cast<PerPixelLinkedListRenderComponent*>(components[i])->getLayer();
                expression = static_cast<PerPixelLinkedListRenderComponent*>(components[i])->getExpression();
                //std::cout<<"layer : "<<layer<<std::endl;
            }
            if (dynamic_cast<ShadowRenderComponent*>(components[i])) {
                componentType = "Shadow";
                layer = static_cast<ShadowRenderComponent*>(components[i])->getLayer();
                expression = static_cast<ShadowRenderComponent*>(components[i])->getExpression();
                //std::cout<<"layer : "<<layer<<std::endl;
            }
            if (dynamic_cast<ReflectRefractRenderComponent*>(components[i])) {
                componentType = "ReflectRefract";
                layer = static_cast<ReflectRefractRenderComponent*>(components[i])->getLayer();
                expression = static_cast<ReflectRefractRenderComponent*>(components[i])->getExpression();
                //std::cout<<"layer : "<<layer<<std::endl;
            }
            if (dynamic_cast<LightRenderComponent*>(components[i])) {
                componentType = "Light";
                layer = static_cast<LightRenderComponent*>(components[i])->getLayer();
                expression = static_cast<LightRenderComponent*>(components[i])->getExpression();
                //std::cout<<"layer : "<<layer<<std::endl;
            }
            oa3(name);
            oa3(componentType);
            oa3(layer);
            oa3(expression);
        }
        file3.close();
        //Save entities updater.
        std::ofstream file4(appliname+"\\"+"timers.oc");
        OTextArchive oa4(file4);
        std::vector<Timer*> timers = getWorld()->getTimers();
        size = timers.size();
        oa4(size);
        for (unsigned int i = 0; i < timers.size(); i++) {
            std::string name = timers[i]->getName();
            oa4(name);
            std::string timerType;
            std::vector<Entity*> animations;
            if (dynamic_cast<AnimUpdater*>(timers[i])) {
                timerType = "AnimationUpdater";
                oa4(timerType);
            }
        }
        file4.close();
        std::ofstream file5(appliname+"\\"+"workers.oc");
        OTextArchive oa5(file5);
        std::vector<EntitySystem*> workers = getWorld()->getWorkers();
        size = workers.size();
        oa5(size);
        for (unsigned int i = 0; i < workers.size(); i++) {
            std::string name = workers[i]->getName();
            oa5(name);
            std::string workerType;
            if (dynamic_cast<EntitiesUpdater*>(workers[i])) {
                workerType = "EntityUpdater";
                oa5(workerType);
            }
            if (dynamic_cast<ParticleSystemUpdater*>(workers[i])) {
                workerType = "ParticleSystemUpdater";
                oa5(workerType);
            }
        }
        file5.close();
        std::ofstream file6(appliname+"\\"+"scenes.oc");
        OTextArchive oa6(file6);
        std::vector<SceneManager*> scenes = getWorld()->getSceneManagers();
        std::vector<Scene*> maps;
        for (unsigned int i = 0; i < scenes.size(); i++) {
            if (dynamic_cast<Scene*>(scenes[i])) {
                maps.push_back(static_cast<Scene*>(scenes[i]));
            }
        }
        oa6(maps);
        /*size = scenes.size();
        oa6(size);
        for (unsigned int i = 0; i < scenes.size(); i++) {
            std::string name;
            std::string type;
            int cellWidth;
            int cellHeight;
            bool is2DisoMatrix;
            if (dynamic_cast<Map*>(scenes[i])) {
                Map* scene = static_cast<Map*>(scenes[i]);
                name = scene->getName();
                cellWidth = scene->getCellWidth();
                cellHeight = scene->getCellHeight();
                type = "Map";
                BaseChangementMatrix bcm = scene->getBaseChangementMatrix();
                is2DisoMatrix = bcm.isIso2DMatrix();
            }
            oa6(name);
            oa6(type);
            oa6(cellWidth);
            oa6(cellHeight);
            oa6(is2DisoMatrix);
        }
        file6.close();*/
        std::ofstream file7(appliname+"\\"+"emitters.oc");
        OTextArchive oa7(file7);
        oa7(emitterParams);
        file7.close();
        std::ifstream file8(appliname+"\\sourceCode.cpp");
        if (file8) {

            pluginSourceCode = "";
            std::string line;
            while (getline(file8, line)) {
                pluginSourceCode += line + "\n";
            }
            if (pluginSourceCode != "")
                rtc.run<void>("createObject", this, true);
        }
        std::ofstream file9(appliname+"\\otherData.oc");
        OTextArchive oa8(file9);
        oa8(callIds);
        oa8(currentId);
        oa8(tmpBps);
        oa8(currentBp);
        file9.close();
     }
     if (item == item18) {
        wNewParticleSystemUpdater->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wNewParticleSystemUpdater);
        tScriptEdit->setEventContextActivated(false);
     }
     if (item == item51) {
        wCreateNewObject->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wCreateNewObject);
        std::cout<<"desactive context"<<std::endl;
        setEventContextActivated(false);
     }
     if (item == item52) {
        wModifyObject->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wModifyObject);
        tScriptEdit->setEventContextActivated(false);
     }
     if (item == item46) {
        for (unsigned int i = 0; i < ground.size(); i++)
            if (ground[i] != nullptr)
                delete ground[i];
        ground.clear();
        for (unsigned int i = 0; i < walls.size(); i++)
            if (walls[i] != nullptr)
                delete walls[i];
        walls.clear();
        walls.resize(g2d::Wall::NB_WALL_TYPES, nullptr);
        wGenerateTerrain->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wGenerateTerrain);
        getRenderComponentManager().setEventContextActivated(false, getRenderWindow());
     }
     if (item == item47) {
        for (unsigned int i = 0; i < ground.size(); i++)
            if (ground[i] != nullptr)
                delete ground[i];
        ground.clear();
        for (unsigned int i = 0; i < walls3D.size(); i++)
            if (walls3D[i] != nullptr)
                delete walls3D[i];
        walls3D.clear();
        walls3D.resize(g2d::Wall::NB_WALL_TYPES, nullptr);
        wGenerate3DTerrain->setVisible(true);
        getRenderComponentManager().setEventContextActivated(true, *wGenerate3DTerrain);
        getRenderComponentManager().setEventContextActivated(false, getRenderWindow());
     }
     if (item == item61) {
        if (selectedObject != nullptr) {
            BoundingBox globalBounds = selectedObject->getGlobalBounds();
            BoundingVolume* bv = new BoundingBox(globalBounds.getPosition().x(), globalBounds.getPosition().y(), globalBounds.getPosition().z(), globalBounds.getSize().x(), globalBounds.getSize().y(), globalBounds.getSize().z());
            boundingVolumes.push_back(bv);
            selectedBoundingVolume = bv;
            collisionBox = RectangleShape(bv->getSize());
            collisionBox.setPosition(bv->getPosition());
            collisionBox.setFillColor(Color(255, 0, 0, 128));
            if (selectedObject->getType() == "E_TILE") {
                displayTileInfos(selectedObject);
            } else if (selectedObject->getType() == "E_BIGTILE") {
                displayBigtileInfos(selectedObject);
            } else if (selectedObject->getType() == "E_WALL") {
                displayWallInfos(selectedObject);
            } else if (selectedObject->getType() == "E_DECOR") {
                displayDecorInfos(selectedObject);
            } else if (selectedObject->getType() == "E_ANIMATION") {
                displayAnimInfos(selectedObject);
            } else if (selectedObject->getType() == "E_PARTICLES") {
                displayParticleSystemInfos(selectedObject);
            } else if (selectedObject->getType() == "E_PONCTUAL_LIGHT") {
                displayPonctualLightInfos(selectedObject);
            } else {
                displayExternalEntityInfo(selectedObject);
            }
        }
     }
}
/*void ODFAEGCreator::addShape(Shape *shape) {
    std::map<std::string, std::string>::iterator it;
    it = cppAppliContent.find(minAppliname+".cpp");
    if (it != cppAppliContent.end()) {
        std::string& content = it->second;
        unsigned int pos = content.find("tm.getResourceByAlias");
        if (pos != std::string::npos && pos < content.size()) {
            std::string subs = content.substr(pos);
            pos += subs.find_first_of('\n') + 1;
            while (subs.find("tm.getResourceByAlias") != std::string::npos) {
                subs = content.substr(pos);
                pos += subs.find_first_of('\n') + 1;
            }
        } else {
            pos = content.find("TextureManager<>& tm = cache.resourceManager<Texture, std::string>(\"TextureManager\");");
            std::string subs = content.substr(pos);
            pos += subs.find_first_of('\n') + 1;
        }
        std::string toInsert = "    std::unique_ptr<Shape> shape"+conversionUIntString(shape->getId())+" = std::make_unique<RectangleShape>(Vec3f(100, 50, 0));\n"
                               "    drawables.push_back(std::move(shape));\n";
        content.insert(pos, toInsert);
    }
}
void ODFAEGCreator::addTile(Tile* tile) {
    std::map<std::string, std::string>::iterator it;
    it = cppAppliContent.find(minAppliname+".cpp");
    if (it != cppAppliContent.end()) {
        std::string& content = it->second;
        unsigned int pos = content.find("tm.getResourceByAlias");
        if (pos != std::string::npos && pos < content.size()) {
            std::string subs = content.substr(pos);
            pos += subs.find_first_of('\n') + 1;
            while (subs.find("tm.getResourceByAlias") != std::string::npos) {
                subs = content.substr(pos);
                pos += subs.find_first_of('\n') + 1;
            }
        } else {
            pos = content.find("TextureManager<>& tm = cache.resourceManager<Texture, std::string>(\"TextureManager\");");
            std::string subs = content.substr(pos);
            pos += subs.find_first_of('\n') + 1;
        }
        std::string toInsert = "Tile* tile"+conversionUIntString(tile->getId())+" = new Tile (nullptr,Vec3f("+conversionFloatString(cursor.getPosition().x())+","+
        conversionFloatString(cursor.getPosition().y())+","+conversionFloatString(cursor.getPosition().z())+"),Vec3f(100,50,0),IntRect(0, 0, "+conversionIntString(gridWidth)+","+
        conversionIntString(gridHeight)+"));\n"+"World::addEntity(tile"+conversionUIntString(tile->getId())+");\n";
        content.insert(pos, toInsert);

    }
}*/
bool ODFAEGCreator::removeShape (unsigned int id) {
   /* std::map<std::string, std::string>::iterator it;
    it = cppAppliContent.find(minAppliname+".cpp");
    if (it != cppAppliContent.end()) {
        std::string content = it->second;*/
        for (auto it = shapes.begin(); it != shapes.end();it++) {
            if ((*it)->getId() == id) {
                /*unsigned int pos = content.find("std::unique_ptr<Shape*> shape"+conversionUIntString(id));
                std::string subs = content.substr(pos);
                unsigned int endpos = pos + subs.find_first_of('\n') + 1;
                content.erase(pos, pos - endpos);
                pos = content.find("drawables.push_back(std::move(shape"+conversionUIntString(id)+"));\n");
                do {
                    std::string subs = content.substr(pos);
                    unsigned int endpos = pos + subs.find_first_of('\n') + 1;
                    content.erase(pos, pos - endpos);
                    pos = content.find("shape"+conversionUIntString(id));
                } while(pos != std::string::npos);*/
                it = shapes.erase(it);
                return true;
            }
        }
    //}
    return false;
}
void ODFAEGCreator::displayInfos (Shape* shape) {
    displayTransformInfos(shape);
    FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
    Label* lId = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif), "Id : shape-"+conversionIntString(shape->getId()), 15);
    lId->setParent(pInfos);
    Node* lIdNode = new Node("LabId", lId, Vec2f(0, 0), Vec2f(1, 0.025), rootInfosNode.get());
    pInfos->addChild(lId);
    lColor = new Label(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Color : ", 15);
    lColor->setParent(pMaterial);
    Node* lColorNode = new Node("LabColor",lColor,Vec2f(0, 0), Vec2f(1.f, 0.025f), rootMaterialNode.get());
    pMaterial->addChild(lColor);
    lRColor = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"r : ", 15);
    lRColor->setParent(pMaterial);
    Node* lRColorNode = new Node("LabRColor",lRColor,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lRColor);
    tRColor = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(shape->getFillColor().r), getRenderWindow());
    tRColor->setTextSize(15);
    tRColor->setParent(pMaterial);
    lRColorNode->addOtherComponent(tRColor, Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tRColor);
    lGColor = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"g : ", 15);
    lGColor->setParent(pMaterial);
    Node* lGColorNode = new Node("LabRColor",lGColor,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lGColor);
    tGColor = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(shape->getFillColor().g), getRenderWindow());
    tGColor->setTextSize(15);
    tGColor->setParent(pMaterial);
    lGColorNode->addOtherComponent(tGColor, Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tGColor);
    lBColor = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"b : ", 15);
    lBColor->setParent(pMaterial);
    Node* lBColorNode = new Node("LabBColor",lBColor,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lBColor);
    tBColor = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(shape->getFillColor().b), getRenderWindow());
    tBColor->setTextSize(15);
    tBColor->setParent(pMaterial);
    lBColorNode->addOtherComponent(tBColor, Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tBColor);
    lAColor = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"a : ", 15);
    lAColor->setParent(pMaterial);
    Node* lAColorNode = new Node("LabAColor",lAColor,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lAColor);
    tAColor = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(shape->getFillColor().a), getRenderWindow());
    tAColor->setTextSize(15);
    tAColor->setParent(pMaterial);
    lAColorNode->addOtherComponent(tAColor,Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tAColor);
    Command cmdRColChanged(FastDelegate<bool>(&TextArea::isTextChanged, tRColor), FastDelegate<void>(&ODFAEGCreator::onObjectColorChanged, this, tRColor));
    tRColor->getListener().connect("TRColorChanged", cmdRColChanged);
    Command cmdGColChanged(FastDelegate<bool>(&TextArea::isTextChanged, tGColor), FastDelegate<void>(&ODFAEGCreator::onObjectColorChanged, this, tGColor));
    tGColor->getListener().connect("TGColorChanged", cmdGColChanged);
    Command cmdBColChanged(FastDelegate<bool>(&TextArea::isTextChanged, tBColor), FastDelegate<void>(&ODFAEGCreator::onObjectColorChanged, this, tBColor));
    tBColor->getListener().connect("TBColorChanged", cmdBColChanged);
    Command cmdAColChanged(FastDelegate<bool>(&TextArea::isTextChanged, tAColor), FastDelegate<void>(&ODFAEGCreator::onObjectColorChanged, this, tAColor));
    tAColor->getListener().connect("TAColorChanged", cmdAColChanged);
    lTexture = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif),"Texture : ", 15);
    lTexture->setParent(pMaterial);
    Node* lTextureNode = new Node("LabTexture",lTexture,Vec2f(0, 0), Vec2f(1.f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lTexture);
    lTexCoordX = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Tex x : ", 15);
    lTexCoordX->setParent(pMaterial);
    Node* lTexCoordXNode = new Node("LTexCoordX", lTexCoordX,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lTexCoordX);
    tTexCoordX = new TextArea (Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"0",getRenderWindow());
    tTexCoordX->setTextSize(15);
    tTexCoordX->setParent(pMaterial);
    lTexCoordXNode->addOtherComponent(tTexCoordX,Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tTexCoordX);
    lTexCoordY = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Tex y : ", 15);
    lTexCoordY->setParent(pMaterial);
    Node* lTexCoordYNode = new Node("LTexCoordY", lTexCoordY,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lTexCoordY);
    tTexCoordY = new TextArea (Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"0",getRenderWindow());
    tTexCoordY->setTextSize(15);
    tTexCoordY->setParent(pMaterial);
    lTexCoordYNode->addOtherComponent(tTexCoordY,Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tTexCoordY);
    lTexCoordW = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Tex w : ", 15);
    lTexCoordW->setParent(pMaterial);
    Node* lTexCoordWNode = new Node("lTexCoordW", lTexCoordW,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lTexCoordW);
    tTexCoordW = new TextArea (Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"0",getRenderWindow());
    tTexCoordW->setTextSize(15);
    tTexCoordW->setParent(pMaterial);
    lTexCoordWNode->addOtherComponent(tTexCoordW,Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tTexCoordW);
    lTexCoordH = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Tex h : ", 15);
    lTexCoordH->setParent(pMaterial);
    Node* lTexCoordHNode = new Node("LTexCoordH", lTexCoordH,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lTexCoordH);
    tTexCoordH = new TextArea (Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"0",getRenderWindow());
    tTexCoordH->setTextSize(15);
    tTexCoordH->setParent(pMaterial);
    lTexCoordHNode->addOtherComponent(tTexCoordH,Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tTexCoordH);
    Command cmdTexCoordXChanged (FastDelegate<bool>(&TextArea::isTextChanged,tTexCoordX), FastDelegate<void>(&ODFAEGCreator::onTexCoordsChanged, this, tTexCoordX));
    tTexCoordX->getListener().connect("TTexCoordXChanged", cmdTexCoordXChanged);
    Command cmdTexCoordYChanged (FastDelegate<bool>(&TextArea::isTextChanged,tTexCoordY), FastDelegate<void>(&ODFAEGCreator::onTexCoordsChanged, this, tTexCoordY));
    tTexCoordY->getListener().connect("TTexCoordYChanged", cmdTexCoordYChanged);
    Command cmdTexCoordWChanged (FastDelegate<bool>(&TextArea::isTextChanged,tTexCoordW), FastDelegate<void>(&ODFAEGCreator::onTexCoordsChanged, this, tTexCoordW));
    tTexCoordW->getListener().connect("TTexCoordWChanged", cmdTexCoordWChanged);
    Command cmdTexCoordHChanged (FastDelegate<bool>(&TextArea::isTextChanged,tTexCoordH), FastDelegate<void>(&ODFAEGCreator::onTexCoordsChanged, this, tTexCoordH));
    tTexCoordH->getListener().connect("TTexCoordXChanged", cmdTexCoordHChanged);
    lTexImage = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif), "Tex Image : ", 15);
    lTexImage->setParent(pMaterial);
    Node* selectTextNode = new Node("SelectTexture",lTexImage,Vec2f(0, 0),Vec2f(0.25f, 0.025f), rootMaterialNode.get());
    pMaterial->addChild(lTexImage);
    dpSelectTexture = new DropDownList(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif),"NONE", 15);
    dpSelectTexture->setName("SELECTTEXT");
    dpSelectTexture->setParent(pMaterial);
    TextureManager<>& tm = cache.resourceManager<Texture, std::string>("TextureManager");
    std::vector<std::string> paths = tm.getPaths();
    for (unsigned int i = 0; i < paths.size(); i++) {
        std::vector<std::string> alias = tm.getAliasByResource(const_cast<Texture*>(tm.getResourceByPath(paths[i])));
        dpSelectTexture->addItem(alias[0], 15);
    }
    Command cmdTxtChanged(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectTexture), FastDelegate<void>(&ODFAEGCreator::onSelectedTextureChanged, this, dpSelectTexture));
    dpSelectTexture->getListener().connect("TextureChanged", cmdTxtChanged);
    Command cmdTxtDroppedDown (FastDelegate<bool>(&DropDownList::isDroppedDown, dpSelectTexture), FastDelegate<void>(&ODFAEGCreator::onSelectedTextureDroppedDown, this, dpSelectTexture));
    dpSelectTexture->getListener().connect("TextureDroppedDown", cmdTxtDroppedDown);
    selectTextNode->addOtherComponent(dpSelectTexture,Vec2f(0.75f, 0.025f));
    pMaterial->addChild(dpSelectTexture);
    bChooseText = new Button(Vec3f(0, 0, 0), Vec3f(100, 100, 0), fm.getResourceByAlias(Fonts::Serif),"New texture", 15, getRenderWindow());
    bChooseText->setParent(pMaterial);
    Node* chooseTextNode = new Node("ChooseText", bChooseText,Vec2f(0, 0), Vec2f(1.f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(bChooseText);
    bChooseText->setName("CHOOSETEXT");
    bChooseText->addActionListener(this);
    StateGroup* sg = new StateGroup("SGADDRECTANGLESHAPE");
    State* stAddRemoveShape = new State("ADDREMOVESHAPE", &se);
    std::ostringstream oss;
    OTextArchive ota(oss);
    ota(shape->getId());
    ota(shape);
    stAddRemoveShape->addParameter("ADDREMOVESHAPE", oss.str());
    sg->addState(stAddRemoveShape);
    stateStack.addStateGroup(sg);
    pScriptsFiles->setAutoResized(true);
}
void ODFAEGCreator::displayChildren(DropDownList* dp) {
    if (dp->getSelectedItem() != "NONE") {
        std::vector<Entity*> children = dynamic_cast<Entity*>(selectedObject)->getChildren();
        Entity* child;
        for (unsigned int i = 0; i < children.size(); i++) {
            if (children[i]->getName() == dp->getSelectedItem()) {
                child = children[i];
            }
        }
        if (selectedObject->getType() == "E_ANIMATION") {
            displayAnimInfos(child);
            selectedObject = child;
        } else if (selectedObject->getType() == "E_WALL") {
            displayWallInfos(child);
            selectedObject = child;
        } else if (selectedObject->getType() == "E_DECOR") {
            displayDecorInfos(child);
            selectedObject = child;
        } else if (selectedObject->getType() == "E_BIGTILE") {
            displayBigtileInfos(child);
            selectedObject = child;
        } else if (selectedObject->getType() == "E_PARTICLES") {
            displayParticleSystemInfos(child);
            selectedObject = child;
        } else if (selectedObject->getType() == "E_PONCTUAL_LIGHT") {
            displayPonctualLightInfos(child);
            selectedObject = child;
        } else if (selectedObject->getType() == "E_TILE") {
            displayTileInfos(child);
            selectedObject = child;
        } else {
            displayExternalEntityInfo(child);
            selectedObject = child;
        }
    }
}
void ODFAEGCreator::displayTransformInfos(Transformable* tile) {
    rootPropNode->deleteAllNodes();
    rootMaterialNode->deleteAllNodes();
    rootInfosNode->deleteAllNodes();
    rootShadowsNode->deleteAllNodes();
    rootCollisionNode->deleteAllNodes();
    pTransform->removeAll();
    pMaterial->removeAll();
    pMaterial->removeSprites();
    pInfos->removeAll();
    pShadows->removeAll();
    pCollisions->removeAll();

    FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
    //Origin.
    lOrigin = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "Origin : ", 15);
    lOrigin->setParent(pTransform);
    Node* nOrigin = new Node("ORIGIN", lOrigin, Vec2f(0, 0), Vec2f(1, 0.025), rootPropNode.get());
    pTransform->addChild(lOrigin);
    //X.
    lOrigX = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "x : ", 15);
    lOrigX->setParent(pTransform);
    Node* nOrigX = new Node("ORIGINX", lOrigX, Vec2f(0, 0), Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lOrigX);
    taOriginX = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getOrigin().x()), getRenderWindow());
    taOriginX->setParent(pTransform);
    pTransform->addChild(taOriginX);
    taOriginX->setTextSize(15);
    nOrigX->addOtherComponent(taOriginX, Vec2f(0.75, 0.025));
    //Y.
    lOrigY = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "y : ", 15);
    lOrigY->setParent(pTransform);
    Node* nOrigY = new Node("ORIGINY", lOrigY, Vec2f(0, 0), Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lOrigY);
    taOriginY = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getOrigin().y()), getRenderWindow());
    taOriginY->setParent(pTransform);
    pTransform->addChild(taOriginY);
    taOriginY->setTextSize(15);
    nOrigY->addOtherComponent(taOriginY, Vec2f(0.75, 0.025));
    //Z.
    lOrigZ = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "z : ", 15);
    lOrigZ->setParent(pTransform);
    Node* nOrigZ = new Node("ORIGINZ", lOrigZ, Vec2f(0, 0), Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lOrigZ);
    taOriginZ = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getOrigin().z()), getRenderWindow());
    taOriginZ->setParent(pTransform);
    pTransform->addChild(taOriginZ);
    taOriginZ->setTextSize(15);
    nOrigZ->addOtherComponent(taOriginZ, Vec2f(0.75, 0.025));
    Command cmdOriginX (FastDelegate<bool>(&TextArea::isTextChanged, taOriginX), FastDelegate<void>(&ODFAEGCreator::onObjectOriginChanged, this,taOriginX));
    Command cmdOriginY (FastDelegate<bool>(&TextArea::isTextChanged, taOriginY), FastDelegate<void>(&ODFAEGCreator::onObjectOriginChanged, this,taOriginY));
    Command cmdOriginZ (FastDelegate<bool>(&TextArea::isTextChanged, taOriginZ), FastDelegate<void>(&ODFAEGCreator::onObjectOriginChanged, this,taOriginZ));
    taOriginX->getListener().connect("tOriginXChanged", cmdOriginX);
    taOriginY->getListener().connect("tOriginYChanged", cmdOriginY);
    taOriginZ->getListener().connect("tOriginZChanged", cmdOriginZ);

    //Position.
    lPosition = new Label(getRenderWindow(),Vec3f(0,0,0),Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Position : ", 15);
    lPosition->setParent(pTransform);
    Node* lPosNode = new Node("LabPosition",lPosition,Vec2f(0, 0), Vec2f(1, 0.025),rootPropNode.get());
    pTransform->addChild(lPosition);
    //X
    lPosX = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"x : ",15);
    lPosX->setParent(pTransform);
    Node* lPosXNode = new Node("LabX",lPosX,Vec2f(0, 0),Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lPosX);
    tPosX = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getPosition().x()),getRenderWindow());
    tPosX->setParent(pTransform);
    tPosX->setTextSize(15);
    lPosXNode->addOtherComponent(tPosX, Vec2f(0.75, 0.025));
    pTransform->addChild(tPosX);
    //Y
    lPosY = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"y : ",15);
    lPosY->setParent(pTransform);
    Node* lPosYNode = new Node("LabY",lPosY,Vec2f(0, 0),Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lPosY);
    tPosY = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getPosition().y()),getRenderWindow());
    tPosY->setParent(pTransform);
    tPosY->setTextSize(15);
    lPosYNode->addOtherComponent(tPosY, Vec2f(0.75, 0.025));
    pTransform->addChild(tPosY);
    //Z
    lPosZ = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"z : ",15);
    lPosZ->setParent(pTransform);
    Node* lPosZNode = new Node("LabZ",lPosZ,Vec2f(0, 0),Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lPosZ);
    tPosZ = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getPosition().z()),getRenderWindow());
    tPosZ->setParent(pTransform);
    tPosZ->setTextSize(15);
    lPosZNode->addOtherComponent(tPosZ, Vec2f(0.75, 0.025));
    pTransform->addChild(tPosZ);
    Command cmdPosX (FastDelegate<bool>(&TextArea::isTextChanged, tPosX), FastDelegate<void>(&ODFAEGCreator::onObjectPosChanged, this,tPosX));
    Command cmdPosY (FastDelegate<bool>(&TextArea::isTextChanged, tPosY), FastDelegate<void>(&ODFAEGCreator::onObjectPosChanged, this,tPosY));
    Command cmdPosZ (FastDelegate<bool>(&TextArea::isTextChanged, tPosZ), FastDelegate<void>(&ODFAEGCreator::onObjectPosChanged, this,tPosZ));
    tPosX->getListener().connect("tPosXChanged", cmdPosX);
    tPosY->getListener().connect("tPosYChanged", cmdPosY);
    tPosZ->getListener().connect("tPosZChanged", cmdPosZ);

    //Size.
    Label* lSize = new Label(getRenderWindow(),Vec3f(0,0,0),Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Size : ", 15);
    lSize->setParent(pTransform);
    Node* lSizeNode = new Node("LabSize",lSize,Vec2f(0, 0), Vec2f(1, 0.025),rootPropNode.get());
    pTransform->addChild(lSize);
    //Width
    Label* lSizeW = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"w : ",15);
    lSizeW->setParent(pTransform);
    Node* lSizeWNode = new Node("LabW",lSizeW,Vec2f(0, 0),Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lSizeW);
    tSizeW = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getSize().x()),getRenderWindow());
    tSizeW->setParent(pTransform);
    tSizeW->setTextSize(15);
    lSizeWNode->addOtherComponent(tSizeW, Vec2f(0.75, 0.025));
    pTransform->addChild(tSizeW);
    //Height
    Label* lSizeH = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"h : ",15);
    lSizeH->setParent(pTransform);
    Node* lSizeHNode = new Node("LabH",lSizeH,Vec2f(0, 0),Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lSizeH);
    tSizeH = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getSize().y()),getRenderWindow());
    tSizeH->setParent(pTransform);
    tSizeH->setTextSize(15);
    lSizeHNode->addOtherComponent(tSizeH, Vec2f(0.75, 0.025));
    pTransform->addChild(tSizeH);
    //Depth
    Label* lSizeD = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"d : ",15);
    lSizeD->setParent(pTransform);
    Node* lSizeDNode = new Node("LabD",lSizeD,Vec2f(0, 0),Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lSizeD);
    tSizeD = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getSize().z()),getRenderWindow());
    tSizeD->setParent(pTransform);
    tSizeD->setTextSize(15);
    lSizeDNode->addOtherComponent(tSizeD, Vec2f(0.75, 0.025));
    pTransform->addChild(tSizeD);
    Command cmdSizeW (FastDelegate<bool>(&TextArea::isTextChanged, tSizeW), FastDelegate<void>(&ODFAEGCreator::onObjectSizeChanged, this,tSizeW));
    Command cmdSizeH (FastDelegate<bool>(&TextArea::isTextChanged, tSizeH), FastDelegate<void>(&ODFAEGCreator::onObjectSizeChanged, this,tSizeH));
    Command cmdSizeD (FastDelegate<bool>(&TextArea::isTextChanged, tSizeD), FastDelegate<void>(&ODFAEGCreator::onObjectSizeChanged, this,tSizeD));
    tSizeW->getListener().connect("tSizeWChanged", cmdSizeW);
    tSizeH->getListener().connect("tSizeHChanged", cmdSizeH);
    tSizeD->getListener().connect("tSizeDChanged", cmdSizeD);

    //Move
    Label* lMove = new Label(getRenderWindow(),Vec3f(0,0,0),Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Translation : ", 15);
    lMove->setParent(pTransform);
    Node* lMoveNode = new Node("LabPosition",lMove,Vec2f(0, 0), Vec2f(1, 0.025),rootPropNode.get());
    pTransform->addChild(lMove);
    //X
    Label *lMoveX = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"x : ",15);
    lMoveX->setParent(pTransform);
    Node* lMoveXNode = new Node("MOVEX",lMoveX,Vec2f(0, 0),Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lMoveX);
    tMoveX = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getTranslation().x()),getRenderWindow());
    tMoveX->setParent(pTransform);
    tMoveX->setTextSize(15);
    lMoveXNode->addOtherComponent(tMoveX, Vec2f(0.75, 0.025));
    pTransform->addChild(tMoveX);
    //Y
    Label* lMoveY = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"y : ",15);
    lMoveY->setParent(pTransform);
    Node* lMoveYNode = new Node("MOVEY",lMoveY,Vec2f(0, 0),Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lMoveY);
    tMoveY = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getTranslation().y()),getRenderWindow());
    tMoveY->setParent(pTransform);
    tMoveY->setTextSize(15);
    lMoveYNode->addOtherComponent(tMoveY, Vec2f(0.75, 0.025));
    pTransform->addChild(tMoveY);
    //Z
    Label* lMoveZ = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"z : ",15);
    lMoveZ->setParent(pTransform);
    Node* lMoveZNode = new Node("MOVEZ",lMoveZ,Vec2f(0, 0),Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lMoveZ);
    tMoveZ = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getTranslation().z()),getRenderWindow());
    tMoveZ->setParent(pTransform);
    tMoveZ->setTextSize(15);
    lMoveZNode->addOtherComponent(tMoveZ, Vec2f(0.75, 0.025));
    pTransform->addChild(tMoveZ);
    Command cmdMoveX (FastDelegate<bool>(&TextArea::isTextChanged, tMoveX), FastDelegate<void>(&ODFAEGCreator::onObjectMoveChanged, this,tMoveX));
    Command cmdMoveY (FastDelegate<bool>(&TextArea::isTextChanged, tMoveY), FastDelegate<void>(&ODFAEGCreator::onObjectMoveChanged, this,tMoveY));
    Command cmdMoveZ (FastDelegate<bool>(&TextArea::isTextChanged, tMoveZ), FastDelegate<void>(&ODFAEGCreator::onObjectMoveChanged, this,tMoveZ));
    tMoveX->getListener().connect("tMoveXChanged", cmdMoveX);
    tMoveY->getListener().connect("tMoveYChanged", cmdMoveY);
    tMoveZ->getListener().connect("tMoveZChanged", cmdMoveZ);

    //Scale.
    Label* lScale = new Label(getRenderWindow(),Vec3f(0,0,0),Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Scale : ", 15);
    lScale->setParent(pTransform);
    Node* lScaleNode = new Node("LabScale",lScale,Vec2f(0, 0), Vec2f(1, 0.025),rootPropNode.get());
    pTransform->addChild(lScale);
    //X
    Label *lScaleX = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"x : ",15);
    lScaleX->setParent(pTransform);
    Node* lScaleXNode = new Node("SCALEX",lScaleX,Vec2f(0, 0),Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lScaleX);
    tScaleX = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getScale().x()),getRenderWindow());
    tScaleX->setParent(pTransform);
    tScaleX->setTextSize(15);
    lScaleXNode->addOtherComponent(tScaleX, Vec2f(0.75, 0.025));
    pTransform->addChild(tScaleX);
    //Y
    Label* lScaleY = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"y : ",15);
    lScaleY->setParent(pTransform);
    Node* lScaleYNode = new Node("SCALEY",lScaleY,Vec2f(0, 0),Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lScaleY);
    tScaleY = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getScale().y()),getRenderWindow());
    tScaleY->setParent(pTransform);
    tScaleY->setTextSize(15);
    lScaleYNode->addOtherComponent(tScaleY, Vec2f(0.75, 0.025));
    pTransform->addChild(tScaleY);
    //Z
    Label* lScaleZ = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"z : ",15);
    lScaleZ->setParent(pTransform);
    Node* lScaleZNode = new Node("SCALEZ",lScaleZ,Vec2f(0, 0),Vec2f(0.25, 0.025), rootPropNode.get());
    pTransform->addChild(lScaleZ);
    tScaleZ = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getScale().z()),getRenderWindow());
    tScaleZ->setParent(pTransform);
    tScaleZ->setTextSize(15);
    lScaleZNode->addOtherComponent(tScaleZ, Vec2f(0.75, 0.025));
    pTransform->addChild(tScaleZ);
    Command cmdScaleX (FastDelegate<bool>(&TextArea::isTextChanged, tScaleX), FastDelegate<void>(&ODFAEGCreator::onObjectScaleChanged, this,tScaleX));
    Command cmdScaleY (FastDelegate<bool>(&TextArea::isTextChanged, tScaleY), FastDelegate<void>(&ODFAEGCreator::onObjectScaleChanged, this,tScaleY));
    Command cmdScaleZ (FastDelegate<bool>(&TextArea::isTextChanged, tScaleZ), FastDelegate<void>(&ODFAEGCreator::onObjectScaleChanged, this,tScaleZ));
    tScaleX->getListener().connect("tScaleXChanged", cmdScaleX);
    tScaleY->getListener().connect("tScaleYChanged", cmdScaleY);
    tScaleZ->getListener().connect("tScaleZChanged", cmdScaleZ);

    //Rotation.

    Label* lRotation = new Label(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"rotation : ", 15);
    lRotation->setParent(pTransform);
    Node* lRotationNode = new Node("ROTATION",lRotation,Vec2f(0, 0), Vec2f(1, 0.025),rootPropNode.get());

    pTransform->addChild(lRotation);

    Label* lAngle = new Label(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"angle : ", 15);
    lAngle->setParent(pTransform);
    Node* lRotAngleNode = new Node("ANGLE",lAngle,Vec2f(0, 0),Vec2f(0.25, 0.025), rootPropNode.get());

    pTransform->addChild(lAngle);
    tRotAngle = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getRotation()),getRenderWindow());
    tRotAngle->setParent(pTransform);
    tRotAngle->setTextSize(15);
    lRotAngleNode->addOtherComponent(tRotAngle, Vec2f(0.75, 0.025));
    pTransform->addChild(tRotAngle);
    Command cmdRotAngle (FastDelegate<bool>(&TextArea::isTextChanged, tRotAngle), FastDelegate<void>(&ODFAEGCreator::onObjectRotationChanged, this, tRotAngle));
    tRotAngle->getListener().connect("ROTANGLECHANGED", cmdRotAngle);
    if (tabPane->getSelectedTab() != "Transform")
        pTransform->setEventContextActivated(false);

}
void ODFAEGCreator::displayEntityInfos(Entity* tile) {

    FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
    Label* lId = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif), "Id : entity-"+conversionIntString(tile->getId()), 15);
    lId->setParent(pInfos);
    Node* lIdNode = new Node("LabId", lId, Vec2f(0, 0), Vec2f(1, 0.025), rootInfosNode.get());
    pInfos->addChild(lId);
    Label* lType = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "type : "+tile->getType(), 15);
    lType->setParent(pInfos);
    Node* typeNode = new Node("Type", lType, Vec2f(0, 0), Vec2f(1,0.025), rootInfosNode.get());
    pInfos->addChild(lType);
    Label* lName = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif), "Name : ", 15);
    lName->setParent(pInfos);
    Node* nameNode = new Node("Name", lName, Vec2f(0, 0), Vec2f(0.25, 0.025), rootInfosNode.get());
    pInfos->addChild(lName);
    taName = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"",getRenderWindow());
    taName->setTextSize(15);
    taName->setParent(pInfos);
    taName->setName("TANAME");
    nameNode->addOtherComponent(taName, Vec2f(0.75, 0.025));
    pInfos->addChild(taName);
    Command cmdName (FastDelegate<bool>(&TextArea::isTextChanged, taName), FastDelegate<void>(&ODFAEGCreator::onObjectNameChanged, this,taName));
    taName->getListener().connect("CMDNAMECHANGED", cmdName);

    if (tile->getParent() == nullptr) {
        lParent = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif), "Parent entity : NONE", 15);
    } else {
        lParent = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif), "Parent entity : "+tile->getParent()->getName(), 15);
    }
    lParent->setParent(pInfos);


    Action a(Action::MOUSE_BUTTON_PRESSED_ONCE, IMouse::Left);
    Command cmdParentClicked(a, FastDelegate<bool>(&Label::isMouseInside, lParent), FastDelegate<void>(&ODFAEGCreator::onParentClicked, this, lParent));
    pInfos->getListener().removeLater("CMDPARENTCLICKED");
    pInfos->getListener().connect("CMDPARENTCLICKED", cmdParentClicked);

    Node* parentNode = new Node("Parent",lParent,Vec2f(0, 0),Vec2f(1, 0.025),rootInfosNode.get());
    pInfos->addChild(lParent);
    Label* lChildren = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(100, 17, 0),fm.getResourceByAlias(Fonts::Serif), "Children : ", 15);

    lChildren->setParent(pInfos);
    Node* childrenNode = new Node("Children",lChildren,Vec2f(0, 0),Vec2f(0.25, 0.025),rootInfosNode.get());
    pInfos->addChild(lChildren);

    dpChildren = new DropDownList(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 17, 0), fm.getResourceByAlias(Fonts::Serif), "NONE", 15);
    for (unsigned int i = 0; i < tile->getChildren().size(); i++) {
        if (tile->getChildren()[i]->getName() != "") {
            dpChildren->addItem(tile->getChildren()[i]->getName(), 15);
        }
    }
    dpChildren->setParent(pInfos);
    pInfos->addChild(dpChildren);
    childrenNode->addOtherComponent(dpChildren, Vec2f(0.75, 0.025));
    Command cmdChildren(FastDelegate<bool>(&DropDownList::isValueChanged, dpChildren), FastDelegate<void>(&ODFAEGCreator::displayChildren, this, dpChildren));
    pInfos->getListener().removeLater("DisplayChildren");
    pInfos->getListener().connect("DisplayChildren", cmdChildren);

    std::vector<SceneManager*> ems = getWorld()->getSceneManagers();
    Label* lEmList = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 14, 0), fm.getResourceByAlias(Fonts::Serif), "Scene : ", 15);
    lEmList->setParent(pInfos);
    pInfos->addChild(lEmList);
    Node* lEmListNode = new Node("LabEmList", lEmList, Vec2f(0, 0), Vec2f(0.25, 0.025), rootInfosNode.get());
    dpSelectEm = new DropDownList(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif),"NONE", 15);
    for (unsigned int i = 0; i < ems.size(); i++) {
        dpSelectEm->addItem(ems[i]->getName(), 15);
    }
    dpSelectEm->setParent(pInfos);
    pInfos->addChild(dpSelectEm);
    Command cmdEmChanged(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectEm), FastDelegate<void>(&ODFAEGCreator::onSelectedEmChanged, this, dpSelectEm));
    dpSelectEm->getListener().connect("EmChanged", cmdEmChanged);
    dpSelectEm->setPriority(-2);
    lEmListNode->addOtherComponent(dpSelectEm, Vec2f(0.75, 0.025));

    //Script selection.

    Label* lSelectScriptLabel = new Label(getRenderWindow(), Vec3f(0, 0, 0),Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"Script : ",15);
    lSelectScriptLabel->setParent(pInfos);
    pInfos->addChild(lSelectScriptLabel);
    Node* nSelectScript = new Node("SelectScript",lSelectScriptLabel,Vec2f(0, 0),Vec2f(0.25, 0.025),rootInfosNode.get());
    dpSelectScript = new DropDownList(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"NONE", 15);
    std::vector<std::string> classes = Class::getClasses(appliname+"\\Scripts");
    for (unsigned int i = 0; i < classes.size(); i++) {
        Class cl = Class::getClass(classes[i]);
        bool isMonoBehaviourScript = false;
        std::vector<Class> superClasses = cl.getSuperClasses();
        for (unsigned int j = 0; j < superClasses.size(); j++) {
            if (superClasses[j].getName() == "MonoBehaviour")
                isMonoBehaviourScript = true;
        }
        if (isMonoBehaviourScript) {
            if (cl.getNamespace() == "") {
                dpSelectScript->addItem(classes[i], 15);
            } else {
                dpSelectScript->addItem(cl.getNamespace()+"::"+classes[i], 15);
            }
        }
    }
    Command cmdScriptChanged(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectScript), FastDelegate<void>(&ODFAEGCreator::onSelectedEmChanged, this, dpSelectScript));
    dpSelectScript->getListener().connect("ScriptChanged", cmdScriptChanged);
    dpSelectScript->setParent(pInfos);
    pInfos->addChild(dpSelectScript);
    nSelectScript->addOtherComponent(dpSelectScript,Vec2f(0.75, 0.025));

    Label* lSelectParent = new Label(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"Parent : ",15);
    lSelectParent->setParent(pInfos);
    pInfos->addChild(lSelectParent);



    Node* selectParentNode = new Node("SelectParent",lSelectParent,Vec2f(0, 0),Vec2f(0.25, 0.025),rootInfosNode.get());
    dpSelectParent = new DropDownList(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"NONE", 15);
    std::vector<Entity*> parents = getWorld()->getEntities("*");

    for (unsigned int i = 0; i < parents.size(); i++) {
        if (!parents[i]->isLeaf() && parents[i]->getName() != "") {
            dpSelectParent->addItem(parents[i]->getName(), 15);
        }
    }
    dpSelectParent->setParent(pInfos);
    pInfos->addChild(dpSelectParent);
    Command cmdParentChanged(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectParent), FastDelegate<void>(&ODFAEGCreator::onSelectedParentChanged, this, dpSelectParent));
    dpSelectParent->getListener().connect("ParentChanged",cmdParentChanged);
    selectParentNode->addOtherComponent(dpSelectParent,Vec2f(0.75, 0.025));


    //Shadow center.

    Label* lShadowCenter = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Shadow center : ", 15);
    Node* nShadowCenter = new Node("ShadowCenter", lShadowCenter, Vec2f(0, 0), Vec2f(1, 0.025),rootShadowsNode.get());
    lShadowCenter->setParent(pShadows);
    pShadows->addChild(lShadowCenter);
    //X
    Label* lXShadowCenter = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "X : ", 15);
    Node* nXShadowCenter = new Node("XShadowCenter", lXShadowCenter, Vec2f(0, 0), Vec2f(0.25, 0.025), rootShadowsNode.get());
    lXShadowCenter->setParent(pShadows);
    pShadows->addChild(lXShadowCenter);
    taXShadowCenter = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getShadowCenter().x()),getRenderWindow());
    nXShadowCenter->addOtherComponent(taXShadowCenter, Vec2f(0.75, 0.025));
    taXShadowCenter->setTextSize(15);
    taXShadowCenter->setParent(pShadows);
    pShadows->addChild(taXShadowCenter);
    //Y
    Label* lYShadowCenter = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Y : ", 15);
    Node* nYShadowCenter = new Node("YShadowCenter", lYShadowCenter, Vec2f(0, 0), Vec2f(0.25, 0.025), rootShadowsNode.get());
    lYShadowCenter->setParent(pShadows);
    pShadows->addChild(lYShadowCenter);
    taYShadowCenter = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getShadowCenter().y()),getRenderWindow());
    nYShadowCenter->addOtherComponent(taYShadowCenter, Vec2f(0.75, 0.025));
    taYShadowCenter->setTextSize(15);
    taYShadowCenter->setParent(pShadows);
    pShadows->addChild(taYShadowCenter);
    //Z
    Label* lZShadowCenter = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Z : ", 15);
    Node* nZShadowCenter = new Node("ZShadowCenter", lZShadowCenter, Vec2f(0, 0), Vec2f(0.25, 0.025), rootShadowsNode.get());
    lZShadowCenter->setParent(pShadows);
    pShadows->addChild(lZShadowCenter);
    taZShadowCenter = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getShadowCenter().z()),getRenderWindow());
    nZShadowCenter->addOtherComponent(taZShadowCenter, Vec2f(0.75, 0.025));
    taZShadowCenter->setTextSize(15);
    taZShadowCenter->setParent(pShadows);
    pShadows->addChild(taZShadowCenter);
    Command cmdXShadowCenter(FastDelegate<bool>(&TextArea::isTextChanged, taXShadowCenter), FastDelegate<void>(&ODFAEGCreator::onShadowCenterChanged, this, taXShadowCenter));
    taXShadowCenter->getListener().connect("XSHADOWCENTERCHANGED", cmdXShadowCenter);
    Command cmdYShadowCenter(FastDelegate<bool>(&TextArea::isTextChanged, taYShadowCenter), FastDelegate<void>(&ODFAEGCreator::onShadowCenterChanged, this, taYShadowCenter));
    taYShadowCenter->getListener().connect("YSHADOWCENTERCHANGED", cmdYShadowCenter);
    Command cmdZShadowCenter(FastDelegate<bool>(&TextArea::isTextChanged, taZShadowCenter), FastDelegate<void>(&ODFAEGCreator::onShadowCenterChanged, this, taZShadowCenter));
    taZShadowCenter->getListener().connect("ZSHADOWCENTERCHANGED", cmdZShadowCenter);
    //Shadow scale.
    Label* lShadowScale = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Shadow scale : ", 15);
    Node* nShadowScale = new Node("ShadowSale", lShadowScale, Vec2f(0, 0), Vec2f(1, 0.025),rootShadowsNode.get());
    lShadowScale->setParent(pShadows);
    pShadows->addChild(lShadowScale);
    //X
    Label* lXShadowScale = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "X : ", 15);
    Node* nXShadowScale = new Node("XShadowCenter", lXShadowScale, Vec2f(0, 0), Vec2f(0.25, 0.025), rootShadowsNode.get());
    lXShadowScale->setParent(pShadows);
    pShadows->addChild(lXShadowScale);
    taXShadowScale = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getShadowScale().x()),getRenderWindow());
    nXShadowScale->addOtherComponent(taXShadowScale, Vec2f(0.75, 0.025));
    taXShadowScale->setTextSize(15);
    taXShadowScale->setParent(pShadows);
    pShadows->addChild(taXShadowScale);
    //Y
    Label* lYShadowScale = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Y : ", 15);
    Node* nYShadowScale = new Node("YShadowScale", lYShadowScale, Vec2f(0, 0), Vec2f(0.25, 0.025), rootShadowsNode.get());
    lYShadowScale->setParent(pShadows);
    pShadows->addChild(lYShadowScale);
    taYShadowScale = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getShadowScale().y()),getRenderWindow());
    nYShadowScale->addOtherComponent(taYShadowScale, Vec2f(0.75, 0.025));
    taYShadowScale->setTextSize(15);
    taYShadowScale->setParent(pShadows);
    pShadows->addChild(taYShadowScale);
    //Z
    Label* lZShadowScale = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Z : ", 15);
    Node* nZShadowScale = new Node("ZShadowScale", lZShadowScale, Vec2f(0, 0), Vec2f(0.25, 0.025), rootShadowsNode.get());
    lZShadowScale->setParent(pShadows);
    pShadows->addChild(lZShadowScale);
    taZShadowScale = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getShadowScale().z()),getRenderWindow());
    nZShadowScale->addOtherComponent(taZShadowScale, Vec2f(0.75, 0.025));
    taZShadowScale->setTextSize(15);
    taZShadowScale->setParent(pShadows);
    pShadows->addChild(taZShadowScale);
    Command cmdXShadowScale(FastDelegate<bool>(&TextArea::isTextChanged, taXShadowScale), FastDelegate<void>(&ODFAEGCreator::onShadowScaleChanged, this, taXShadowScale));
    taXShadowScale->getListener().connect("XSHADOWSCALECHANGED", cmdXShadowScale);
    Command cmdYShadowScale(FastDelegate<bool>(&TextArea::isTextChanged, taYShadowScale), FastDelegate<void>(&ODFAEGCreator::onShadowScaleChanged, this, taYShadowScale));
    taYShadowScale->getListener().connect("YSHADOWSCALECHANGED", cmdYShadowScale);
    Command cmdZShadowScale(FastDelegate<bool>(&TextArea::isTextChanged, taZShadowScale), FastDelegate<void>(&ODFAEGCreator::onShadowScaleChanged, this, taZShadowScale));
    taZShadowScale->getListener().connect("ZSHADOWSCALECHANGED", cmdZShadowScale);
    //Shadow rotation angle.
    Label* lShadowRotation = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Shadow rotation : ", 15);
    Node* nShadowRotation = new Node("ShadowRotation", lShadowRotation, Vec2f(0, 0), Vec2f(1, 0.025),rootShadowsNode.get());
    lShadowRotation->setParent(pShadows);
    pShadows->addChild(lShadowRotation);
    //X
    Label* lShadowRotAngle = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "A : ", 15);
    Node* nShadowRotAngle = new Node("ShadowRotAngle", lShadowRotAngle, Vec2f(0, 0), Vec2f(0.25, 0.025), rootShadowsNode.get());
    lShadowRotAngle->setParent(pShadows);
    pShadows->addChild(lShadowRotAngle);
    taShadowRotAngle = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(tile->getShadowRotationAngle()),getRenderWindow());
    nShadowRotAngle->addOtherComponent(taShadowRotAngle, Vec2f(0.75, 0.025));
    taShadowRotAngle->setTextSize(15);
    taShadowRotAngle->setParent(pShadows);
    pShadows->addChild(taShadowRotAngle);
    Command cmdRotAngle(FastDelegate<bool>(&TextArea::isTextChanged, taShadowRotAngle), FastDelegate<void>(&ODFAEGCreator::onShadowRotAngleChanged, this, taShadowRotAngle));
    taShadowRotAngle->getListener().connect("SHADOWROTANGLECHANGED", cmdRotAngle);
    if (selectedBoundingVolume != nullptr) {
        collisionBox = RectangleShape(selectedBoundingVolume->getSize());
        collisionBox.setPosition(selectedBoundingVolume->getPosition());
        collisionBox.setFillColor(Color(255, 0, 0, 128));
        //Parent collision volume.
        //std::cout<<"parent : "<<selectedBoundingVolume->getParent()<<std::endl;
        lParentCV = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "parent : " + (selectedBoundingVolume->getParent() == nullptr) ? "NONE" : selectedBoundingVolume->getParent()->getName(), 15);
        lParentCV->setParent(pCollisions);
        pCollisions->addChild(lParentCV);
        Node* nParentCV = new Node("P Collision", lParentCV, Vec2f(0, 0), Vec2f(1, 0.025), rootCollisionNode.get());
        Command cmdCVParentClicked (a, FastDelegate<bool>(&Label::isMouseInside, lParentCV), FastDelegate<void>(&ODFAEGCreator::onParentClickedCV, this, lParentCV));
        pCollisions->getListener().removeLater("CMDCVPARENTCLICKED");
        pCollisions->getListener().connect("CMDCVPARENTCLICKED", cmdCVParentClicked);

        //Children collision volume.
        Label* lChildrenCV = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Children : ", 15);
        lChildrenCV->setParent(pCollisions);
        pCollisions->addChild(lChildrenCV);
        Node* nChildrenCV = new Node ("C Collision",lChildrenCV,Vec2f(0, 0), Vec2f(0.25, 0.025), rootCollisionNode.get());
        dpChildrenCV = new DropDownList(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "NONE", 15);
        std::vector<BoundingVolume*> bvs = selectedBoundingVolume->getChildren();
        for (unsigned int i = 0; i < bvs.size(); i++) {
            if (bvs[i]->getName() != "") {
                dpChildrenCV->addItem(bvs[i]->getName(), 15);
            }
        }
        pCollisions->addChild(dpChildrenCV);
        dpChildrenCV->setParent(pCollisions);
        nChildrenCV->addOtherComponent(dpChildrenCV, Vec2f(0.75, 0.025));
        Command cmdChildrenCV (FastDelegate<bool>(&DropDownList::isValueChanged, dpChildrenCV), FastDelegate<void>(&ODFAEGCreator::displayChildrenCV, this, dpChildrenCV));
        pCollisions->getListener().removeLater("CMDCHILDRENCV");
        pCollisions->getListener().connect("CMDCHILDRENCV", cmdChildrenCV);

        //Collision volume name.
        Label* lNameCV = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "name : ", 15);
        lNameCV->setParent(pCollisions);
        pCollisions->addChild(lNameCV);
        Node* nNameCV = new Node("NNameCV", lNameCV, Vec2f(0, 0), Vec2f(0.25, 0.025), rootCollisionNode.get());
        taNameCV = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif), "", getRenderWindow());
        taNameCV->setTextSize(15);
        pCollisions->addChild(taNameCV);
        taNameCV->setParent(pCollisions);
        nNameCV->addOtherComponent(taNameCV, Vec2f(0.75, 0.025));
        Command cmdNameCV(FastDelegate<bool>(&TextArea::isTextChanged, taNameCV), FastDelegate<void>(&ODFAEGCreator::onNameCVChanged, this, taNameCV));

        //Select parent collision volume.
        Label* lSelectParentCV = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "parent : ", 15);
        lSelectParentCV->setParent(pCollisions);
        pCollisions->addChild(lSelectParentCV);
        Node* nSelectParentCV = new Node("SelectParentCV", lSelectParentCV, Vec2f(0, 0), Vec2f(0.25, 0.025), rootCollisionNode.get());
        dpSelectParentCV = new DropDownList(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "NONE", 15);
        for (unsigned int i = 0; i < boundingVolumes.size(); i++) {
            if (boundingVolumes[i]->getName() != "") {
                dpSelectParentCV->addItem(boundingVolumes[i]->getName(), 15);
            }
        }
        pCollisions->addChild(dpSelectParentCV);
        dpSelectParentCV->setParent(pCollisions);
        nSelectParentCV->addOtherComponent(dpSelectParentCV, Vec2f(0.75, 0.025));
        Command cmdSelectParentCV(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectParentCV), FastDelegate<void>(&ODFAEGCreator::onSelectedParentCV, this, dpSelectParentCV));
        pCollisions->getListener().connect("CMDSELECTPARENTCV", cmdSelectParentCV);

        //Collision.
        Label* lCollision = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Collision box : ", 15);
        Node* nCollision = new Node("Collision", lCollision, Vec2f(0, 0), Vec2f(1, 0.025),rootCollisionNode.get());
        lCollision->setParent(pCollisions);
        pCollisions->addChild(lCollision);
        //X.
        Label* lCollisionXPos = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "X : ", 15);
        Node* nCollisionPosX = new Node("CollisionPosX", lCollisionXPos,Vec2f(0, 0), Vec2f(0.25, 0.025),rootCollisionNode.get());
        lCollisionXPos->setParent(pCollisions);
        pCollisions->addChild(lCollisionXPos);
        taBoundingBoxColX = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), (selectedBoundingVolume != nullptr) ? conversionFloatString(selectedBoundingVolume->getPosition().x()) : "0",getRenderWindow());
        taBoundingBoxColX->setTextSize(15);
        nCollisionPosX->addOtherComponent(taBoundingBoxColX, Vec2f(0.75, 0.025));
        taBoundingBoxColX->setParent(pCollisions);
        pCollisions->addChild(taBoundingBoxColX);
        //Y.
        Label* lCollisionYPos = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Y : ", 15);
        Node* nCollisionPosY = new Node("CollisionPosY", lCollisionYPos,Vec2f(0, 0), Vec2f(0.25, 0.025),rootCollisionNode.get());
        lCollisionYPos->setParent(pCollisions);
        pCollisions->addChild(lCollisionYPos);
        taBoundingBoxColY = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), (selectedBoundingVolume != nullptr) ? conversionFloatString(selectedBoundingVolume->getPosition().y()) : "0",getRenderWindow());
        taBoundingBoxColY->setTextSize(15);
        nCollisionPosY->addOtherComponent(taBoundingBoxColY, Vec2f(0.75, 0.025));
        taBoundingBoxColY->setParent(pCollisions);
        pCollisions->addChild(taBoundingBoxColY);
        //Z.
        Label* lCollisionZPos = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Z : ", 15);
        Node* nCollisionPosZ = new Node("CollisionPosZ", lCollisionZPos,Vec2f(0, 0), Vec2f(0.25, 0.025),rootCollisionNode.get());
        lCollisionZPos->setParent(pCollisions);
        pCollisions->addChild(lCollisionZPos);
        taBoundingBoxColZ = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), (selectedBoundingVolume != nullptr) ? conversionFloatString(selectedBoundingVolume->getPosition().z()) : "0",getRenderWindow());
        taBoundingBoxColZ->setTextSize(15);
        nCollisionPosZ->addOtherComponent(taBoundingBoxColZ, Vec2f(0.75, 0.025));
        taBoundingBoxColZ->setParent(pCollisions);
        pCollisions->addChild(taBoundingBoxColZ);
        //Width.
        Label* lCollisionWPos = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "W : ", 15);
        Node* nCollisionPosW = new Node("CollisionPosW", lCollisionWPos,Vec2f(0, 0), Vec2f(0.25, 0.025),rootCollisionNode.get());
        lCollisionWPos->setParent(pCollisions);
        pCollisions->addChild(lCollisionWPos);
        taBoundingBoxColW = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), (selectedBoundingVolume != nullptr) ? conversionFloatString(selectedBoundingVolume->getSize().x()) : "0",getRenderWindow());
        taBoundingBoxColW->setTextSize(15);
        nCollisionPosW->addOtherComponent(taBoundingBoxColW, Vec2f(0.75, 0.025));
        taBoundingBoxColW->setParent(pCollisions);
        pCollisions->addChild(taBoundingBoxColW);
        //Height.
        Label* lCollisionHPos = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "H : ", 15);
        Node* nCollisionPosH = new Node("CollisionPosH", lCollisionHPos,Vec2f(0, 0), Vec2f(0.25, 0.025),rootCollisionNode.get());
        lCollisionHPos->setParent(pCollisions);
        pCollisions->addChild(lCollisionHPos);
        taBoundingBoxColH = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), (selectedBoundingVolume != nullptr) ? conversionFloatString(selectedBoundingVolume->getSize().y()) : "0",getRenderWindow());
        taBoundingBoxColH->setTextSize(15);
        nCollisionPosH->addOtherComponent(taBoundingBoxColH, Vec2f(0.75, 0.025));
        taBoundingBoxColH->setParent(pCollisions);
        pCollisions->addChild(taBoundingBoxColH);
        //Depth.

        Label* lCollisionDPos = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "D : ", 15);

        Node* nCollisionPosD = new Node("CollisionPosD", lCollisionDPos,Vec2f(0, 0), Vec2f(0.25, 0.025),rootCollisionNode.get());

        lCollisionDPos->setParent(pCollisions);

        pCollisions->addChild(lCollisionDPos);


        taBoundingBoxColD = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), (tile->getCollisionVolume() != nullptr) ? conversionFloatString(tile->getCollisionVolume()->getSize().z()) : "0",getRenderWindow());

        taBoundingBoxColD->setTextSize(15);

        nCollisionPosD->addOtherComponent(taBoundingBoxColD, Vec2f(0.75, 0.025));

        taBoundingBoxColD->setParent(pCollisions);
        pCollisions->addChild(taBoundingBoxColD);

        Command cmdBBColPosX (FastDelegate<bool>(&TextArea::isTextChanged, taBoundingBoxColX), FastDelegate<void>(&ODFAEGCreator::onCollisionBoundingBoxChanged, this, taBoundingBoxColX));
        taBoundingBoxColX->getListener().connect("BBColPosXChanged", cmdBBColPosX);
        Command cmdBBColPosY (FastDelegate<bool>(&TextArea::isTextChanged, taBoundingBoxColY), FastDelegate<void>(&ODFAEGCreator::onCollisionBoundingBoxChanged, this, taBoundingBoxColY));
        taBoundingBoxColY->getListener().connect("BBColPosYChanged", cmdBBColPosY);
        Command cmdBBColPosZ (FastDelegate<bool>(&TextArea::isTextChanged, taBoundingBoxColZ), FastDelegate<void>(&ODFAEGCreator::onCollisionBoundingBoxChanged, this, taBoundingBoxColZ));
        taBoundingBoxColZ->getListener().connect("BBColPosZChanged", cmdBBColPosZ);
        Command cmdBBColPosW (FastDelegate<bool>(&TextArea::isTextChanged, taBoundingBoxColW), FastDelegate<void>(&ODFAEGCreator::onCollisionBoundingBoxChanged, this, taBoundingBoxColW));
        taBoundingBoxColW->getListener().connect("BBColPosWChanged", cmdBBColPosW);
        Command cmdBBColPosH (FastDelegate<bool>(&TextArea::isTextChanged, taBoundingBoxColH), FastDelegate<void>(&ODFAEGCreator::onCollisionBoundingBoxChanged, this, taBoundingBoxColH));
        taBoundingBoxColH->getListener().connect("BBColPosHChanged", cmdBBColPosH);
        Command cmdBBColPosD (FastDelegate<bool>(&TextArea::isTextChanged, taBoundingBoxColD), FastDelegate<void>(&ODFAEGCreator::onCollisionBoundingBoxChanged, this, taBoundingBoxColD));
        taBoundingBoxColD->getListener().connect("BBColPosDChanged", cmdBBColPosD);

        //Assigning collision volume.
        bAssignCollisionVolume = new Button(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "Assign CV", 15, getRenderWindow());
        pCollisions->addChild(bAssignCollisionVolume);
        bAssignCollisionVolume->setParent(pCollisions);
        Node* nAssignCV = new Node("AssignCV", bAssignCollisionVolume, Vec2f(0, 0), Vec2f(1, 0.025), rootCollisionNode.get());
        bAssignCollisionVolume->addActionListener(this);
    }
    if (tabPane->getSelectedTab() != "Collisions")
        pCollisions->setEventContextActivated(false);
    if (tabPane->getSelectedTab() != "Shadow")
        pShadows->setEventContextActivated(false);
    if (tabPane->getSelectedTab() != "Material")
        pMaterial->setEventContextActivated(false);


}
void ODFAEGCreator::displayExternalEntityInfo(Entity* entity) {
    displayTransformInfos(entity);
    displayEntityInfos(entity);
}
void ODFAEGCreator::displayDecorInfos(Entity* decor) {
    displayTransformInfos(decor);
    displayEntityInfos(decor);
    if (tabPane->getSelectedTab() != "Informations")
        pInfos->setEventContextActivated(false);
}
void ODFAEGCreator::displayBigtileInfos(Entity* bigTile) {
    displayTransformInfos(bigTile);
    displayEntityInfos(bigTile);
}
void ODFAEGCreator::displayAnimInfos(Entity* anim) {
    displayTransformInfos(anim);

    displayEntityInfos(anim);

    FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
    Label* lFrameRate = new Label(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"frame rate : ", 15);
    Node* frNode = new Node("FRAMERATE",lFrameRate,Vec2f(0, 0),Vec2f(0.25, 0.025),rootInfosNode.get());
    lFrameRate->setParent(pInfos);
    pInfos->addChild(lFrameRate);
    taFrameRate = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif),"",getRenderWindow());
    taFrameRate->setTextSize(15);
    taFrameRate->setParent(pInfos);
    pInfos->addChild(taFrameRate);
    frNode->addOtherComponent(taFrameRate, Vec2f(0.75, 0.025));

    Command cmdFRChanged(FastDelegate<bool> (&TextArea::isTextChanged, taFrameRate), FastDelegate<void>(&ODFAEGCreator::onFrameRateChanged, this, taFrameRate));

    taFrameRate->getListener().connect("FRCHANGED", cmdFRChanged);


    Label* lSelectAU = new Label(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"Anim updater : ", 15);
    Node* animUpdaterNode = new Node("ANIMUPDATER",lSelectAU,Vec2f(0, 0),Vec2f(0.25, 0.025),rootInfosNode.get());
    lSelectAU->setParent(pInfos);
    pInfos->addChild(lSelectAU);
    dpSelectAU = new DropDownList(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"NONE",15);
    //std::cout<<"add timers"<<std::endl;
    std::vector<Timer*> timers = getWorld()->getTimers();
    for (unsigned int i = 0; i < timers.size(); i++) {
        dpSelectAU->addItem(timers[i]->getName(), 15);
    }
    dpSelectAU->setParent(pInfos);
    pInfos->addChild(dpSelectAU);
    animUpdaterNode->addOtherComponent(dpSelectAU, Vec2f(0.75, 0.025));

    Command cmdAUChanged(FastDelegate<bool>(&DropDownList::isValueChanged,dpSelectAU), FastDelegate<void>(&ODFAEGCreator::onAnimUpdaterChanged, this, dpSelectAU));
    dpSelectAU->getListener().connect("ANIMUPDATERCHANGED", cmdAUChanged);
    if (tabPane->getSelectedTab() != "Informations")
        pInfos->setEventContextActivated(false);


}
void ODFAEGCreator::displayParticleSystemInfos(Entity* ps) {
    displayTransformInfos(ps);
    displayEntityInfos(ps);
    FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
    Label* lselectPSU = new Label(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"Particle system updater : ", 15);
    Node* psUpdaterNode = new Node("PSUPDATER",lselectPSU,Vec2f(0, 0),Vec2f(0.25, 0.025),rootInfosNode.get());
    lselectPSU->setParent(pInfos);
    pInfos->addChild(lselectPSU);
    dpSelectPSU = new DropDownList(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"NONE",15);
    std::vector<EntitySystem*> workers = getWorld()->getWorkers();
    for (unsigned int i = 0; i < workers.size(); i++) {
        if (dynamic_cast<ParticleSystemUpdater*>(workers[i]))
            dpSelectPSU->addItem(workers[i]->getName(), 15);
    }
    dpSelectPSU->setParent(pInfos);
    pInfos->addChild(dpSelectPSU);
    psUpdaterNode->addOtherComponent(dpSelectPSU, Vec2f(0.75, 0.025));
    Command cmdPSUChanged(FastDelegate<bool>(&DropDownList::isValueChanged,dpSelectPSU), FastDelegate<void>(&ODFAEGCreator::onParticleSystemUpdaterChanged, this, dpSelectPSU));
    dpSelectPSU->getListener().connect("PSUUPDATERCHANGED", cmdPSUChanged);
    lTexture = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif),"Texture : ", 15);
    lTexture->setParent(pMaterial);
    Node* lTextureNode = new Node("LabTexture",lTexture,Vec2f(0, 0), Vec2f(1.f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lTexture);
    lTexCoordX = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Tex x : ", 15);
    lTexCoordX->setParent(pMaterial);
    Node* lTexCoordXNode = new Node("LTexCoordX", lTexCoordX,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lTexCoordX);
    tTexCoordX = new TextArea (Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"0",getRenderWindow());
    tTexCoordX->setTextSize(15);
    tTexCoordX->setParent(pMaterial);
    lTexCoordXNode->addOtherComponent(tTexCoordX,Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tTexCoordX);
    lTexCoordY = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Tex y : ", 15);
    lTexCoordY->setParent(pMaterial);
    Node* lTexCoordYNode = new Node("LTexCoordY", lTexCoordY,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lTexCoordY);
    tTexCoordY = new TextArea (Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"0",getRenderWindow());
    tTexCoordY->setTextSize(15);
    tTexCoordY->setParent(pMaterial);
    lTexCoordYNode->addOtherComponent(tTexCoordY,Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tTexCoordY);
    lTexCoordW = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Tex w : ", 15);
    lTexCoordW->setParent(pMaterial);
    Node* lTexCoordWNode = new Node("lTexCoordW", lTexCoordW,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lTexCoordW);
    tTexCoordW = new TextArea (Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"0",getRenderWindow());
    tTexCoordW->setTextSize(15);
    tTexCoordW->setParent(pMaterial);
    lTexCoordWNode->addOtherComponent(tTexCoordW,Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tTexCoordW);
    lTexCoordH = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Tex h : ", 15);
    lTexCoordH->setParent(pMaterial);
    Node* lTexCoordHNode = new Node("LTexCoordH", lTexCoordH,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lTexCoordH);
    tTexCoordH = new TextArea (Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"0",getRenderWindow());
    tTexCoordH->setTextSize(15);
    tTexCoordH->setParent(pMaterial);
    lTexCoordHNode->addOtherComponent(tTexCoordH,Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tTexCoordH);
    Command cmdTexCoordXChanged (FastDelegate<bool>(&TextArea::isTextChanged,tTexCoordX), FastDelegate<void>(&ODFAEGCreator::onTexCoordsChanged, this, tTexCoordX));
    tTexCoordX->getListener().connect("TTexCoordXChanged", cmdTexCoordXChanged);
    Command cmdTexCoordYChanged (FastDelegate<bool>(&TextArea::isTextChanged,tTexCoordY), FastDelegate<void>(&ODFAEGCreator::onTexCoordsChanged, this, tTexCoordY));
    tTexCoordY->getListener().connect("TTexCoordYChanged", cmdTexCoordYChanged);
    Command cmdTexCoordWChanged (FastDelegate<bool>(&TextArea::isTextChanged,tTexCoordW), FastDelegate<void>(&ODFAEGCreator::onTexCoordsChanged, this, tTexCoordW));
    tTexCoordW->getListener().connect("TTexCoordWChanged", cmdTexCoordWChanged);
    Command cmdTexCoordHChanged (FastDelegate<bool>(&TextArea::isTextChanged,tTexCoordH), FastDelegate<void>(&ODFAEGCreator::onTexCoordsChanged, this, tTexCoordH));
    tTexCoordH->getListener().connect("TTexCoordXChanged", cmdTexCoordHChanged);
    lTexImage = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif), "Tex Image : ", 15);
    lTexImage->setParent(pMaterial);
    Node* selectTextNode = new Node("SelectTexture",lTexImage,Vec2f(0, 0),Vec2f(0.25f, 0.025f), rootMaterialNode.get());
    pMaterial->addChild(lTexImage);
    dpSelectTexture = new DropDownList(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif),"NONE", 15);
    dpSelectTexture->setName("SELECTTEXT");
    dpSelectTexture->setParent(pMaterial);
    TextureManager<>& tm = cache.resourceManager<Texture, std::string>("TextureManager");
    std::vector<std::string> paths = tm.getPaths();
    for (unsigned int i = 0; i < paths.size(); i++) {
        std::vector<std::string> alias = tm.getAliasByResource(const_cast<Texture*>(tm.getResourceByPath(paths[i])));
        dpSelectTexture->addItem(alias[0], 15);
    }
    Command cmdTxtChanged(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectTexture), FastDelegate<void>(&ODFAEGCreator::onSelectedTextureChanged, this, dpSelectTexture));
    dpSelectTexture->getListener().connect("TextureChanged", cmdTxtChanged);
    Command cmdTxtDroppedDown (FastDelegate<bool>(&DropDownList::isDroppedDown, dpSelectTexture), FastDelegate<void>(&ODFAEGCreator::onSelectedTextureDroppedDown, this, dpSelectTexture));
    dpSelectTexture->getListener().connect("TextureDroppedDown", cmdTxtDroppedDown);
    Command cmdTxtNotDroppedDown(FastDelegate<bool>(&DropDownList::isNotDroppedDown, dpSelectTexture), FastDelegate<void>(&ODFAEGCreator::onSelectedTextureNotDroppedDown, this, dpSelectTexture));
    dpSelectTexture->getListener().connect("TextureNotDroppedDown", cmdTxtNotDroppedDown);
    selectTextNode->addOtherComponent(dpSelectTexture,Vec2f(0.75f, 0.025f));
    pMaterial->addChild(dpSelectTexture);
    lTexId = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 100, 0),fm.getResourceByAlias(Fonts::Serif),"Tex id : ", 15);
    lTexId->setParent(pMaterial);
    pMaterial->addChild(lTexId);
    Node* texIdNode = new Node("TexId", lTexId,Vec2f(0, 0), Vec2f(0.5f, 0.025f), rootMaterialNode.get());
    tTexId = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 100, 0), fm.getResourceByAlias(Fonts::Serif), "", getRenderWindow());
    tTexId->setParent(pMaterial);
    pMaterial->addChild(tTexId);
    texIdNode->addOtherComponent(tTexId, Vec2f(0.5f, 0.025f));
    bChooseText = new Button(Vec3f(0, 0, 0), Vec3f(100, 100, 0), fm.getResourceByAlias(Fonts::Serif),"New texture", 15, getRenderWindow());
    bChooseText->setParent(pMaterial);
    Node* chooseTextNode = new Node("ChooseText", bChooseText,Vec2f(0, 0), Vec2f(0.5f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(bChooseText);
    bChooseText->setName("CHOOSETEXT");
    bChooseText->addActionListener(this);
    bAddTexRect = new Button(Vec3f(0, 0, 0), Vec3f(100, 100, 0), fm.getResourceByAlias(Fonts::Serif),"Add tex rect", 15, getRenderWindow());
    bAddTexRect->setParent(pMaterial);
    pMaterial->addChild(bAddTexRect);
    chooseTextNode->addOtherComponent(bAddTexRect,Vec2f(0.5f, 0.025f));
    bAddTexRect->addActionListener(this);
}
void ODFAEGCreator::displayPonctualLightInfos(Entity* pl) {
    displayTransformInfos(pl);
    displayEntityInfos(pl);
    FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
    //Intensity.
    Label* lIntensity = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif), "Intensity : ", 15);
    Node* nIntensity = new Node("Intensity",lIntensity,Vec2f(0, 0),Vec2f(0.25, 0.025),rootInfosNode.get());
    lIntensity->setParent(pInfos);
    pInfos->addChild(lIntensity);
    taIntensity = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionIntString(pl->getIntensity()),getRenderWindow());
    taIntensity->setTextSize(15);
    nIntensity->addOtherComponent(taIntensity, Vec2f(0.75, 0.025));
    taIntensity->setParent(pInfos);
    pInfos->addChild(taIntensity);
    //Quality.
    Label* lQuality = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif), "Quality : ", 15);
    Node* nQuality = new Node("Quality",lQuality,Vec2f(0, 0),Vec2f(0.25, 0.025),rootInfosNode.get());
    lQuality->setParent(pInfos);
    pInfos->addChild(lQuality);
    taQuality = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"16",getRenderWindow());
    taQuality->setTextSize(15);
    nQuality->addOtherComponent(taQuality, Vec2f(0.75, 0.025));
    taQuality->setParent(pInfos);
    pInfos->addChild(taQuality);
    //Color.
    lColor = new Label(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"Color : ", 15);
    lColor->setParent(pMaterial);
    Node* lColorNode = new Node("LabColor",lColor,Vec2f(0, 0), Vec2f(1.f, 0.025f), rootMaterialNode.get());
    pMaterial->addChild(lColor);
    lRColor = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"r : ", 15);
    lRColor->setParent(pMaterial);
    Node* lRColorNode = new Node("LabRColor",lRColor,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lRColor);
    tRColor = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(pl->getColor().r), getRenderWindow());
    tRColor->setTextSize(15);
    tRColor->setParent(pMaterial);
    lRColorNode->addOtherComponent(tRColor, Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tRColor);
    lGColor = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"g : ", 15);
    lGColor->setParent(pMaterial);
    Node* lGColorNode = new Node("LabRColor",lGColor,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lGColor);
    tGColor = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(pl->getColor().g), getRenderWindow());
    tGColor->setTextSize(15);
    tGColor->setParent(pMaterial);
    lGColorNode->addOtherComponent(tGColor, Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tGColor);
    lBColor = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"b : ", 15);
    lBColor->setParent(pMaterial);
    Node* lBColorNode = new Node("LabBColor",lBColor,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lBColor);
    tBColor = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(pl->getColor().b), getRenderWindow());
    tBColor->setTextSize(15);
    tBColor->setParent(pMaterial);
    lBColorNode->addOtherComponent(tBColor, Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tBColor);
    lAColor = new Label(getRenderWindow(),Vec3f(0, 0, 0), Vec3f(200, 17, 0),fm.getResourceByAlias(Fonts::Serif),"a : ", 15);
    lAColor->setParent(pMaterial);
    Node* lAColorNode = new Node("LabAColor",lAColor,Vec2f(0, 0), Vec2f(0.25f, 0.025f),rootMaterialNode.get());
    pMaterial->addChild(lAColor);
    tAColor = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),conversionFloatString(pl->getColor().a), getRenderWindow());
    tAColor->setTextSize(15);
    tAColor->setParent(pMaterial);
    lAColorNode->addOtherComponent(tAColor,Vec2f(0.75f, 0.025f));
    pMaterial->addChild(tAColor);
    Command cmdRColChanged(FastDelegate<bool>(&TextArea::isTextChanged, tRColor), FastDelegate<void>(&ODFAEGCreator::onObjectColorChanged, this, tRColor));
    tRColor->getListener().connect("TRColorChanged", cmdRColChanged);
    Command cmdGColChanged(FastDelegate<bool>(&TextArea::isTextChanged, tGColor), FastDelegate<void>(&ODFAEGCreator::onObjectColorChanged, this, tGColor));
    tGColor->getListener().connect("TGColorChanged", cmdGColChanged);
    Command cmdBColChanged(FastDelegate<bool>(&TextArea::isTextChanged, tBColor), FastDelegate<void>(&ODFAEGCreator::onObjectColorChanged, this, tBColor));
    tBColor->getListener().connect("TBColorChanged", cmdBColChanged);
    Command cmdAColChanged(FastDelegate<bool>(&TextArea::isTextChanged, tAColor), FastDelegate<void>(&ODFAEGCreator::onObjectColorChanged, this, tAColor));
    tAColor->getListener().connect("TAColorChanged", cmdAColChanged);
}
void ODFAEGCreator::displayWallInfos(Entity* wall) {
    displayTransformInfos(wall);
    displayEntityInfos(wall);
    FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
    Label* lType = new Label(getRenderWindow(),Vec3f(0, 0, 0),Vec3f(100, 50, 0),fm.getResourceByAlias(Fonts::Serif),"type : ",15);
    lType->setParent(pInfos);
    pInfos->addChild(lType);
    Node* node = new Node("WALLTYPE",lType,Vec2f(0, 0),Vec2f(0.25, 0.025),rootInfosNode.get());
    dpWallType = new DropDownList(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "TOP_LEFT", 15);
    dpWallType->addItem("TOP_RIGHT", 15);
    dpWallType->addItem("BOTTOM_RIGHT", 15);
    dpWallType->addItem("BOTTOM_LEFT", 15);
    dpWallType->addItem("TOP_BOTTOM", 15);
    dpWallType->addItem("RIGHT_LEFT", 15);
    dpWallType->addItem("T_TOP", 15);
    dpWallType->addItem("T_RIGHT", 15);
    dpWallType->addItem("T_LEFT", 15);
    dpWallType->addItem("T_BOTTOM", 15);
    dpWallType->addItem("X", 15);

    Command cmdWallType(FastDelegate<bool>(&DropDownList::isValueChanged, dpWallType), FastDelegate<void>(&ODFAEGCreator::onWallTypeChanged,this,dpWallType));
    dpWallType->getListener().connect("WallTypeChanged", cmdWallType);
    dpWallType->setParent(pInfos);
    pInfos->addChild(dpWallType);
    node->addOtherComponent(dpWallType, Vec2f(0.75, 0.025));
    if (tabPane->getSelectedTab() != "Informations")
        pInfos->setEventContextActivated(false);
}
void ODFAEGCreator::displayTileInfos (Entity* tile) {
    try {
        //std::cout << "tile : " << tile << std::endl;
        displayTransformInfos(tile);
        displayEntityInfos(tile);
        FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
        lColor = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "Color : ", 15);
        lColor->setParent(pMaterial);
        Node* lColorNode = new Node("LabColor", lColor, Vec2f(0, 0), Vec2f(1.f, 0.025f), rootMaterialNode.get());
        pMaterial->addChild(lColor);
        lRColor = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "r : ", 15);
        lRColor->setParent(pMaterial);
        Node* lRColorNode = new Node("LabRColor", lRColor, Vec2f(0, 0), Vec2f(0.25f, 0.025f), rootMaterialNode.get());
        pMaterial->addChild(lRColor);
        tRColor = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), conversionFloatString(tile->getColor().r), getRenderWindow());
        tRColor->setTextSize(15);
        tRColor->setParent(pMaterial);
        lRColorNode->addOtherComponent(tRColor, Vec2f(0.75f, 0.025f));
        pMaterial->addChild(tRColor);
        lGColor = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "g : ", 15);
        lGColor->setParent(pMaterial);
        Node* lGColorNode = new Node("LabRColor", lGColor, Vec2f(0, 0), Vec2f(0.25f, 0.025f), rootMaterialNode.get());
        pMaterial->addChild(lGColor);
        tGColor = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), conversionFloatString(tile->getColor().g), getRenderWindow());
        tGColor->setTextSize(15);
        tGColor->setParent(pMaterial);
        lGColorNode->addOtherComponent(tGColor, Vec2f(0.75f, 0.025f));
        pMaterial->addChild(tGColor);
        lBColor = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "b : ", 15);
        lBColor->setParent(pMaterial);
        Node* lBColorNode = new Node("LabBColor", lBColor, Vec2f(0, 0), Vec2f(0.25f, 0.025f), rootMaterialNode.get());
        pMaterial->addChild(lBColor);
        tBColor = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), conversionFloatString(tile->getColor().b), getRenderWindow());
        tBColor->setTextSize(15);
        tBColor->setParent(pMaterial);
        lBColorNode->addOtherComponent(tBColor, Vec2f(0.75f, 0.025f));
        pMaterial->addChild(tBColor);
        lAColor = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "a : ", 15);
        lAColor->setParent(pMaterial);
        Node* lAColorNode = new Node("LabAColor", lAColor, Vec2f(0, 0), Vec2f(0.25f, 0.025f), rootMaterialNode.get());
        pMaterial->addChild(lAColor);
        tAColor = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), conversionFloatString(tile->getColor().a), getRenderWindow());
        tAColor->setTextSize(15);
        tAColor->setParent(pMaterial);
        lAColorNode->addOtherComponent(tAColor, Vec2f(0.75f, 0.025f));
        pMaterial->addChild(tAColor);
        Command cmdRColChanged(FastDelegate<bool>(&TextArea::isTextChanged, tRColor), FastDelegate<void>(&ODFAEGCreator::onObjectColorChanged, this, tRColor));
        tRColor->getListener().connect("TRColorChanged", cmdRColChanged);
        Command cmdGColChanged(FastDelegate<bool>(&TextArea::isTextChanged, tGColor), FastDelegate<void>(&ODFAEGCreator::onObjectColorChanged, this, tGColor));
        tGColor->getListener().connect("TGColorChanged", cmdGColChanged);
        Command cmdBColChanged(FastDelegate<bool>(&TextArea::isTextChanged, tBColor), FastDelegate<void>(&ODFAEGCreator::onObjectColorChanged, this, tBColor));
        tBColor->getListener().connect("TBColorChanged", cmdBColChanged);
        Command cmdAColChanged(FastDelegate<bool>(&TextArea::isTextChanged, tAColor), FastDelegate<void>(&ODFAEGCreator::onObjectColorChanged, this, tAColor));
        tAColor->getListener().connect("TAColorChanged", cmdAColChanged);
        lTexture = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "Texture : ", 15);
        lTexture->setParent(pMaterial);
        Node* lTextureNode = new Node("LabTexture", lTexture, Vec2f(0, 0), Vec2f(1.f, 0.025f), rootMaterialNode.get());
        pMaterial->addChild(lTexture);
        lTexCoordX = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "Tex x : ", 15);
        lTexCoordX->setParent(pMaterial);
        Node* lTexCoordXNode = new Node("LTexCoordX", lTexCoordX, Vec2f(0, 0), Vec2f(0.25f, 0.025f), rootMaterialNode.get());
        pMaterial->addChild(lTexCoordX);
        pMaterial->setName("PMATERIAL");
        tTexCoordX = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "0", getRenderWindow());
        tTexCoordX->setTextSize(15);
        tTexCoordX->setParent(pMaterial);
        lTexCoordXNode->addOtherComponent(tTexCoordX, Vec2f(0.75f, 0.025f));
        pMaterial->addChild(tTexCoordX);
        lTexCoordY = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "Tex y : ", 15);
        lTexCoordY->setParent(pMaterial);
        Node* lTexCoordYNode = new Node("LTexCoordY", lTexCoordY, Vec2f(0, 0), Vec2f(0.25f, 0.025f), rootMaterialNode.get());
        pMaterial->addChild(lTexCoordY);
        tTexCoordY = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "0", getRenderWindow());
        tTexCoordY->setTextSize(15);
        tTexCoordY->setParent(pMaterial);
        tTexCoordX->setName("TEXCOORDY");
        lTexCoordYNode->addOtherComponent(tTexCoordY, Vec2f(0.75f, 0.025f));
        pMaterial->addChild(tTexCoordY);
        lTexCoordW = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "Tex w : ", 15);
        lTexCoordW->setParent(pMaterial);
        Node* lTexCoordWNode = new Node("lTexCoordW", lTexCoordW, Vec2f(0, 0), Vec2f(0.25f, 0.025f), rootMaterialNode.get());
        pMaterial->addChild(lTexCoordW);
        tTexCoordW = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "0", getRenderWindow());
        tTexCoordW->setTextSize(15);
        tTexCoordW->setParent(pMaterial);
        lTexCoordWNode->addOtherComponent(tTexCoordW, Vec2f(0.75f, 0.025f));
        pMaterial->addChild(tTexCoordW);
        lTexCoordH = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "Tex h : ", 15);
        lTexCoordH->setParent(pMaterial);
        Node* lTexCoordHNode = new Node("LTexCoordH", lTexCoordH, Vec2f(0, 0), Vec2f(0.25f, 0.025f), rootMaterialNode.get());
        pMaterial->addChild(lTexCoordH);
        tTexCoordH = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "0", getRenderWindow());
        tTexCoordH->setTextSize(15);
        tTexCoordH->setParent(pMaterial);
        lTexCoordHNode->addOtherComponent(tTexCoordH, Vec2f(0.75f, 0.025f));
        pMaterial->addChild(tTexCoordH);
        Command cmdTexCoordXChanged(FastDelegate<bool>(&TextArea::isTextChanged, tTexCoordX), FastDelegate<void>(&ODFAEGCreator::onTexCoordsChanged, this, tTexCoordX));
        tTexCoordX->getListener().connect("TTexCoordXChanged", cmdTexCoordXChanged);
        Command cmdTexCoordYChanged(FastDelegate<bool>(&TextArea::isTextChanged, tTexCoordY), FastDelegate<void>(&ODFAEGCreator::onTexCoordsChanged, this, tTexCoordY));
        tTexCoordY->getListener().connect("TTexCoordYChanged", cmdTexCoordYChanged);
        Command cmdTexCoordWChanged(FastDelegate<bool>(&TextArea::isTextChanged, tTexCoordW), FastDelegate<void>(&ODFAEGCreator::onTexCoordsChanged, this, tTexCoordW));
        tTexCoordW->getListener().connect("TTexCoordWChanged", cmdTexCoordWChanged);
        Command cmdTexCoordHChanged(FastDelegate<bool>(&TextArea::isTextChanged, tTexCoordH), FastDelegate<void>(&ODFAEGCreator::onTexCoordsChanged, this, tTexCoordH));
        tTexCoordH->getListener().connect("TTexCoordXChanged", cmdTexCoordHChanged);
        lTexImage = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(200, 17, 0), fm.getResourceByAlias(Fonts::Serif), "Tex Image : ", 15);
        lTexImage->setParent(pMaterial);
        Node* selectTextNode = new Node("SelectTexture", lTexImage, Vec2f(0, 0), Vec2f(0.25f, 0.025f), rootMaterialNode.get());
        pMaterial->addChild(lTexImage);
        dpSelectTexture = new DropDownList(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 50, 0), fm.getResourceByAlias(Fonts::Serif), "NONE", 15);
        dpSelectTexture->setName("SELECTTEXT");
        dpSelectTexture->setParent(pMaterial);
        TextureManager<>& tm = cache.resourceManager<Texture, std::string>("TextureManager");
        std::vector<std::string> paths = tm.getPaths();
        for (unsigned int i = 0; i < paths.size(); i++) {
            std::vector<std::string> alias = tm.getAliasByResource(const_cast<Texture*>(tm.getResourceByPath(paths[i])));
            dpSelectTexture->addItem(alias[0], 15);
        }
        Command cmdTxtChanged(FastDelegate<bool>(&DropDownList::isValueChanged, dpSelectTexture), FastDelegate<void>(&ODFAEGCreator::onSelectedTextureChanged, this, dpSelectTexture));
        dpSelectTexture->getListener().connect("TextureChanged", cmdTxtChanged);
        Command cmdTxtDroppedDown(FastDelegate<bool>(&DropDownList::isDroppedDown, dpSelectTexture), FastDelegate<void>(&ODFAEGCreator::onSelectedTextureDroppedDown, this, dpSelectTexture));
        dpSelectTexture->getListener().connect("TextureDroppedDown", cmdTxtDroppedDown);
        Command cmdTxtNotDroppedDown(FastDelegate<bool>(&DropDownList::isNotDroppedDown, dpSelectTexture), FastDelegate<void>(&ODFAEGCreator::onSelectedTextureNotDroppedDown, this, dpSelectTexture));
        dpSelectTexture->getListener().connect("TextureNotDroppedDown", cmdTxtNotDroppedDown);
        selectTextNode->addOtherComponent(dpSelectTexture, Vec2f(0.75f, 0.025f));
        pMaterial->addChild(dpSelectTexture);
        lTexId = new Label(getRenderWindow(), Vec3f(0, 0, 0), Vec3f(100, 100, 0),fm.getResourceByAlias(Fonts::Serif),"Tex id : ", 15);
        lTexId->setParent(pMaterial);
        pMaterial->addChild(lTexId);
        Node* texIdNode = new Node("TexId", lTexId,Vec2f(0, 0), Vec2f(0.5f, 0.025f), rootMaterialNode.get());
        tTexId = new TextArea(Vec3f(0, 0, 0), Vec3f(100, 100, 0), fm.getResourceByAlias(Fonts::Serif), "", getRenderWindow());
        tTexId->setParent(pMaterial);
        pMaterial->addChild(tTexId);
        texIdNode->addOtherComponent(tTexId, Vec2f(0.5f, 0.025f));

        bChooseText = new Button(Vec3f(0, 0, 0), Vec3f(100, 100, 0), fm.getResourceByAlias(Fonts::Serif), "New texture", 15, getRenderWindow());
        bChooseText->setParent(pMaterial);
        Node* chooseTextNode = new Node("ChooseText", bChooseText, Vec2f(0, 0), Vec2f(0.5f, 0.025f), rootMaterialNode.get());
        pMaterial->addChild(bChooseText);
        bChooseText->setName("CHOOSETEXT");
        bChooseText->addActionListener(this);
        if (isGeneratingTerrain || isGenerating3DTerrain) {
            bSetTexRect = new Button(Vec3f(0, 0, 0), Vec3f(100, 100, 0), fm.getResourceByAlias(Fonts::Serif),"Set tex rect", 15, getRenderWindow());
            bSetTexRect->setParent(pMaterial);
            pMaterial->addChild(bSetTexRect);
            chooseTextNode->addOtherComponent(bSetTexRect,Vec2f(0.5f, 0.025f));
            bSetTexRect->addActionListener(this);
        }
        StateGroup* sg = new StateGroup("SGADDRECTANGLETILE");
        State* stAddRemoveShape = new State("ADDREMOVETILE", &se);
        std::ostringstream oss;
        OTextArchive ota(oss);
        int id = tile->getId();
        ota(id);
        Entity* root = tile->getRootEntity();
        ota(root);
        stAddRemoveShape->addParameter("ADDREMOVETILE", oss.str());
        sg->addState(stAddRemoveShape);
        stateStack.addStateGroup(sg);
        pScriptsFiles->setAutoResized(true);
        if (tabPane->getSelectedTab() != "Informations")
            pInfos->setEventContextActivated(false);
        if (tabPane->getSelectedTab() != "Material")
            pMaterial->setEventContextActivated(false);
    }
    catch (Erreur& erreur) {
        std::cerr << erreur.what() << std::endl;
    }
}
void ODFAEGCreator::moveCursor(Vec2f mousePos) {
    BoundingBox bb (guiPos.x(), guiPos.y(), guiPos.z(), guiSize.x(), guiSize.y(), guiSize.z());
    //std::cout<<"move cursor"<<std::endl;
    if (bb.isPointInside(Vec3f(mousePos.x(), mousePos.y(), 0))) {

        if (getWorld() != nullptr && getWorld()->getCurrentSceneManager() == nullptr) {
            int x = mousePos.x()-getRenderWindow().getView().getSize().x() * 0.5f;
            int y = mousePos.y()-getRenderWindow().getView().getSize().y() * 0.5f;
            cursor.setPosition(Vec3f(x, y, 0));
        } else {
            if (!alignToGrid) {
                int x = mousePos.x()-getRenderWindow().getView().getSize().x() * 0.5f;
                int y = mousePos.y()-getRenderWindow().getView().getSize().y() * 0.5f;
                cursor.setPosition(Vec3f(x, y, 0));
            } else {
                int x = ((int) mousePos.x()/gridWidth*gridWidth)-getRenderWindow().getView().getSize().x() * 0.5f;
                int y = ((int) mousePos.y()/gridHeight*gridHeight)-getRenderWindow().getView().getSize().y() * 0.5f;
                cursor.setPosition(Vec3f(x, y, 0));
            }
        }
    }
}
/*void ODFAEGCreator::updateScriptPos(Transformable* shape) {
    std::map<std::string, std::string>::iterator it;
    it = cppAppliContent.find(minAppliname+".cpp");
    if (it != cppAppliContent.end()) {
        std::string& content = it->second;
        if (dynamic_cast<Shape*>(shape)) {
            if(content.find("shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+"->setPosition") == std::string::npos) {
                unsigned int pos = content.find("shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+" = std::make_unique<RectangleShape>");
                if (pos != std::string::npos) {
                    std::string subs = content.substr(pos);
                    pos += subs.find_first_of('\n') + 1;
                    content.insert(pos,"    shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+"->setPosition(Vec3f("+conversionIntString(shape->getPosition().x())+","
                    +conversionIntString(shape->getPosition().y())+","+conversionIntString(shape->getPosition().z())+");\n");
                }
            } else {
                unsigned int pos = content.find("shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+"->setPosition");
                if (pos != std::string::npos) {
                    std::string subs = content.substr(pos);
                    unsigned int endpos = subs.find_first_of('\n') + pos + 1;
                    content.erase(pos, endpos - pos);
                    content.insert(pos,"shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+"->setPosition(Vec3f("+conversionIntString(shape->getPosition().x())+","
                    +conversionIntString(shape->getPosition().y())+","+conversionIntString(shape->getPosition().z())+"));\n");
                }
            }
        }
        if (dynamic_cast<Tile*>(shape)) {
            if(content.find("tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+"->setPosition") == std::string::npos) {
                unsigned int pos = content.find("tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+" = new Tile");
                if (pos != std::string::npos) {
                    std::string subs = content.substr(pos);
                    pos += subs.find_first_of('\n') + 1;
                    content.insert(pos,"    tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+"->setPosition(Vec3f("+conversionIntString(shape->getPosition().x())+","
                    +conversionIntString(shape->getPosition().y())+","+conversionIntString(shape->getPosition().z())+");\n");
                }
            } else {
                unsigned int pos = content.find("tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+"->setPosition");
                if (pos != std::string::npos) {
                    std::string subs = content.substr(pos);
                    unsigned int endpos = subs.find_first_of('\n') + pos + 1;
                    content.erase(pos, endpos - pos);
                    content.insert(pos,"tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+"->setPosition(Vec3f("+conversionIntString(shape->getPosition().x())+","
                    +conversionIntString(shape->getPosition().y())+","+conversionIntString(shape->getPosition().z())+"));\n");
                }
            }
        }
    }
}*/
void ODFAEGCreator::onShadowScaleChanged(TextArea* ta) {
    if (ta == taXShadowScale) {
        if (is_number(ta->getText())) {
            float xShadScale = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEXSHADSCALE"+conversionFloatString(xShadScale));
            State* state = new State("SCHANGEXSHADSCALE", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getShadowScale().x());
            state->addParameter("NEWVALUE", xShadScale);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            selectedObject->setShadowScale(Vec3f(xShadScale, selectedObject->getShadowScale().y(), selectedObject->getShadowScale().z()));
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEXSHADSCALE", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getShadowScale().x());
                selectState->addParameter("NEWVALUE", xShadScale);
                rectSelect.getItems()[i]->setShadowScale(Vec3f(xShadScale, rectSelect.getItems()[i]->getShadowScale().y(), rectSelect.getItems()[i]->getShadowScale().z()));
                sg->addState(selectState);
            }
        }
    }
    if (ta == taYShadowScale) {
        if (is_number(ta->getText())) {
            float yShadScale = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEYSHADSCALE"+conversionFloatString(yShadScale));
            State* state = new State("SCHANGEYSHADSCALE", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getShadowScale().y());
            state->addParameter("NEWVALUE", yShadScale);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            selectedObject->setShadowScale(Vec3f(selectedObject->getShadowScale().x(), yShadScale, selectedObject->getShadowScale().z()));
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEYSHADSCALE", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getShadowScale().y());
                selectState->addParameter("NEWVALUE", yShadScale);
                rectSelect.getItems()[i]->setShadowScale(Vec3f(rectSelect.getItems()[i]->getShadowScale().x(), yShadScale, rectSelect.getItems()[i]->getShadowScale().z()));
                sg->addState(selectState);
            }
        }
    }
    if (ta == taZShadowScale) {
        if (is_number(ta->getText())) {
            float zShadScale = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEZSHADSCALE"+conversionFloatString(zShadScale));
            State* state = new State("SCHANGEZSHADSCALE", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getShadowScale().z());
            state->addParameter("NEWVALUE", zShadScale);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            selectedObject->setShadowScale(Vec3f(selectedObject->getShadowScale().x(), selectedObject->getShadowScale().y(), zShadScale));
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEZSHADSCALE", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getShadowScale().z());
                selectState->addParameter("NEWVALUE", zShadScale);
                rectSelect.getItems()[i]->setShadowScale(Vec3f(rectSelect.getItems()[i]->getShadowScale().x(), rectSelect.getItems()[i]->getShadowScale().y(), zShadScale));
                sg->addState(selectState);
            }
        }
    }
}
void ODFAEGCreator::onShadowRotAngleChanged(TextArea* ta) {
    if (ta == taShadowRotAngle) {
        if (is_number(ta->getText())) {
            float rotAngle = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGESHADROTANGLE"+conversionFloatString(rotAngle));
            State* state = new State("SGCHANGESHADROTANGLE", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getShadowRotationAngle());
            state->addParameter("NEWVALUE", rotAngle);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            selectedObject->setShadowRotation(rotAngle);
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SGCHANGESHADROTANGLE", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getShadowRotationAngle());
                selectState->addParameter("NEWVALUE", rotAngle);
                rectSelect.getItems()[i]->setShadowRotation(rotAngle);
                sg->addState(selectState);
            }
        }
    }
}
void ODFAEGCreator::onShadowCenterChanged(TextArea* ta) {
    if (ta == taXShadowCenter) {
        if (is_number(ta->getText())) {
            float xShadCenter = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEXSHADCENTER"+conversionFloatString(xShadCenter));
            State* state = new State("SCHANGEXSHADCENTER", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getShadowCenter().x());
            state->addParameter("NEWVALUE", xShadCenter);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            selectedObject->setShadowCenter(Vec3f(xShadCenter, selectedObject->getShadowCenter().y(), selectedObject->getShadowCenter().z()));
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEXSHADCENTER", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getShadowCenter().x());
                selectState->addParameter("NEWVALUE", xShadCenter);
                rectSelect.getItems()[i]->setShadowCenter(Vec3f(xShadCenter, rectSelect.getItems()[i]->getShadowCenter().y(), rectSelect.getItems()[i]->getShadowCenter().z()));
                sg->addState(selectState);
            }
        }
    } else if (ta == taYShadowCenter) {
        if(is_number(ta->getText())) {
            float yShadCenter = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEYSHADCENTER"+conversionFloatString(yShadCenter));
            State* state = new State("SCHANGEYSHADCENTER", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getShadowCenter().y());
            state->addParameter("NEWVALUE", yShadCenter);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            selectedObject->setShadowCenter(Vec3f(selectedObject->getShadowCenter().x(), yShadCenter, selectedObject->getShadowCenter().z()));
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEYSHADCENTER", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getShadowCenter().x());
                selectState->addParameter("NEWVALUE", yShadCenter);
                rectSelect.getItems()[i]->setShadowCenter(Vec3f(rectSelect.getItems()[i]->getShadowCenter().x(), yShadCenter, rectSelect.getItems()[i]->getShadowCenter().z()));
                sg->addState(selectState);
            }
        }
    } else if (ta == taZShadowCenter) {
        if(is_number(ta->getText())) {
            float zShadCenter = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEZSHADCENTER"+conversionFloatString(zShadCenter));
            State* state = new State("SCHANGEZSHADCENTER", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getShadowCenter().z());
            state->addParameter("NEWVALUE", zShadCenter);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            selectedObject->setShadowCenter(Vec3f(selectedObject->getShadowCenter().x(), selectedObject->getShadowCenter().y(), zShadCenter));
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEYSHADCENTER", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getShadowCenter().z());
                selectState->addParameter("NEWVALUE", zShadCenter);
                rectSelect.getItems()[i]->setShadowCenter(Vec3f(rectSelect.getItems()[i]->getShadowCenter().x(), rectSelect.getItems()[i]->getShadowCenter().y(), zShadCenter));
                sg->addState(selectState);
            }
        }
    }
}
void ODFAEGCreator::onObjectPosChanged(TextArea* ta) {
    if (ta == tPosX) {
        if (is_number(ta->getText())) {
            float newXPos = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEXPOS"+conversionFloatString(newXPos));
            State* state = new State("SCHANGEXPOS", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getPosition().x());
            state->addParameter("NEWVALUE", newXPos);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            getWorld()->removeEntity(selectedObject);
            selectedObject->setPosition(Vec3f(newXPos, selectedObject->getPosition().y(), selectedObject->getPosition().z()));
            getWorld()->addEntity(selectedObject);
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEXPOS", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getPosition().x());
                selectState->addParameter("NEWVALUE", newXPos);
                getWorld()->removeEntity(rectSelect.getItems()[i]);
                rectSelect.getItems()[i]->setPosition(Vec3f(newXPos, rectSelect.getItems()[i]->getPosition().y(), rectSelect.getItems()[i]->getPosition().z()));
                getWorld()->addEntity(rectSelect.getItems()[i]);
                sg->addState(selectState);
            }
        }
    } else if (ta == tPosY) {
        if(is_number(ta->getText())) {
            float newYPos = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEYPOS"+conversionFloatString(newYPos));
            State* state = new State("SCHANGEYPOS", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getPosition().y());
            state->addParameter("NEWVALUE", newYPos);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            getWorld()->removeEntity(selectedObject);
            selectedObject->setPosition(Vec3f(selectedObject->getPosition().x(), newYPos, selectedObject->getPosition().z()));
            getWorld()->addEntity(selectedObject);
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEYPOS", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getPosition().y());
                selectState->addParameter("NEWVALUE", newYPos);
                getWorld()->removeEntity(rectSelect.getItems()[i]);
                rectSelect.getItems()[i]->setPosition(Vec3f(rectSelect.getItems()[i]->getPosition().x(), newYPos, rectSelect.getItems()[i]->getPosition().z()));
                getWorld()->addEntity(rectSelect.getItems()[i]);
                sg->addState(selectState);
            }
        }
    } else if (ta == tPosZ) {
        if(is_number(ta->getText())) {
            float newZPos = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEZPOS"+conversionFloatString(newZPos));
            State* state = new State("SCHANGEZPOS", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getPosition().z());
            state->addParameter("NEWVALUE", newZPos);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            getWorld()->removeEntity(selectedObject);
            selectedObject->setPosition(Vec3f(selectedObject->getPosition().x(), selectedObject->getPosition().y(), newZPos));
            getWorld()->addEntity(selectedObject);
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEZPOS", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getPosition().z());
                selectState->addParameter("NEWVALUE", newZPos);
                getWorld()->removeEntity(rectSelect.getItems()[i]);
                rectSelect.getItems()[i]->setPosition(Vec3f(rectSelect.getItems()[i]->getPosition().x(), rectSelect.getItems()[i]->getPosition().y(), newZPos));
                getWorld()->addEntity(rectSelect.getItems()[i]);
                sg->addState(selectState);
            }
        }
    }
}
void ODFAEGCreator::onObjectRotationChanged(TextArea* ta) {
    if (ta == tRotAngle) {
        if (is_number(ta->getText())) {
            float newAngle = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEROTANGLE"+conversionFloatString(newAngle));
            State* state = new State("SCHANGEROTANGLE", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getRotation());
            state->addParameter("NEWVALUE", newAngle);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            getWorld()->removeEntity(selectedObject);
            selectedObject->setRotation(newAngle);
            getWorld()->addEntity(selectedObject);
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEROTANGLE", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getRotation());
                selectState->addParameter("NEWVALUE", newAngle);
                getWorld()->removeEntity(rectSelect.getItems()[i]);
                rectSelect.getItems()[i]->setRotation(newAngle);
                getWorld()->addEntity(rectSelect.getItems()[i]);
                sg->addState(selectState);
            }
        }
    }
}
void ODFAEGCreator::onObjectSizeChanged(TextArea* ta) {
    if (ta == tSizeW) {
        if (is_number(ta->getText())) {
            float newSizeW = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGESIZEW"+conversionFloatString(newSizeW));
            State* state = new State("SCHANGESIZEW", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getSize().x());
            state->addParameter("NEWVALUE", newSizeW);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            Vec3f oldSize = selectedObject->getSize();
            getWorld()->removeEntity(selectedObject);
            selectedObject->setSize(Vec3f(newSizeW, selectedObject->getSize().y(), selectedObject->getSize().z()));
            getWorld()->addEntity(selectedObject);
            //std::cout<<"size  : "<<oldSize * selectedObject->getScale()<<std::endl;
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {

                State* selectState = new State("SCHANGESIZEW", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getSize().x());
                selectState->addParameter("NEWVALUE", newSizeW);
                Vec3f oldSize = rectSelect.getItems()[i]->getSize();
                getWorld()->removeEntity(rectSelect.getItems()[i]);
                rectSelect.getItems()[i]->setSize(Vec3f(newSizeW, rectSelect.getItems()[i]->getSize().y(), rectSelect.getItems()[i]->getSize().z()));
                getWorld()->addEntity(rectSelect.getItems()[i]);
                //std::cout<<"size  : "<<oldSize * rectSelect.getItems()[i]->getScale()<<std::endl;
                sg->addState(selectState);

            }
        }
    } else if (ta == tSizeH) {
        if(is_number(ta->getText())) {
            float newSizeH = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGESIZEW"+conversionFloatString(newSizeH));
            State* state = new State("SCHANGESIZEW", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getSize().y());
            state->addParameter("NEWVALUE", newSizeH);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            getWorld()->removeEntity(selectedObject);
            selectedObject->setSize(Vec3f(selectedObject->getSize().x(), newSizeH, selectedObject->getSize().z()));
            getWorld()->addEntity(selectedObject);
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGESIZEH", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getSize().y());
                selectState->addParameter("NEWVALUE", newSizeH);
                getWorld()->removeEntity(rectSelect.getItems()[i]);
                rectSelect.getItems()[i]->setSize(Vec3f(rectSelect.getItems()[i]->getSize().x(), newSizeH, rectSelect.getItems()[i]->getSize().z()));
                getWorld()->addEntity(rectSelect.getItems()[i]);
                sg->addState(selectState);
            }
        }
    } else if (ta == tSizeD) {
        if(is_number(ta->getText())) {
            float newSizeD = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGESIZEZ"+conversionFloatString(newSizeD));
            State* state = new State("SCHANGESIZEZ", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getSize().z());
            state->addParameter("NEWVALUE", newSizeD);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            getWorld()->removeEntity(selectedObject);
            selectedObject->setSize(Vec3f(selectedObject->getSize().x(), selectedObject->getSize().y(), newSizeD));
            getWorld()->addEntity(selectedObject);
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGESIZEZ", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getSize().z());
                selectState->addParameter("NEWVALUE", newSizeD);
                getWorld()->removeEntity(rectSelect.getItems()[i]);
                rectSelect.getItems()[i]->setSize(Vec3f(rectSelect.getItems()[i]->getSize().x(), rectSelect.getItems()[i]->getSize().y(), newSizeD));
                getWorld()->addEntity(rectSelect.getItems()[i]);
                sg->addState(selectState);
            }
        }
    }
}
void ODFAEGCreator::onObjectScaleChanged(TextArea* ta) {
    if (ta == tScaleX) {
        if (is_number(ta->getText())) {
            float newScaleX = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGESCALEX"+conversionFloatString(newScaleX));
            State* state = new State("SCHANGESCALEX", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getScale().x());
            state->addParameter("NEWVALUE", newScaleX);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            getWorld()->removeEntity(selectedObject);
            selectedObject->scale(Vec3f(newScaleX / selectedObject->getScale().x(), selectedObject->getScale().y(), selectedObject->getScale().z()));
            getWorld()->addEntity(selectedObject);
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGESCALEX", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getScale().x());
                selectState->addParameter("NEWVALUE", newScaleX);
                getWorld()->removeEntity(rectSelect.getItems()[i]);
                rectSelect.getItems()[i]->scale(Vec3f(newScaleX / rectSelect.getItems()[i]->getScale().x(), rectSelect.getItems()[i]->getScale().y(), rectSelect.getItems()[i]->getScale().z()));
                getWorld()->addEntity(rectSelect.getItems()[i]);
                sg->addState(selectState);
            }
        }
    } else if (ta == tScaleY) {
        if(is_number(ta->getText())) {
            float newScaleY = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGESCALEY"+conversionFloatString(newScaleY));
            State* state = new State("SCHANGESCALEY", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getScale().y());
            state->addParameter("NEWVALUE", newScaleY);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            getWorld()->removeEntity(selectedObject);
            selectedObject->scale(Vec3f(selectedObject->getScale().x(), newScaleY/selectedObject->getScale().y(), selectedObject->getScale().z()));
            getWorld()->addEntity(selectedObject);
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGESCALEY", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getScale().y());
                selectState->addParameter("NEWVALUE", newScaleY);
                getWorld()->removeEntity(rectSelect.getItems()[i]);
                rectSelect.getItems()[i]->scale(Vec3f(rectSelect.getItems()[i]->getScale().x(), newScaleY/rectSelect.getItems()[i]->getScale().y(), rectSelect.getItems()[i]->getScale().z()));
                getWorld()->addEntity(rectSelect.getItems()[i]);
                sg->addState(selectState);
            }
        }
    } else if (ta == tScaleZ) {
        if(is_number(ta->getText())) {
            float newScaleZ = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGESCALEZ"+conversionFloatString(newScaleZ));
            State* state = new State("SCHANGESCALEZ", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getScale().z());
            state->addParameter("NEWVALUE", newScaleZ);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            getWorld()->removeEntity(selectedObject);
            selectedObject->scale(Vec3f(selectedObject->getScale().x(), selectedObject->getScale().y(), newScaleZ/selectedObject->getScale().z()));
            getWorld()->addEntity(selectedObject);
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGESCALEZ", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getScale().z());
                selectState->addParameter("NEWVALUE", newScaleZ);
                getWorld()->removeEntity(rectSelect.getItems()[i]);
                rectSelect.getItems()[i]->setPosition(Vec3f(rectSelect.getItems()[i]->getScale().x(), rectSelect.getItems()[i]->getScale().y(), newScaleZ/rectSelect.getItems()[i]->getScale().z()));
                getWorld()->addEntity(rectSelect.getItems()[i]);
                sg->addState(selectState);
            }
        }
    }
}
void ODFAEGCreator::onObjectMoveChanged(TextArea* ta) {
    if (ta == tMoveX) {
        if (is_number(ta->getText())) {
            float newMoveX = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEXTRANS"+conversionFloatString(newMoveX));
            State* state = new State("SCHANGEXTRANS", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getTranslation().x());
            state->addParameter("NEWVALUE", newMoveX);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            selectedObject->move(Vec3f(newMoveX-selectedObject->getTranslation().x(), selectedObject->getTranslation().y(), selectedObject->getTranslation().z()));
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEXTRANS", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getTranslation().x());
                selectState->addParameter("NEWVALUE", newMoveX);
                rectSelect.getItems()[i]->setPosition(Vec3f(newMoveX-rectSelect.getItems()[i]->getTranslation().x(), rectSelect.getItems()[i]->getTranslation().y(), rectSelect.getItems()[i]->getTranslation().z()));
                sg->addState(selectState);
            }
        }
    } else if (ta == tMoveY) {
        if(is_number(ta->getText())) {
            float newMoveY = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEYTRANS"+conversionFloatString(newMoveY));
            State* state = new State("SCHANGEYTRANS", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getTranslation().y());
            state->addParameter("NEWVALUE", newMoveY);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            selectedObject->move(Vec3f(selectedObject->getTranslation().x(), newMoveY-selectedObject->getTranslation().y(), selectedObject->getTranslation().z()));
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEYTRANS", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getTranslation().y());
                selectState->addParameter("NEWVALUE", newMoveY);
                rectSelect.getItems()[i]->move(Vec3f(rectSelect.getItems()[i]->getTranslation().x(), newMoveY-rectSelect.getItems()[i]->getTranslation().y(), rectSelect.getItems()[i]->getTranslation().z()));
                sg->addState(selectState);
            }
        }
    } else if (ta == tMoveZ) {
        if(is_number(ta->getText())) {
            float newMoveZ = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEZTRANS"+conversionFloatString(newMoveZ));
            State* state = new State("SCHANGEZTRANS", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getTranslation().z());
            state->addParameter("NEWVALUE", newMoveZ);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            selectedObject->move(Vec3f(selectedObject->getTranslation().x(), selectedObject->getTranslation().y(), newMoveZ-selectedObject->getTranslation().z()));
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEMOVEZ", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getTranslation().z());
                selectState->addParameter("NEWVALUE", newMoveZ);
                rectSelect.getItems()[i]->setPosition(Vec3f(rectSelect.getItems()[i]->getTranslation().x(), rectSelect.getItems()[i]->getTranslation().y(), newMoveZ-rectSelect.getItems()[i]->getTranslation().z()));
                sg->addState(selectState);
            }
        }
    }
}
/*void ODFAEGCreator::updateScriptColor(Transformable* shape) {
    std::map<std::string, std::string>::iterator it;
    it = cppAppliContent.find(minAppliname+".cpp");
    if (it != cppAppliContent.end()) {
        std::string& content = it->second;
        if (dynamic_cast<Shape*>(shape)) {
            if(content.find("shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+"->setFillColor") == std::string::npos) {
                unsigned int pos = content.find("shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+" = std::make_unique<RectangleShape>");
                std::string subs = content.substr(pos);
                pos += subs.find_first_of('\n') + 1;
                content.insert(pos,"    shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+"->setFillColor(Color("+conversionIntString(static_cast<Shape*>(shape)->getFillColor().r)+","
                +conversionIntString(static_cast<Shape*>(shape)->getFillColor().g)+","+conversionIntString(static_cast<Shape*>(shape)->getFillColor().b)+","+conversionIntString(static_cast<Shape*>(shape)->getFillColor().a)+"));\n");
            } else {
                unsigned int pos = content.find("shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+"->setFillColor");
                std::string subs = content.substr(pos);
                unsigned int endpos = subs.find_first_of('\n') + pos + 1;
                content.erase(pos, endpos - pos);
                content.insert(pos,"    shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+"->setFillColor(Color("+conversionIntString(static_cast<Shape*>(shape)->getFillColor().r)+","
                +conversionIntString(static_cast<Shape*>(shape)->getFillColor().g)+","+conversionIntString(static_cast<Shape*>(shape)->getFillColor().b)+","+conversionIntString(static_cast<Shape*>(shape)->getFillColor().a)+"));\n");
            }
        }
        if (dynamic_cast<Tile*>(shape)) {
            if(content.find("tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+"->setColor") == std::string::npos) {
                unsigned int pos = content.find("tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+" = new Tile");
                std::string subs = content.substr(pos);
                pos += subs.find_first_of('\n') + 1;
                content.insert(pos,"    tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+"->setColor(Color("+conversionIntString(static_cast<Tile*>(shape)->getColor().r)+","
                +conversionIntString(static_cast<Tile*>(shape)->getColor().g)+","+conversionIntString(static_cast<Tile*>(shape)->getColor().b)+","+conversionIntString(static_cast<Tile*>(shape)->getColor().a)+"));\n");
            } else {
                unsigned int pos = content.find("tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+"->setPosition");
                std::string subs = content.substr(pos);
                unsigned int endpos = subs.find_first_of('\n') + pos + 1;
                content.erase(pos, endpos - pos);
                content.insert(pos,"    tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+"->setColor(Color("+conversionIntString(static_cast<Tile*>(shape)->getColor().r)+","
                +conversionIntString(static_cast<Tile*>(shape)->getColor().g)+","+conversionIntString(static_cast<Tile*>(shape)->getColor().b)+","+conversionIntString(static_cast<Tile*>(shape)->getColor().a)+"));\n");
            }
        }
    }
}*/
void ODFAEGCreator::onObjectNameChanged(TextArea* ta) {
    selectedObject->setName(ta->getText());
}
void ODFAEGCreator::onNameCVChanged(TextArea* ta) {
    selectedBoundingVolume->setName(ta->getText());
}
void ODFAEGCreator::onWallTypeChanged(DropDownList* dp) {
    if (dp->getSelectedItem() == "TOP_LEFT")
        dynamic_cast<Wall*>(selectedObject)->setWallType(Wall::TOP_LEFT);
    if (dp->getSelectedItem() == "TOP_RIGHT")
        dynamic_cast<Wall*>(selectedObject)->setWallType(Wall::TOP_RIGHT);
    if (dp->getSelectedItem() == "BOTTOM_RIGHT")
        dynamic_cast<Wall*>(selectedObject)->setWallType(Wall::BOTTOM_RIGHT);
    if (dp->getSelectedItem() == "BOTTOM_LEFT")
        dynamic_cast<Wall*>(selectedObject)->setWallType(Wall::BOTTOM_LEFT);
    if (dp->getSelectedItem() == "TOP_BOTTOM")
        dynamic_cast<Wall*>(selectedObject)->setWallType(Wall::TOP_BOTTOM);
    if (dp->getSelectedItem() == "RIGHT_LEFT")
        dynamic_cast<Wall*>(selectedObject)->setWallType(Wall::RIGHT_LEFT);
    if (dp->getSelectedItem() == "T_TOP")
        dynamic_cast<Wall*>(selectedObject)->setWallType(Wall::T_TOP);
    if (dp->getSelectedItem() == "T_RIGHT")
        dynamic_cast<Wall*>(selectedObject)->setWallType(Wall::T_RIGHT);
    if (dp->getSelectedItem() == "T_LEFT")
        dynamic_cast<Wall*>(selectedObject)->setWallType(Wall::T_LEFT);
    if (dp->getSelectedItem() == "T_BOTTOM")
        dynamic_cast<Wall*>(selectedObject)->setWallType(Wall::T_BOTTOM);
    if (dp->getSelectedItem() == "X")
        dynamic_cast<Wall*>(selectedObject)->setWallType(Wall::X);
}
void ODFAEGCreator::onFrameRateChanged(TextArea* ta) {
    if (dynamic_cast<Anim*>(selectedObject) && is_number(ta->getText())) {
        static_cast<Anim*>(selectedObject)->setFrameRate(conversionStringFloat(ta->getText()));
    }
}
void ODFAEGCreator::onObjectColorChanged(TextArea* ta) {
    if (ta == tRColor) {
        if (is_number(tRColor->getText())) {
            unsigned int color = conversionStringInt(tRColor->getText());
            StateGroup* sg = new StateGroup("SGCHANGERCOLOR"+conversionUIntString(color));
            State* state = new State("SCHANGERCOLOR", &se);
            state->addParameter("OBJECT", selectedObject);

            if (dynamic_cast<Tile*>(selectedObject)) {
                state->addParameter("OLDVALUE", static_cast<Tile*>(selectedObject)->getColor().r);
            }
            if (dynamic_cast<PonctualLight*>(selectedObject)) {
                state->addParameter("OLDVALUE", static_cast<PonctualLight*>(selectedObject)->getColor().r);
            }
            state->addParameter("NEWVALUE", color);
            sg->addState(state);

            if (dynamic_cast<Tile*>(selectedObject)) {
                static_cast<Tile*>(selectedObject)->setColor(Color(Math::clamp(color, 0, 255), static_cast<Tile*>(selectedObject)->getColor().g,static_cast<Tile*>(selectedObject)->getColor().b, static_cast<Tile*>(selectedObject)->getColor().a));

            }
            if (dynamic_cast<PonctualLight*>(selectedObject)) {
                static_cast<PonctualLight*>(selectedObject)->setColor(Color(Math::clamp(color, 0, 255), static_cast<Tile*>(selectedObject)->getColor().g,static_cast<Tile*>(selectedObject)->getColor().b, static_cast<Tile*>(selectedObject)->getColor().a));

            }
            //updateScriptColor(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGERCOLOR", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);

                if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                    selectState->addParameter("OLDVALUE", static_cast<Tile*>(rectSelect.getItems()[i])->getColor().r);
                    static_cast<Tile*>(rectSelect.getItems()[i])->setColor(Color(Math::clamp(color, 0, 255), static_cast<Tile*>(rectSelect.getItems()[i])->getColor().g,static_cast<Tile*>(rectSelect.getItems()[i])->getColor().b, static_cast<Tile*>(rectSelect.getItems()[i])->getColor().a));

                }
                if (dynamic_cast<PonctualLight*>(rectSelect.getItems()[i])) {
                    selectState->addParameter("OLDVALUE", static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().r);
                    static_cast<PonctualLight*>(rectSelect.getItems()[i])->setColor(Color(Math::clamp(color, 0, 255), static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().g,static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().b, static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().a));

                }
                selectState->addParameter("NEWVALUE", color);
                sg->addState(selectState);
            }
            stateStack.addStateGroup(sg);
        }
    }
    if (ta == tGColor) {
        if (is_number(tGColor->getText())) {
            unsigned int color = conversionStringInt(tGColor->getText());
            StateGroup* sg = new StateGroup("SGCHANGEGCOLOR"+conversionUIntString(color));
            State* state = new State("SCHANGEGCOLOR", &se);
            state->addParameter("OBJECT", selectedObject);

            if (dynamic_cast<Tile*>(selectedObject)) {
                state->addParameter("OLDVALUE", static_cast<Tile*>(selectedObject)->getColor().g);
            }
            if (dynamic_cast<PonctualLight*>(selectedObject)) {
                state->addParameter("OLDVALUE", static_cast<PonctualLight*>(selectedObject)->getColor().g);
            }
            //updateScriptColor(selectedObject);
            state->addParameter("NEWVALUE", color);
            sg->addState(state);

            if (dynamic_cast<Tile*>(selectedObject)) {
                static_cast<Tile*>(selectedObject)->setColor(Color(static_cast<Tile*>(selectedObject)->getColor().r, Math::clamp(color, 0, 255),static_cast<Tile*>(selectedObject)->getColor().b, static_cast<Tile*>(selectedObject)->getColor().a));

            }
            if (dynamic_cast<PonctualLight*>(selectedObject)) {
                static_cast<PonctualLight*>(selectedObject)->setColor(Color(static_cast<PonctualLight*>(selectedObject)->getColor().r, Math::clamp(color, 0, 255),static_cast<PonctualLight*>(selectedObject)->getColor().b, static_cast<PonctualLight*>(selectedObject)->getColor().a));

            }
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEGCOLOR", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);

                if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                    selectState->addParameter("OLDVALUE", static_cast<Tile*>(rectSelect.getItems()[i])->getColor().g);
                    static_cast<Tile*>(rectSelect.getItems()[i])->setColor(Color(static_cast<Tile*>(rectSelect.getItems()[i])->getColor().r, Math::clamp(color, 0, 255),static_cast<Tile*>(rectSelect.getItems()[i])->getColor().b, static_cast<Tile*>(rectSelect.getItems()[i])->getColor().a));

                }
                if (dynamic_cast<PonctualLight*>(rectSelect.getItems()[i])) {
                    selectState->addParameter("OLDVALUE", static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().g);
                    static_cast<PonctualLight*>(rectSelect.getItems()[i])->setColor(Color(static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().r, Math::clamp(color, 0, 255),static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().b, static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().a));

                }
                selectState->addParameter("NEWVALUE", color);
                sg->addState(selectState);
            }
            stateStack.addStateGroup(sg);
        }
    }
    if (ta == tBColor) {
        if (is_number(tBColor->getText())) {
            unsigned int color = conversionStringInt(tBColor->getText());
            StateGroup* sg = new StateGroup("SGCHANGEBCOLOR"+conversionUIntString(color));
            State* state = new State("SCHANGEBCOLOR", &se);
            state->addParameter("OBJECT", selectedObject);

            if (dynamic_cast<Tile*>(selectedObject)) {
                state->addParameter("OLDVALUE", static_cast<Tile*>(selectedObject)->getColor().b);
            }
            if (dynamic_cast<PonctualLight*>(selectedObject)) {
                state->addParameter("OLDVALUE", static_cast<PonctualLight*>(selectedObject)->getColor().b);
            }
            //updateScriptColor(selectedObject);
            state->addParameter("NEWVALUE", color);
            sg->addState(state);

            if (dynamic_cast<Tile*>(selectedObject)) {
                static_cast<Tile*>(selectedObject)->setColor(Color(static_cast<Tile*>(selectedObject)->getColor().r, static_cast<Tile*>(selectedObject)->getColor().g, Math::clamp(color, 0, 255), static_cast<Tile*>(selectedObject)->getColor().a));

            }
            if (dynamic_cast<PonctualLight*>(selectedObject)) {
                static_cast<PonctualLight*>(selectedObject)->setColor(Color(static_cast<PonctualLight*>(selectedObject)->getColor().r, static_cast<PonctualLight*>(selectedObject)->getColor().g, Math::clamp(color, 0, 255), static_cast<PonctualLight*>(selectedObject)->getColor().a));

            }
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEBCOLOR", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);

                if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                    selectState->addParameter("OLDVALUE", static_cast<Tile*>(rectSelect.getItems()[i])->getColor().b);
                    static_cast<Tile*>(rectSelect.getItems()[i])->setColor(Color(static_cast<Tile*>(rectSelect.getItems()[i])->getColor().r, static_cast<Tile*>(rectSelect.getItems()[i])->getColor().g, Math::clamp(color, 0, 255), static_cast<Tile*>(rectSelect.getItems()[i])->getColor().a));

                }
                if (dynamic_cast<PonctualLight*>(rectSelect.getItems()[i])) {
                    selectState->addParameter("OLDVALUE", static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().b);
                    static_cast<PonctualLight*>(rectSelect.getItems()[i])->setColor(Color(static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().r, static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().g, Math::clamp(color, 0, 255), static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().a));

                }
                selectState->addParameter("NEWVALUE", color);
                sg->addState(selectState);
            }
            stateStack.addStateGroup(sg);
        }
    }
    if (ta == tAColor) {
        if (is_number(tAColor->getText())) {
            unsigned int color = conversionStringInt(tAColor->getText());
            StateGroup* sg = new StateGroup("SGCHANGEACOLOR"+conversionUIntString(color));
            State* state = new State("SCHANGEACOLOR", &se);
            state->addParameter("OBJECT", selectedObject);

            if (dynamic_cast<Tile*>(selectedObject)) {
                state->addParameter("OLDVALUE", static_cast<Tile*>(selectedObject)->getColor().a);
            }
            if (dynamic_cast<PonctualLight*>(selectedObject)) {
                state->addParameter("OLDVALUE", static_cast<PonctualLight*>(selectedObject)->getColor().a);
            }
            //updateScriptColor(selectedObject);
            state->addParameter("NEWVALUE", color);
            sg->addState(state);

            if (dynamic_cast<Tile*>(selectedObject)) {
                static_cast<Tile*>(selectedObject)->setColor(Color(static_cast<Tile*>(selectedObject)->getColor().r, static_cast<Tile*>(selectedObject)->getColor().g,static_cast<Tile*>(selectedObject)->getColor().b, Math::clamp(color, 0, 255)));

            }
            if (dynamic_cast<PonctualLight*>(selectedObject)) {
                static_cast<PonctualLight*>(selectedObject)->setColor(Color(static_cast<PonctualLight*>(selectedObject)->getColor().r, static_cast<PonctualLight*>(selectedObject)->getColor().g,static_cast<PonctualLight*>(selectedObject)->getColor().b, Math::clamp(color, 0, 255)));

            }
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEACOLOR", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);

                if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                    selectState->addParameter("OLDVALUE", static_cast<Tile*>(rectSelect.getItems()[i])->getColor().a);
                    static_cast<Tile*>(rectSelect.getItems()[i])->setColor(Color(static_cast<Tile*>(rectSelect.getItems()[i])->getColor().r, static_cast<Tile*>(rectSelect.getItems()[i])->getColor().g,static_cast<Tile*>(rectSelect.getItems()[i])->getColor().b, Math::clamp(color, 0, 255)));

                }
                if (dynamic_cast<PonctualLight*>(rectSelect.getItems()[i])) {
                    selectState->addParameter("OLDVALUE", static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().a);
                    static_cast<PonctualLight*>(rectSelect.getItems()[i])->setColor(Color(static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().r, static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().g,static_cast<PonctualLight*>(rectSelect.getItems()[i])->getColor().b, Math::clamp(color, 0, 255)));

                }
                selectState->addParameter("NEWVALUE", color);
                sg->addState(selectState);
            }
            stateStack.addStateGroup(sg);
        }
    }
}
void ODFAEGCreator::onSelectedComponentChanged(DropDownList* dp) {
    std::string name = dp->getSelectedItem();
    if(name == "MAIN WINDOW") {
        taComponentExpression->setText("");
    } else {
        std::vector<Component*> components = getRenderComponentManager().getRenderComponents();
        for (unsigned int i = 0; i < components.size(); i++) {
            if (components[i]->getName() == name && dynamic_cast<HeavyComponent*>(components[i])) {
                selectedComponentView = components[i]->getFrameBuffer()->getView();
                taChangeComponentExpression->setText(static_cast<HeavyComponent*>(components[i])->getExpression());
            }
        }
    }
}
void ODFAEGCreator::onParentClicked(Label* label) {
    std::vector<std::string> parts = split(label->getText(), ":");
    std::string pName = parts[1].substr(1, parts[1].size()-1);
    if (pName != "NONE") {
        Entity* parent = selectedObject->getParent();
        if (parent->getType() == "E_ANIMATION") {
            displayAnimInfos(parent);
            selectedObject = parent;
        } else if (parent->getType() == "E_WALL") {
            displayWallInfos(parent);
            selectedObject = parent;
        } else if (parent->getType() == "E_DECOR") {
            displayDecorInfos(parent);
            selectedObject = parent;
        } else if (parent->getType() == "E_BIGTILE") {
            displayBigtileInfos(parent);
            selectedObject = parent;
        } else if (parent->getType() == "E_PARTICLES") {
            displayParticleSystemInfos(parent);
            selectedObject = parent;
        } else if (parent->getType() == "E_PONCTUAL_LIGHT") {
            displayPonctualLightInfos(parent);
            selectedObject = parent;
        } else if (parent->getType() == "E_TILE") {
            displayTileInfos(parent);
            selectedObject = parent;
        } else {
            displayExternalEntityInfo(parent);
            selectedObject = parent;
        }
    }
}
void ODFAEGCreator::onSelectedParentChanged(DropDownList* dp) {
    if (dp->getSelectedItem() == "NONE") {
        selectedObject->setParent(nullptr);
    } else if (dp->getSelectedItem() != "") {
        Entity* entity = getWorld()->getEntity(dp->getSelectedItem());
        //std::cout<<"entity : "<<entity<<std::endl;
        if (entity != nullptr && dynamic_cast<Entity*>(selectedObject)) {

            if (!entity->isAnimated()) {
                getWorld()->removeEntity(selectedObject);
                getWorld()->removeEntity(static_cast<Entity*>(entity));
                selectedObject->setParent(entity);
                entity->addChild(selectedObject);
                if (!selectedObject->getRootEntity()->isAnimated())
                    getWorld()->addEntity(entity);
            } else {
                if (dynamic_cast<Anim*>(entity)) {
                    //std::cout<<"add frame"<<std::endl;
                    getWorld()->removeEntity(selectedObject);
                    getWorld()->removeEntity(entity);
                    static_cast<Anim*>(entity)->addFrame(selectedObject);
                    static_cast<Anim*>(entity)->play(true);

                    getWorld()->addEntity(entity);
                }
            }
            lParent->setText("Parent entity : "+entity->getName());
        }
    }
}
void ODFAEGCreator::onSelectedParentCV (DropDownList* dp) {
    for (unsigned int i = 0; i < boundingVolumes.size(); i++) {
        if (boundingVolumes[i]->getName() == dp->getSelectedItem()) {
            boundingVolumes[i]->addChild(selectedBoundingVolume);
            lParentCV->setText("Parent : "+boundingVolumes[i]->getName());
        }
    }
}
void ODFAEGCreator::onAnimUpdaterChanged(DropDownList* dp) {
    Timer* timer = getWorld()->getTimer(dp->getSelectedItem());
    if (dynamic_cast<AnimUpdater*>(timer) && dynamic_cast<Anim*>(selectedObject)) {
        std::cout<<"add anim updater"<<std::endl;
        static_cast<AnimUpdater*>(timer)->addAnim(static_cast<Anim*>(selectedObject));
    }
}
void ODFAEGCreator::onParticleSystemUpdaterChanged(DropDownList* dp) {
    Timer* worker = getWorld()->getTimer(dp->getSelectedItem());
    if (dynamic_cast<ParticleSystemUpdater*>(worker) && dynamic_cast<ParticleSystem*>(selectedObject)) {
        std::cout<<"add particle system to updater"<<std::endl;
        static_cast<ParticleSystemUpdater*>(worker)->addParticleSystem(static_cast<ParticleSystem*>(selectedObject));
    }
}
void ODFAEGCreator::onSelectedTextureChanged(DropDownList* dp) {
    pMaterial->removeSprites();

    const Texture* oldTexture = nullptr;

    if (dynamic_cast<Tile*>(selectedObject)) {
        std::cout<<"tile : "<<std::endl;
        oldTexture = static_cast<Tile*>(selectedObject)->getFace(0)->getMaterial().getTexture();
    }
    if (dp->getSelectedItem() == "NONE") {

        if (dynamic_cast<Tile*>(selectedObject)) {
            Tile* selectedTile = static_cast<Tile*>(selectedObject);
            selectedTile->getFace(0)->getMaterial().clearTextures();
            selectedTile->getFace(0)->getMaterial().addTexture(nullptr, IntRect(0, 0, gridWidth, gridHeight));
            static_cast<Tile*>(selectedObject)->getFace(0)->getMaterial().setTexId("");
            //updateScriptText(static_cast<Tile*>(selectedObject), nullptr);
        }
    } else {
        TextureManager<>& tm = cache.resourceManager<Texture, std::string>("TextureManager");
        /*for (unsigned int i = 0; i < textPaths.size(); i++) {
            std::cout<<"searching tex path : "<<dp->getSelectedItem()<<std::endl;
            if (textPaths[i].find(dp->getSelectedItem())) {*/
                const Texture* text = tm.getResourceByAlias(dp->getSelectedItem());
                std::vector<std::string> alias = tm.getAliasByResource(const_cast<Texture*>(text));
                pMaterial->clearDrawables();
                Sprite sprite (*text, Vec3f(pScriptsFiles->getPosition().x(), bChooseText->getPosition().y() + bChooseText->getSize().y()+10, 0),Vec3f(text->getSize().x(), text->getSize().y(), 0),IntRect(0, 0, text->getSize().x(),text->getSize().y()));
                pMaterial->addSprite(sprite);
                IntRect textRect;

                if (dynamic_cast<Tile*>(selectedObject)) {
                    //std::cout<<"add texture"<<std::endl;
                    static_cast<Tile*>(selectedObject)->getFace(0)->getMaterial().clearTextures();
                    static_cast<Tile*>(selectedObject)->getFace(0)->getMaterial().addTexture(text, IntRect(0, 0, text->getSize().x(), text->getSize().y()));
                    static_cast<Tile*>(selectedObject)->getFace(0)->getMaterial().setTexId(alias[0]);
                    textRect = IntRect(0, 0, text->getSize().x(), text->getSize().y());
                    std::cout<<"tile texture set"<<std::endl;
                    //updateScriptText(static_cast<Tile*>(selectedObject), text);
                }
                if (dynamic_cast<ParticleSystem*>(selectedObject)) {
                    static_cast<ParticleSystem*>(selectedObject)->setTexture(*text);
                    textRect = IntRect(0, 0, text->getSize().x(), text->getSize().y());
                    static_cast<ParticleSystem*>(selectedObject)->getFace(0)->getMaterial().setTexId(alias[0]);
                }
                tTexCoordX->setText(conversionIntString(textRect.left));
                tTexCoordY->setText(conversionIntString(textRect.top));
                tTexCoordW->setText(conversionIntString(textRect.width));
                tTexCoordH->setText(conversionIntString(textRect.height));
                sTextRect = new RectangleShape(Vec3f(textRect.width, textRect.height, 0));
                sTextRect->setPosition(Vec3f(textRect.left+pScriptsFiles->getPosition().x(), textRect.top + bChooseText->getPosition().y() + bChooseText->getSize().y()+10, 0));
                sTextRect->setFillColor(Color::Transparent);
                sTextRect->setOutlineColor(Color::Red);
                sTextRect->setOutlineThickness(1);
                sTextRect->setName("TexRect");
                pMaterial->addShape(sTextRect);
            /*}
        }*/
    }
    StateGroup* sg = new StateGroup("SGCHANGETEXTURE"+conversionLongString(reinterpret_cast<uint64_t>(oldTexture)));
    State* state = new State("SCHANGETEXTURE", &se);
    state->addParameter("OLDVALUE",oldTexture);

    if (dynamic_cast<Tile*>(selectedObject)) {
        state->addParameter("NEWVALUE",static_cast<Tile*>(selectedObject)->getFace(0)->getMaterial().getTexture());
    }
    state->addParameter("OBJECT", selectedObject);
    sg->addState(state);

    for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {

        if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
            oldTexture = static_cast<Tile*>(rectSelect.getItems()[i])->getFace(0)->getMaterial().getTexture();
        }
        if (dp->getSelectedItem() == "NONE") {

            if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                Tile* selectedTile = static_cast<Tile*>(rectSelect.getItems()[i]);
                selectedTile->getFace(0)->getMaterial().clearTextures();
                selectedTile->getFace(0)->getMaterial().addTexture(nullptr, IntRect(0, 0, gridWidth, gridHeight));
            }
        } else {
            TextureManager<>& tm = cache.resourceManager<Texture, std::string>("TextureManager");
            const Texture* text = tm.getResourceByAlias(dp->getSelectedItem());
            std::vector<std::string> alias = tm.getAliasByResource(const_cast<Texture*>(text));

            if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                //std::cout<<"add texture"<<std::endl;
                static_cast<Tile*>(rectSelect.getItems()[i])->getFace(0)->getMaterial().clearTextures();
                static_cast<Tile*>(rectSelect.getItems()[i])->getFace(0)->getMaterial().addTexture(text, IntRect(0, 0, text->getSize().x(), text->getSize().y()));
                static_cast<Tile*>(rectSelect.getItems()[i])->getFace(0)->getMaterial().setTexId(alias[0]);
            }
        }
        State* selectState = new State("SCHANGETEXTURE", &se);
        selectState->addParameter("OLDVALUE", oldTexture);

        if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
            selectState->addParameter("NEWVALUE",static_cast<Tile*>(rectSelect.getItems()[i])->getFace(0)->getMaterial().getTexture());
        }
        sg->addState(selectState);
    }
    stateStack.addStateGroup(sg);
    std::cout<<"activate bchoosetext context"<<std::endl;

    pMaterial->updateScrolls();

}
void ODFAEGCreator::onTexCoordsChanged (TextArea* ta) {
    const Texture* tex = nullptr;
    IntRect texRect;

    if (selectedObject->getFaces().size() > 0) {
        tex = selectedObject->getFace(0)->getMaterial().getTexture();
        texRect = IntRect(conversionStringInt(tTexCoordX->getText()),conversionStringInt(tTexCoordY->getText()),conversionStringInt(tTexCoordW->getText()),conversionStringInt(tTexCoordH->getText()));
    }
    if (tex != nullptr) {
        if (ta == tTexCoordX) {

            if (is_number(ta->getText())) {
                //std::cout<<"tex coord x changed : "<<ta->getText()<<std::endl;
                int texCoordX = conversionStringInt(ta->getText());
                StateGroup* sg = new StateGroup("SGCHANGEXTEXCOORD");
                State* state = new State("SCHANGEXTEXCOORD", &se);

                if (dynamic_cast<Tile*>(selectedObject)) {
                    state->addParameter("OLDVALUE", static_cast<Tile*>(selectedObject)->getTexCoords().left);
                }
                state->addParameter("NEWVALUE", texCoordX);
                state->addParameter("OBJECT", selectedObject);
                sg->addState(state);


                if (dynamic_cast<Tile*>(selectedObject)) {
                    static_cast<Tile*>(selectedObject)->setTexRect(IntRect(Math::abs(texCoordX), texRect.top, texRect.width, texRect.height));
                }
                for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                    State* selectState = new State("SCHANGEXTEXCOORD", &se);
                    state->addParameter("OBJECT", rectSelect.getItems()[i]);

                    if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                        state->addParameter("OLDVALUE", static_cast<Tile*>(rectSelect.getItems()[i])->getTexCoords().left);
                    }
                    selectState->addParameter("NEWVALUE", texCoordX);
                    sg->addState(state);

                    if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                        static_cast<Tile*>(rectSelect.getItems()[i])->setTexRect(IntRect(Math::abs(texCoordX), texRect.top, texRect.width, texRect.height));
                    }
                }
                stateStack.addStateGroup(sg);
            }
        }
        if (ta == tTexCoordY) {
            if (is_number(ta->getText())) {
                //std::cout<<"tex coord y changed : "<<ta->getText()<<std::endl;
                int texCoordY = conversionStringInt(ta->getText());
                StateGroup* sg = new StateGroup("SGCHANGEYTEXCOORD");
                State* state = new State("SCHANGEYTEXCOORD", &se);

                if (dynamic_cast<Tile*>(selectedObject)) {
                    state->addParameter("OLDVALUE", static_cast<Tile*>(selectedObject)->getTexCoords().top);
                }
                state->addParameter("NEWVALUE", texCoordY);
                state->addParameter("OBJECT", selectedObject);
                sg->addState(state);


                if (dynamic_cast<Tile*>(selectedObject)) {
                    static_cast<Tile*>(selectedObject)->setTexRect(IntRect(texRect.left, Math::abs(texCoordY), texRect.width, texRect.height));
                }
                for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                    State* selectState = new State("SCHANGEXTEYCOORD", &se);
                    state->addParameter("OBJECT", rectSelect.getItems()[i]);

                    if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                        state->addParameter("OLDVALUE", static_cast<Tile*>(rectSelect.getItems()[i])->getTexCoords().top);
                    }
                    selectState->addParameter("NEWVALUE", texCoordY);
                    sg->addState(state);

                    if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                        static_cast<Tile*>(rectSelect.getItems()[i])->setTexRect(IntRect(texRect.left, Math::abs(texCoordY), texRect.width, texRect.height));
                    }
                }
                stateStack.addStateGroup(sg);
            }
        }
        if (ta == tTexCoordW) {
            if (is_number(ta->getText())) {
                //std::cout<<"tex coord w changed : "<<ta->getText()<<std::endl;
                int texCoordW = conversionStringInt(ta->getText());
                StateGroup* sg = new StateGroup("SGCHANGEWTEXCOORD"+conversionIntString(texCoordW));
                State* state = new State("SCHANGEWTEXCOORD", &se);

                if (dynamic_cast<Tile*>(selectedObject)) {
                    state->addParameter("OLDVALUE", static_cast<Tile*>(selectedObject)->getTexCoords().width);
                }
                state->addParameter("NEWVALUE", texCoordW);
                state->addParameter("OBJECT", selectedObject);
                sg->addState(state);


                if (dynamic_cast<Tile*>(selectedObject)) {
                    static_cast<Tile*>(selectedObject)->setTexRect(IntRect(texRect.left, texRect.top, Math::abs(texCoordW), texRect.height));
                }
                for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                    State* selectState = new State("SCHANGEWTEXCOORD", &se);
                    state->addParameter("OBJECT", rectSelect.getItems()[i]);

                    if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                        state->addParameter("OLDVALUE", static_cast<Tile*>(rectSelect.getItems()[i])->getTexCoords().width);
                    }
                    selectState->addParameter("NEWVALUE", texCoordW);
                    sg->addState(state);

                    if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                        static_cast<Tile*>(rectSelect.getItems()[i])->setTexRect(IntRect(texRect.left, texRect.top, Math::abs(texCoordW), texRect.height));
                    }
                }
                stateStack.addStateGroup(sg);
            }
        }
        if (ta == tTexCoordH) {
            if (is_number(ta->getText())) {
                //std::cout<<"tex coord h changed : "<<ta->getText()<<std::endl;
                int texCoordH = conversionStringInt(ta->getText());
                StateGroup* sg = new StateGroup("SGCHANGEHTEXCOORD"+conversionIntString(texCoordH));
                State* state = new State("SCHANGEHTEXCOORD", &se);

                if (dynamic_cast<Tile*>(selectedObject)) {
                    state->addParameter("OLDVALUE", static_cast<Tile*>(selectedObject)->getTexCoords().height);
                }
                state->addParameter("NEWVALUE", texCoordH);
                state->addParameter("OBJECT", selectedObject);
                sg->addState(state);


                if (dynamic_cast<Tile*>(selectedObject)) {
                    static_cast<Tile*>(selectedObject)->setTexRect(IntRect(texRect.left, texRect.top, texRect.width, Math::abs(texCoordH)));
                }
                for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                    State* selectState = new State("SCHANGEHTEXCOORD", &se);
                    state->addParameter("OBJECT", rectSelect.getItems()[i]);

                    if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                        state->addParameter("OLDVALUE", static_cast<Tile*>(rectSelect.getItems()[i])->getTexCoords().height);
                    }
                    selectState->addParameter("NEWVALUE", texCoordH);
                    sg->addState(state);

                    if (dynamic_cast<Tile*>(rectSelect.getItems()[i])) {
                        static_cast<Tile*>(rectSelect.getItems()[i])->setTexRect(IntRect(texRect.left, texRect.top, texRect.width, Math::abs(texCoordH)));
                    }
                }
                stateStack.addStateGroup(sg);
            }
        }

        if ((ta == tTexCoordX || ta == tTexCoordY || ta == tTexCoordW || ta == tTexCoordH)
            && is_number(tTexCoordX->getText()) && is_number(tTexCoordY->getText()) && is_number(tTexCoordW->getText()) && is_number(tTexCoordH->getText())) {

            sTextRect->setPosition(Vec3f(conversionStringInt(tTexCoordX->getText())+pScriptsFiles->getPosition().x()-pMaterial->getDeltas().x(), conversionStringInt(tTexCoordY->getText()) + bChooseText->getPosition().y() + bChooseText->getSize().y()+10-pMaterial->getDeltas().y(), 0));
            sTextRect->setSize(Vec3f(conversionStringInt(tTexCoordW->getText()), conversionStringInt(tTexCoordH->getText()), 0));
        }
    }
    bChooseText->setEventContextActivated(true);
    if (bAddTexRect != nullptr)
        bAddTexRect->setEventContextActivated(true);
    if (bSetTexRect != nullptr)
        bSetTexRect->setEventContextActivated(true);
}
/*void ODFAEGCreator::updateScriptTextCoords(Transformable* shape) {
    std::map<std::string, std::string>::iterator it;
    it = cppAppliContent.find(minAppliname+".cpp");
    if (it != cppAppliContent.end()) {
        std::string& content = it->second;
        if (dynamic_cast<Shape*>(shape)) {
            if(content.find("shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+"->setTextureRect") == std::string::npos) {
                unsigned int pos = content.find("shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+" = std::make_unique<RectangleShape>");
                if (pos != std::string::npos) {
                    std::string subs = content.substr(pos);
                    pos += subs.find_first_of('\n') + 1;
                    content.insert(pos,"    shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+"->setTextureRect(IntRect("+conversionIntString(static_cast<Shape*>(shape)->getTextureRect().left)+","
                    +conversionIntString(static_cast<Shape*>(shape)->getTextureRect().top)+","+conversionIntString(static_cast<Shape*>(shape)->getTextureRect().width)+","+conversionIntString(static_cast<Shape*>(shape)->getTextureRect().height)+"));\n");
                }
            } else {
                unsigned int pos = content.find("shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+"->setTextureRect");
                if (pos != std::string::npos) {
                    std::string subs = content.substr(pos);
                    unsigned int endpos = subs.find_first_of('\n') + pos + 1;
                    content.erase(pos, endpos - pos);
                    content.insert(pos,"    shape"+conversionUIntString(static_cast<Shape*>(shape)->getId())+"->setTextureRect(IntRect("+conversionIntString(static_cast<Shape*>(shape)->getTextureRect().left)+","
                    +conversionIntString(static_cast<Shape*>(shape)->getTextureRect().top)+","+conversionIntString(static_cast<Shape*>(shape)->getTextureRect().width)+","+conversionIntString(static_cast<Shape*>(shape)->getTextureRect().height)+"));\n");
                }
            }
        }
        if (dynamic_cast<Tile*>(shape)) {
            if(content.find("tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+"->setTexRect") == std::string::npos) {
                unsigned int pos = content.find("tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+" = new Tile");
                if (pos != std::string::npos) {
                    std::string subs = content.substr(pos);
                    pos += subs.find_first_of('\n') + 1;
                    content.insert(pos,"    tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+"->setTexRect(IntRect("+conversionIntString(static_cast<Tile*>(shape)->getTexCoords().left)+","
                    +conversionIntString(static_cast<Tile*>(shape)->getTexCoords().top)+","+conversionIntString(static_cast<Tile*>(shape)->getTexCoords().width)+","+conversionIntString(static_cast<Tile*>(shape)->getTexCoords().height)+"));\n");
                }
            } else {
                unsigned int pos = content.find("tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+"->setPosition");
                if (pos != std::string::npos) {
                    std::string subs = content.substr(pos);
                    unsigned int endpos = subs.find_first_of('\n') + pos + 1;
                    content.erase(pos, endpos - pos);
                    content.insert(pos,"    tile"+conversionUIntString(static_cast<Tile*>(shape)->getId())+"->setTexRect(IntRect("+conversionIntString(static_cast<Tile*>(shape)->getTexCoords().left)+","
                    +conversionIntString(static_cast<Tile*>(shape)->getTexCoords().top)+","+conversionIntString(static_cast<Tile*>(shape)->getTexCoords().width)+","+conversionIntString(static_cast<Tile*>(shape)->getTexCoords().height)+"));\n");
                }
            }
        }
    }
}*/
void ODFAEGCreator::onSelectedEmChanged(DropDownList* dp) {
    if (dp->getSelectedItem() == "NONE") {

        if (getWorld()->getCurrentSceneManager() != nullptr)
            getWorld()->removeEntity(selectedObject);

    } else {
        getWorld()->setCurrentSceneManager(dp->getSelectedItem());
        getWorld()->addEntity(selectedObject);
    }
}
void ODFAEGCreator::onComponentExpressionChanged(TextArea* ta) {
    //std::cout<<"component expression changed"<<std::endl;
    std::string name = dpSelectComponent->getSelectedItem();
    std::vector<Component*> components = getRenderComponentManager().getRenderComponents();
    for (unsigned int i = 0; i < components.size(); i++) {
        if (name == components[i]->getName() && dynamic_cast<HeavyComponent*>(components[i])) {
            static_cast<HeavyComponent*>(components[i])->setExpression(ta->getText());
        }
    }
}
void ODFAEGCreator::onCollisionBoundingBoxChanged(TextArea* ta) {
    if (selectedBoundingVolume != nullptr) {
        Vec3f newPosition = Vec3f(conversionStringFloat(taBoundingBoxColX->getText()), conversionStringFloat(taBoundingBoxColY->getText()),conversionStringFloat(taBoundingBoxColZ->getText()));
        Vec3f t = newPosition = selectedBoundingVolume->getPosition();
        selectedBoundingVolume->move(t);

        Vec3f newSize = Vec3f(conversionStringFloat(taBoundingBoxColW->getText()), conversionStringFloat(taBoundingBoxColH->getText()), conversionStringFloat(taBoundingBoxColD->getText()));
        Vec3f s = newSize / selectedBoundingVolume->getSize();
        selectedBoundingVolume->scale(s);
        collisionBox.setSize(selectedBoundingVolume->getSize());
        collisionBox.setPosition(selectedBoundingVolume->getPosition());
        collisionBox.setFillColor(Color(255, 0, 0, 128));
    }
}
void ODFAEGCreator::displayChildrenCV(DropDownList* dp) {
    if (dp->getSelectedItem() != "NONE") {
        std::vector<BoundingVolume*> children = selectedBoundingVolume->getChildren();
        for (unsigned int i = 0; i < children.size(); i++) {
            if (children[i]->getName() == dp->getSelectedItem()) {
                selectedBoundingVolume = children[i];
            }
        }
        if (selectedObject->getType() == "E_ANIMATION") {
            displayAnimInfos(selectedObject);
        } else if (selectedObject->getType() == "E_WALL") {
            displayWallInfos(selectedObject);
        } else if (selectedObject->getType() == "E_DECOR") {
            displayDecorInfos(selectedObject);
        } else if (selectedObject->getType() == "E_BIGTILE") {
            displayBigtileInfos(selectedObject);
        } else if (selectedObject->getType() == "E_PARTICLES") {
            displayParticleSystemInfos(selectedObject);
        } else if (selectedObject->getType() == "E_PONCTUAL_LIGHT") {
            displayPonctualLightInfos(selectedObject);
        } else if (selectedObject->getType() == "E_TILE") {
            displayTileInfos(selectedObject);
        } else {
            displayExternalEntityInfo(selectedObject);
        }
        collisionBox.setSize(selectedBoundingVolume->getSize());
        collisionBox.setPosition(selectedBoundingVolume->getPosition());
    }
}
void ODFAEGCreator::onSelectedClassChanged(DropDownList* dp) {
    if (dp->getSelectedItem() != "Select class") {
        //std::cout<<"selected class : "<<dp->getSelectedItem()<<std::endl;
        dpSelectFunction->removeAllItems();
        dpSelectPointerType->removeAllItems();
        std::string selectedItem = dp->getSelectedItem();
        std::vector<std::string> parts = split(selectedItem, "::");
        Class cl = Class::getClass(parts[parts.size() - 1]);
        std::vector<Constructor> constructors = cl.getConstructors();
        dpSelectFunction->addItem("Select function", 15);
        //std::cout<<"size : "<<constructors.size()<<std::endl;
        for (unsigned int i = 0; i < constructors.size(); i++) {
            std::string name = constructors[i].getName()+"(";
            std::vector<std::string> argsTypes = constructors[i].getArgsTypes();
            for (unsigned int j = 0; j < argsTypes.size(); j++) {
                //std::cout<<"add arg type : "<<argsTypes[j]<<std::endl;
                name += argsTypes[j];
                if (j != argsTypes.size() - 1) {
                    name += ",";
                }
            }
            name += ")";
            //std::cout<<"add function : "<<name<<std::endl;
            dpSelectFunction->addItem(name, 15);
            //std::cout<<"add function : "<<i<<" : "<<name<<std::endl;
        }
        dpSelectPointerType->addItem("No pointer", 15);
        if (cl.getNamespace() == "")
            dpSelectPointerType->addItem(cl.getName(), 15);
        else
            dpSelectPointerType->addItem(cl.getNamespace()+"::"+cl.getName(), 15);
        std::vector<Class> superClasses = cl.getSuperClasses();
        while (superClasses.size() > 0) {
            for (unsigned int i = 0; i < superClasses.size(); i++) {
                std::string name;
                if (superClasses[i].getNamespace() == "")
                    name = superClasses[i].getName();
                else
                    name = superClasses[i].getNamespace() + "::" + superClasses[i].getName();

                dpSelectPointerType->setName("POINTERTYPE");
                //std::cout<<"add item : "<<name<<std::endl;
                dpSelectPointerType->addItem(name, 15);
                std::vector<Class> tmpSuperClasses = superClasses[i].getSuperClasses();
                for (unsigned int j = 0; j < tmpSuperClasses.size(); j++) {
                    superClasses.push_back(tmpSuperClasses[j]);
                }
                if (superClasses.size() > 0)
                    superClasses.erase(superClasses.begin(), superClasses.begin()+1);
            }
        }
    }
    /*dpSelectFunction->setEventContextActivated(true);
    dpSelectPointerType->setEventContextActivated(true);*/
    bCreateObject->setEventContextActivated(true);
}
void ODFAEGCreator::onSelectedFunctionChanged(DropDownList* dp) {
    if (dp->getSelectedItem() != "Select function" && dpSelectClass->getSelectedItem() != "Select class") {
        rootObjectParams->deleteAllNodes();
        pObjectsParameters->removeAll();
        std::string selectedItem = dpSelectClass->getSelectedItem();
        std::vector<std::string> parts = split(selectedItem, "::");
        //std::cout<<"parts : "<<parts[parts.size() - 1]<<std::endl;
        Class cl = Class::getClass(parts[parts.size() - 1]);
        tmpTextAreas.clear();
        std::vector<Constructor> constructors = cl.getConstructors();
        bool found = false;
        Constructor* c=nullptr;
        for (unsigned int i = 0; i < constructors.size() && !found; i++) {
            std::string name = constructors[i].getName()+"(";
            std::vector<std::string> argsTypes = constructors[i].getArgsTypes();
            for (unsigned int j = 0; j < argsTypes.size(); j++) {
                name += argsTypes[j];
                if (j != argsTypes.size() - 1) {
                    name += ",";
                }
            }
            name += ")";

            if (name == dp->getSelectedItem()) {
                std::cout<<"select constructor"<<std::endl;
                c = &constructors[i];
                found = true;
                argsTps = argsTypes;
            }
        }
        if (c != nullptr) {
            FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
            std::vector<std::string> argsTypes = c->getArgsTypes();
            std::vector<std::string> argsNames = c->getArgsNames();
            for (unsigned int i = 0; i < argsTypes.size(); i++) {
                //std::cout<<"add gui"<<std::endl;
                Label* label = new Label(*wCreateNewObject, Vec3f(0, 0, 0), Vec3f(200, 50, 0),fm.getResourceByAlias(Fonts::Serif), argsNames[i]+" : ", 15);
                Node* node = new Node(argsNames[i], label, Vec2f(0, 0), Vec2f(0.25, 0.025),rootObjectParams.get());
                label->setParent(pObjectsParameters);
                pObjectsParameters->addChild(label);
                if (argsTypes[i] == "odfaeg::physic::BoundingPolyhedron") {
                    Button* button = new Button(Vec3f(0, 0, 0), Vec3f(200, 50, 0),fm.getResourceByAlias(Fonts::Serif), "Select Area", 15, *wCreateNewObject);
                    node->addOtherComponent(button, Vec2f(0.75, 0.025));
                    button->setParent(pObjectsParameters);
                    pObjectsParameters->addChild(button);
                    button->addActionListener(this);
                } else {
                    TextArea* ta = new TextArea(Vec3f(0, 0, 0), Vec3f(200, 50, 0),fm.getResourceByAlias(Fonts::Serif), "", *wCreateNewObject);
                    node->addOtherComponent(ta, Vec2f(0.75, 0.025));
                    ta->setText(argsTypes[i]);
                    ta->setTextSize(15);
                    ta->setParent(pObjectsParameters);
                    pObjectsParameters->addChild(ta);
                    tmpTextAreas.push_back(ta);
                }
            }
        }
    }
    //dpSelectPointerType->setEventContextActivated(true);
    bCreateObject->setEventContextActivated(true);
}
void ODFAEGCreator::onSelectedMClassChanged(DropDownList *dp) {
    if (dp->getSelectedItem() != "Select class") {
        //std::cout<<"selected class : "<<dp->getSelectedItem()<<std::endl;
        dpSelectMFunction->removeAllItems();
        std::string selectedItem = dp->getSelectedItem();
        std::vector<std::string> parts = split(selectedItem, "::");
        Class cl = Class::getClass(parts[parts.size() - 1]);
        std::vector<MemberFunction> functions = cl.getMembersFunctions();
        dpSelectMFunction->addItem("Select function", 15);
        for (unsigned int i = 0; i < functions.size(); i++) {
            std::string name = functions[i].getReturnType()+" "+functions[i].getName()+"(";
            std::vector<std::string> argsTypes = functions[i].getArgsTypes();
            for (unsigned int j = 0; j < argsTypes.size(); j++) {
                //std::cout<<"add arg type : "<<argsTypes[j]<<std::endl;
                name += argsTypes[j];
                if (j != argsTypes.size() - 1) {
                    name += ",";
                }
            }
            name += ")";
            //std::cout<<"add function : "<<name<<std::endl;
            dpSelectMFunction->addItem(name, 15);
            //std::cout<<"add function : "<<i<<" : "<<name<<std::endl;
        }
    }
}
void ODFAEGCreator::onSelectedMFunctionChanged(DropDownList *dp) {
    if (dp->getSelectedItem() != "Select function" && dpSelectMClass->getSelectedItem() != "Select class") {
        rootMObjectParams->deleteAllNodes();
        pMObjectsParameters->removeAll();
        std::string selectedItem = dpSelectMClass->getSelectedItem();
        std::vector<std::string> parts = split(selectedItem, "::");
        //std::cout<<"parts : "<<parts[parts.size() - 1]<<std::endl;
        Class cl = Class::getClass(parts[parts.size() - 1]);
        tmpTextAreas.clear();
        std::vector<MemberFunction> functions = cl.getMembersFunctions();
        bool found = false;
        MemberFunction* f=nullptr;
        for (unsigned int i = 0; i < functions.size() && !found; i++) {
            std::string name = functions[i].getReturnType()+" "+functions[i].getName()+"(";
            std::vector<std::string> argsTypes = functions[i].getArgsTypes();
            for (unsigned int j = 0; j < argsTypes.size(); j++) {
                name += argsTypes[j];
                if (j != argsTypes.size() - 1) {
                    name += ",";
                }
            }
            name += ")";
            if (name == dp->getSelectedItem()) {
                //std::cout<<"select constructor"<<std::endl;
                f = &functions[i];
                found = true;
            }
        }
        if (f != nullptr) {
            FontManager<Fonts>& fm = cache.resourceManager<Font, Fonts>("FontManager");
            std::vector<std::string> argsTypes = f->getArgsTypes();
            std::vector<std::string> argsNames = f->getArgsNames();
            for (unsigned int i = 0; i < argsTypes.size(); i++) {
                //std::cout<<"add gui"<<std::endl;
                Label* label = new Label(*wModifyObject, Vec3f(0, 0, 0), Vec3f(200, 50, 0),fm.getResourceByAlias(Fonts::Serif), argsNames[i]+" : ", 15);
                Node* node = new Node(argsTypes[i], label, Vec2f(0, 0), Vec2f(0.25, 0.025),rootObjectParams.get());
                label->setParent(pMObjectsParameters);
                pMObjectsParameters->addChild(label);
                TextArea* ta = new TextArea(Vec3f(0, 0, 0), Vec3f(200, 50, 0),fm.getResourceByAlias(Fonts::Serif), "", *wModifyObject);
                ta->setText(argsTypes[i]);
                node->addOtherComponent(ta, Vec2f(0.75, 0.025));
                ta->setTextSize(15);
                ta->setParent(pMObjectsParameters);
                pMObjectsParameters->addChild(ta);
                tmpTextAreas.push_back(ta);
            }
        }
    }
    bModifyObject->setEventContextActivated(true);
}
void ODFAEGCreator::onDroppedDown(DropDownList* dp) {
    if (dp == dpSelectClass || dp == dpSelectFunction || dp == dpSelectPointerType) {
        bCreateObject->setEventContextActivated(false);
    }
    if (dp == dpSelectMClass || dp == dpSelectMFunction) {
        bModifyObject->setEventContextActivated(false);
    }
}
void ODFAEGCreator::convertSlash(std::string& path) {
    std::replace(path.begin(),path.end(), '\\', '/');
}
void ODFAEGCreator::onSelectPointerType(DropDownList* dp) {
    bCreateObject->setEventContextActivated(true);
}
void ODFAEGCreator::onSelectedScriptChanged(DropDownList* dp) {
    selectedObject->attachScript(dp->getSelectedItem());
    std::map<std::pair<std::string, std::string>, std::string>::iterator it;
    it = cppAppliContent.find(std::make_pair(getWorld()->projectName, minAppliname+".cpp"));
    if (it != cppAppliContent.end()) {
        if (it->second.find("if(entities[i]->getScript() == "+dp->getSelectedItem()+") {\n") == std::string::npos) {
            std::string toFind = "";
            toFind += "    std::vector<Entity*> entities = getWorld()->getRootEntities(\"*\");\n";
            toFind += "    for (unsigned int i = 0; i < entities.size(); i++) {\n";
            int pos = it->second.find(toFind)+toFind.size();
            std::string toInsert = "";
            toInsert += "    if(entities[i]->getScript() == "+dp->getSelectedItem()+") {\n";
            toInsert += "       MonoBehaviour* behaviour = new "+dp->getSelectedItem()+"();\n";
            toInsert += "       entities[i]->setBehaviour(behaviour);\n";
            toInsert += "    }";
        }
    }
}
void ODFAEGCreator::onViewPerspectiveChanged(DropDownList* dp) {
    if (dp->getSelectedItem() == "Ortho 2D") {
        std::string name = dpSelectComponent->getSelectedItem();
        std::vector<Component*> components = getRenderComponentManager().getRenderComponents();
        for (unsigned int i = 0; i < components.size(); i++) {
            if (name == components[i]->getName() && dynamic_cast<HeavyComponent*>(components[i])) {
                View view(getRenderWindow().getSize().x(), getRenderWindow().getSize().y(), getRenderWindow().getView().getViewport().getPosition().z(), getRenderWindow().getView().getDepth());
                static_cast<HeavyComponent*>(components[i])->setView(view);
                selectedComponentView = view;
            }
        }
    } else {
        std::string name = dpSelectComponent->getSelectedItem();
        std::vector<Component*> components = getRenderComponentManager().getRenderComponents();
        for (unsigned int i = 0; i < components.size(); i++) {
            if (name == components[i]->getName() && dynamic_cast<HeavyComponent*>(components[i])) {
                View view(getRenderWindow().getSize().x(), getRenderWindow().getSize().y(), 80, 1, 1000);
                view.setConstrains(80, 0);
                static_cast<HeavyComponent*>(components[i])->setView(view);
                selectedComponentView = view;
            }
        }
    }

    std::vector<Entity*> entities = getWorld()->getEntities("E_BIGTILE");
    if (entities.size() > 0) {
        Entity* heightMap = entities[0]->getRootEntity();
        for (unsigned int i = 0; i < getRenderComponentManager().getNbComponents(); i++) {
            if (getRenderComponentManager().getRenderComponent(i) != nullptr) {
               if (!getRenderComponentManager().getRenderComponent(i)->getView().isOrtho()) {
                    View view = getRenderComponentManager().getRenderComponent(i)->getView();
                    float height;
                    bool isOnHeightMap = heightMap->getHeight(Vec2f(view.getPosition().x(), view.getPosition().z()), height);
                    if (isOnHeightMap) {
                        view.setCenter(Vec3f(view.getPosition().x(), height + 10, view.getPosition().z()));
                        getRenderComponentManager().getRenderComponent(i)->setView(view);
                    }
               }
            }
        }
    }
}
void ODFAEGCreator::onSelectedTextureDroppedDown(DropDownList* dp) {

    bChooseText->setEventContextActivated(false);
    //std::cout<<"b choose tex false"<<std::endl;
    if (bAddTexRect != nullptr)
        bAddTexRect->setEventContextActivated(false);
    if (bSetTexRect != nullptr)
        bSetTexRect->setEventContextActivated(false);
        //std::cout<<"b set tex rect false"<<std::endl;
}
void ODFAEGCreator::onSelectedTextureNotDroppedDown (DropDownList* dp) {
    //std::cout<<"activate context bchoose text"<<std::endl;
    bChooseText->setEventContextActivated(true);
    if (bAddTexRect != nullptr)
        bAddTexRect->setEventContextActivated(true);
    if (bSetTexRect != nullptr)
        bSetTexRect->setEventContextActivated(true);
}
void ODFAEGCreator::onSelectedWallTypeDroppedDown(odfaeg::graphic::gui::DropDownList* dp) {
    if (dp == dpSelectWallType)
        bGenerateTerrain->setEventContextActivated(false);
    else if (dp == dpSelectWallType3D)
        bGenerate3DTerrain->setEventContextActivated(false);
}
void ODFAEGCreator::onSelectedWallTypeNotDroppedDown(odfaeg::graphic::gui::DropDownList* dp) {
    if (dp == dpSelectWallType)
        bGenerateTerrain->setEventContextActivated(true);
    else if (dp == dpSelectWallType3D)
        bGenerate3DTerrain->setEventContextActivated(true);
}
void ODFAEGCreator::onObjectOriginChanged(TextArea* ta) {
    if (ta == taOriginX) {
        if (is_number(ta->getText())) {
            float newXPos = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEXPOS"+conversionFloatString(newXPos));
            State* state = new State("SCHANGEXPOS", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getOrigin().x());
            state->addParameter("NEWVALUE", newXPos);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            selectedObject->setOrigin(Vec3f(newXPos, selectedObject->getOrigin().y(), selectedObject->getOrigin().z()));
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEXPOS", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getPosition().x());
                selectState->addParameter("NEWVALUE", newXPos);
                rectSelect.getItems()[i]->setOrigin(Vec3f(newXPos, rectSelect.getItems()[i]->getOrigin().y(), rectSelect.getItems()[i]->getOrigin().z()));
                sg->addState(selectState);
            }
        }
    } else if (ta == taOriginY) {
        if(is_number(ta->getText())) {
            float newYPos = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEYPOS"+conversionFloatString(newYPos));
            State* state = new State("SCHANGEYPOS", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getOrigin().y());
            state->addParameter("NEWVALUE", newYPos);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            selectedObject->setOrigin(Vec3f(selectedObject->getOrigin().x(), newYPos, selectedObject->getOrigin().z()));
            std::cout<<"center wall : "<<selectedObject->getCenter()<<std::endl;
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEYPOS", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getOrigin().y());
                selectState->addParameter("NEWVALUE", newYPos);
                rectSelect.getItems()[i]->setOrigin(Vec3f(rectSelect.getItems()[i]->getOrigin().x(), newYPos, rectSelect.getItems()[i]->getOrigin().z()));
                sg->addState(selectState);
            }
        }
    } else if (ta == tPosZ) {
        if(is_number(ta->getText())) {
            float newZPos = conversionStringFloat(ta->getText());
            StateGroup* sg = new StateGroup("SGCHANGEZPOS"+conversionFloatString(newZPos));
            State* state = new State("SCHANGEZPOS", &se);
            state->addParameter("OBJECT", selectedObject);
            state->addParameter("OLDVALUE", selectedObject->getOrigin().z());
            state->addParameter("NEWVALUE", newZPos);
            sg->addState(state);
            stateStack.addStateGroup(sg);
            selectedObject->setOrigin(Vec3f(selectedObject->getOrigin().x(), selectedObject->getOrigin().y(), newZPos));
            //updateScriptPos(selectedObject);
            for (unsigned int i = 1; i < rectSelect.getItems().size(); i++) {
                State* selectState = new State("SCHANGEZPOS", &se);
                selectState->addParameter("OBJECT", rectSelect.getItems()[i]);
                selectState->addParameter("OLDVALUE", rectSelect.getItems()[i]->getOrigin().z());
                selectState->addParameter("NEWVALUE", newZPos);
                rectSelect.getItems()[i]->setOrigin(Vec3f(rectSelect.getItems()[i]->getOrigin().x(), rectSelect.getItems()[i]->getOrigin().y(), newZPos));
                sg->addState(selectState);
            }
        }
    }
}
void ODFAEGCreator::onMouseOnMenu(MenuItem* menuItem) {
    if (menuItem->isVisible() && !menuItem->isContextChanged()) {
        std::cout<<"desactivate event context menu item"<<std::endl;
        setEventContextActivated(false);
        menuItem->setContextChanged(true);
    }
}
void ODFAEGCreator::onParentClickedCV (Label* label) {
    std::vector<std::string> parts = split(label->getText(), ":");
    std::string pName = parts[1].substr(1, parts[1].size()-1);
    if (pName != "NONE") {
        if (selectedObject->getType() == "E_ANIMATION") {
            displayAnimInfos(selectedObject);
        } else if (selectedObject->getType() == "E_WALL") {
            displayWallInfos(selectedObject);
        } else if (selectedObject->getType() == "E_DECOR") {
            displayDecorInfos(selectedObject);
        } else if (selectedObject->getType() == "E_BIGTILE") {
            displayBigtileInfos(selectedObject);
        } else if (selectedObject->getType() == "E_PARTICLES") {
            displayParticleSystemInfos(selectedObject);
        } else if (selectedObject->getType() == "E_PONCTUAL_LIGHT") {
            displayPonctualLightInfos(selectedObject);
        } else if (selectedObject->getType() == "E_TILE") {
            displayTileInfos(selectedObject);
        } else {
            displayExternalEntityInfo(selectedObject);
        }
        selectedBoundingVolume = selectedBoundingVolume->getParent();
        collisionBox.setSize(selectedBoundingVolume->getSize());
        collisionBox.setPosition(selectedBoundingVolume->getPosition());
    }
}
void ODFAEGCreator::makeTransparent(Entity* entity) {
    for (unsigned int  j = 0; j < entity->getNbFaces(); j++) {
        for (unsigned int k = 0; k < entity->getFace(j)->getVertexArray().getVertexCount(); k++) {
            entity->getFace(j)->getVertexArray()[k].color = Color::Transparent;
        }
    }
    std::vector<Entity*> children = entity->getChildren();
    for (unsigned int i = 0; i < children.size(); i++) {
        makeTransparent(children[i]);
    }
}
EntityFactory& ODFAEGCreator::getEntityFactory() {
    return factory;
}
unsigned int ODFAEGCreator::getCurrentId() {
    return currentId;
}
void ODFAEGCreator::addId(unsigned int id) {
    std::map<unsigned int, bool>::iterator it = callIds.find(id);
    if (it == callIds.end()) {
        callIds.insert(std::make_pair(id, false));
    }
}
void ODFAEGCreator::setIsCalled(unsigned int id) {
    std::map<unsigned int, bool>::iterator it = callIds.find(id);
    if (it != callIds.end()) {
        it->second = true;
    }
}
bool ODFAEGCreator::isAlreadyCalled(unsigned int id) {
    std::map<unsigned int, bool>::iterator it = callIds.find(id);
    if (it != callIds.end()) {
            return it->second;
    }
}
void ODFAEGCreator::removeLine(unsigned int line, std::string& content) {
    std::string cpContent = content;
    unsigned int tmpLine=0;
    unsigned int size = 0, pos = 0;
    while(line > 0 && tmpLine < line) {
        pos = size;
        unsigned int size = cpContent.find("\n")+1;
        cpContent.erase(0, size);
        tmpLine++;
    }
    content.erase(pos, size);
}
unsigned int ODFAEGCreator::findLine(std::string toFind, std::string content) {
    unsigned int line = 0;
    unsigned int pos = content.find("\n");
    unsigned int pos2 = content.find(toFind);
    unsigned int pos3 = 0;
    while (pos < pos2) {
        line++;
        pos3 = content.find("\n")+1;
        pos += pos3;
        content.erase(0, pos3);
    }
    return line;
}
void ODFAEGCreator::addBoundingVolumes(BoundingVolume* bv) {
    boundingVolumes.push_back(bv);
    for (unsigned int i = 0; i <  bv->getChildren().size(); i++) {
        addBoundingVolumes(bv->getChildren()[i]);
    }
}
std::string ODFAEGCreator::getNamespaceIn(std::string fileContent, unsigned int posInFile) {
    //check each namespaces englobing the class.
    std::string namespc="";
    int j= 0;
    while(fileContent.find("namespace ") != std::string::npos) {
        //Find the namespace pos.
        unsigned int pos = fileContent.find("namespace ");
        //Check the namespace name.
        fileContent = fileContent.substr(pos+9, fileContent.size()-pos-9);

        while(fileContent.size() > 0 && fileContent.at(0) == ' ') {
            fileContent.erase(0, 1);
        }
        std::vector<std::string> parts = split(fileContent, " ");
        //We add :: for each sub namespaces found.
        if (j == 0)
            namespc += parts[0];
        else
            namespc += "::"+parts[0];
        //we must check if the namespace is declared before the class.
        pos = fileContent.find_first_of("{");
        fileContent = fileContent.substr(pos+1, fileContent.size()-pos-1);
        pos = fileContent.find("namespace ");
        unsigned int pos2 = posInFile;

        //if there is no more namespace declaration after the class name we can check if the class is in the given namespace.
        if (pos > pos2) {
            //Erase eveything which is before the namespace declaration.
            fileContent = fileContent.substr(pos2);
        }
        j++;
    }
    return namespc;
}
std::string ODFAEGCreator::getHeaderContent(std::string content, unsigned int posInFile) {
    //On recherche si le contenu est celui d'un fichier .h sinon on recherche le fichier .h correspondant.
    std::map<std::pair<std::string, std::string>, std::string>::iterator it;
    for (it = cppAppliContent.begin(); it != cppAppliContent.end(); it++) {
        //On est déjà dans un fichier.h on le retourne.
        if ((it->first.second.find(".h") || it->first.second.find(".hpp")) && it->second == content);
            return it->second;
    }
    //Extract class names :
    unsigned int cumPos = 0;
    std::vector<std::string> parts = split(content, "::");
    for (unsigned int i = 0; i < parts.size(); i++) {

        std::vector<std::string> names = split(parts[i], " ");
        int pos = content.find(parts[i]);
        if (pos != std::string::npos) {
            cumPos += pos;

            std::string content = content.substr(pos, content.size()-pos);

            int pos2 = 0;
            std::string cpContent = content;
            findLastBracket(cpContent, 0, pos2);
            if (pos2 != std::string::npos) {

                //If we are arrived at the class definition.
                if (cumPos < posInFile && posInFile < cumPos + pos2) {
                    //Check where the class is defined.
                    for (it = cppAppliContent.begin(); it != cppAppliContent.end(); it++) {
                        std::string subContent2 = it->second;
                        int posC = subContent2.find("class");
                        while (posC != std::string::npos) {
                            int pos2 = subContent2.find(names[names.size()-1]);
                            int pos3 = subContent2.find("{");
                            if (pos2 != std::string::npos && pos3 != std::string::npos) {
                                if (posC < pos2 && pos2 < pos3)
                                    return it->second;
                            }
                            subContent2.erase(0, pos);
                            posC = subContent2.find("class");
                        }
                    }
                }
                content.erase(0, pos2);
                cumPos += pos2;
            }
        }
    }
    return "";
}
void ODFAEGCreator::findLastBracket(std::string& fileContent, unsigned int nbBlocks, int& p) {
    //Recherche la position du dernier crochet fermant d'un ensemble de blocs.
    unsigned int pos, pos2;
    do {

        pos = fileContent.find("{");
        pos2 = fileContent.find("}");
        if (pos != std::string::npos && pos2 != std::string::npos) {
            //Crocher ouvrant avant crochet fermant : nouveau sous bloc.
            if (pos < pos2) {
                nbBlocks++;
                //On supprime le crochet car il a déjà été traité.
                fileContent.erase(pos, 1);
                p++;
            } else {
                //Crochet fermant après crochet ouvrant :  fin du bloc.
                nbBlocks--;
                //On supprime le crochet car il a déjà été traité.
                fileContent.erase(pos2, 1);
                p++;
            }
        }
        //On répète tant que l'on trouve des crochets et qu'ils sont dans le premier bloc de fileContent.
    } while (nbBlocks > 0 || pos == std::string::npos || pos2 == std::string::npos);
    p += pos2;
}
void ODFAEGCreator::removeSpacesChars(std::string& str) {
    //remove spaces and \n at the beginning and at the end.
    while (str.size() > 0 && str.at(0) == ' ' || str.at(0) == '\n') {
        str = str.erase(0, 1);
    }
    while (str.size() > 0 && str.at(str.size()-1) == ' ' || str.at(str.size()-1) == '\n') {
        str = str.erase(str.size()-1, 1);
    }
}
std::vector<std::string> ODFAEGCreator::checkCompletionNames(std::string letters, unsigned int posInFile) {
    std::vector<std::string> namesToPropose;
    std::string content = tScriptEdit->getText();
    //Contenu du fichier.h
    std::string header_content = getHeaderContent(content,posInFile);
    //Recherche du namespace dans lequel on se trouver.
    std::string namespc = getNamespaceIn(content, posInFile);
    //On recherche dans quel bloc de quelle fonction de quelle classe on se trouve.
    std::vector<std::string> classes = Class::getClassesFromMemory(header_content);
    unsigned int cumPos = 0;
    for (unsigned int i = 0; i < classes.size(); i++) {
        Class classs = Class::getClassFromMemory(classes[i], "", header_content);
        //Ok si la classe est dans le même namespace que dans le fichier.
        if (classs.getNamespace() == namespc) {
            std::vector<MemberFunction> functions = classs.getMembersFunctions();
            for (unsigned int f = 0; f < functions.size(); f++) {
                int p = content.find(functions[f].getName());
                if (p != std::string::npos) {
                    cumPos += p;
                    std::string subContent = content.substr(p, content.size()-p);
                    int pos = subContent.find("{");
                    if (pos != std::string::npos) {
                        subContent = subContent.substr(pos, subContent.size() - pos);

                        cumPos += pos;

                        int pos2 = 0;
                        std::string cSubContent = subContent;
                        findLastBracket(cSubContent, 0, pos2);
                        if (pos2 != std::string::npos) {

                            //Si on est entre les crochets d'ouverture et de fermeture de la fonction on est dans le bon bloc.
                            if (cumPos < posInFile && posInFile < cumPos + pos2) {
                                std::string bloc = subContent.substr(pos, pos2-pos);
                                BlocInfo parentBloc;
                                parentBloc.blocStart = cumPos;
                                parentBloc.blocEnd = cumPos + pos2;
                                for (unsigned int a = 0; a < functions[f].getArgsTypes().size(); a++) {
                                    parentBloc.blocInstances.insert(std::make_pair(pos, std::make_pair(functions[f].getArgsTypes()[a], functions[f].getArgsNames()[a])));
                                }
                                parentBloc.blocInstances.insert(std::make_pair(pos, std::make_pair(classs.getName(), "this")));
                                std::vector<std::string> instructions = split(bloc, ";");
                                unsigned int currentInst = 0;
                                unsigned int currentPos = cumPos;
                                //Recherche les informations sur les variables dans les blocs.
                                findComplVarsInBloc(instructions, parentBloc, currentInst, currentPos);
                                //Recherche des noms que l'on peut proposer pour la completion.
                                checkNamesToPropose(parentBloc, namesToPropose, letters, posInFile);
                            }
                            //Contenu traité on l'efface.
                            content.erase(0, pos2);
                            cumPos += pos2;
                        }
                    }
                }
            }
        }
    }
    return namesToPropose;
}
void ODFAEGCreator::findComplVarsInBloc(std::vector<std::string>& instructions, BlocInfo& parentBloc, unsigned int& currentInst, unsigned int& currentPos) {
    BlocInfo subBloc;
    while (currentInst < instructions.size()) {
        std::string inst = instructions[currentInst];
        removeSpacesChars(inst);
        int pos = inst.find("{");
        int pos2 = inst.find("}");
        if (pos != std::string::npos) {
            inst.erase(0, 1);
            subBloc.blocStart = currentPos;
            findComplVarsInBloc(instructions, subBloc, currentInst, currentPos);
        } else if (pos2 != std::string::npos) {
            inst.erase(0, 1);
            subBloc.blocEnd = currentPos;
            parentBloc.subBlocs.push_back(subBloc);
            return;
        } else {
            processInst(inst, currentPos, subBloc);
            currentInst++;
        }
        if (currentInst < instructions[currentInst].size())
            currentPos += instructions[currentInst].size();
    }
}
void ODFAEGCreator::processInst(std::string inst, unsigned int currentPos, BlocInfo& bloc) {
    std::vector<std::string> classes = Class::getClasses("");
    std::vector<std::string> allTypeNames;
    for (unsigned int i = 0; i < primitiveTypes.size(); i++) {
        allTypeNames.push_back(primitiveTypes[i]);
    }
    for (unsigned int i = 0; i < classes.size(); i++) {
        allTypeNames.push_back(classes[i]);
    }

    for (unsigned int i = 0; i < allTypeNames.size(); i++) {
        if (inst.find(allTypeNames[i]) != std::string::npos) {
            if (inst.find(",") != std::string::npos) {
                std::vector<std::string> argsComa = split(inst, ",");
                for (unsigned int j = 0; j < argsComa.size(); j++) {
                    removeSpacesChars(argsComa[j]);
                    std::string argName = "";
                    if (argsComa[j].find("=") != std::string::npos) {
                        std::vector<std::string> argsEqual = split(argsComa[j], "=");
                        removeSpacesChars(argsEqual[0]);
                        std::vector<std::string> argTypeName = split(argsEqual[0], " ");
                        for (unsigned int k = 0; k < argTypeName.size(); k++) {
                            removeSpacesChars(argTypeName[k]);
                        }
                        if (j == 0) {
                            if (argTypeName[0] == "const" || argTypeName[0] == "unsigned") {
                                argName = argTypeName[2];
                            } else {
                                argName = argTypeName[1];
                            }
                        } else {
                            argName = argTypeName[0];
                        }
                    } else {
                        std::vector<std::string> argTypeName = split(argsComa[j], " ");
                        for (unsigned int k = 0; k < argTypeName.size(); k++) {
                            removeSpacesChars(argTypeName[k]);
                        }
                        if (j == 0) {
                            if (argTypeName[0] == "const" || argTypeName[0] == "unsigned") {
                                argName = argTypeName[2];
                            } else {
                                argName = argTypeName[1];
                            }
                        } else {
                            argName = argTypeName[0];
                        }
                    }
                    bloc.blocInstances.insert(std::make_pair(currentPos, std::make_pair(allTypeNames[i], argName)));
                }
            } else {
                std::string argName = "";
                if (inst.find("=") != std::string::npos) {
                    std::vector<std::string> argsEqual = split(inst, "=");
                    removeSpacesChars(argsEqual[0]);
                    std::vector<std::string> argTypeName = split(argsEqual[0], " ");
                    for (unsigned int k = 0; k < argTypeName.size(); k++) {
                        removeSpacesChars(argTypeName[k]);
                    }
                    //check if there is a const qualifier.
                    if (argTypeName[0] == "const" || argTypeName[0] == "unsigned") {
                        argName = argTypeName[2];
                    } else {
                        argName = argTypeName[1];
                    }
                } else {
                    std::vector<std::string> argTypeName = split(inst, " ");
                    for (unsigned int k = 0; k < argTypeName.size(); k++) {
                        removeSpacesChars(argTypeName[k]);
                    }
                    //check if there is a const qualifier.
                    if (argTypeName[0] == "const" || argTypeName[0] == "unsigned") {
                        argName = argTypeName[2];
                    } else {
                        argName = argTypeName[1];
                    }
                }
                bloc.blocInstances.insert(std::make_pair(currentPos, std::make_pair(allTypeNames[i], argName)));
            }
        }
    }
}
bool ODFAEGCreator::isCharsOk(std::string strsearch, std::string str) {
    if (strsearch.size() <= str.size()) {
        for (unsigned int i = 0; i < strsearch.size(); i++) {
            if(strsearch.at(i) != str.at(i)) {
                return false;
            }
        }
        return true;
    }
    return false;
}
void ODFAEGCreator::checkNamesToPropose(BlocInfo parentBloc, std::vector<std::string>& namesToPropose, std::string strsearch, unsigned int posInFile) {
    if (strsearch.find(".") != std::string::npos || strsearch.find("->") != std::string::npos) {
        std::vector<std::string> parts = split(strsearch, ".");
        std::string name = parts[0];
        std::map<unsigned int, std::pair<std::string, std::string>>::iterator it;
        for (it = parentBloc.blocInstances.begin(); it != parentBloc.blocInstances.end(); it++) {
            if (posInFile > it->first && parentBloc.blocStart < posInFile && parentBloc.blocEnd > posInFile) {
                if (it->second.second == name) {
                    Class classs = Class::getClass(it->second.first);
                    for (unsigned int i = 0; i < classs.getMembersFunctions().size(); i++) {
                        bool charsOk = isCharsOk(strsearch, classs.getMembersFunctions()[i].getName());
                        if (charsOk)
                            namesToPropose.push_back(classs.getMembersFunctions()[i].getName());
                    }
                    for (unsigned int i = 0; i < classs.getMembersVariables().size(); i++) {
                        bool charsOk = isCharsOk(strsearch, classs.getMembersVariables()[i].getVarName());
                        if (charsOk)
                        namesToPropose.push_back(classs.getMembersVariables()[i].getVarName());
                    }
                    std::queue<Class> q;
                    for(auto& sc : classs.getSuperClasses())
                        q.push(sc);
                    while (!q.empty()) {
                        Class current = q.front();
                        q.pop();
                        for (unsigned int i = 0; i < current.getMembersFunctions().size(); i++) {
                            bool charsOk = isCharsOk(strsearch, current.getMembersFunctions()[i].getName());
                            if (charsOk)
                                namesToPropose.push_back(current.getMembersFunctions()[i].getName());
                        }
                        for (unsigned int i = 0; i < current.getMembersVariables().size(); i++) {
                            bool charsOk = isCharsOk(strsearch, current.getMembersVariables()[i].getVarName());
                            if (charsOk)
                            namesToPropose.push_back(current.getMembersVariables()[i].getVarName());
                        }
                        // Ajouter ses super-classes dans la file
                        for (auto& sc : current.getSuperClasses()) {
                            q.push(sc);
                        }
                    }
                }
            }
        }
    } else {
        std::map<unsigned int, std::pair<std::string, std::string>>::iterator it;
        //Parcours de toutes les instances du bloc.
        for (it = parentBloc.blocInstances.begin(); it != parentBloc.blocInstances.end(); it++) {
            //Si on est après la position de la déclaration de la variable et dans le bloc on peut rechercher.
            if (posInFile > it->first && parentBloc.blocStart < posInFile && parentBloc.blocEnd > posInFile) {
                //Si les caractères correspondent on peut proposer le nom de la variable.
                bool charsOk = isCharsOk(strsearch, it->second.second);
                if (charsOk) {
                    namesToPropose.push_back(it->second.second);
                }
            }
        }
    }
    for (unsigned int i = 0; i < parentBloc.subBlocs.size(); i++) {
        checkNamesToPropose(parentBloc.subBlocs[i], namesToPropose, strsearch, posInFile);
    }
}
void ODFAEGCreator::onTextEntered(char caracter) {
    if (!std::isalpha(caracter) && caracter != '.' && caracter != '-' && caracter != '>' && caracter != ':') {
        strsearch = "";
    } else {
        strsearch += caracter;
    }
    unsigned int charPosInFile = tScriptEdit->getCharacterIndexAtCursorPos();
    std::vector<std::string> completionNames = checkCompletionNames(strsearch, charPosInFile);
    for (unsigned int i = 0; i < completionNames.size(); i++) {
        std::cout<<"completion name "<<i<<" : "<<completionNames[i]<<std::endl;
    }
}

