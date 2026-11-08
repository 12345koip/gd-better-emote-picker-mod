#include <Geode/Geode.hpp>
#include <Geode/modify/EditorWindowPopup.hpp>
#include <Geode/modify/EmotePickerPopup.hpp>
#include "../Search/Search.hpp"

using namespace geode::prelude;

//header against offset from top.
const std::unordered_map<const char*, float> kHeaderInformation {
    {"Recents", 6.0f},
    {"Ratings", 68.0f},
    {"Faces", 158.0f},
    {"Hands & Gestures", 430.0f},
    {"Speech bubbles", 495.0f},
    {"Game objects", 560.0f},
    {"Reactions & Symbols", 701.0f},
    {"Boomlings", 817.0f}
};


//genuinely the best way i could come up with to do this
static void placeEmoteCell(CCNode* node, float& y, int& col, int& placedInSection) {
    if (col == 8) { //max columns, make new line
        y -= 26.0f;
        col = 0;
    }

    node->setVisible(true);
    node->setPosition({21.0f + (26.0f * col), y - 13.0f});

    ++col;
    ++placedInSection;
}

static void endSection(float& y, const int col, const int numPlacedInSection) {
    if (numPlacedInSection == 0) return;

    if (col > 0) //newline
        y -= 26.0f;

    y -= 12.0f;
}

class $modify(EmotePickerPopup) {
    struct Fields {
        std::string searchQuery;
        geode::Label* noSearchResultsLabel = nullptr;
        std::array<geode::Label*, 8> sectionTitles {};
        bool isInited = false; //base call in init calls layoutEmoteGrid, so we gotta block it
    };

    bool init(CCTextInputNode* input, CCPoint position, int maxEmotes, int maxLength) {
        if (!EmotePickerPopup::init(input, position, maxEmotes, maxLength))
            return false;

        //section headers
        uint32_t i = 0;
        for (const auto [header, offset]: kHeaderInformation) {
            auto label = geode::Label::create(header, "goldFont.fnt");
            label->setScale(0.35f);
            label->setAnchorPoint({0.0f, 0.5f});
            label->setVisible(false);
            label->setPosition({8.0f, -offset});
            label->setID(fmt::format("{}-header"_spr, header));
            this->m_scrollContentMenu->addChild(label);

            this->m_fields->sectionTitles[i++] = label;
        }

        //this is just to show the user they can drag
        auto draggerIcon = CCSprite::create("dragger.png"_spr);
        draggerIcon->setScale(0.8f);
        draggerIcon->setOpacity(125);
        draggerIcon->setID("dragger-icon"_spr);
        draggerIcon->setColor({110, 110, 110});
        draggerIcon->setPosition({
            this->m_background->getPositionX() + 53.0f,
            this->m_background->getPositionY() + 61.0f
        });
        this->m_mainLayer->addChild(draggerIcon);

        //label for no search results
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

        this->m_fields->isInited = true;

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

        if (!isSearching) currentYPos -= 6.0f; //space for recents label.

        //all the section headers first
        //note to self: do not remove if statement. EmotePickerPopup::init calls layoutEmoteGrid
        for (auto* label: this->m_fields->sectionTitles)
            if (label) label->setVisible(!isSearching);


        if (!isSearching && !this->m_recentEmoteButtons.empty()) { //no search = show recents
            for (auto* cell: this->m_recentEmoteButtons) {
                placeEmoteCell(cell, currentYPos, col, numPlacedInSection);
                ++totalPlaced;
            }

            currentYPos -= 38.0f;
            col = 0;
            numPlacedInSection = 0;
        } else {
            for (auto* cell: this->m_recentEmoteButtons)
                cell->setVisible(false);

            if (!isSearching) //still leave a gap even if no recents were placed.
                currentYPos -= 12.0f;
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

    //reimplementation of what's in the binary except we typecheck in the iteration
    //to avoid the crash from the blind virtual call :p
    void refreshEnabled() {
        if (!this->m_scrollContentMenu) return;

        bool newCanAdd = this->canAddEmote();
        if (newCanAdd == this->m_canAdd) return;
        this->m_canAdd = newCanAdd;

        for (auto* child: this->m_scrollContentMenu->getChildrenExt<CCNode>()) {
            auto emoteCell = typeinfo_cast<CCMenuItemSpriteExtra*>(child);
            if (!emoteCell) continue;

            emoteCell->setEnabled(newCanAdd);
            emoteCell->setOpacity(newCanAdd? 255: 90);
        }
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