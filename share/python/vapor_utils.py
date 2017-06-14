''' The vapor_utils module contains:
	mag3d - calculate magnitude of 3-vector
	mag2d - calculate magnitude of 2-vector
	deriv_findiff - calculate derivative using 6th order finite differences
	curl_findiff - calculate curl using 6th order finite differences
	div_findiff - calculate divergence using 6 order finite differences
	grad_findiff - calculate gradient using 6th order finite differences.
	deriv_var_findiff - calculate a derivative of one 3D 
variable with respect to another variable.
	interp3d - interpolate a 3D variable to a vertical level surface of another variable.
	vector_rotate - rotate and scale vector field for lat-lon grid.	
'''

def mag3d(a1,a2,a3): 
	'''Calculate the magnitude of a 3-vector.
	Calling sequence: MAG = mag3d(A,B,C)
	Where:  A, B, and C are float32 numpy arrays.
	Result MAG is a float32 numpy array containing the square root
	of the sum of the squares of A, B, and C.'''
	from numpy import sqrt
	return sqrt(a1*a1 + a2*a2 + a3*a3)

def mag2d(a1,a2): 
	'''Calculate the magnitude of a 2-vector.
	Calling sequence: MAG = mag2d(A,B)
	Where:  A, and B are float32 numpy arrays.
	Result MAG is a float32 numpy array containing the square root
	of the sum of the squares of A and B.'''
	from numpy import sqrt
	return sqrt(a1*a1 + a2*a2)

def deriv_findiff2(a,dir,dx):
	''' Function that calculates first-order derivatives 
	using 2nd order finite differences in regular Cartesian grids.'''

	import numpy 
	s = numpy.shape(a)	#size of the input array
	aprime = numpy.array(a)	#output has the same size than input

	#
	# derivative for user dir=1, in python this is third coordinate
	#
	if dir == 1: 

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
	# derivative for dir=2
	#
	if dir == 2: 
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
	# derivative for user dir=3
	#
	if dir == 3:
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

def deriv_findiff4(a,dir,dx):
	''' Function that calculates first-order derivatives 
	using 4th order finite differences in regular Cartesian grids.'''

	import numpy 
	s = numpy.shape(a)	#size of the input array
	aprime = numpy.array(a)	#output has the same size than input

	#
	# derivative for user dir=1, in python this is third coordinate
	#
	if dir == 1: 

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
	# derivative for dir=2
	#
	if dir == 2: 
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
	# derivative for user dir=3
	#
	if dir == 3:
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

