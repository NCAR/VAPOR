#pragma once
#include <string>
#include <functional>

namespace VAPoR {
class Progress {
public:
    typedef std::function<void(const std::string&, long, bool)> Start_t;
    typedef std::function<void(long, bool*)> Update_t;
    typedef std::function<void()> Finish_t;
    
    static void Start(const std::string &name, long total, bool cancelable=false);
    static void StartIndefinite(const std::string &name, bool cancelable=false);
    static void Update(long completed);
    static inline bool Cancelled() { return _cancelled; }
    static void Finish();
    static void SetHandlers(Start_t start, Update_t update, Finish_t finish);
    
#ifndef NDEBUG
    static void Sleep(double s);
#endif
    
private:
    static bool _cancelled;
    static long _total;
    
    static Start_t  _start;
    static Update_t _update;
    static Finish_t _finish;
};
}
