# %% [md]
#
# # Creating Animations
#
# %%
import example_utils
from vapor import session, renderer, dataset, camera
from vapor.animation import Animation

# %%
ses = session.Session()
data = example_utils.OpenExampleDataset(ses)
dimension = 2
U,V = data.GetDataVarNames(dimension)[0:2]

ren:renderer.FlowRenderer = data.NewRenderer(renderer.FlowRenderer)
ren.SetFieldVariableNames([U, V])
ses.GetCamera().ViewAll()
ren.SetRenderType(ren.RenderType.RenderTypeStream)
ren.SetRenderRadiusScalar(3)
ren.SetRenderGeom3D(True)
ren.SetColorMapVariableName(U)
# ses.Show()

# %%
anim = Animation(ses)
for i in range(0, 200, 2):
    ren.SetSteadyNumOfSteps(i)
    anim.CaptureFrame()
    print(f"Rendering Animation [{'#'*round(i/5)}{' '*round(40-i/5)}] {(i+1)/2:.0f}%", end="\r")
anim.Show()

# %%
anim.ShowInteractive()

# %%
anim.SaveMP4("test.mp4")
