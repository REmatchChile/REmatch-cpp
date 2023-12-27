#include "default_segment_manager.hpp"

namespace rematch {

DefaultSegmentManager::DefaultSegmentManager(std::string_view document)
    : document_(document) {}

std::unique_ptr<Span> DefaultSegmentManager::next() {
  if (already_read_)
    return nullptr;
  already_read_ = true;
  return std::make_unique<Span>(0, document_.size());
}

size_t DefaultSegmentManager::get_search_dfa_size() {
  return 0;
}

size_t DefaultSegmentManager::get_search_nfa_size() {
  return 0;
}

}  // namespace rematch