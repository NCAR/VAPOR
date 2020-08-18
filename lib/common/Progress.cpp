#include <vapor/Progress.h>
#include <cassert>

using namespace VAPoR;

Progress::Start_t  Progress::_start = [](const std::string &, long, bool) {};
Progress::Update_t Progress::_update = [](long, bool *) {};
Progress::Finish_t Progress::_finish = []() {};

bool Progress::_cancelled = false;
long Progress::_total = 1;

void Progress::Start(const std::string &name, long total, bool cancelable)
{
    _start(name, total, cancelable);
    _cancelled = false;
    _total = total;
}

void Progress::StartIndefinite(const std::string &name, bool cancelable) { Progress::Start(name, 0, cancelable); }

void Progress::Update(long completed)
{
    if (_total > 100 && completed % (_total / 100) != 0 && completed != 0) return;
    _update(completed, &_cancelled);
}

void Progress::Finish() { _finish(); }

void Progress::SetHandlers(Start_t start, Update_t update, Finish_t finish)
{
    assert((bool)start);
    assert((bool)update);
    assert((bool)finish);

    _start = start;
    _update = update;
    _finish = finish;
}

#ifndef NDEBUG
    #include <unistd.h>
void Progress::Sleep(double s) { usleep(s * 1e6L); }
#endif
