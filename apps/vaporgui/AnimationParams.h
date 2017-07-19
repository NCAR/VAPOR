//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		AnimationParams.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		January 2005
//
//	Description:	Defines the AnimationParams class
//		This is derived from the Params class
//		It contains all the parameters required for animation

//
#ifndef ANIMATIONPARAMS_H
#define ANIMATIONPARAMS_H

#include <vapor/ParamsBase.h>

//! \class AnimationParams
//! \ingroup Public_Params
//! \brief A class that specifies parameters used in animation
//! \author Alan Norton
//! \version 3.0
//! \date    February 2014

//! When this class is local, it controls the time-steps in one visualizer.
//! The global (shared) AnimationParams controls the animation in any number of visualizers.

class PARAMS_API AnimationParams : public VAPoR::ParamsBase {
public:
    AnimationParams(ParamsBase::StateSave *ssave);

    AnimationParams(VAPoR::ParamsBase::StateSave *ssave, VAPoR::XmlNode *node);

    virtual ~AnimationParams();

    //! Identify the current data timestep being used
    //! \retval long current time step
    //
    size_t GetCurrentTimestep() const { return (size_t)GetValueLong(_currentTimestepTag, 0); }

    //! Set the current data timestep being used
    //! \param long current time step
    //! \retval int 0 if successful
    //
    void SetCurrentTimestep(size_t ts) { SetValueLong(_currentTimestepTag, "Set timestep", (long)ts); }

    //! Identify the starting time step currently set in the UI.
    //! \retval int starting frame number.
    //
    size_t GetStartTimestep() const { return (size_t)GetValueLong(_startTimestepTag, 0); }

    //! set the starting time step
    //! \param int starting timestep
    //! \retval int 0 if successful
    //
    void SetStartTimestep(size_t ts) { SetValueLong(_startTimestepTag, "Set start timestep", (long)ts); }

    //! Identify the ending time step used during playback
    //! \retval int ending timestep
    //
    size_t GetEndTimestep() const { return (size_t)GetValueLong(_endTimestepTag, (long)0); }

    //! set the ending time step
    //! \param int ending timestep
    //! \retval int 0 if success
    //
    void SetEndTimestep(size_t val) { SetValueLong(_endTimestepTag, "Set end timestep", (long)val); }

    //! Get the current play direction
    //! \retval bool True if playing backwards
    //
    bool GetPlayBackwards() const { return GetValueLong(_playBackwardsTag, 0); }

    //! Set the play direction
    //! \param int play direction
    //
    void SetPlayBackwards(bool val) { SetValueLong(_playBackwardsTag, "Set play direction", (long)val); }

    //! Get the maximum frame step size for skipping
    //! \retval int current frame step size.
    //
    size_t GetFrameStepSize() const { return (size_t)GetValueLong(_stepSizeTag, 1); }

    //! Set the frame step size (for skipping)
    //! \param int val step size
    //! \retval int 0 if successful
    //
    void SetFrameStepSize(size_t val) { SetValueLong(_stepSizeTag, "Set frame stepsize", (size_t)val); }

    //! Determine max frames per second
    //! \retval double max frames per second
    //
    double GetMaxFrameRate() { return GetValueDouble(_maxRateTag, 1.0); }

    //! Set max frames per second
    //! \param double fps
    //! \retval int 0 if successful
    //!
    void SetMaxFrameRate(double rate) { SetValueDouble(_maxRateTag, "Set max frame rate", rate); }

    //! Determine if repeat play is on
    //! \retval bool true if repeating
    //
    bool GetRepeating() const { return (GetValueLong(_repeatTag, false)); }

    //! Set the whether repeat play is on
    //! \param bool repeat is on if true
    //! \retval int 0 if successful
    //
    void SetRepeating(bool onOff) { SetValueLong(_repeatTag, "enable repeat play", (long)onOff); }

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("AnimationParams"); }

private:
    static const string _repeatTag;
    static const string _maxRateTag;
    static const string _stepSizeTag;
    static const string _startTimestepTag;
    static const string _endTimestepTag;
    static const string _playBackwardsTag;
    static const string _currentTimestepTag;

    //! Put a params instance into default state with no data.
    void _init();
};

#endif    // ANIMATIONPARAMS_H
