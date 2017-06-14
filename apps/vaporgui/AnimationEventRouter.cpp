//************************************************************************
//									*
//		     Copyright (C)  2006				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		AnimationEventRouter.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2006
//
//	Description:	Implements the animationEventRouter class.
//		This class supports routing messages from the gui to the params
//		This is derived from the animation tab
//
#ifdef WIN32
//Annoying unreferenced formal parameter warning
#pragma warning( disable : 4100 )
#endif

#include <vector>
#include <string>
#include <iostream>
#include <QTimer>


#include "vapor/AnimationParams.h"
#include "RangeCombos.h"
#include "AnimationEventRouter.h"

#include "../images/playreverseA.xpm"
#include "../images/playforwardA.xpm" 
#include "../images/pauseA.xpm"
#include "../images/stepfwdA.xpm"
#include "../images/stepbackA.xpm"
#include "../images/toendA.xpm"
#include "../images/tobeginA.xpm"
#include "../images/replayA.xpm"

QT_USE_NAMESPACE

using namespace VAPoR;


AnimationEventRouter::AnimationEventRouter(
    QWidget *parent, ControlExec *ce
) : QWidget(parent),
    Ui_AnimationTab(),
    EventRouter(ce, AnimationParams::GetClassType())
{
	setupUi(this);	

	QPixmap* playForwardIcon = new QPixmap(playforward);
	playForwardButton->setIcon(QIcon(*playForwardIcon));
	playForwardButton->setIconSize(QSize(30,18));
	QPixmap* playReverseIcon = new QPixmap(playreverse);
	playReverseButton->setIcon(QIcon(*playReverseIcon));
	playReverseButton->setIconSize(QSize(30,18));
	QPixmap* pauseIcon = new QPixmap( pause_);
	pauseButton->setIcon(QIcon(*pauseIcon));
	pauseButton->setIconSize(QSize(30,18));
	QPixmap* toBeginIcon = new QPixmap(toBegin_);
	toBeginButton->setIcon(QIcon(*toBeginIcon));
	toBeginButton->setIconSize(QSize(30,18));
	QPixmap* toEndIcon = new QPixmap(toEnd_);
	toEndButton->setIcon(QIcon(*toEndIcon));
	toEndButton->setIconSize(QSize(30,18));
	QPixmap* replayIcon = new QPixmap( replay_);
	replayButton->setIcon(QIcon(*replayIcon));
	replayButton->setIconSize(QSize(30,18));
	QPixmap* stepfwdIcon = new QPixmap(stepfwd);
	stepForwardButton->setIcon(QIcon(*stepfwdIcon));
	stepForwardButton->setIconSize(QSize(30,18));
	QPixmap* stepbackIcon = new QPixmap(stepback);
	stepReverseButton->setIcon(QIcon(*stepbackIcon));
	stepReverseButton->setIconSize(QSize(30,18));

	_timestepSelectCombo = new Combo(currentTimestepEdit, animationSlider,true);

	_frameStepCombo = new Combo(frameStepEdit, frameStepSlider,true);

	_frameRateCombo = new Combo(maxFrameRateEdit, maxFrameRateSlider,true);

	minframeLabel->setText("Min Time Step:");
	maxframeLabel->setText("Max Time Step:");

	_myTimer = new QTimer(this);

	// Only editable when key framing is enabled, which is not currently
	// supported.
	//
	currentFrameEdit->setReadOnly(true);
	frame->setEnabled(false);
	gridLayout->setEnabled(false);

}


AnimationEventRouter::~AnimationEventRouter(){
}

/**********************************************************
 * Whenever a new Animationtab is created it must be hooked up here
 ************************************************************/
void AnimationEventRouter::hookUpTab()
{
	//Signals and slots:
	
	connect (
		startFrameEdit, SIGNAL(returnPressed()),
		this, SLOT(setStart())
	);

	connect (
		endFrameEdit, SIGNAL(returnPressed()),
		this, SLOT(setEnd())
	);

	//Button clicking for toggle buttons:
	connect(
		pauseButton, SIGNAL(clicked()),
		this, SLOT(AnimationPause())
	);
	connect(
		playReverseButton, SIGNAL(clicked()),
		this, SLOT(AnimationPlayReverse())
	);
	connect(
		playForwardButton, SIGNAL(clicked()),
		this, SLOT(AnimationPlayForward())
	);

	connect(
		replayButton, SIGNAL(clicked()),
		this, SLOT(AnimationReplay())
	);

	
	//and non-toggle buttons:
	connect(
		toBeginButton, SIGNAL(clicked()),
		this, SLOT(AnimationGoToBegin())
	);
	connect(
		toEndButton, SIGNAL(clicked()),
		this, SLOT(AnimationGoToEnd())
	);
	connect(
		stepReverseButton, SIGNAL(clicked()),
		this, SLOT(AnimationStepReverse())
	);
	connect(
		stepForwardButton, SIGNAL(clicked()),
		this, SLOT(AnimationStepForward())
	);

	connect(
		_timestepSelectCombo, SIGNAL(valueChanged(int)),
		this, SLOT(SetTimeStep(int))
	);

	connect(
		_frameStepCombo, SIGNAL(valueChanged(int)),
		this, SLOT(SetFrameStep(int))
	);

	connect(
		_frameRateCombo, SIGNAL(valueChanged(int)),
		this, SLOT(SetFrameRate(int))
	);

}


