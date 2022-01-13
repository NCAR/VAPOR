""" The vapor_utils module contains:
    StaggeredToUnstaggeredGrid - resample staggered grid
    DerivVarFinDiff - calculate a derivative of one 3D variable with respect to another variable.
    interp3d - interpolate a 3D variable to a vertical level surface of another variable.
"""

import numpy as np

def _StaggeredToUnstaggeredGrid2D(a, axis):
    assert isinstance(a, np.ndarray), 'A is not np.ndarray'
    assert a.ndim == 2
    assert axis >= 0 and axis < a.ndim

    from scipy.interpolate import RectBivariateSpline

    x = np.arange(a.shape[1])
    y = np.arange(a.shape[0])

    if (axis == 1):
        xprime = np.arange(0.5, a.shape[axis] - 0.5)
        yprime = y
    else:
        xprime = x
        yprime = np.arange(0.5, a.shape[axis] - 0.5)
        

    interp_spline = RectBivariateSpline(y, x,a)
    return interp_spline(yprime, xprime)

def StaggeredToUnstaggeredGrid(a, axis):

    """Resample a numpy array on a staggered grid to an unstaggered grid

    This function is useful for resampling data sampled on an Arakawa C-grid
    to an Arakawa A-grid. E.g. resampling the velocity grid to the mass
    grid. It simply down samples the specified axis specified by `axis`
    by one grid point, locating the new grid points in the returned
    array at the midpoints of the samples in the original array, `a`

    Parameters
    -----------
    a : numpy.ndarray
        A two or three dimensional Numpy array

    axis : int 

        An integer in the range 0..n, where n is a.ndim - 1, 
        specifying which axis should be downsampled. Zero is the slowest
        varying dimension.

    Returns
    -------
    aprime: numpy.ndarray:
        The resampled array

    """

 
    assert isinstance(a, np.ndarray), 'A is not np.ndarray'
    assert a.ndim >= 2 and a.ndim <= 3
    assert axis >= 0 and axis < a.ndim

    if (a.ndim == 2):
        return _StaggeredToUnstaggeredGrid2D(a,axis)

    newshape = list(a.shape)
    newshape[axis] -= 1;
    aprime = np.empty(newshape, a.dtype)

    if (axis == 1 or axis == 2):
        for k in range(0,aprime.shape[0]):
            aprime[k,::,::] =  _StaggeredToUnstaggeredGrid2D(a[k,::,::],axis-1)

    else:
        for i in range(0,aprime.shape[2]):
            aprime[::,::,i] =  _StaggeredToUnstaggeredGrid2D(a[::,::,i],axis)

    return(aprime)

def Mag(*argv):

    """Return the magnitude of one or more vectors

    This method computes the vector magnitude of the Numpy arrays in
    *args*.  Each array in *args* must have the same number of dimensions.
    The arrays may be a mixture of staggered and unstaggered arrays. I.e.
    for any axis the dimension length may differ by no more than one.
    Staggered arrays are downsampled along the staggered axis to have the
    same dimension length as the unstaggered array. Thus all arrays are
    resampled as necessary to have the same shape prior to computing
    the array magnitude.

    Parameters
    ----------
    *argv : tuple of numpy.ndarray
        A a list of two or three-dimensional Numpy arrays

    Returns
    -------
    a : numpy.ndarray
        The vector magnitude

    """

    for arg in argv:
        assert isinstance(arg, np.ndarray), 'A is not np.ndarray'

    ndim = argv[0].ndim
    for i in range(0,len(argv)-1):
        assert ndim == argv[i].ndim, 'Arrays must all have same rank'

    shapes = np.empty(ndim*len(argv), dtype=int).reshape(len(argv), ndim)
    for i in range(0,len(argv)):
        shapes[i,::] = argv[i].shape
    
    baseshape = np.empty(ndim, dtype=int)
    for i in range(0,ndim):
        baseshape[i] = np.amin(shapes[::,i])

    for i in range(0,len(argv)):
        if (np.sum(np.array(argv[i].shape)-baseshape) > 1):
            raise ValueError("array dimensions may only differ by one")

    magsqr = np.zeros(np.prod(baseshape), argv[0].dtype).reshape(baseshape)

    for i in range(0,len(argv)):
        myshape = np.array(argv[i].shape)

        if np.array_equal(myshape,baseshape):
            magsqr += argv[i] * argv[i]
        else:
            axis = -1
            for j in range(0,myshape.size):
                if (myshape[j] == baseshape[j] + 1):
                    axis = j

            tmp = StaggeredToUnstaggeredGrid(argv[i], axis)
            magsqr += tmp * tmp


    return(np.sqrt(magsqr))

