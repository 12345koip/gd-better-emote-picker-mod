#include "SearchBar.hpp"

using namespace BetterEmojiPicker;
using namespace geode::prelude;

CCNode* Search::makeSearchBar(EmotePickerPopup* popup) {
    auto bar = CCNode::create();
    bar->setID("search-bar"_spr);
    bar->setAnchorPoint({0.5f, 0.5f});
    bar->setContentSize({popup->m_windowWidth, kSearchBarHeight});

    auto input = TextInput::create(popup->m_windowWidth - 16.f, "Search", "bigFont.fnt");
    input->setID("search-input-bar"_spr);

    float scale = kSearchBarHeight / input->getContentHeight();
    input->setScale(scale);
    input->setPositionX(bar->getContentSize().width + 5.0f);
    bar->addChild(input);

    popup->m_mainLayer->addChild(bar, 5);
    popup->applyWindowSize();
    return bar;
}