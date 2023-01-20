.. _vapor.session.ColorbarAnnotation:


vapor.session.ColorbarAnnotation
--------------------------------


Help on class ColorbarAnnotation in vapor.session:

vapor.session.ColorbarAnnotation = class ColorbarAnnotation(vapor.params.ParamsWrapper)
 |  vapor.session.ColorbarAnnotation(p: cppyy.gbl.VAPoR.ParamsBase)
 |  
 |  Wraps VAPoR::ColorbarPbase
 |  Settings for color bar displayed in scene Intended to be used in any Params class.
 |  The ColorbarPbase class is a ParamsBase class that manages the settings associated with a color bar. There is a corresponding ColorbarFrame class in the GUI that manages display of these settings.
 |  
 |  Method resolution order:
 |      ColorbarAnnotation
 |      vapor.params.ParamsWrapper
 |      vapor.smartwrapper.SmartWrapper
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  GetBackgroundColor(...)
 |      vector<double> VAPoR::ColorbarPbase::GetBackgroundColor()
 |          Get the background color as an rgb triple
 |      Returns
 |          rgb color
 |  
 |  GetCornerPosition(...)
 |      vector<double> VAPoR::ColorbarPbase::GetCornerPosition()
 |          Get the X,Y corner (upper left) coordinates Relative to [0,1]
 |      Returns
 |          pair of x,y coordinates
 |  
 |  GetFontSize(...)
 |      long VAPoR::ColorbarPbase::GetFontSize()
 |      Determine colorbar text size
 |  
 |  GetNumDigits(...)
 |      long VAPoR::ColorbarPbase::GetNumDigits()
 |      Determine colorbar num digits to display
 |  
 |  GetNumTicks(...)
 |      long VAPoR::ColorbarPbase::GetNumTicks()
 |      Determine colorbar num tics
 |  
 |  GetSize(...)
 |      vector<double> VAPoR::ColorbarPbase::GetSize()
 |          Get the X,Y size Relative to [0,1]
 |      Returns
 |          pair of x,y sizes
 |  
 |  GetTitle(...)
 |      string VAPoR::ColorbarPbase::GetTitle()
 |          Get the title text (displayed after variable name)
 |      Returns
 |          title
 |  
 |  GetUseScientificNotation(self) -> bool
 |  
 |  IsEnabled(...)
 |      bool VAPoR::ColorbarPbase::IsEnabled()
 |      Determine if colorbar is enabled
 |  
 |  SetBackgroundColor(...)
 |      void VAPoR::ColorbarPbase::SetBackgroundColor(vector< double > color)
 |          Set the background color as an rgb triple
 |      Parameters
 |          color = (r,g,b)
 |  
 |  SetCornerPosition(...)
 |      void VAPoR::ColorbarPbase::SetCornerPosition(vector< double > posn)
 |          Set the X,Y corner (upper left) coordinates Relative to [0,1]
 |      Parameters
 |          posn = x,y coordinates
 |  
 |  SetEnabled(...)
 |      void VAPoR::ColorbarPbase::SetEnabled(bool val)
 |          Enable or disable colorbar
 |      Parameters
 |          bool true if enabled
 |  
 |  SetFontSize(...)
 |      void VAPoR::ColorbarPbase::SetFontSize(long val)
 |          Set colorbar text size
 |      Parameters
 |          val text point size
 |  
 |  SetNumDigits(...)
 |      void VAPoR::ColorbarPbase::SetNumDigits(long val)
 |          Set colorbar number of digits
 |      Parameters
 |          val number of digits
 |  
 |  SetNumTicks(...)
 |      void VAPoR::ColorbarPbase::SetNumTicks(long val)
 |          Set colorbar number of tic marks
 |      Parameters
 |          val number of tics
 |  
 |  SetSize(...)
 |      void VAPoR::ColorbarPbase::SetSize(vector< double > sz)
 |          Set the X,Y sizes Relative to [0,1]
 |      Parameters
 |          posn = x,y sizes
 |  
 |  SetTitle(...)
 |      void VAPoR::ColorbarPbase::SetTitle(string text)
 |          Set the title text
 |      Parameters
 |          text to display
 |  
 |  SetUseScientificNotation(self, value: bool)
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from vapor.params.ParamsWrapper:
 |  
 |  __init__(self, p: cppyy.gbl.VAPoR.ParamsBase)
 |      Initialize self.  See help(type(self)) for accurate signature.
 |  
 |  ----------------------------------------------------------------------
 |  Class methods inherited from vapor.smartwrapper.SmartWrapper:
 |  
 |  __subclasses_rec__() from vapor.smartwrapper.SmartWrapperMeta
 |  
 |  ----------------------------------------------------------------------
 |  Data descriptors inherited from vapor.smartwrapper.SmartWrapper:
 |  
 |  __dict__
 |      dictionary for instance variables (if defined)
 |  
 |  __weakref__
 |      list of weak references to the object (if defined)

