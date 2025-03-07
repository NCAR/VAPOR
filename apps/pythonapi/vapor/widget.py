import anywidget
from traitlets import Unicode, Bytes, Tuple, Bool, observe, Int
import traitlets

##########################################
#          Development utility
import importlib
vapor_spec = importlib.util.find_spec("vapor")
found = vapor_spec is not None

if not found:
    import sys
    import os
    from pathlib import Path
    sys.path.append(os.path.expanduser('~/Work/build-work/python'))
#########################################

from PIL import Image
from PIL import ImageDraw
from base64 import b64encode
from io import BytesIO
import pathlib
import os, sys, random

# class VaporVisualizerWidget(anywidget.AnyWidget):
#     _esm = pathlib.Path("widget.js")


class CanvasStreamWidget(anywidget.AnyWidget):
    _esm = pathlib.Path(__file__).parent / "widget.js"
    _css = pathlib.Path(__file__).parent / "widget.css"

    imageData = Unicode('').tag(sync=True)
    resolution = Tuple((640, 480)).tag(sync=True)
    mousePos = Tuple((0.0, 0.0)).tag(sync=True)
    mouseDown = Bool(False).tag(sync=True)
    mouseButton = Int(1).tag(sync=True)
    _debugLog = Unicode('').tag(sync=True)

    value = Int(0).tag(sync=True)



    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if "debug" in kwargs and kwargs["debug"]:
            self.on_trait_change(self.valueChanged, "value")
            self.on_trait_change(self._DebugShowMousePos, "mousePos")
        open(os.path.expanduser("~/Desktop/log.txt"), "w").write("")


    def test(self):
        self.SetImage(Image.new('RGB', self.resolution, (0,0,100)))
        # self.value = 999
        print("TEST ===============")
        print("TEST ===============", file=sys.stderr)
        open(os.path.expanduser("~/Desktop/log.txt"), "a").write("test")

    def valueChanged(self):
        print("VALUE CHANGED TO", self.value)
        self.SetImage(Image.new('RGB', self.resolution, (random.randint(0,255),random.randint(0,255),random.randint(0,255))))


    def SetImage(self, img: Image):
        buf = BytesIO()
        img.save(buf, format="jpeg")
        self.resolution = img.size
        self.imageData = b64encode(buf.getvalue())

    def _DebugShowMousePos(self):
        print("DEBUG SHOW MOUSE POS")
        print("DEBUG SHOW MOUSE POS", file=sys.stderr)
        image = Image.new('RGB', self.resolution)
        draw = ImageDraw.Draw(image)

        iw = int(image.size[0])
        ih = int(image.size[1])
        mx = int(self.mousePos[0] * iw)
        my = int(self.mousePos[1] * ih)
        # self.value = mx
        self._debugLog = f"""
                    mousePos: {mx}, {my} <br>
                    imagesize = {iw, ih} <br>
                    self.res = {self.resolution}
                    """
        white = (255, 255, 255)
        red = (255, 0, 0)
        draw.rectangle([0, 0, iw, ih], outline=red, width=5)
        draw.line([mx, 0, mx, ih], fill=white, width=5)
        draw.line([0, my, iw, my], fill=white, width=5)

        self.SetImage(image)



# widget setup.py installer will try to run tests which will fail 
# because CPPYY cannot be present at build time due to conda bugs

import importlib
if importlib.util.find_spec("cppyy") is not None:

    from vapor import session, renderer, dataset, camera, utils
    # from examples import example_utils
    
    from vapor import link
    link.include('vapor/TrackBall.h')
    
    link.include('vapor/ControlExecutive.h')
    link.include("vapor/NavigationUtils.h")
    
    NavigationUtils = link.NavigationUtils
    
    class VaporVisualizerWidget(CanvasStreamWidget):
        """
        Creates an interactive visualizer widget for a Vapor session.
        Controls are the same as the Vapor GUI application (They are shown as a tooltip on the visualizer for reference).
        """
        class TrackballButton:
            Left = 1
            Right = 3
            Middle = 2
    
        def __init__(self, ses:session.Session, *args, **kwargs):
            # import sys
            # sys.stdout = open("stdout.txt", "w")
            # sys.stderr = open("stderr.txt", "w")
    
            super().__init__(*args, **kwargs)

            self._ses = ses
            self._trackball = link.Trackball()

            self.on_trait_change(self.mouseDownChanged, "mouseDown")
            self.on_trait_change(self.mousePosChanged, "mousePos")

            self.Render()


        def Render(self, fast=False):
            """
            Update the visualizer with a current rendering of the session.
            :fast: renders with lower fidelity but faster. Useful for interactive rendering such as to observe a changing value with a slider.
            """
            self.SetImage(self._ses.RenderToImage(fast=fast))

    
        def mouseDownChanged(self):
            w, h = map(int, self.resolution)
            x, y = map(int, (lambda x,y:(x*w,y*h))(*self.mousePos))
    
            if self.mouseDown:
                NavigationUtils.ConfigureTrackball(self._ses.ce, self._trackball)
                self._trackball.MouseOnTrackball(0, self.mouseButton, x, y, w, h)
            else:
                self._trackball.MouseOnTrackball(2, self.mouseButton, x, y, w, h)
                self._trackball.TrackballSetMatrix()
                NavigationUtils.SetAllCameras(self._ses.ce, self._trackball)
                self.Render(fast=False)
    
            # self.value = f"mouseDown change {change['old']} -> {change['new']}"
    
    
        def mousePosChanged(self):
            # self.value = f"mousePos change {change['old']} -> {change['new']}"
            w, h = map(int, self.resolution)
            x, y = map(int, (lambda x,y:(x*w,y*h))(*self.mousePos))
    
            if self.mouseDown:
                self._trackball.MouseOnTrackball(1, self.mouseButton, x, y, w, h)
                self._trackball.TrackballSetMatrix()
                NavigationUtils.SetAllCameras(self._ses.ce, self._trackball)
    
                self.Render(fast=True)
