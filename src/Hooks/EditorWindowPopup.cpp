#include <Geode/Geode.hpp>
#include <Geode/modify/EditorWindowPopup.hpp>

using namespace geode::prelude;

static void placeSearchBar(const EditorWindowPopup* popup) {
    auto bar = popup->m_mainLayer->getChildByID("search-bar"_spr);
    if (!bar) return;

    const auto centre = popup->m_background->getPosition();
    bar->setPosition({
        centre.x - 33.0f,
        centre.y + (popup->m_windowHeight / 2) - 20.0f
    });
}

class $modify(EditorWindowPopup) {
    void layoutWindow() {
        EditorWindowPopup::layoutWindow();
        if (!this->m_isEmotePicker) return;

        this->m_closeSprite->setPositionX(
            this->m_background->getPositionX() + (this->m_background->getContentSize().width / 2 - 1.0f)
        );
    }

    void applyWindowSize() {
        EditorWindowPopup::applyWindowSize();
        if (!this->m_isEmotePicker) return;

        this->m_scrollContentLayer->setViewSize({
            this->m_windowWidth,
            this->m_windowHeight - 34.0f
        });
        typeinfo_cast<EmotePickerPopup*>(this)->layoutEmoteGrid(); //dont ask its the only thing that worked
        placeSearchBar(this);
    }
};