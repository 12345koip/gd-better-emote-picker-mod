#pragma once

#include <Geode/Geode.hpp>

namespace BetterEmojiPicker::Search {
    constexpr float kSearchBarHeight = 20.0f;
    cocos2d::CCNode* makeSearchBar(EmotePickerPopup* popup);
}