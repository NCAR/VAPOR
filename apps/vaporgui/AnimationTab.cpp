#include "AnimationTab.h"
#include <QVBoxLayout>
#include "PWidgets.h"
#include "PTimestepInput.h"
#include "PTotalTimestepsDisplay.h"

using namespace VAPoR;


AnimationTab::AnimationTab(QWidget *parent, ControlExec *ce) : QWidget(parent), EventRouter(ce, AnimationParams::GetClassType())
{
    _g = new PGroup({
        new PSection("Timestep",
                     {
                         new PLineItem("Current Timestep", new PTimestepInput(_controlExec)),
                         new PTotalTimestepsDisplay(_controlExec),
                     }),
        new PSection("Animation",
                     {
                         _startTS = new PIntegerInput(AnimationParams::_startTimestepTag, "Animation Start Timestep"),
                         _stopTS = new PIntegerInput(AnimationParams::_endTimestepTag, "Animation Last Timestep"),
                         new PCheckbox(AnimationParams::_repeatTag, "Loop Animation Playback"),
                         (new PIntegerInput(AnimationParams::_stepSizeTag, "Animation Play Step Size"))->SetRange(1, 10),
                         (new PDoubleSliderEdit(AnimationParams::_maxRateTag, "Max Animation Frames Per Second"))->SetRange(1, 60),
                     }),
    });

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    layout()->addWidget(_g);
    l->addStretch();
}


void AnimationTab::_updateTab()
{
    if (!isEnabled()) return;

    _g->Update(GetAnimationParams());

    size_t totalTs = _controlExec->GetDataStatus()->GetTimeCoordinates().size();
    _startTS->SetRange(0, totalTs);
    _stopTS->SetRange(0, totalTs);
}