void AnimationEventRouter::GetWebHelp(
    vector <pair <string, string> > &help
) const {
	help.clear();

	help.push_back(make_pair(
		"Animation overview",
		"http://www.vapor.ucar.edu/docs/vapor-how-guide/animation#AnimationOverview"
	));

	help.push_back(make_pair(
		"Animation capture",
		"http://www.vapor.ucar.edu/docs/vapor-how-guide/animation#AnimationCapture"
	));

	help.push_back(make_pair(
		"Settings for key-framed animation",
		"http://www.vapor.ucar.edu/docs/vapor-how-guide/animation#KeyFrameSettings"
	));

	help.push_back(make_pair(
		"How to create a key-framed animation sequence",
		"http://www.vapor.ucar.edu/docs/vapor-gui-help/creating-key-framed-animation"
	));

}

/////////////////////////////////////////////////////////////////////////////
//
// Slots associated with AnimationTab:
//
/////////////////////////////////////////////////////////////////////////////



//Insert values from params into tab panel
//
void AnimationEventRouter::_updateTab() {


	const int minStepSize = 1; 
	const int maxStepSize = 10; 
	const int minFrameRate = 1; 
	const int maxFrameRate = 60; 
	
	DataStatus * dataStatus = _controlExec->getDataStatus();
	AnimationParams* aParams = (AnimationParams*) GetActiveParams();

	size_t numTS = dataStatus->getNumTimesteps();
	assert (numTS >= 1);
	
	size_t startFrame = aParams->GetStartTimestep();
	size_t endFrame = aParams->GetEndTimestep();
	size_t currentFrame = aParams->GetCurrentTimestep();
	size_t frameStepSize = aParams->GetFrameStepSize();
	size_t frameRate = aParams->GetMaxFrameRate();

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
		aParams->SetCurrentTimestep(currentFrame);
	}

	if (currentFrame > endFrame) {
		currentFrame = endFrame;
		aParams->SetCurrentTimestep(currentFrame);
	}

	if (frameStepSize < minStepSize || frameStepSize > maxStepSize) {
		frameStepSize = minStepSize;
		aParams->SetFrameStepSize(frameStepSize);
	}

	if (frameRate < minFrameRate || frameRate > maxFrameRate) {
		frameRate = minFrameRate;
		aParams->SetMaxFrameRate(frameRate);
	}

	_timestepSelectCombo->Update(startFrame, endFrame, currentFrame);
	_frameStepCombo->Update(minStepSize, maxStepSize, frameStepSize);
	_frameRateCombo->Update(minFrameRate, maxFrameRate, frameRate);

	startFrameEdit->setText(QString::number(startFrame));
	currentFrameEdit->setText(QString::number(currentFrame));
	endFrameEdit->setText(QString::number(endFrame));
	minFrameLabel->setText(QString::number(aParams->GetStartTimestep()));
	maxFrameLabel->setText(QString::number(aParams->GetEndTimestep()));
	replayButton->setChecked(aParams->GetRepeating());

#ifdef	DEAD
	
	int playDirection = aParams->GetPlayBackwards();

	pauseButton->setChecked(true);
	playForwardButton->setChecked(false);
	playReverseButton->setChecked(false);
	stepForwardButton->setEnabled(true);
	stepReverseButton->setEnabled(true);
#endif

}

//Reinitialize Animation tab settings, session has changed


/*************************************************************************************
 *  slots associated with AnimationTab
 *************************************************************************************/


//Respond to pause button press.

void AnimationEventRouter::AnimationPause(){
	enableWidgets(true);

	playForwardButton->setChecked(false);
	playReverseButton->setChecked(false);
	pauseButton->setChecked(true);

	setPlay(0);
}

void AnimationEventRouter::AnimationPlayReverse(){
	enableWidgets(false);

	playForwardButton->setChecked(false);
	playReverseButton->setChecked(true);
	pauseButton->setChecked(false);
	setPlay(-1);
}

void AnimationEventRouter::AnimationPlayForward(){

	enableWidgets(false);
	
	playForwardButton->setChecked(true);
	playReverseButton->setChecked(false);
	pauseButton->setChecked(false);
	setPlay(1);
}


void AnimationEventRouter::AnimationReplay() {
	AnimationParams* aParams = (AnimationParams*) GetActiveParams();
	
	aParams->SetRepeating(replayButton->isChecked());
	
	_updateTab();

}

void AnimationEventRouter::AnimationGoToBegin(){
	AnimationParams* aParams = (AnimationParams*) GetActiveParams();

	aParams->SetCurrentTimestep(aParams->GetStartTimestep());

	_updateTab();
}

