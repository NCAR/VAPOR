# Run this file to create an example DCP dataset


from netCDF4 import Dataset
import numpy as np


N = 200
dim = 3
np.random.seed(0)
pos = np.random.random_sample((N,dim))*200-100
vel = np.random.random_sample((N,dim))*20-1


# Generate example particles for next timestep
def StepSimulation() -> None:
    global pos, vel
    G = 3000
    r = np.sqrt(np.sum(pos**2, axis=-1))
    r = np.clip(r, 10, 100**2)
    d = -pos/r[:,None]
    vel += d*(G/r**2)[:,None]
    pos += vel * 0.1
    filt = np.column_stack((pos,vel))[np.all((pos>-100) & (pos<100), axis=1)]
    pos = filt[:, 0:dim]
    vel = filt[:, dim:dim*2]


# Wraps an array in an additional dimension
# This is required for data time varying data that is only spacial
# as is this case in this demo
def AddTimeDim(data: np.ndarray) -> np.ndarray:
    return np.expand_dims(data, axis=0)


def WriteTimestep(ts: float, positionData: np.ndarray, **kwargs: np.ndarray) -> None:
    dataset = Dataset(f"particles_{ts:03}.nc", "w", format="NETCDF4")

    particleCount:int = positionData.shape[0]
    dataset.createDimension("P", particleCount) # The P dimension represents the number of particles at this timestep
    dataset.createDimension("T", None) # Time dimension
    dataset.createDimension("axis", 3) # Utility dimension for packing 3 components for 3D particles into a single variable

    # Time coordinate
    T = dataset.createVariable("T", "f8", ("T",))
    T.units = "seconds"
    T[:] = np.array([ts])

    # 3D vars can be packed in a single variable by adding the axis dimension
    Position = dataset.createVariable("Position", "f4", ("T", "P", "axis"), zlib=True)
    # positionData is 2D (numParticles * axis) whereas Position is 3D (time * numParticles * axis)
    Position[:] = AddTimeDim(positionData)
    # Alternatively, you could do the following:
    # Position_x = dataset.createVariable("Position_x", "f4", ("T", "P"), zlib=True)
    # Position_x[:] = AddTimeDim(positionData_x:1d array)
    # and so on for the other 2 axes

    # Save all remaining particle properties passed in to nc file
    for name, data in kwargs.items():
        var = dataset.createVariable(name, "f4", ("T", "P", "axis")[0:data.ndim + 1], zlib=True)
        var[:] = AddTimeDim(data)

    dataset.close()


for ts in range(20):
    # Compute magnitude of velocity for each particle
    speed = np.sqrt(np.sum(vel**2, axis=-1))
    # Since 3-component properties such as velocity are common for 3D particles,
    # DCP allows packing them as a 2D array of size N_particles by 3
    # vel is an array of size Nx3 and speed is an array of size N
    WriteTimestep(ts, pos, vel=vel, speed=speed)
    # The following would also work
    # WriteTimestep(ts, pos, vel_x=vel[:,0], vel_y=vel[:,1], vel_z=vel[:,2], speed=speed)
    StepSimulation()
