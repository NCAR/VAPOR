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
cam.ViewAll()
ses.Show()

# %%
cam.AlignView("-X")
ses.Show()

# %%
cam.Zoom(-0.4)
ses.Show()

# %%
cam.LookAt((32, -100, 100), ren.GetTransform().GetOrigin())
ses.Show()