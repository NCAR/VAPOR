import os

import cv2
import tempfile
from base64 import b64encode
from io import BytesIO

from .session import *

class Animation:
    def __init__(self, ses:Session):
        self._ses = ses
        self._frames = []


    def CaptureFrame(self):
        frame = self._ses.RenderToImage()

        if self._frames and frame.size != self._frames[0].size:
            raise ValueError(f"Frame resolution {frame.size} is different from animation resolution {self._frames[0].size}")

        self._frames.append(frame)


    def ShowInteractive(self):
        self.__requireIPython()

        from IPython.display import display
        import ipywidgets as widgets

        # displayHandle = display(None, display_id=True)

        # def callback(frame):
        #     displayHandle.update(self._frames[frame])

        play = widgets.Play(
            value=0,
            min=0,
            max=len(self._frames) - 1,
            step=1,
            interval=80,
            # _repeat=True,
        )

        def PILtoJPG(img):
            buf = BytesIO()
            img.save(buf, format="jpeg")
            return buf.getvalue()

        imageWidget = widgets.Image(
            value=PILtoJPG(self._frames[0]),
            format='jpg',
            width=self._frames[0].size[0],
            height=self._frames[0].size[1]
        )

        # def callback(frame):
        #     imageWidget.value = frame
        #     return imageWidget

        frameSlider = widgets.IntSlider(0, 0, len(self._frames) - 1)
        widgets.jslink((play, 'value'), (frameSlider, 'value'))

        intervalSlider = widgets.IntSlider(80, 30, 1000)
        widgets.jslink((intervalSlider, 'value'), (play, 'interval'))
        intervalWidget = widgets.HBox([widgets.Label("Animation Interval"), intervalSlider])

        # output = widgets.interactive_output(callback, {'frame': frameSlider})

        def frameChanged(change):
            imageWidget.value = PILtoJPG(self._frames[change.new])

        frameSlider.observe(frameChanged, names='value')

        w = widgets.VBox([imageWidget, widgets.HBox([play, frameSlider]), intervalWidget])
        display(w)


    def Show(self, framerate=15):
        self.__requireIPython()

        import IPython.display

        f = tempfile.NamedTemporaryFile(suffix='.mp4', delete=False)
        path = f.name
        f.close()

        self.SaveMP4(path, framerate)
        with open(path, "rb") as f:
            data = f.read()

        IPython.display.display(IPython.display.Video(data=data, embed=True, mimetype="video/mp4"))

        os.unlink(path)


    def SaveMP4(self, path:str, framerate=15):
        fourcc = cv2.VideoWriter_fourcc(*'avc1')
        video = cv2.VideoWriter(path, fourcc, 15, self._frames[0].size)
        for i in self._frames:
            video.write(cv2.cvtColor(np.array(i), cv2.COLOR_RGB2BGR))
        video.release()

        if config.IsRunningFromIPython():
            import IPython.display
            return IPython.display.FileLink(path)


    def __requireIPython(self):
        if not config.IsRunningFromIPython():
            raise RuntimeError(f"{self.__class__}.Show() only supported within an IPython environment")
