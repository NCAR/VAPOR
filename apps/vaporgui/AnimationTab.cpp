#include "AnimationTab.h"
#include <QVBoxLayout>
#include "PWidgets.h"
#include "PTimestepInput.h"
#include "PTotalTimestepsDisplay.h"
#include "PTimestepSliderEdit.h"
#include <vapor/NavigationUtils.h>
#include <vapor/AnimationParams.h>

using namespace VAPoR;


AnimationTab::AnimationTab(ControlExec *ce) : _controlExec(ce)
{
    _g = new PGroup({
        new PSection("Timestep",
                     {
                         new PTimestepSliderEdit(_controlExec),
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


void AnimationTab::Update()
{
    if (!isEnabled()) return;

    _g->Update(NavigationUtils::GetAnimationParams(_controlExec));

    size_t totalTs = _controlExec->GetDataStatus()->GetTimeCoordinates().size();
    _startTS->SetRange(0, totalTs);
    _stopTS->SetRange(0, totalTs);
}
