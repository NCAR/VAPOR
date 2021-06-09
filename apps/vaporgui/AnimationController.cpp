#include "AnimationController.h"
#include <vapor/ControlExecutive.h>
#include "AnimationParams.h"
#include "NavigationUtils.h"

using namespace VAPoR;

AnimationController::AnimationController(VAPoR::ControlExec *ce) : _controlExec(ce), _myTimer(new QTimer(this)) {}


void AnimationController::setCurrentTimestep(size_t ts) const { NavigationUtils::SetTimestep(_controlExec, ts); }

// Insert values from params into tab panel
//
void AnimationController::Update()
{
    DataStatus *     dataStatus = _controlExec->GetDataStatus();
    AnimationParams *aParams = (AnimationParams *)GetActiveParams();

    size_t numTS = dataStatus->GetTimeCoordinates().size();
    if (numTS == 0) {
        // no data
        return;
    }

    size_t startFrame = aParams->GetStartTimestep();
    size_t endFrame = aParams->GetEndTimestep();
    size_t currentFrame = aParams->GetCurrentTimestep();

    if (startFrame >= numTS) {
        startFrame = numTS - 1;
        aParams->SetStartTimestep(startFrame);
    }

    if (endFrame >= numTS) {
        endFrame = numTS - 1;
        aParams->SetEndTimestep(endFrame);
    }

    if (startFrame > endFrame) {
        startFrame = endFrame;
        aParams->SetStartTimestep(startFrame);
    }

    if (currentFrame < startFrame) {
        currentFrame = startFrame;
        setCurrentTimestep(currentFrame);
    }

    if (currentFrame > endFrame) {
        currentFrame = endFrame;
        setCurrentTimestep(currentFrame);
    }
}


void AnimationController::AnimationPause()
{
    _animationOn = false;
    setPlay(0);
}

void AnimationController::AnimationPlayReverse()
{
    _animationOn = true;
    setPlay(-1);
}

void AnimationController::AnimationPlayForward()
{
    _animationOn = true;
    setPlay(1);
}

void AnimationController::AnimationStepForward()
{
    AnimationParams *aParams = (AnimationParams *)GetActiveParams();

    int currentFrame = aParams->GetCurrentTimestep();
    int endFrame = aParams->GetEndTimestep();

    currentFrame++;

    if (currentFrame > endFrame) return;

    setCurrentTimestep(currentFrame);
}

void AnimationController::AnimationStepReverse()
{
    AnimationParams *aParams = (AnimationParams *)GetActiveParams();

    int currentFrame = aParams->GetCurrentTimestep();
    int startFrame = aParams->GetStartTimestep();

    currentFrame--;

    if (currentFrame < startFrame) return;

    setCurrentTimestep(currentFrame);
}

void AnimationController::SetTimeStep(int ts) { setCurrentTimestep((size_t)ts); }

// Following are set by gui, result in save history state.
// Whenever play is pressed, it wakes up the animation controller.

void AnimationController::setPlay(int direction)
{
    _direction = direction;

    if (_direction) {
        // If on is true send notification (signal) that we are in
        // animation mode. Set a timer to go off at the appropriate
        // interval based on the frame rate. The timer slot will advance
        // the frame, etc.
        //
        emit AnimationOnOffSignal(true);

        AnimationParams *aParams = (AnimationParams *)GetActiveParams();

        int frameRate = aParams->GetMaxFrameRate();

        int msec = (int)(1.0 / (float)frameRate * 1000.0);

        connect(_myTimer, SIGNAL(timeout()), this, SLOT(playNextFrame()));

        _myTimer->start(msec);

    } else {
        // Done animating. Disable timer and send notification
        //
        disconnect(_myTimer, 0, 0, 0);

        emit AnimationOnOffSignal(false);
    }
}

void AnimationController::playNextFrame()
{
    VAssert(_direction == -1 || _direction == 1);

    // Draw the frame, and then advance the frame count
    //
    emit AnimationDrawSignal();

    AnimationParams *aParams = (AnimationParams *)GetActiveParams();

    int  startFrame = aParams->GetStartTimestep();
    int  endFrame = aParams->GetEndTimestep();
    int  currentFrame = aParams->GetCurrentTimestep();
    int  frameStepSize = aParams->GetFrameStepSize();
    bool loop = aParams->GetRepeating();

    currentFrame += (int)(_direction * frameStepSize);

    if (!loop && (currentFrame < startFrame || currentFrame > endFrame)) {
        AnimationPause();
        return;
    }

    if (currentFrame < startFrame) currentFrame = endFrame;
    if (currentFrame > endFrame) currentFrame = startFrame;

    setCurrentTimestep(currentFrame);

    // playNextFrame() is called via a timer and bypasses main event
    // loop. So we need to call updateTab ourselves
    //
    _updateTab();
}

AnimationParams *AnimationController::GetActiveParams() const { return NavigationUtils::GetAnimationParams(_controlExec); }

void AnimationController::_updateTab() { Update(); }
