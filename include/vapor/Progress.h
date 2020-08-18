#pragma once
#include <string>
#include <functional>
#include <vapor/common.h>

namespace VAPoR {

//! \class Progress
//! Used for displaying the progress of actions to the user. The actual method
//! for displaying the progress can vary based on the callback.
//!
//! The situations where this should be used is top-level calculations
//! that may take longer than a second. This should not be used (although
//! it will not break) for non-top level calculations, i.e. in the DC library
//! as typically loading data will be part of a higher-level calculation.
//!
//! The primary use case would be a renderer that has to precompute data, e.g.
//! the Flow renderer computing particle advection.

class COMMON_API Progress {
public:
    typedef std::function<void(const std::string &name, long nTotal, bool cancellable)> Start_t;
    typedef std::function<void(long nDone, bool *cancelled)>                            Update_t;
    typedef std::function<void()>                                                       Finish_t;

    //! Signifies the beginning of a computation called "name" with "total" elements
    //! Can be called multiple times to signifiy a series of computations but must
    //! always end with a Finish
    static void Start(const std::string &name, long total, bool cancelable = false);
    //! Same as start but the progress is unknown. Update(0) still needs to be called periodically
    static void StartIndefinite(const std::string &name, bool cancelable = false);
    //! Update the progress status.
    static void Update(long completed);
    //! If Start was called with cancelled=true, the user will have the option to cancel
    //! the computation.
    static inline bool Cancelled() { return _cancelled; }
    //! Signify the computation was cancelled.
    static void Finish();
    //! This class does not handle actually displaying progress information to
    //! the user. The interface (e.g. vaporgui) must set callbacks to accomplish this.
    static void SetHandlers(Start_t start, Update_t update, Finish_t finish);

#ifndef NDEBUG
    //! For testing purposes only.
    static void Sleep(double s);
#endif

private:
    static bool _cancelled;
    static long _total;

    static Start_t  _start;
    static Update_t _update;
    static Finish_t _finish;
};
}    // namespace VAPoR
