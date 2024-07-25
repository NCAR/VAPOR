from ..session import Session
from ..animation import Animation

def animate_camera_keyframes(session_paths, steps = None):
    """
    Given a list of file paths to sessions with different camera angles, returns an
    animation that performs keyframing with linear interpolation between those camera angles.
    """
    # Default for steps
    if steps == None:
        steps = [30] * (len(session_paths) - 1)

    # Parameter checks
    if len(session_paths) - len(steps) != 1:
        raise TypeError(f"With {len(session_paths)} keyframes given, 'steps' should be " +
                        f"length {len(session_paths) - 1} (currently {len(steps)})")
    
    # Load key frames as sessions
    key_frames = []
    for path in session_paths:
        ses = Session()
        ses.Load(path)
        key_frames.append(ses)

    # Visualization will use renderers from first session in list. Other sessions are only for camera angles
    primary_session = key_frames[0]
    anim = Animation(primary_session)
    cam = primary_session.GetCamera()
    total_frames = sum(steps)
    
    # Interpolate camera information between each key frame
    n = 0
    for i in range(len(key_frames) - 1):
        start = key_frames[i]
        end = key_frames[i+1]
        frames = steps[i]
        # Get starting information
        cam1 = start.GetCamera()
        dir1 = cam1.GetDirection()
        pos1 = cam1.GetPosition()
        up1 = cam1.GetUp()

        # Get ending information
        cam2 = end.GetCamera()
        dir2 = cam2.GetDirection()
        pos2 = cam2.GetPosition()
        up2 = cam2.GetUp()

        # Difference between camera positions on each axis
        dPositionX  = (pos2[0] - pos1[0])
        dPositionY  = (pos2[1] - pos1[1])
        dPositionZ  = (pos2[2] - pos1[2])

        # Difference between camera direction vectors on each axis
        dDirectionX = (dir2[0] - dir1[0])
        dDirectionY = (dir2[1] - dir1[1])
        dDirectionZ = (dir2[2] - dir1[2])

        # Difference between camera up vectors on each axis
        dUpX        = (up2[0] - up1[0])
        dUpY        = (up2[1] - up1[1])
        dUpZ        = (up2[2] - up1[2])

        # Linear interpolation between start and end
        for j in range(frames):
            position = [
                pos1[0]+dPositionX*j/frames,
                pos1[1]+dPositionY*j/frames,
                pos1[2]+dPositionZ*j/frames
            ]
            cam.SetPosition( position )

            direction = [
                dir1[0]+dDirectionX*j/frames,
                dir1[1]+dDirectionY*j/frames,
                dir1[2]+dDirectionZ*j/frames
            ]
            cam.SetDirection( direction )

            up = [
                up1[0]+dUpX*j/frames,
                up1[1]+dUpY*j/frames,
                up1[2]+dUpZ*j/frames
            ]
            cam.SetUp( up )
            anim.CaptureFrame()
            
            # Print status
            print(f"Rendering Animation [{'#'*round((j+n)*40/total_frames)}{' '*round(40-((j+n)*40/total_frames))}] {(j+1+n)*100/total_frames:.0f}%", end="\r")
        n += steps[i]
    return anim
    