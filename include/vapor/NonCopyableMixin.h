#pragma once

namespace VAPoR {
class NonCopyableMixin {
  protected:
    NonCopyableMixin() {}
    ~NonCopyableMixin() {}

  private:
    NonCopyableMixin(const NonCopyableMixin &) = delete;
    NonCopyableMixin &operator=(const NonCopyableMixin &) = delete;
};
}; // namespace VAPoR