def _deriv_findiff2(a,axis,dx):
    """Function that calculates first-order derivatives 
    using 2nd order finite differences in regular Cartesian grids.
    """

    s = np.shape(a)    #size of the input array
    aprime = np.array(a)    #output has the same size than input

    #
    # derivative for user axis=2. In python this is the slowest 
    # varying 
    #
    if axis == 2: 

        #forward differences near the first boundary
        for i in range(1):
            aprime[:,:,i] = (-a[:,:,i]+a[:,:,i+1]) / (dx)     

        #centered differences
        for i in range(1,s[2]-1):
            aprime[:,:,i] = (-a[:,:,i-1]+a[:,:,i+1])/(2*dx) 

        #backward differences near the second boundary
        for i in range(s[2]-1,s[2]):
            aprime[:,:,i] = (a[:,:,i-1]-a[:,:,i]) /(dx)     

    #
    # derivative for axis=1
    #
    if axis == 1: 
        #forward differences near the first boundary
        for i in range(1):
            aprime[:,i,:] = (-a[:,i,:]+a[:,i+1,:]) / (dx)     

        #centered differences
        for i in range(1,s[1]-1):
            aprime[:,i,:] = (-a[:,i-1,:]+a[:,i+1,:])/(2*dx) 

        #backward differences near the second boundary
        for i in range(s[1]-1,s[1]):
            aprime[:,i,:] = (a[:,i-1,:]-a[:,i,:]) /(dx)     

    #
    # derivative for user axis=0
    #
    if axis == 0:
        #forward differences near the first boundary
        for i in range(1):
            aprime[i,:,:] = (-a[i,:,:]+a[i+1,:,:]) / (dx)     

        #centered differences
        for i in range(1,s[0]-1):
            aprime[i,:,:] = (-a[i-1,:,:]+a[i+1,:,:])/(2*dx) 

        #backward differences near the second boundary
        for i in range(s[0]-1,s[0]):
            aprime[i,:,:] = (a[i-1,:,:]-a[i,:,:]) /(dx)     

    return aprime

def _deriv_findiff4(a,axis,dx):
    """Function that calculates first-order derivatives 
    using 4th order finite differences in regular Cartesian grids.
    """

    s = np.shape(a)    #size of the input array
    aprime = np.array(a)    #output has the same size than input

    if axis == 2: 

        #forward differences near the first boundary
        for i in range(2):
            aprime[:,:,i] = (-3*a[:,:,i]+4*a[:,:,i+1]-a[:,:,i+2]) / (2*dx)     

        #centered differences
        for i in range(2,s[2]-2):
            aprime[:,:,i] = (a[:,:,i-2]-8*a[:,:,i-1]+8*a[:,:,i+1]-a[:,:,i+2])/(12*dx) 

        #backward differences near the second boundary
        for i in range(s[2]-2,s[2]):
            aprime[:,:,i] = (a[:,:,i-2]-4*a[:,:,i-1]+3*a[:,:,i]) /(2*dx)     

    #
    # derivative for axis=2
    #
    if axis == 1: 
        #forward differences near the first boundary
        for i in range(2):
            aprime[:,i,:] = (-3*a[:,i,:]+4*a[:,i+1,:]-a[:,i+2,:]) / (2*dx)     

        #centered differences
        for i in range(2,s[1]-2):
            aprime[:,i,:] = (a[:,i-2,:]-8*a[:,i-1,:]+8*a[:,i+1,:]-a[:,i+2,:])/(12*dx) 

        #backward differences near the second boundary
        for i in range(s[1]-2,s[1]):
            aprime[:,i,:] = (a[:,i-2,:]-4*a[:,i-1,:]+3*a[:,i,:]) /(2*dx)     

    #
    # derivative for user axis=3
    #
    if axis == 0:
        #forward differences near the first boundary
        for i in range(2):
            aprime[i,:,:] = (-3*a[i,:,:]+4*a[i+1,:,:]-a[i+2,:,:]) / (2*dx)     

        #centered differences
        for i in range(2,s[0]-2):
            aprime[i,:,:] = (a[i-2,:,:]-8*a[i-1,:,:]+8*a[i+1,:,:]-a[i+2,:,:])/(12*dx) 

        #backward differences near the second boundary
        for i in range(s[0]-2,s[0]):
            aprime[i,:,:] = (a[i-2,:,:]-4*a[i-1,:,:]+3*a[i,:,:]) /(2*dx)     


    return aprime

