.. _vapor.animation.Camera:


vapor.animation.Camera
----------------------


Help on class Camera in vapor.animation:

vapor.animation.Camera = class Camera(builtins.object)
 |  vapor.animation.Camera(ce)
 |  
 |  Methods defined here:
 |  
 |  AlignView(self, axis: str)
 |      Align camera looking down an axis
 |      Axis format: [+-][XYZ]
 |  
 |  GetDirection(self)
 |  
 |  GetPosition(self)
 |  
 |  GetTarget(self)
 |  
 |  GetUp(self)
 |  
 |  LookAt(self, camera_position: tuple[float, float, float], target: tuple[float, float, float], up: tuple[float, float, float] = (0, 0, 1))
 |      Moves the camera to camera_position facing target. up can be used to adjust the camera roll.
 |  
 |  SetDirection(self, v: tuple[float, float, float])
 |  
 |  SetPosition(self, v: tuple[float, float, float])
 |  
 |  SetTarget(self, v: tuple[float, float, float])
 |  
 |  SetUp(self, v: tuple[float, float, float])
 |  
 |  ViewAll(self)
 |      Places the camera above the dataset looking down so that it is visible in its entirety.
 |      This is the default view when opening a new dataset.
 |  
 |  Zoom(self, fractionOfDistanceToTarget: float)
 |      Moves the camera a fractionOfDistanceToTarget with positive zooming in and negative zooming out.
 |  
 |  __init__(self, ce)
 |      Initialize self.  See help(type(self)) for accurate signature.
 |  
 |  ----------------------------------------------------------------------
 |  Data descriptors defined here:
 |  
 |  __dict__
 |      dictionary for instance variables (if defined)
 |  
 |  __weakref__
 |      list of weak references to the object (if defined)