def deriv_findiff(a,dir,dx,order=6):
	''' Function that calculates first-order derivatives 
	using sixth order finite differences in regular Cartesian grids.
	Calling sequence: deriv = deriv_findiff(ary,dir,delta, order=6)
	ary is a 3-dimensional float32 numpy array
	dir is 1, 2, or 3 (for X, Y, or Z directions in user coordinates)
	delta is the grid spacing in the direction dir.
	order is the accuracy order (one of 6,4,2)
	Returned array 'deriv' is the derivative of ary in the 
	specified direction dir. '''
	import numpy 

	if order == 4:
		return deriv_findiff4(a,dir,dx)

	if order == 2:
		return deriv_findiff2(a,dir,dx)

	s = numpy.shape(a)	#size of the input array
	aprime = numpy.array(a)	#output has the same size than input

	#
	# derivative for user dir=1, in python this is third coordinate
	#
	if dir == 1: 
		if (s[2] < 2):
			return numpy.zeros_like(a)
		if (s[2] < 4):
			return deriv_findiff2(a,dir,dx)
		if (s[2] < 6):
			return deriv_findiff4(a,dir,dx)

		#forward differences near the first boundary
		for i in range(3):
			aprime[:,:,i] = (-11*a[:,:,i]+18*a[:,:,i+1]-9*a[:,:,i+2]+2*a[:,:,i+3]) / (6*dx)     

		#centered differences
		for i in range(3,s[2]-3):
			aprime[:,:,i] = (-a[:,:,i-3]+9*a[:,:,i-2]-45*a[:,:,i-1]+45*a[:,:,i+1] -9*a[:,:,i+2]+a[:,:,i+3])/(60*dx) 

		#backward differences near the second boundary
		for i in range(s[2]-3,s[2]):
			aprime[:,:,i] = (-2*a[:,:,i-3]+9*a[:,:,i-2]-18*a[:,:,i-1]+11*a[:,:,i]) /(6*dx)     

	#
	# derivative for dir=2
	#
	if dir == 2: 
		if (s[1] < 2):
			return numpy.zeros_like(a)
		if (s[1] < 4):
			return deriv_findiff2(a,dir,dx)
		if (s[1] < 6):
			return deriv_findiff4(a,dir,dx)

		for i in range(3):
			aprime[:,i,:] = (-11*a[:,i,:]+18*a[:,i+1,:]-9*a[:,i+2,:]+2*a[:,i+3,:]) /(6*dx)     #forward differences near the first boundary

		for i in range(3,s[1]-3):
			aprime[:,i,:] = (-a[:,i-3,:]+9*a[:,i-2,:]-45*a[:,i-1,:]+45*a[:,i+1,:] -9*a[:,i+2,:]+a[:,i+3,:])/(60*dx) #centered differences

		for i in range(s[1]-3,s[1]):
			aprime[:,i,:] = (-2*a[:,i-3,:]+9*a[:,i-2,:]-18*a[:,i-1,:]+11*a[:,i,:]) /(6*dx)     #backward differences near the second boundary

	#
	# derivative for user dir=3
	#
	if dir == 3:
		if (s[0] < 2):
			return numpy.zeros_like(a)
		if (s[0] < 4):
			return deriv_findiff2(a,dir,dx)
		if (s[0] < 6):
			return deriv_findiff4(a,dir,dx)

		for i in range(3):
			aprime[i,:,:] = (-11*a[i,:,:]+18*a[i+1,:,:]-9*a[i+2,:,:]+2*a[i+3,:,:]) /(6*dx)     #forward differences near the first boundary

		for i in range(3,s[0]-3):
			aprime[i,:,:] = (-a[i-3,:,:]+9*a[i-2,:,:]-45*a[i-1,:,:]+45*a[i+1,:,:] -9*a[i+2,:,:]+a[i+3,:,:])/(60*dx) #centered differences

		for i in range(s[0]-3,s[0]):
			aprime[i,:,:] = (-2*a[i-3,:,:]+9*a[i-2,:,:]-18*a[i-1,:,:]+11*a[i,:,:]) /(6*dx)     #backward differences near the second boundary

	return aprime

def deriv_var_findiff2(a,var,dir):
	''' Function that calculates first-order derivatives 
	using 2nd order finite differences in regular Cartesian grids.'''

	import numpy 
	s = numpy.shape(a)	#size of the input array
	aprime = numpy.array(a)	#output has the same size than input

	#
	# derivative for user dir=1, in python this is third coordinate
	#
	if dir == 1: 

		#forward differences near the first boundary
		for i in range(1):
			aprime[:,:,i] = (-a[:,:,i]+a[:,:,i+1]) / (-var[:,:,i]+var[:,:,i+1]) 

		#centered differences
		for i in range(1,s[2]-1):
			aprime[:,:,i] = (-a[:,:,i-1]+a[:,:,i+1])/(-var[:,:,i-1]+var[:,:,i+1])

		#backward differences near the second boundary
		for i in range(s[2]-1,s[2]):
			aprime[:,:,i] = (a[:,:,i-1]-a[:,:,i]) / (var[:,:,i-1]-var[:,:,i])

	#
	# derivative for dir=2
	#
	if dir == 2: 
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
	# derivative for user dir=3
	#
	if dir == 3:
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