def _deriv_var_findiff2(a,var,axis):

    s = np.shape(a)    #size of the input array
    aprime = np.array(a)    #output has the same size than input

    if axis == 2: 

        #forward differences near the first boundary
        for i in range(1):
            aprime[:,:,i] = (-a[:,:,i]+a[:,:,i+1]) / (-var[:,:,i]+var[:,:,i+1]) 

        #centered differences
        for i in range(1,s[2]-1):
            aprime[:,:,i] = (-a[:,:,i-1]+a[:,:,i+1])/(-var[:,:,i-1]+var[:,:,i+1])

        #backward differences near the second boundary
        for i in range(s[2]-1,s[2]):
            aprime[:,:,i] = (a[:,:,i-1]-a[:,:,i]) / (var[:,:,i-1]-var[:,:,i])

    if axis == 1:
        #forward differences near the first boundary
        for i in range(1):
            aprime[:,i,:] = (-a[:,i,:]+a[:,i+1,:]) / (-var[:,i,:]+var[:,i+1,:]) 

        #centered differences
        for i in range(1,s[1]-1):
            aprime[:,i,:] = (-a[:,i-1,:]+a[:,i+1,:])/(-var[:,i-1,:]+var[:,i+1,:])

        #backward differences near the second boundary
        for i in range(s[1]-1,s[1]):
            aprime[:,i,:] = (a[:,i-1,:]-a[:,i,:]) / (var[:,i-1,:]-var[:,i,:])

    #
    #
    if axis == 0:
        #forward differences near the first boundary
        for i in range(1):
            aprime[i,:,:] = (-a[i,:,:]+a[i+1,:,:]) / (-var[i,:,:]+var[i+1,:,:])

        #centered differences
        for i in range(1,s[0]-1):
            aprime[i,:,:] = (-a[i-1,:,:]+a[i+1,:,:])/ (-var[i-1,:,:]+var[i+1,:,:])

        #backward differences near the second boundary
        for i in range(s[0]-1,s[0]):
            aprime[i,:,:] = (a[i-1,:,:]-a[i,:,:]) / (var[i-1,:,:]-var[i,:,:])

    return aprime



def _deriv_var_findiff4(a,var,axis):

    s = np.shape(a)    #size of the input array
    aprime = np.array(a)    #output has the same size than input

    #
    # derivative for user axis=2
    #
    if axis == 2: 

        #forward differences near the first boundary
        for i in range(2):
            aprime[:,:,i] = (-3*a[:,:,i]+4*a[:,:,i+1]-a[:,:,i+2]) / (-3*var[:,:,i]+4*var[:,:,i+1]-var[:,:,i+2]) 

        #centered differences
        for i in range(2,s[2]-2):
            aprime[:,:,i] = (a[:,:,i-2]-8*a[:,:,i-1]+8*a[:,:,i+1]-a[:,:,i+2])/(var[:,:,i-2]-8*var[:,:,i-1]+8*var[:,:,i+1]-var[:,:,i+2])

        #backward differences near the second boundary
        for i in range(s[2]-2,s[2]):
            aprime[:,:,i] = (a[:,:,i-2]-4*a[:,:,i-1]+3*a[:,:,i]) / (var[:,:,i-2]-4*var[:,:,i-1]+3*var[:,:,i])

    #
    #
    if axis == 1: 
        #forward differences near the first boundary
        for i in range(2):
            aprime[:,i,:] = (-3*a[:,i,:]+4*a[:,i+1,:]-a[:,i+2,:]) / (-3*var[:,i,:]+4*var[:,i+1,:]-var[:,i+2,:])

        #centered differences
        for i in range(2,s[1]-2):
            aprime[:,i,:] = (a[:,i-2,:]-8*a[:,i-1,:]+8*a[:,i+1,:]-a[:,i+2,:])/ (var[:,i-2,:]-8*var[:,i-1,:]+8*var[:,i+1,:]-var[:,i+2,:])

        #backward differences near the second boundary
        for i in range(s[1]-2,s[1]):
            aprime[:,i,:] = (a[:,i-2,:]-4*a[:,i-1,:]+3*a[:,i,:]) / (var[:,i-2,:]-4*var[:,i-1,:]+3*var[:,i,:])

    #
    #
    if axis == 0:
        #forward differences near the first boundary
        for i in range(2):
            aprime[i,:,:] = (-3*a[i,:,:]+4*a[i+1,:,:]-a[i+2,:,:]) / (-3*var[i,:,:]+4*var[i+1,:,:]-var[i+2,:,:])

        #centered differences
        for i in range(2,s[0]-2):
            aprime[i,:,:] = (a[i-2,:,:]-8*a[i-1,:,:]+8*a[i+1,:,:]-a[i+2,:,:])/ (var[i-2,:,:]-8*var[i-1,:,:]+8*var[i+1,:,:]-var[i+2,:,:])

        #backward differences near the second boundary
        for i in range(s[0]-2,s[0]):
            aprime[i,:,:] = (a[i-2,:,:]-4*a[i-1,:,:]+3*a[i,:,:]) / (var[i-2,:,:]-4*var[i-1,:,:]+3*var[i,:,:])


    return aprime

