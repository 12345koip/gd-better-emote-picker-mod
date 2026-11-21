#include <Geode/Geode.hpp>
#include <Geode/modify/EmotePickerPopup.hpp>
#include "../Search/Search.hpp"

using namespace geode::prelude;

static constexpr std::array<const char*, 8> kHeaderNames {
    "Recents",
    "Ratings",
    "Faces",
    "Hands & Gestures",
    "Speech bubbles",
    "Game objects",
    "Symbols & Stats",
    "Boomlings"
};


static void placeEmoteCell(CCNode* node, float& y, int& col, int& placedInSection, const int maxCol) {
    if (col >= maxCol) { //max columns, make new line
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

    //newline
    if (col > 0) // + one emoji row if any got placed
        y -= 38.0f;
    else
        y -= 12.0f;
}

class $modify(EmotePickerPopup) {
    struct Fields {
        std::string searchQuery;
        geode::Label* noSearchResultsLabel = nullptr;
        CCSprite* draggerIcon = nullptr;
        std::array<geode::Label*, 8> sectionTitles {};
    };

    bool init(CCTextInputNode* input, CCPoint position, int maxEmotes, int maxLength) {
        if (!EmotePickerPopup::init(input, position, maxEmotes, maxLength))
            return false;

        //section headers
        uint32_t i = 0;
        for (const auto header: kHeaderNames) {
            auto label = geode::Label::create(header, "goldFont.fnt");
            label->setScale(0.4f);
            label->setAnchorPoint({0.0f, 0.5f});
            label->setVisible(false);
            label->setID(fmt::format("{}-header"_spr, header));
            this->m_scrollContentMenu->addChild(label);

            this->m_fields->sectionTitles[i++] = label;
        }

        //dragger area
        auto draggerIcon = CCSprite::create("dragger.png"_spr);
        draggerIcon->setScale(0.8f);
        draggerIcon->setOpacity(125);
        draggerIcon->setID("dragger-icon"_spr);
        draggerIcon->setColor({125, 125, 125});
        draggerIcon->setPosition({
            this->m_background->getPositionX() + 53.0f,
            this->m_background->getPositionY() + 61.0f
        });
        this->m_mainLayer->addChild(draggerIcon);
        this->m_fields->draggerIcon = draggerIcon;

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
        const auto oldBg = this->m_background;
        const auto newBg = CCScale9Sprite::create("GJ_square01.png", {0, 0, 80, 80});
        newBg->setContentSize(oldBg->getContentSize());
        newBg->setPosition(oldBg->getPosition());
        newBg->setZOrder(oldBg->getZOrder());
        newBg->setOpacity(oldBg->getOpacity());

        this->m_mainLayer->addChild(newBg);
        oldBg->removeFromParent();
        this->m_background = newBg;

        //retexture close button
        const auto cache = CCSpriteFrameCache::sharedSpriteFrameCache();
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

        this->m_mainLayer->addChild(searchBar, this->m_scrollContentLayer->getZOrder());

        //un-screw (not unscrew) mobile platforms
        const auto screenSize = CCDirector::sharedDirector()->getWinSize();
        const auto availableHeight = std::max(this->m_minHeight, screenSize.height - 48.0f);
        const auto maxHeight = std::min(this->m_maxHeight, availableHeight);
        this->m_windowHeight = std::clamp(190.0f, this->m_minHeight, maxHeight);
        this->m_windowPosition = this->clampWindowOnScreen(this->m_windowPosition);
        this->applyWindowSize();

        return true;
    }



    //a lot of this code might look funky. lost of those parts are a (possibly slightly modified)
    //reimplementation of what's in the binary (not my fault).
    //the rest of it is probably just me writing funky code late at night. thanks :3
    void layoutEmoteGrid() {
        float currentYPos = -4.0f;
        int col = 0;
        int numPlacedInSection = 0;
        uint8_t totalPlaced = 0;
        uint8_t sectionIndex = 1; //skip recents label (placed separately)
        const int maxCols = std::max(3, static_cast<int>((this->m_windowWidth - 8.0f) / 26.0f));

        //there is one call per popup to this function before the modifications are applied, being from
        //the original init. so... if no headers exist, we can just ignore it bc we'll just call
        //this right after, still before the user sees it.
        geode::Label* recentsLabel = this->m_fields->sectionTitles[0];
        if (!recentsLabel) return;


        const auto& searchQuery = this->m_fields->searchQuery;
        const bool isSearching = !searchQuery.empty();
        const auto matchingIndices = BetterEmotePicker::Search::GetMatchingEmoteIndicesForSearchQuery(searchQuery);

        for (auto* header: this->m_fields->sectionTitles) //when searching all emotes are just bundled together
            header->setVisible(!isSearching);

        if (isSearching || this->m_recentEmoteButtons.empty()) { //recents are also hidden during search.
            recentsLabel->setVisible(false);
            for (auto* cell: this->m_recentEmoteButtons)
                cell->setVisible(false);
        } else {
            //place header + cells
            recentsLabel->setVisible(true);
            recentsLabel->setPosition({10.0f, currentYPos - 5.0f});
            currentYPos -= 10.0f;

            for (auto* cell: this->m_recentEmoteButtons) {
                placeEmoteCell(cell, currentYPos, col, numPlacedInSection, maxCols);
                ++totalPlaced;
            }

            currentYPos -= 38.0f;
            col = 0;
            numPlacedInSection = 0;
        }

        //now we place the ones which match the query.
        for (int i = 0; i < EmoteSprite::defCount(); ++i) {
            auto def = EmoteSprite::defAtIndex(i);
            if (def->m_isAlias) continue;

            if (!def->m_name && !isSearching) { //a section terminator
                endSection(currentYPos, col, numPlacedInSection);
                col = 0;
                numPlacedInSection = 0;

                //place the header for the next section
                auto* currentHeader = this->m_fields->sectionTitles[sectionIndex++];
                currentHeader->setVisible(true);
                currentHeader->setPosition({10.0f, currentYPos - 5.0f});
                currentYPos -= 10.0f;

                continue;
            }

            //thank goodness robtop tagged them by index and not by something else stupid.
            auto cell = this->m_scrollContentMenu->getChildByTag(i);
            if (!cell) continue;

            const std::string_view name = def->m_name;
            if (isSearching && std::ranges::find(matchingIndices, def->m_particleIndex) == matchingIndices.end() && name.find(searchQuery) == std::string::npos) {
                cell->setVisible(false);
                continue;
            }

            placeEmoteCell(cell, currentYPos, col, numPlacedInSection, maxCols);
            ++totalPlaced;
        }

        if (col > 0) //final emote row.
            currentYPos -= 26.0f;

        this->m_fields->noSearchResultsLabel->setVisible(totalPlaced == 0);
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

    //allows dragging inside the dragger label without having to mess with that class
    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        if (!this->m_isEmotePicker) //yeah the virtual is in EditorWindowPopup but casting there is pain soooo
            return EmotePickerPopup::ccTouchBegan(touch, event);

        auto icon = this->m_fields->draggerIcon;
        auto localSpace = icon->getParent()->convertToNodeSpace(touch->getLocation());
        this->m_allowDraggingAnywhere = icon->boundingBox().containsPoint(localSpace);

        return EmotePickerPopup::ccTouchBegan(touch, event);
    }
};