def deriv_var_findiff4(a,var,dir):
	''' Function that calculates first-order derivatives of a 	variable 
	with respect to another variable 'var' 
	using 4th order finite differences in regular Cartesian grids.'''

	import numpy 
	s = numpy.shape(a)	#size of the input array
	aprime = numpy.array(a)	#output has the same size than input

	#
	# derivative for user dir=1, in python this is third coordinate
	#
	if dir == 1: 

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
	# derivative for dir=2
	#
	if dir == 2: 
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
	# derivative for user dir=3
	#
	if dir == 3:
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

def deriv_var_findiff(a,var,dir,order=6):
	''' Function that calculates first-order derivatives of a 	variable 
	with respect to another variable 'var' 
	using sixth order finite differences in regular Cartesian grids.
	The variable var should be increasing or decreasing in the
	specified coordinate. 
	Calling sequence: aprime = deriv_var_findiff(a,var,dir,order)
	a and var are 3-dimensional float32 numpy arrays
	dir is 1, 2, or 3 (for X, Y, or Z directions in user coords.
	order is the accuracy order, one of (2,4 or 6)
	Note that these correspond to Z,Y,X directions in python coords.

	Returned array 'aprime' is the derivative of a wrt var. '''

	import numpy 

	if order == 4:
		return deriv_var_findiff4(a,var,dir)

	if order == 2:
		return deriv_var_findiff2(a,var,dir)

	s = numpy.shape(a)	#size of the input array
	aprime = numpy.array(a)	#output has the same size than input

	#
	# derivative for dir=1
	#
	if dir == 1: 
		if (s[2] < 2):
			return numpy.zeros_like(a)
		if (s[2] < 4):
			return deriv_var_findiff2(a,var,dir)
		if (s[2] < 6):
			return deriv_var_findiff4(a,var,dir)

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
	# derivative for dir=2
	#
	if dir == 2: 
		if (s[1] < 2):
			return numpy.zeros_like(a)
		if (s[1] < 4):
			return deriv_var_findiff2(a,var,dir)
		if (s[1] < 6):
			return deriv_var_findiff4(a,var,dir)

		for i in range(3):
			aprime[:,i,:] = (-11*a[:,i,:]+18*a[:,i+1,:]-9*a[:,i+2,:]+2*a[:,i+3,:]) /(-11*var[:,i,:]+18*var[:,i+1,:]-9*var[:,i+2,:]+2*var[:,i+3,:])      #forward differences near the first boundary

		for i in range(3,s[1]-3):
			aprime[:,i,:] = (-a[:,i-3,:]+9*a[:,i-2,:]-45*a[:,i-1,:]+45*a[:,i+1,:] -9*a[:,i+2,:]+a[:,i+3,:])/(-var[:,i-3,:]+9*var[:,i-2,:]-45*var[:,i-1,:]+45*var[:,i+1,:] -9*var[:,i+2,:]+var[:,i+3,:]) #centered differences

		for i in range(s[1]-3,s[1]):
			aprime[:,i,:] = (-2*a[:,i-3,:]+9*a[:,i-2,:]-18*a[:,i-1,:]+11*a[:,i,:]) /(-2*var[:,i-3,:]+9*var[:,i-2,:]-18*var[:,i-1,:]+11*var[:,i,:])     #backward differences near the second boundary

	#
	# derivative for dir=3
	#
	if dir == 3:
		if (s[0] < 2):
			return numpy.zeros_like(a)
		if (s[0] < 4):
			return deriv_var_findiff2(a,var,dir)
		if (s[0] < 6):
			return deriv_var_findiff4(a,var,dir)

		for i in range(3):
			aprime[i,:,:] = (-11*a[i,:,:]+18*a[i+1,:,:]-9*a[i+2,:,:]+2*a[i+3,:,:]) /(-11*var[i,:,:]+18*var[i+1,:,:]-9*var[i+2,:,:]+2*var[i+3,:,:])     #forward differences near the first boundary

		for i in range(3,s[0]-3):
			aprime[i,:,:] = (-a[i-3,:,:]+9*a[i-2,:,:]-45*a[i-1,:,:]+45*a[i+1,:,:] -9*a[i+2,:,:]+a[i+3,:,:])/(-var[i-3,:,:]+9*var[i-2,:,:]-45*var[i-1,:,:]+45*var[i+1,:,:] -9*var[i+2,:,:]+var[i+3,:,:]) #centered differences

		for i in range(s[0]-3,s[0]):
			aprime[i,:,:] = (-2*a[i-3,:,:]+9*a[i-2,:,:]-18*a[i-1,:,:]+11*a[i,:,:]) /(-2*var[i-3,:,:]+9*var[i-2,:,:]-18*var[i-1,:,:]+11*var[i,:,:])     #backward differences near the second boundary

	return aprime

