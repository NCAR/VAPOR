#ifndef FLAGS_H
#define FLAGS_H

//! Bit masks to indicate what type of variables are to be supported by
//! a particular VariablesWidget instance. These flags correspond
//! to variable names returned by methods:
//!
//! SCALAR : RenderParams::GetVariableName()
//! VECTOR : RenderParams::GetFieldVariableNames()
//! HGT : RenderParams::GetHeightVariableName()
//! COLOR : RenderParams::GetColorMapVariableNames()
//!

enum DisplayFlags {
    SCALAR = (1u << 0),
    VECTOR = (1u << 1),
    COLOR = (1u << 2),
    AUXILIARY = (1u << 3),
    HEIGHT = (1u << 4),
};

//! Bit mask to indicate whether 2D, 3D, or 2D and 3D variables are to
//! be supported
//
enum DimFlags {
    TWOD = (1u << 0),
    THREED = (1u << 1),
};

#endif
