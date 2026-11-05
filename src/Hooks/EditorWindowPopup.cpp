#include <Geode/Geode.hpp>
#include <Geode/modify/EditorWindowPopup.hpp>

using namespace geode::prelude;

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
        this->placeSearchBar();
    }

    void placeSearchBar() {
        auto bar = this->m_mainLayer->getChildByID("search-bar"_spr);
        if (!bar) return;

        const auto centre = this->m_background->getPosition();
        bar->setPosition({
            centre.x - 33.0f,
            centre.y + (this->m_windowHeight / 2) - 20.0f
        });
    }
};