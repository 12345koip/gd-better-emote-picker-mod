#include <Geode/Geode.hpp>
#include <Geode/modify/EmotePickerPopup.hpp>
#include <Geode/modify/EditorWindowPopup.hpp>
#include "Search/SearchBar.hpp"

using namespace geode::prelude;
using namespace BetterEmojiPicker::Search;

class $modify(EmotePickerPopup) {

    bool init(CCTextInputNode* input, CCPoint at, int maxEmotes, int maxLength) {
        if (!EmotePickerPopup::init(input, at, maxEmotes, maxLength))
            return false;

        auto oldBg = this->m_background;
        auto newBg = CCScale9Sprite::create("GJ_square01.png", {0, 0, 80, 80});

        newBg->setContentSize(oldBg->getContentSize());
        newBg->setPosition(oldBg->getPosition());
        newBg->setZOrder(oldBg->getZOrder());
        newBg->setOpacity(oldBg->getOpacity());

        this->m_mainLayer->addChild(newBg);
        oldBg->removeFromParent();
        this->m_background = newBg;

        //search bar.
        makeSearchBar(this);

        //replace close button.
        auto cache = CCSpriteFrameCache::sharedSpriteFrameCache();
        if (auto frame = cache->spriteFrameByName("GJ_closeBtn_001.png")) {
            this->m_closeSprite->setDisplayFrame(frame);
            this->m_closeSprite->setScale(0.7f);
            this->m_closeSprite->setPositionX(newBg->getPositionX() + ((newBg->getContentSize().width / 2) - 1.0f));
        }

        return true;
    }

    void onBuildWindowContent() override {
        EmotePickerPopup::onBuildWindowContent();
    }

    void layoutEmoteGrid() {
        log::info("layoutEmoteGrid called");
        EmotePickerPopup::layoutEmoteGrid();
    }

    CCMenuItemSpriteExtra* addCell(const EmoteDef* p1, int p2) {
        log::info("addCell called");
        return EmotePickerPopup::addCell(p1, p2);
    }
};

class $modify(SearchWindow, EditorWindowPopup) {

    // positions: reset by layoutWindow on init, resize, every drag move, touch end/cancel
    void layoutWindow() {
        EditorWindowPopup::layoutWindow();

        if (!this->m_isEmotePicker) return;
        if (!this->m_mainLayer->getChildByID("search-bar"_spr)) return;

        this->m_background->setPositionY(this->m_background->getPositionY() + kSearchBarHeight / 2);

        // layoutWindow just reset the close button to the unextended corner, so shift it fresh each time.
        if (this->m_closeSprite) {
            this->m_closeSprite->setPosition({
                this->m_background->getPositionX() + (this->m_background->getContentSize().width / 2 - 1.0f),
                this->m_closeSprite->getPositionY() + kSearchBarHeight});
        }

        this->placeSearchBar();
    }

    void placeSearchBar() {
        auto bar = this->m_mainLayer->getChildByID("search-bar"_spr);
        auto scroller = this->m_scrollContentLayer;
        if (!bar || !scroller) return;

        const auto viewSize = scroller->m_viewSize;
        const auto pos = scroller->getPosition();
        const float scrollTop = pos.y + (1.0f - scroller->getAnchorPoint().y) * viewSize.height;
        const float bgTop = this->m_background->getPositionY() + this->m_background->getContentSize().height / 2.0f;
        const float barWidth = std::min(viewSize.width, this->m_background->getContentSize().width - 8.0f);

        bar->setContentSize({barWidth, kSearchBarHeight});
        bar->setPosition({
            this->m_background->getPositionX(),
            std::min(
                scrollTop + kSearchBarHeight / 2.0f,
                bgTop - kSearchBarHeight / 2.0f
            )
        });

        if (auto input = static_cast<TextInput*>(bar->getChildByID("search-input-bar"_spr))) {
            input->setWidth(barWidth - 16.0f);
            input->setPosition(bar->getContentSize() / 2.0f);
        }
    }
};