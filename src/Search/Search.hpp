#pragma once

#include <Geode/Geode.hpp>

namespace BetterEmotePicker::Search {
    std::vector<int> GetMatchingEmoteIndicesForSearchQuery(const std::string_view searchQuery);
}