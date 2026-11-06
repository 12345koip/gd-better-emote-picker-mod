#include "Search.hpp"
#include "EmoteTags.hpp"

using namespace BetterEmotePicker;

std::vector<int> Search::GetMatchingEmoteIndicesForSearchQuery(const std::string_view searchQuery) {
    if (searchQuery.empty()) return {}; //no point populating it if it's not going to be used.
    std::vector<int> out {};

    for (const auto& [tag, matchingIndices]: g_emoteTagsMap) {
        if (tag.find(searchQuery) != std::string::npos)
            out.insert(out.end(), matchingIndices.begin(), matchingIndices.end());
    }

    std::ranges::sort(out);
    const auto uniqueEnd = std::ranges::unique(out).begin();
    out.erase(uniqueEnd, out.end());

    return out;
}
