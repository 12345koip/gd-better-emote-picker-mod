#include <Geode/Geode.hpp>
#include <Geode/modify/EditorWindowPopup.hpp>
#include <Geode/modify/EmotePickerPopup.hpp>
#include "../Search/Search.hpp"

using namespace geode::prelude;

constexpr float kCellRowSeparatorGap = 26.0f;
constexpr float kEmoteSectionsGap = 12.0f;
constexpr int kMaxNumCols = 8;
constexpr float kFirstCellX = 21.0f;

//genuinely the best way i could come up with to do this
static void placeEmoteCell(CCNode* node, float& y, int& col, int& placedInSection) {
    if (col == kMaxNumCols) {
        y -= kCellRowSeparatorGap;
        col = 0;
    }

    node->setVisible(true);
    node->setPosition({kFirstCellX + (col * kCellRowSeparatorGap), y - (kCellRowSeparatorGap / 2)});

    ++col;
    ++placedInSection;
}

static void endSection(float& y, const int col, const int numPlacedInSection) {
    if (numPlacedInSection == 0) return;

    if (col > 0)
        y -= kCellRowSeparatorGap;

    y -= kEmoteSectionsGap;
}

class $modify(EmotePickerPopup) {
    struct Fields {
        std::string searchQuery;
        geode::Label* noSearchResultsLabel = nullptr;
    };

    bool init(CCTextInputNode* input, CCPoint position, int maxEmotes, int maxLength) {
        if (!EmotePickerPopup::init(input, position, maxEmotes, maxLength))
            return false;


        auto* noSearchResultsLabel = geode::Label::create("No results :(", "bigFont.fnt");
        noSearchResultsLabel->setScale(0.55f);
        noSearchResultsLabel->setOpacity(200);
        noSearchResultsLabel->setPosition(this->m_background->getPosition());
        noSearchResultsLabel->setVisible(false);
        noSearchResultsLabel->setID("no-search-results-label"_spr);
        this->m_fields->noSearchResultsLabel = noSearchResultsLabel;
        this->m_mainLayer->addChild(noSearchResultsLabel);


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
            this->m_fields->searchQuery = geode::utils::string::toLower(input);
            this->layoutEmoteGrid();
        });

        //I know ZOrder is how touch prio is meant to work in v6. But... surprise surprise!! it doesn't. Only this does. Why? No clue!
        auto searchBarNode = searchBar->getChildByType<CCTextInputNode>(0);
        CCDirector::get()->getTouchDispatcher()->addTargetedDelegate(searchBarNode, -501, true);

        this->m_mainLayer->addChild(searchBar, this->m_scrollContentLayer->getZOrder());
        this->applyWindowSize();

        return true;
    }

    //this is basically a reimplementation of whats in the binary except we filter based on the query
    //warning: ugly (sorry)
    void layoutEmoteGrid() {
        float currentYPos = -4.0f;
        int col = 0;
        int numPlacedInSection = 0;
        uint32_t totalPlaced = 0;

        const auto& searchQuery = this->m_fields->searchQuery;
        const bool isSearching = !searchQuery.empty();
        const auto matchingIndices = BetterEmotePicker::Search::GetMatchingEmoteIndicesForSearchQuery(searchQuery);

        if (!isSearching && !this->m_recentEmoteButtons.empty()) { //no search = show recents
            for (auto* cell: this->m_recentEmoteButtons) {
                placeEmoteCell(cell, currentYPos, col, numPlacedInSection);
                ++totalPlaced;
            }

            currentYPos -= (kCellRowSeparatorGap + kEmoteSectionsGap);
            col = 0;
            numPlacedInSection = 0;
        } else {
            for (auto* cell: this->m_recentEmoteButtons)
                cell->setVisible(false);
        }

        //now we place the ones which match the query.
        for (int i = 0; i < EmoteSprite::defCount(); ++i) {
            auto def = EmoteSprite::defAtIndex(i);
            if (def->m_isAlias) continue;

            if (!def->m_name) { //a section terminator.
                endSection(currentYPos, col, numPlacedInSection);
                col = 0;
                numPlacedInSection = 0;
                continue;
            }

            auto cell = this->m_scrollContentMenu->getChildByTag(i);
            if (!cell) continue;

            const std::string_view name = def->m_name;
            if (isSearching && (std::ranges::find(matchingIndices, def->m_particleIndex) == matchingIndices.end() && name.find(searchQuery) == std::string::npos))
                cell->setVisible(false);
            else {
                placeEmoteCell(cell, currentYPos, col, numPlacedInSection);
                ++totalPlaced;
            }
        }

        if (auto label = this->m_fields->noSearchResultsLabel)
            label->setVisible(totalPlaced == 0);
        this->m_scrollContentLayer->setContentHeight(5.0f - currentYPos);
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override { //drag anywhere except in the scroller
        if (!this->m_isEmotePicker)
            return EmotePickerPopup::ccTouchBegan(touch, event);

        auto touchPoint = this->m_scrollContentLayer->convertToNodeSpace(touch->getLocation());
        CCRect touchArea {0.0f, 0.0f, this->m_scrollContentLayer->getContentWidth(), this->m_scrollContentLayer->getContentHeight()};
        this->m_allowDraggingAnywhere = !touchArea.containsPoint(touchPoint);
        return EmotePickerPopup::ccTouchBegan(touch, event);
    }
};