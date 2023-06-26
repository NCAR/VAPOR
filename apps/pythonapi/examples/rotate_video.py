# %% [md]
#
# # Rotate Video
#
# This script will render an animated rotating view of your session file.
# 
# Also requires the `scipy` package.
#
# %%
import example_utils
import cv2, os
from vapor import session, animation
from numpy import cross, eye, dot, radians, asarray, array
from scipy.linalg import expm, norm
UseValueFromSessionFile = None

# %% [md]
#
# ## Configuration
#
# %%
session_path = "/path/to/session.vs3"
output = "animation.mp4"
video_framerate = 30
video_resolution = (640, 480)
data_timestep_framerate = 6  # set to zero to disable
duration = 4  # seconds
rotate_speed = 90  # deg/s
rotation_axis = [0,0,1]  # Z (up)
rotation_center = UseValueFromSessionFile  # Can be replaced with [x,y,z] coordinates here
save_individual_frames = False

# %% ---------------------------------------------------------------------------------------
session_path, output = [os.path.expanduser(p) for p in (session_path, output)]
n_frames = video_framerate * duration

ses = session.Session()
ses.Load(session_path)
ses.SetResolution(*video_resolution)
cam = ses.GetCamera()
pos, dir, up, tgt = [asarray(x) for x in [cam.GetPosition(), cam.GetDirection(), cam.GetUp(), cam.GetTarget()]]
if rotation_center:
    tgt = asarray(rotation_center)

def rotation_matrix(axis, theta):
    return expm(cross(eye(3), axis / norm(axis) * theta))

anim = animation.Animation(ses)
for i in range(0, n_frames):
    print(f"Rendering... [{'#'*round(40*i/(n_frames-1))}{' '*round(40*(1-i/(n_frames-1)))}] {100*(i+1)/n_frames:.0f}%", end="\r" if i < n_frames-1 else "\n")

    ses.SetTimestep(int(data_timestep_framerate * i / video_framerate))

    M = rotation_matrix(rotation_axis, radians(rotate_speed) * i / video_framerate)
    cam.SetPosition(dot(M, pos - tgt) + tgt)
    cam.SetDirection(dot(M, dir))
    cam.SetUp(dot(M, up))
    anim.CaptureFrame()
    if save_individual_frames:
        ses.Render(f"{output}_{i:04}.png")

anim.SaveMP4(output, video_framerate)