void AnimationEventRouter::AnimationGoToEnd(){
	AnimationParams* aParams = (AnimationParams*) GetActiveParams();

	aParams->SetCurrentTimestep(aParams->GetEndTimestep());

	_updateTab();
}

void AnimationEventRouter::AnimationStepForward(){
	AnimationParams* aParams = (AnimationParams*) GetActiveParams();

	int currentFrame = aParams->GetCurrentTimestep();
	int endFrame = aParams->GetEndTimestep();

	currentFrame++;

	if (currentFrame > endFrame) return;

	aParams->SetCurrentTimestep(currentFrame);
	
	_updateTab();
}

void AnimationEventRouter::AnimationStepReverse(){
	AnimationParams* aParams = (AnimationParams*) GetActiveParams();

	int currentFrame = aParams->GetCurrentTimestep();
	int startFrame = aParams->GetStartTimestep();

	currentFrame--;

	if (currentFrame < startFrame) return;

	aParams->SetCurrentTimestep(currentFrame);

	_updateTab();
}


void AnimationEventRouter::SetTimeStep(int ts) {
	AnimationParams* aParams =  (AnimationParams*)GetActiveParams();

	aParams->SetCurrentTimestep((size_t) ts);

	_updateTab();
}

void AnimationEventRouter::SetFrameStep(int step) {
	AnimationParams* aParams =  (AnimationParams*)GetActiveParams();

	aParams->SetFrameStepSize((size_t) step);

	_updateTab();
}

void AnimationEventRouter::SetFrameRate(int rate) {
	AnimationParams* aParams =  (AnimationParams*)GetActiveParams();

	aParams->SetMaxFrameRate(rate);

	_updateTab();
}


//Following are set by gui, result in save history state.
//Whenever play is pressed, it wakes up the animation controller.

void AnimationEventRouter::setPlay(int direction) {
	_direction = direction;

	if (_direction) {

		// If on is true send notification (signal) that we are in
		// animation mode. Set a timer to go off at the appropriate 
		// interval based on the frame rate. The timer slot will advance
		// the frame, etc.
		//
		emit AnimationOnOffChanged(true);

		AnimationParams* aParams =  (AnimationParams*)GetActiveParams();

		int frameRate = aParams->GetMaxFrameRate();

		int msec = (int) (1.0 / (float) frameRate * 1000.0);

		connect(
			_myTimer,SIGNAL(timeout()), this, SLOT(playNextFrame())
		);

		_myTimer->start(msec);

	}
	else {
		// Done animating. Disable timer and send notification
		//
		disconnect(_myTimer,0,0,0);

		emit AnimationOnOffChanged(false);
	}
}
	
	
void AnimationEventRouter::playNextFrame() {
	assert (_direction == -1 || _direction == 1);

	// Draw the frame, and then advance the frame count
	//
	emit AnimationDraw();

	AnimationParams* aParams =  (AnimationParams*)GetActiveParams();

	int startFrame = aParams->GetStartTimestep();
	int endFrame = aParams->GetEndTimestep();
	int currentFrame = aParams->GetCurrentTimestep();
	int frameStepSize = aParams->GetFrameStepSize();
	bool loop = aParams->GetRepeating();

	currentFrame += (int) (_direction * frameStepSize);

	if (! loop && (currentFrame < startFrame || currentFrame > endFrame)) {
		AnimationPause();
		return;
	}

	if (currentFrame < startFrame) currentFrame = endFrame;
	if (currentFrame > endFrame) currentFrame = startFrame;

	aParams->SetCurrentTimestep(currentFrame);

	_updateTab();
	
}

void AnimationEventRouter::setStart() {
	int ts = startFrameEdit->text().toInt();

	AnimationParams* aParams = (AnimationParams*) GetActiveParams();

	aParams->SetStartTimestep(ts);

	_updateTab();
}

void AnimationEventRouter::setEnd() {
	int ts = endFrameEdit->text().toInt();

	AnimationParams* aParams = (AnimationParams*) GetActiveParams();

	aParams->SetEndTimestep(ts);

	_updateTab();
}

void AnimationEventRouter::enableWidgets(bool on) {

	startFrameEdit->setEnabled(on);
	currentFrameEdit->setEnabled(on);
	endFrameEdit->setEnabled(on);
	playForwardButton->setEnabled(on);
	playReverseButton->setEnabled(on);
	toBeginButton->setEnabled(on);
	toEndButton->setEnabled(on);
	replayButton->setEnabled(on);
	stepForwardButton->setEnabled(on);
	stepReverseButton->setEnabled(on);

	_timestepSelectCombo->SetEnabled(on);
	_frameStepCombo->SetEnabled(on);
	_frameRateCombo->SetEnabled(on);

}

#ifdef	DEAD
//Set change bits when global/local change occurs, so that the animation
//controller will change the local/global status at the end of the next rendering.
void AnimationEventRouter::SetLocal(ParamsBase* p, bool lg){
	EventRouter::SetLocal(p,lg);
	VizWin* viz = VizWinMgr::getInstance()->getActiveVizWin();
	viz->updateGL();
}
#endif
