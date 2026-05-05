from ..session import Session
from ..animation import Animation

def animate_camera_keyframes(session_paths, steps = None, time_interpolation = 'static', time_frames = None):
    """
    Given a list of file paths to sessions with different camera angles, returns an
    animation that performs keyframing with linear interpolation between those camera angles.
    Args:
        session_paths (list): List of file paths to sessions with different camera angles.
        steps (list, optional): List of number of frames to interpolate between each keyframe. 
                                Defaults to None, which will use 30 frames between each keyframe.
        time_interpolation (str, optional): How to handle time interpolation. 
                                            Options are 'static', 'auto', or 'manual'. Defaults to 'static'.
        time_frames (list, optional): If time_interpolation is 'manual', this should be a list of timesteps
                                      corresponding to the frames in the animation. 
                                      len(time_frames) should equal sum(steps).
    """
    # Check time_interpolation
    if(time_interpolation not in ["static", "auto", 'manual']):
        raise ValueError("time_interpolation must be one of 'static', 'auto', or 'manual'")

    # Default for steps
    if steps == None:
        steps = [30] * (len(session_paths) - 1)

    # Check steps
    if len(session_paths) - len(steps) != 1:
        raise ValueError(f"With {len(session_paths)} keyframes given, 'steps' should be " +
                        f"length {len(session_paths) - 1} (currently {len(steps)})")
    total_frames = sum(steps)

    # Load primary session
    primary_session = session.Session()
    primary_session.Load(session_paths[0])

    # If time is interpolated, compute time frames
    if time_interpolation == "auto":
        total_timesteps = primary_session.GetTimesteps()
        # Compute base number of frames per timestep
        base = total_frames // total_timesteps
        remainder = total_frames % total_timesteps
        # Distribute the remainder by giving one extra frame to the first 'remainder' timesteps
        time_frames = []
        for timestep in range(total_timesteps):
            count = base + (1 if timestep < remainder else 0)
            time_frames.extend([timestep] * count)

        
    
    if time_interpolation == "manual":
        if time_frames is None:
            raise ValueError("If time_interpolation is 'manual', time_frames must be provided")
        if len(time_frames) != total_frames:
            raise ValueError(f"With {len(session_paths)} steps given, 'time_frames' should be " +
                            f"length {total_frames} (currently {len(time_frames)})")
    

    
    # Load key frames as sessions
    key_frames = []
    for path in session_paths:
        ses = session.Session()
        ses.Load(path)
        key_frames.append(ses)

    # Visualization will use renderers from first session in list. Other sessions are only for camera angles
    primary_session = key_frames[0]
    anim = Animation(primary_session)
    cam = primary_session.GetCamera()
    
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

            # If time is interpolated, advance the timestep
            if (time_interpolation == "auto") | (time_interpolation == "manual"):
                timestep = time_frames[n+j]
                primary_session.SetTimestep(int(timestep))

            anim.CaptureFrame()
            
            # Print status
            print(f"Rendering Animation [{'#'*round((j+n)*40/total_frames)}{' '*round(40-((j+n)*40/total_frames))}] {(j+1+n)*100/total_frames:.0f}%", end="\r")
        n += steps[i]
    return anim



def animate_camera_keyframes_camerafiles(primary_session, camera_paths, steps=None, time_interpolation='static', time_frames=None):
    """
    Given a session and a list of paths to saved camera files, returns an animation that performs keyframing
    with linear interpolation on the saved camera angles. Can also handle time interpolation.
    
    Args:
        primary_session (Session): The primary session to render from.
        camera_paths (list): List of paths to saved camera files.
        steps (list, optional): Number of frames to interpolate between each keyframe. Defaults to 30 per segment.
        time_interpolation (str, optional): How to interpolate timestep. Options: 'static', 'auto', 'manual'. Default is 'static'.
        time_frames (list, optional): Required if time_interpolation is 'manual'. Should be a list of timesteps with length equal to total frames.
    """
    # Validate time_interpolation mode
    if time_interpolation not in ["static", "auto", "manual"]:
        raise ValueError("time_interpolation must be one of 'static', 'auto', or 'manual'")
    
    # Default steps
    if steps is None:
        steps = [30] * (len(camera_paths) - 1)
    
    # Validate steps length
    if len(camera_paths) - len(steps) != 1:
        raise ValueError(f"With {len(camera_paths)} keyframes, 'steps' should have length {len(camera_paths) - 1} (got {len(steps)})")
    
    total_frames = sum(steps)

    # Handle time_interpolation: auto
    if time_interpolation == "auto":
        total_timesteps = primary_session.GetTimesteps()
        base = total_frames // total_timesteps
        remainder = total_frames % total_timesteps
        time_frames = []
        for timestep in range(total_timesteps):
            count = base + (1 if timestep < remainder else 0)
            time_frames.extend([timestep] * count)

    # Handle time_interpolation: manual
    if time_interpolation == "manual":
        if time_frames is None:
            raise ValueError("If time_interpolation is 'manual', time_frames must be provided.")
        if len(time_frames) != total_frames:
            raise ValueError(f"time_frames length ({len(time_frames)}) must match total_frames ({total_frames})")

    anim = Animation(primary_session)
    cam = primary_session.GetCamera()
    
    n = 0
    for i in range(len(camera_paths) - 1):
        start = camera_paths[i]
        end = camera_paths[i+1]
        frames = steps[i]
        
        # Load start camera
        cam.LoadFromFile(start)
        dir1 = cam.GetDirection()
        pos1 = cam.GetPosition()
        up1 = cam.GetUp()

        # Load end camera
        cam.LoadFromFile(end)
        dir2 = cam.GetDirection()
        pos2 = cam.GetPosition()
        up2 = cam.GetUp()

        # Compute deltas
        dPosition = [p2 - p1 for p1, p2 in zip(pos1, pos2)]
        dDirection = [d2 - d1 for d1, d2 in zip(dir1, dir2)]
        dUp = [u2 - u1 for u1, u2 in zip(up1, up2)]

        for j in range(frames):
            f = j / frames
            position = [pos1[k] + dPosition[k]*f for k in range(3)]
            direction = [dir1[k] + dDirection[k]*f for k in range(3)]
            up = [up1[k] + dUp[k]*f for k in range(3)]

            cam.SetPosition(position)
            cam.SetDirection(direction)
            cam.SetUp(up)

            # Set time if applicable
            if time_interpolation in ["auto", "manual"]:
                timestep = time_frames[n + j]
                primary_session.SetTimestep(int(timestep))

            anim.CaptureFrame()
            print(f"Rendering Animation [{'#'*round((j+n)*40/total_frames)}{' '*round(40-((j+n)*40/total_frames))}] {(j+1+n)*100/total_frames:.0f}%", end="\r")
        
        n += frames

    return anim
