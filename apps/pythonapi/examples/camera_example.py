# %% [md]
#
# # Controlling the Camera
#
# %%
import example_utils
from vapor import session, renderer, dataset, camera

ses = session.Session()
data = example_utils.OpenExampleDataset(ses)

ren = data.NewRenderer(renderer.VolumeIsoRenderer)
ren.SetIsoValues([-0.10, 0.2])

# Show 3D orientation arrows.
ses.GetSceneAnnotations().SetAxisArrowEnabled(True)

# %%
cam = ses.GetCamera()

# %%
help(cam.ViewAll)
cam.ViewAll()
ses.Show()

# %%
help(cam.AlignView)
cam.AlignView("-X")
ses.Show()

# %%
help(cam.Zoom)
cam.Zoom(-0.4)
ses.Show()

# %%
help(cam.LookAt)
cam.LookAt((32, -100, 100), ren.GetTransform().GetOrigin())
ses.Show()
