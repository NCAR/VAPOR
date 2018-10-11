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

enum VariableFlags {
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
    TWODXY = (1u << 0),
    TWODXZ = (1u << 1),
    TWODYZ = (1u << 2),
    THREED = (1u << 3),
};

//! Bit mask to indicate whether the GeometryWidget should control a
//! single point, or 3D extents with Min/Max controllers
enum GeometryFlags {
    SINGLEPOINT = (1u << 0),
    MINMAX = (1u << 1),
};

//! Bit masks to indicate whether the TFWidget maps constant color
//! to a variable, or maps a secondary color variable
//! COLORVAR_FOR_TF1 - Indicates whether the main MappingFrame/MapperFunction
//! refer to to a color-mapped variable.  This is the case with Barbs, where
//! there is no primary variable.  We don't use the vector variables for our
//! transfer function, we use the color variable.
//! COLORVAR_FOR_TF2 - This indicates whether we need to
//! display an additional transfer function for displaying the colormapped
//! variable, as we do with isosurfaces. IE - when we have a MappingFrame for
//! the primary variable, as well as a MappingFrame for the colormapped variable
//! CONSTANT_COLOR   - Indicates whether constant color options are enabled, for
//! renderers like Barbs and Isosurfaces
enum TFFlags {
    COLORMAP_VAR_IS_IN_TF1 = (1u << 0),
    COLORMAP_VAR_IS_IN_TF2 = (1u << 1),
    CONSTANT_COLOR = (1u << 2),
};

#endif