def DerivVarFinDiff(a,var,axis,order=6):

    """ Function that calculates first-order derivatives on Cartesian grids
    with respect to another variable

    This function computes the partial derivative of a multidimensional
    array using 2nd, 4th, or 6th order finite differences with respect
    to a second multidimensional array of the same shape.

    Parameters
    ----------

    a : numpy.ndarray
        A two or three-dimensional Numpy array

    var : numpy.ndarray
        A two or three-dimensional Numpy array of the same shape as
        `a`

    axis : int
        The axis along which the derivative should be taken. The slowest
        varying axis is 0. The next slowest is 1.

    order : int, optional
        The accuracy order of finite difference method. The default is 6. Valid 
        values are 2, 4, 6.

    Calling sequence
    ----------------

    >>> deriv = DerivFinDiff(a,var,delta, order=6)

    Returns
    -------

    da_var : numpy.ndarray
        The derivative of `a` with respect to `var` along `axis` 

    """

    if order == 4:
        return deriv_var_findiff4(a,var,axis)

    if order == 2:
        return deriv_var_findiff2(a,var,axis)

    s = np.shape(a)    #size of the input array
    aprime = np.array(a)    #output has the same size than input

    #
    # derivative for axis=2
    #
    if axis == 2:
        if (s[2] < 2):
            return np.zeros_like(a)
        if (s[2] < 4):
            return deriv_var_findiff2(a,var,axis)
        if (s[2] < 6):
            return deriv_var_findiff4(a,var,axis)

        #forward differences near the first boundary
        for i in range(3):
            aprime[:,:,i] = (-11*a[:,:,i]+18*a[:,:,i+1]-9*a[:,:,i+2]+2*a[:,:,i+3]) / (-11*var[:,:,i]+18*var[:,:,i+1]-9*var[:,:,i+2]+2*var[:,:,i+3])     

        #centered differences
        for i in range(3,s[2]-3):
            aprime[:,:,i] = (-a[:,:,i-3]+9*a[:,:,i-2]-45*a[:,:,i-1]+45*a[:,:,i+1] -9*a[:,:,i+2]+a[:,:,i+3])/(-var[:,:,i-3]+9*var[:,:,i-2]-45*var[:,:,i-1]+45*var[:,:,i+1] -9*var[:,:,i+2]+var[:,:,i+3]) 

        #backward differences near the second boundary
        for i in range(s[2]-3,s[2]):
            aprime[:,:,i] = (-2*a[:,:,i-3]+9*a[:,:,i-2]-18*a[:,:,i-1]+11*a[:,:,i]) /(-2*var[:,:,i-3]+9*var[:,:,i-2]-18*var[:,:,i-1]+11*var[:,:,i])     

    #
    # derivative for axis=1
    #
    if axis == 1: 
        if (s[1] < 2):
            return np.zeros_like(a)
        if (s[1] < 4):
            return deriv_var_findiff2(a,var,axis)
        if (s[1] < 6):
            return deriv_var_findiff4(a,var,axis)

        for i in range(3):
            aprime[:,i,:] = (-11*a[:,i,:]+18*a[:,i+1,:]-9*a[:,i+2,:]+2*a[:,i+3,:]) /(-11*var[:,i,:]+18*var[:,i+1,:]-9*var[:,i+2,:]+2*var[:,i+3,:])      #forward differences near the first boundary

        for i in range(3,s[1]-3):
            aprime[:,i,:] = (-a[:,i-3,:]+9*a[:,i-2,:]-45*a[:,i-1,:]+45*a[:,i+1,:] -9*a[:,i+2,:]+a[:,i+3,:])/(-var[:,i-3,:]+9*var[:,i-2,:]-45*var[:,i-1,:]+45*var[:,i+1,:] -9*var[:,i+2,:]+var[:,i+3,:]) #centered differences

        for i in range(s[1]-3,s[1]):
            aprime[:,i,:] = (-2*a[:,i-3,:]+9*a[:,i-2,:]-18*a[:,i-1,:]+11*a[:,i,:]) /(-2*var[:,i-3,:]+9*var[:,i-2,:]-18*var[:,i-1,:]+11*var[:,i,:])     #backward differences near the second boundary

    #
    # derivative for axis=0
    #
    if axis == 0:
        if (s[0] < 2):
            return np.zeros_like(a)
        if (s[0] < 4):
            return deriv_var_findiff2(a,var,axis)
        if (s[0] < 6):
            return deriv_var_findiff4(a,var,axis)

        for i in range(3):
            aprime[i,:,:] = (-11*a[i,:,:]+18*a[i+1,:,:]-9*a[i+2,:,:]+2*a[i+3,:,:]) /(-11*var[i,:,:]+18*var[i+1,:,:]-9*var[i+2,:,:]+2*var[i+3,:,:])     #forward differences near the first boundary

        for i in range(3,s[0]-3):
            aprime[i,:,:] = (-a[i-3,:,:]+9*a[i-2,:,:]-45*a[i-1,:,:]+45*a[i+1,:,:] -9*a[i+2,:,:]+a[i+3,:,:])/(-var[i-3,:,:]+9*var[i-2,:,:]-45*var[i-1,:,:]+45*var[i+1,:,:] -9*var[i+2,:,:]+var[i+3,:,:]) #centered differences

        for i in range(s[0]-3,s[0]):
            aprime[i,:,:] = (-2*a[i-3,:,:]+9*a[i-2,:,:]-18*a[i-1,:,:]+11*a[i,:,:]) /(-2*var[i-3,:,:]+9*var[i-2,:,:]-18*var[i-1,:,:]+11*var[i,:,:])     #backward differences near the second boundary

    return aprime

