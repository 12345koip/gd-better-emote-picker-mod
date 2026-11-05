#include <Geode/Geode.hpp>
#include <Geode/modify/EditorWindowPopup.hpp>
#include "../Search/SearchBar.hpp"

using namespace geode::prelude;
using namespace BetterEmojiPicker::Search;

class $modify(ModifiedEmotePickerPopup, EditorWindowPopup) {
    void applyWindowSize() {
        EditorWindowPopup::applyWindowSize();

        if (!this->m_isEmotePicker) return;
        auto bar = this->m_mainLayer->getChildByID("search-bar"_spr);
        auto scroller = this->m_scrollContentLayer;
        if (!bar || !scroller) return;

        const auto viewSize = scroller->m_viewSize;
        scroller->setViewSize({viewSize.width, viewSize.height - kSearchBarHeight});
        scroller->clampOffset();
        scroller->applyContentPosition();

        this->placeSearchBar();
    }

    void layoutWindow() {
        EditorWindowPopup::layoutWindow();

        if (!this->m_isEmotePicker || !this->m_mainLayer->getChildByID("search-bar"_spr)) return;

        this->m_closeSprite->setPositionX(this->m_background->getPositionX() + (this->m_background->getContentSize().width / 2 - 1.0f));
        this->placeSearchBar();
    }

    void placeSearchBar() {
        auto bar = this->m_mainLayer->getChildByID("search-bar"_spr);
        auto scroller = this->m_scrollContentLayer;
        if (!bar || !scroller) return;

        const auto viewSize = scroller->m_viewSize;
        const auto pos = scroller->getPosition();
        const float scrollTop = pos.y + viewSize.height;
        const float barWidth = std::min(viewSize.width, this->m_background->getContentSize().width - 8.0f);

        bar->setContentSize({barWidth, kSearchBarHeight});
        bar->setPosition({this->m_background->getPositionX(), scrollTop + kSearchBarHeight / 2.0f});

        if (auto input = static_cast<TextInput*>(bar->getChildByID("search-input-bar"_spr))) {
            input->setWidth((barWidth - 16.0f) / input->getScale());
            input->setPosition(bar->getContentSize() / 2.0f);
        }
    }
};