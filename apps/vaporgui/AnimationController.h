#pragma once

#include <QTimer>

class AnimationParams;
namespace VAPoR {
class ControlExec;
}

//! \class AnimationController
//! \brief This class is just migrated legacy code to de-spaghetti other legacy code.
//! (It is not written by me)

class AnimationController : public QObject {
    Q_OBJECT
    VAPoR::ControlExec *_controlExec;
    QTimer *            _myTimer;
    int                 _direction;
    bool                _animationOn = false;

public:
    AnimationController(VAPoR::ControlExec *ce);
    void Update();

public slots:
    void AnimationPause();
    void AnimationPlayReverse();
    void AnimationPlayForward();
    void AnimationStepForward();
    void AnimationStepReverse();
    void SetTimeStep(int ts);

signals:

    // Emitted when animation is turned on (true) or off (false)
    //
    void AnimationOnOffSignal(bool onOff);

    // Emitted when the client should draw a frame during animation. Only
    // Emitted if AnimationOnOffChanged() was most recently called with
    // onOff == true;
    //
    void AnimationDrawSignal();

private:
    void             setCurrentTimestep(size_t ts) const;
    void             setPlay(int direction);
    AnimationParams *GetActiveParams() const;
    void             _updateTab();

private slots:
    void playNextFrame();
};
