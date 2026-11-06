#include <Geode/Geode.hpp>
#include <Geode/modify/EditorWindowPopup.hpp>
#include <Geode/modify/EmotePickerPopup.hpp>
#include "../Search/Search.hpp"

using namespace geode::prelude;
using namespace BetterEmotePicker::Search;

constexpr int kSearchBarTouchPriority = -501;
constexpr int kRecentEmoteTagGap = 100000;
constexpr uint32_t kTotalEmoteTableEntries = 262;
constexpr uint8_t kCellSeparateFactor = 26.0f;
constexpr float kRecentsSeparateFactor = 38.0f;
constexpr float kWindowWidthPad = 8.0f;

void placeEmoteCell(CCNode* node, int numCols, float xPad, float& y, int& col, int& placedInSection) {
    if (col == numCols) {
        y -= kCellSeparateFactor;
        col = 0;
    }

    node->setVisible(true);
    node->setPosition({
        xPad + (col + 0.5f) * kCellSeparateFactor,
        y - (kCellSeparateFactor / 2.0f)
    });

    ++col;
    ++placedInSection;
}

class $modify(EmotePickerPopup) {
    struct Fields {
        std::string searchQuery;
    };

    bool init(CCTextInputNode* input, CCPoint position, int maxEmotes, int maxLength) {
        if (!EmotePickerPopup::init(input, position, maxEmotes, maxLength))
            return false;

        //replace bg bc it looks nicer
        auto oldBg = this->m_background;
        auto newBg = CCScale9Sprite::create("GJ_square01.png", {0, 0, 80, 80});
        newBg->setContentSize(oldBg->getContentSize());
        newBg->setPosition(oldBg->getPosition());
        newBg->setZOrder(oldBg->getZOrder());
        newBg->setOpacity(oldBg->getOpacity());

        this->m_mainLayer->addChild(newBg);
        oldBg->removeFromParent();
        this->m_background = newBg;

        //close button
        auto cache = CCSpriteFrameCache::sharedSpriteFrameCache();
        if (auto frame = cache->spriteFrameByName("GJ_closeBtn_001.png")) {
            this->m_closeSprite->setDisplayFrame(frame);
            this->m_closeSprite->setScale(0.7f);
        }

        //search bar
        auto searchBar = TextInput::create(this->m_windowWidth - 35.0f, "Search...");
        searchBar->setID("search-bar"_spr);
        searchBar->setScale(0.75f);
        searchBar->setCallback([this](const std::string& input) -> void {
            this->m_fields->searchQuery = input;
            this->layoutEmoteGrid();
        });

        //I know ZOrder is how touch prio is meant to work in v6. But... surprise surprise!! it doesn't. Only this does. Why? No clue!
        auto searchBarNode = searchBar->getChildByType<CCTextInputNode>(0);
        CCDirector::get()->getTouchDispatcher()->addTargetedDelegate(searchBarNode, kSearchBarTouchPriority, true);

        this->m_mainLayer->addChild(searchBar, this->m_scrollContentLayer->getZOrder());
        this->applyWindowSize();

        return true;
    }

    //this is basically a reimplementation of whats in the binary except we filter based on the query
    //warning: ugly (sorry)
    void layoutEmoteGrid() {
        const int numColsPerRow = std::max(3.0f, (this->m_windowWidth - kWindowWidthPad) / kCellSeparateFactor);
        const float xPad = (this->m_windowWidth - numColsPerRow * kCellSeparateFactor) * 0.5f;
        float currentYPos = -4.0f;
        int col = 0;
        int numPlacedInSection = 0;

        const auto& searchQuery = this->m_fields->searchQuery;
        const bool isSearching = !searchQuery.empty();
        const auto matchingIndices = GetMatchingEmoteIndicesForSearchQuery(searchQuery);

        if (!isSearching) { //no search = show recents
            uint32_t placedRecents = 0;
            for (auto* cell: this->m_recentEmoteButtons) {
                int index = cell->getTag() - kRecentEmoteTagGap;
                placeEmoteCell(cell, numColsPerRow, xPad, currentYPos, col, numPlacedInSection);
                ++placedRecents;
            }

            if (!this->m_recentEmoteButtons.empty()) {
                currentYPos -= kRecentsSeparateFactor;
                col = 0;
            }
        } else {
            for (auto* cell: this->m_recentEmoteButtons)
                cell->setVisible(false);
        }

        //now we place the ones which match the query.
        numPlacedInSection = 0;
        const auto numEntries = EmoteSprite::defCount();
        for (int i = 0; i < numEntries; ++i) {
            auto def = EmoteSprite::defAtIndex(i);

            if (!def->m_name) {
                if (numPlacedInSection == 0) continue;

                if (col > 0)
                    currentYPos -= kCellSeparateFactor;

                currentYPos -= kCellSeparateFactor / 2.0f;
                col = 0;
                numPlacedInSection = 0;
                continue;
            }
            if (def->m_isAlias) continue;

            auto cell = this->m_scrollContentMenu->getChildByTag(i);
            if (!cell) continue;

            if (isSearching && std::ranges::find(matchingIndices, def->m_particleIndex) == matchingIndices.end()) {
                cell->setVisible(false);
                continue;
            }

            placeEmoteCell(cell, numColsPerRow, xPad, currentYPos, col, numPlacedInSection);
        }

        this->m_scrollContentLayer->setContentHeight(5.0f - currentYPos);
    }
};