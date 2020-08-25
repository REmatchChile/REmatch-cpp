#ifndef AUTOMATA__TRANSITION_HPP
#define AUTOMATA__TRANSITION_HPP

#include <memory>

#include "automata/detstate.hpp"
#include "captures.hpp"

class DetState;
class NodeList;

using DetStates = std::vector<DetState*>;
using Captures = std::vector<std::unique_ptr<Capture>>;

namespace rematch {

struct Transition {
  enum class Type {
    kEmpty                = 0,
    kDirect               = 1,
    kSingleCapture        = 2,
    kDirectSingleCapture  = 3,
    kMultiCapture         = 4,
    kDirectMultiCapture   = 5
  };

  Type type_;
  DetState* direct_;
  std::unique_ptr<Capture> capture_;
  Captures captures_;

  // Default = EmptyTransition
  Transition(): type_(Type::kEmpty) {}

  Transition(std::unique_ptr<Capture> capture)
    : type_(Type::kSingleCapture),
      capture_(std::move(capture)) {}

  Transition(DetState* state): type_(Type::kDirect), direct_(state) {}

  void add_capture(std::unique_ptr<Capture> capture) {
    switch (type_) {
      case Type::kEmpty:
        throw std::logic_error("Can't add capture to empty transition.");
        break;
      case Type::kDirect:
        type_ = Type::kDirectSingleCapture;
        capture_ = std::move(capture);
        break;
      case Type::kSingleCapture:
        type_ = Type::kMultiCapture;
        captures_.push_back(std::move(capture_));
        captures_.push_back(std::move(capture));
        break;
      case Type::kDirectSingleCapture:
        type_ = Type::kDirectMultiCapture;
        captures_.push_back(std::move(capture_));
        captures_.push_back(std::move(capture));
        break;
      default:
        captures_.push_back(std::move(capture));
        break;
    }
  }

  void add_direct(DetState* state) {
    switch (type_) {
      case Type::kEmpty:
        throw std::logic_error("Can't add direct to empty transition.");
        break;
      case Type::kDirect:
      case Type::kDirectSingleCapture:
      case Type::kDirectMultiCapture:
        throw std::logic_error("Can't add direct because a direct is already present.");
        break;
      case Type::kSingleCapture:
        type_ = Type::kDirectSingleCapture;
        direct_ = state;
        break;
      case Type::kMultiCapture:
        type_ = Type::kDirectMultiCapture;
        direct_ = state;
        break;
    }
  }
}; // end struct Transition

} // namespace rematch

#endif // AUTOMATA__TRANSITION_HPP