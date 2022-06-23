from . import link
from .common import *
import numpy as np

link.include('vapor/ControlExecutive.h')
link.include("vapor/NavigationUtils.h")

NavigationUtils = link.NavigationUtils

class Camera():

    __axisDict = {
        '+X': 2,
        '+Y': 3,
        '+Z': 4,
        '-X': 5,
        '-Y': 6,
        '-Z': 7
    }

    def __init__(self, ce):
        self.ce: link.VAPoR.ControlExec = ce

    def AlignView(self, axis:str):
        """
        Align camera looking down an axis
        Axis format: [+-][XYZ]
        """
        viewNum = self.__axisDict[axis.upper()]
        NavigationUtils.AlignView(self.ce, viewNum)

    def ViewAll(self):
        """Places the camera above the dataset looking down so that it is visible in its entirety.
        This is the default view when opening a new dataset."""
        NavigationUtils.ViewAll(self.ce)

    def LookAt(self, camera_position:Vec3, target:Vec3, up:Vec3 = (0, 0, 1)):
        """Moves the camera to camera_position facing target. up can be used to adjust the camera roll."""
        NavigationUtils.LookAt(self.ce, camera_position, target, up)

    def Zoom(self, fractionOfDistanceToTarget:float):
        """Moves the camera a fractionOfDistanceToTarget with positive zooming in and negative zooming out."""
        tgt = self.GetTarget()
        pos = self.GetPosition()
        dist = np.linalg.norm(tgt - pos)
        newDist = dist * (1 - fractionOfDistanceToTarget)
        newPos = tgt + ((pos-tgt)/dist * newDist)
        self.SetPosition(newPos)

    def GetPosition (self): return np.array(NavigationUtils.GetCameraPosition(self.ce))
    def GetDirection(self): return np.array(NavigationUtils.GetCameraDirection(self.ce))
    def GetUp       (self): return np.array(NavigationUtils.GetCameraUp(self.ce))
    def GetTarget   (self): return np.array(NavigationUtils.GetCameraTarget(self.ce))
    def SetPosition (self, v:Vec3): NavigationUtils.SetCameraPosition(self.ce, v)
    def SetDirection(self, v:Vec3): NavigationUtils.SetCameraDirection(self.ce, v)
    def SetUp       (self, v:Vec3): NavigationUtils.SetCameraUp(self.ce, v)
    def SetTarget   (self, v:Vec3): NavigationUtils.SetCameraTarget(self.ce, v)