def curl_findiff(A,B,C,order=6):
	''' Operator that calculates the curl of a vector field 
	using 6th order finite differences, on a regular Cartesian grid.
	Calling sequence:  curlfield = curl_findiff(A,B,C,order)
	Where:
	A,B,C are three 3-dimensional float32 arrays that define a
	vector field.  
	order is the accuracy order (6,4, or 2)
	curlfield is a 3-tuple of 3-dimensional float32 arrays that is 
	returned by this operator.'''
	import vapor
	ext = (vapor.BOUNDS[3]-vapor.BOUNDS[0], vapor.BOUNDS[4]-vapor.BOUNDS[1],vapor.BOUNDS[5]-vapor.BOUNDS[2])
	usrmax = vapor.MapVoxToUser([vapor.BOUNDS[3],vapor.BOUNDS[4],vapor.BOUNDS[5]],vapor.REFINEMENT,vapor.LOD)
	usrmin = vapor.MapVoxToUser([vapor.BOUNDS[0],vapor.BOUNDS[1],vapor.BOUNDS[2]],vapor.REFINEMENT,vapor.LOD)
	dx =  (usrmax[2]-usrmin[2])/ext[2]	# grid delta is dx in user coord system
	dy =  (usrmax[1]-usrmin[1])/ext[1]
	dz =  (usrmax[0]-usrmin[0])/ext[0]
	
	aux1 = deriv_findiff(C,2,dy,order)       #x component of the curl
	aux2 = deriv_findiff(B,3,dz,order)     
	outx = aux1-aux2						

	aux1 = deriv_findiff(A,3,dz,order)       #y component of the curl
	aux2 = deriv_findiff(C,1,dx,order)
	outy = aux1-aux2

	aux1 = deriv_findiff(B,1,dx,order)       #z component of the curl
	aux2 = deriv_findiff(A,2,dy,order)
	outz = aux1-aux2

	return outx, outy, outz     	#return results in user coordinate order


# Calculate divergence
def div_findiff(A,B,C,order=6):
	''' Operator that calculates the divergence of a vector field
	using 6th order finite differences.
	Calling sequence:  DIV = div_findiff(A,B,C)
	Where:
	A, B, and C are 3-dimensional float32 arrays defining a vector field.
	order is the accuracy order, one of (6,4, or 2)
	Resulting DIV is a 3-dimensional float3d array consisting of
	the divergence of the triple (A,B,C).'''
	import vapor
	ext = (vapor.BOUNDS[3]-vapor.BOUNDS[0], vapor.BOUNDS[4]-vapor.BOUNDS[1],vapor.BOUNDS[5]-vapor.BOUNDS[2])
        usrmax = vapor.MapVoxToUser([vapor.BOUNDS[3],vapor.BOUNDS[4],vapor.BOUNDS[5]],vapor.REFINEMENT,vapor.LOD)
        usrmin = vapor.MapVoxToUser([vapor.BOUNDS[0],vapor.BOUNDS[1],vapor.BOUNDS[2]],vapor.REFINEMENT,vapor.LOD)
        dx = (usrmax[2]-usrmin[2])/ext[2]       # grid delta-x in user coord system
        dy = (usrmax[1]-usrmin[1])/ext[1]
        dz = (usrmax[0]-usrmin[0])/ext[0]
# in User coords, A,B,C are x,y,z components
        return deriv_findiff(C,3,dz,order)+deriv_findiff(B,2,dy,order)+deriv_findiff(A,1,dx,order)


