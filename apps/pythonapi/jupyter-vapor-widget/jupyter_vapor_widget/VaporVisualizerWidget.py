import ipywidgets as widgets
from traitlets import Unicode, Bytes, Tuple, Bool, observe, Int


##########################################
#          Development utility
import importlib
vapor_spec = importlib.util.find_spec("vapor")
found = vapor_spec is not None

if not found:
    import sys
    import os
    from pathlib import Path
    sys.path.append(os.path.expanduser('~/Work/build-python/python'))
#########################################

from PIL import Image
from PIL import ImageDraw
from base64 import b64encode
from io import BytesIO


@widgets.register
class CanvasStreamWidget(widgets.DOMWidget):
    """Streams image data to an HTML5 Canvas."""

    # Name of the widget view class in front-end
    _view_name = Unicode('VaporVisualizerView').tag(sync=True)

    # Name of the widget model class in front-end
    _model_name = Unicode('VaporVisualizerModel').tag(sync=True)

    # Name of the front-end module containing widget view
    _view_module = Unicode('jupyter-vapor-widget').tag(sync=True)

    # Name of the front-end module containing widget model
    _model_module = Unicode('jupyter-vapor-widget').tag(sync=True)

    # Version of the front-end module containing widget view
    _view_module_version = Unicode('^1.0').tag(sync=True)
    # Version of the front-end module containing widget model
    _model_module_version = Unicode('^1.0').tag(sync=True)

    # Widget specific property.
    # Widget properties are defined as traitlets. Any property tagged with `sync=True`
    # is automatically synced to the frontend *any* time it changes in Python.
    # It is synced back to Python from the frontend *any* time the model is touched.
    value = Unicode('Visualizer is here').tag(sync=True)
    imageData = Unicode('').tag(sync=True)
    imageFormat = Unicode('').tag(sync=True)
    resolution = Tuple((640, 480)).tag(sync=True)

    mousePos = Tuple((0, 0)).tag(sync=True)
    mouseDown = Bool(False).tag(sync=True)
    mouseButton = Int(1).tag(sync=True)


    def __init__(self, *args, **kwargs):
        # print(f"PYTHON VaporVisualizerWidget({args}, {kwargs})")
        super().__init__(**kwargs)


    def SetImage(self, img: Image):
        buf = BytesIO()
        img.save(buf, format="jpeg")
        enc = b64encode(buf.getvalue())
        self.resolution = img.size
        self.imageData = enc
        self.imageFormat = "jpeg"
        # self.value = "Image was set"

    def _DebugShowMousePos(self):
        image = Image.new('RGB', self.resolution)
        draw = ImageDraw.Draw(image)

        iw = int(image.size[0])
        ih = int(image.size[1])
        mx = int(self.mousePos[0] * iw)
        my = int(self.mousePos[1] * ih)

        self.value = f"""
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
    
    @widgets.register
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
            self._ses = ses
            self._trackball = link.Trackball()
            self.Render()
    
            # import sys
            # sys.stdout = open("stdout.txt", "w")
            # sys.stderr = open("stderr.txt", "w")
    
            super().__init__(*args, **kwargs)


        def Render(self, fast=False):
            """
            Update the visualizer with a current rendering of the session.
            :fast: renders with lower fidelity but faster. Useful for interactive rendering such as to observe a changing value with a slider.
            """
            self.SetImage(self._ses.RenderToImage(fast=fast))

    
        @observe('mouseDown')
        def mouseDownChanged(self, change):
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
    
            self.value = f"mouseDown change {change['old']} -> {change['new']}"
    
    
        @observe('mousePos')
        def mousePosChanged(self, change):
            self.value = f"mousePos change {change['old']} -> {change['new']}"
            w, h = map(int, self.resolution)
            x, y = map(int, (lambda x,y:(x*w,y*h))(*self.mousePos))
    
            if self.mouseDown:
                self._trackball.MouseOnTrackball(1, self.mouseButton, x, y, w, h)
                self._trackball.TrackballSetMatrix()
                NavigationUtils.SetAllCameras(self._ses.ce, self._trackball)
    
                self.Render(fast=True)
