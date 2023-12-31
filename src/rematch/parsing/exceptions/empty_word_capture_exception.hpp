#include <stdexcept>

namespace REMatch {

class EmptyWordCaptureException : public std::runtime_error {
 public:
  EmptyWordCaptureException(const std::string& error)
      : std::runtime_error(error) {}
};

}  // namespace REMatch