def grad_findiff(A,order=6):
	''' Operator that calculates the gradient of a scalar field 
	using 6th order finite differences.
	Calling sequence:  GRD = grad_findiff(A)
	Where:
	A is a float32 array defining a scalar field.
	order is the accuracy order, one of (6,4,or 2)
	Result GRD is a triple of 3 3-dimensional float3d arrays consisting of
	the gradient of A.'''
	import vapor
	ext = (vapor.BOUNDS[3]-vapor.BOUNDS[0], vapor.BOUNDS[4]-vapor.BOUNDS[1],vapor.BOUNDS[5]-vapor.BOUNDS[2]) #note that BOUNDS are in python coord order
	usrmax = vapor.MapVoxToUser([vapor.BOUNDS[3],vapor.BOUNDS[4],vapor.BOUNDS[5]],vapor.REFINEMENT,vapor.LOD)
	usrmin = vapor.MapVoxToUser([vapor.BOUNDS[0],vapor.BOUNDS[1],vapor.BOUNDS[2]],vapor.REFINEMENT,vapor.LOD)
	dx = (usrmax[2]-usrmin[2])/ext[2] #delta in python z coordinate, user x
	dy =  (usrmax[1]-usrmin[1])/ext[1]
	dz =  (usrmax[0]-usrmin[0])/ext[0]# user z

	aux1 = deriv_findiff(A,1,dx,order)       #x component of the gradient (in python system)
	aux2 = deriv_findiff(A,2,dy,order)
	aux3 = deriv_findiff(A,3,dz,order)
	
	return aux1,aux2,aux3 # return in user coordinate (x,y,z) order

# Method that vertically interpolates one 3D variable to a level determined by 
# another variable.  The second variable (PR) is typically pressure.  
# The second variable must decrease
# as a function of z (elevation).  The returned value is a 2D variable having
# values interpolated to the surface defined by PR = val
# Sweep array from bottom to top
def interp3d(A,PR,val):
	import numpy 
	s = numpy.shape(PR)	#size of the input arrays
	ss = [s[1],s[2]] # shape of 2d arrays
	interpVal = numpy.empty(ss,numpy.float32)
	ratio = numpy.zeros(ss,numpy.float32)

	#  the LEVEL value is determine the lowest level where P<=val
	LEVEL = numpy.empty(ss,numpy.int32)
	LEVEL[:,:] = -1 #value where PR<=val has not been found
	for K in range(s[0]):
		#LEVNEED is true if this is first time PR<val.
		LEVNEED = numpy.logical_and(numpy.less(LEVEL,0), numpy.less(PR[K,:,:] , val))
		LEVEL[LEVNEED]=K
		ratio[LEVNEED] = (val-PR[K,LEVNEED])/(PR[K-1,LEVNEED]-PR[K,LEVNEED])
		interpVal[LEVNEED] = ratio[LEVNEED]*A[K,LEVNEED]+(1-ratio[LEVNEED])*A[K-1,LEVNEED] 
		LEVNEED = numpy.greater(LEVEL,0)
	# Set unspecified values to value of A at top of data:
	LEVNEED = numpy.less(LEVEL,0)
	interpVal[LEVNEED] = A[s[0]-1,LEVNEED]
	return interpVal

def vector_rotate(angleRad, latDeg, u, v):
	'''Rotate and scale vectors u,v for integration on
	lon-lat grid.
	Calling sequence: 
	rotfield=vector_rotate(angleRad, latDeg, u,v)
	Where:  
	angleRad: 2D var, rotation from East in radians
	latDeg: 2D var, latitude in degrees
	u,v: 3D vars, x,y components of a vector field
	rotfield is a 2-tuple of 3-dimensional float32 arrays,
	representing rotation of u,v, returned by this operator.
	''' 	
	import numpy 
	import math
	umod = numpy.cos(angleRad)*u + numpy.sin(angleRad)*v
	vmod = -numpy.sin(angleRad)*u + numpy.cos(angleRad)*v
	umod = umod/numpy.cos(latDeg*math.pi/180.)
	return umod,vmod


