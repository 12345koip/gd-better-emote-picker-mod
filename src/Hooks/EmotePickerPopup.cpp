#include <Geode/Geode.hpp>
#include <Geode/modify/EditorWindowPopup.hpp>
#include <Geode/modify/EmotePickerPopup.hpp>

using namespace geode::prelude;

constexpr int kSearchBarTouchPriority = -501;
constexpr int kRecentEmoteTagGap = 100000;
constexpr uint32_t kTotalEmoteTableEntries = 262;
constexpr uint8_t kCellSeparateFactor = 26.0f;
constexpr float kRecentsSeparateFactor = 38.0f;
constexpr float kWindowWidthPad = 8.0f;


bool matchesEmote(const EmoteDef* def, const std::string_view query) { //TODO: give emotes tags and compare them here
    return query.empty() || geode::utils::string::toLower(def->m_name).find(query) != std::string::npos;
}

void placeEmoteCell(CCNode* node, int numCols, float xPad, float& y, int& col, int& placedInSection) {
    if (col >= numCols) {
        y -= kCellSeparateFactor;
        col = 0;
    }

    node->setVisible(true);
    static_cast<CCMenuItem*>(node)->setEnabled(true);
    node->setPosition({
        xPad + (col + 0.5f) * kCellSeparateFactor,
        y - (kCellSeparateFactor / 2.0f)
    });

    ++col;
    ++placedInSection;
}

class $modify(EmotePickerPopup) {
    struct Fields {
        std::string query;
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



        auto cache = CCSpriteFrameCache::sharedSpriteFrameCache();
        if (auto frame = cache->spriteFrameByName("GJ_closeBtn_001.png")) {
            this->m_closeSprite->setDisplayFrame(frame);
            this->m_closeSprite->setScale(0.7f);
        }



        auto searchBar = TextInput::create(this->m_windowWidth - 35.0f, "Search...");
        searchBar->setID("search-bar"_spr);
        searchBar->setScale(0.75f);
        searchBar->setCallback([this](const std::string& input) -> void {
            this->m_fields->query = input;
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
        const auto& query = this->m_fields->query;

        int numCols = std::max(3.0f, (this->m_windowWidth - kWindowWidthPad) / kCellSeparateFactor);
        float xPad = (this->m_windowWidth - numCols * kCellSeparateFactor) * 0.5f;
        float y = -4.0f;
        int col = 0;
        int placedInSection = 0;

        //recents first (tag of +100000, why??)
        int placedRecents = 0;
        for (auto cell: this->m_recentEmoteButtons) {
            auto def = EmoteSprite::defAtIndex(cell->getTag() - kRecentEmoteTagGap);

            if (!def || !matchesEmote(def, query)) {
                cell->setVisible(false);
                continue;
            }

            placeEmoteCell(cell, numCols, xPad, y, col, placedInSection);
            ++placedRecents;
        }

        if (placedRecents != 0) {
            y -= kRecentsSeparateFactor;
            col = 0;
        }

        //now do the rest!
        placedInSection = 0;
        for (int i = 0, n = EmoteSprite::defCount(); i < n && i < kTotalEmoteTableEntries; ++i) {
            auto def = EmoteSprite::defAtIndex(i);

            if (!def || !def->m_name) { //section separator.
                if (placedInSection == 0) continue;

                if (col > 0) y -= kCellSeparateFactor;
                y -= (kCellSeparateFactor / 2.0f);
                col = 0;
                placedInSection = 0;
                continue;
            }

            if (def->m_isAlias) continue;
            auto cell = this->m_scrollContentMenu->getChildByTag(i);
            if (!cell) continue;

            if (!matchesEmote(def, query)) {
                cell->setVisible(false);
                continue;
            }

            placeEmoteCell(cell, numCols, xPad, y, col, placedInSection);
        }

        if (col > 0) y -= kCellSeparateFactor;
        this->m_scrollContentLayer->setContentHeight(5.0f - y);
    }
};