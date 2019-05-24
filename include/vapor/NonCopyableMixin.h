#pragma once

namespace VAPoR {

//! \class NonCopyableMixin
//! \ingroup Public_Common
//! \brief Mixin class that prevents copying
//! \author Stas Jaroszynski
//! \version 1.0
//! \date May 2019
//!
//! If a class inherits from this class, attempting to copy it will throw
//! a compiler error.
//!
//! This class is used as follows:
//!
//! class NonCopyableObject : private NonCopyableMixin

class NonCopyableMixin {
  protected:
    NonCopyableMixin() {}
    ~NonCopyableMixin() {}

  private:
    NonCopyableMixin(const NonCopyableMixin &) = delete;
    NonCopyableMixin &operator=(const NonCopyableMixin &) = delete;
};
}; // namespace VAPoR
