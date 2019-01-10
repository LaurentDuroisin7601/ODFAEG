#ifndef ODFAEGCREATOR
#define ODFAEGCREATOR
#include "odfaeg/Core/application.h"
#include "odfaeg/Graphics/GUI/menubar.hpp"
#include "odfaeg/Graphics/GUI/menuItem.hpp"
#include "odfaeg/Graphics/GUI/filedialog.hpp"
#include "odfaeg/Graphics/GUI/textArea.hpp"
#include "odfaeg/Graphics/GUI/button.hpp"
#include "odfaeg/Graphics/GUI/label.hpp"
#include "odfaeg/Graphics/GUI/dropDownList.hpp"
class ODFAEGCreator : public odfaeg::core::Application,
                      public odfaeg::graphic::gui::MenuItemListener,
                      public odfaeg::graphic::gui::ActionListener {
    public :
    ODFAEGCreator (sf::VideoMode vm, std::string title);
    void onLoad();
    void onInit ();
    void onRender(odfaeg::graphic::RenderComponentManager *cm);
    void onDisplay(odfaeg::graphic::RenderWindow* window);
    void onUpdate (odfaeg::graphic::RenderWindow*, sf::Event& event);
    void onExec ();
    void processKeyHeldDown(sf::Keyboard::Key key);
    void actionPerformed(odfaeg::graphic::gui::Button* item);
    void actionPerformed(odfaeg::graphic::gui::MenuItem* item);
    void showProjectsFiles(odfaeg::graphic::gui::Label* label);
    void showSourcesFiles(odfaeg::graphic::gui::Label* label);
    void showHeadersFiles(odfaeg::graphic::gui::Label* label);
    void showGUI(odfaeg::graphic::gui::Label* label);
    void showFileContent(odfaeg::graphic::gui::Label* lab);
    enum Fonts {
        Serif
    };
    private :
        odfaeg::graphic::gui::MenuBar* menuBar;
        odfaeg::graphic::gui::Menu *menu1, *menu2, *menu3;
        odfaeg::graphic::gui::MenuItem *item11, *item12, *item13, *item21, *item22, *item23, *item31, *item32, *item33;
        odfaeg::core::ResourceCache<> cache;
        odfaeg::graphic::gui::FileDialog* fdTexturePath;
        odfaeg::graphic::RenderWindow* wApplicationNew;
        odfaeg::graphic::gui::TextArea* ta;
        odfaeg::graphic::gui::DropDownList* dpList;
        odfaeg::graphic::gui::Label *lWidth, *lHeight;
        odfaeg::graphic::gui::TextArea *taWidth, *taHeight, *tScriptEdit;
        odfaeg::graphic::gui::Panel *pProjects, *pScriptsFiles, *pScriptsEdit;
        std::string appliname, minAppliname;
        std::string applitype;
        std::string path;
};
#endif