def interp3d(A,PR,val):
    '''Method that vertically interpolates one 3D variable to a level determined by 
    another variable.  The second variable (PR) is typically pressure.  
    The second variable must decrease
    as a function of z (elevation).  The returned value is a 2D variable having
    values interpolated to the surface defined by PR = val
    Sweep array from bottom to top'''
    s = np.shape(PR)    #size of the input arrays
    ss = [s[1],s[2]] # shape of 2d arrays
    interpVal = np.empty(ss,np.float32)
    ratio = np.zeros(ss,np.float32)

    #  the LEVEL value is determine the lowest level where P<=val
    LEVEL = np.empty(ss,np.int32)
    LEVEL[:,:] = -1 #value where PR<=val has not been found
    for K in range(s[0]):
        #LEVNEED is true if this is first time PR<val.
        LEVNEED = np.logical_and(np.less(LEVEL,0), np.less(PR[K,:,:] , val))
        LEVEL[LEVNEED]=K
        ratio[LEVNEED] = (val-PR[K,LEVNEED])/(PR[K-1,LEVNEED]-PR[K,LEVNEED])
        interpVal[LEVNEED] = ratio[LEVNEED]*A[K,LEVNEED]+(1-ratio[LEVNEED])*A[K-1,LEVNEED] 
        LEVNEED = np.greater(LEVEL,0)
    # Set unspecified values to value of A at top of data:
    LEVNEED = np.less(LEVEL,0)
    interpVal[LEVNEED] = A[s[0]-1,LEVNEED]
    return interpVal
