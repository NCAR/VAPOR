''' vapor_wrf module includes following WRF-based utilities:
	CTT - cloud-top temperature (2D)
	DBZ - radar reflectivity
	DBZ_MAX - max radar reflectivity over vertical column
	ETH - equivalent potential temperature
	RH - relative humidity
	PV - potential vorticity
	SHEAR - horizontal wind shear
	SLP - sea-level pressure (2D)
	TD - dewpoint temperature
	TK - temperature in degrees Kelvin.
	wrf_deriv_findiff - 6th order finite-difference derivative 	wrf_curl_findiff - finite-difference curl
	wrf_grad_findiff - finite-difference gradient
	wrf_div_findiff - finite-difference divergence'''
	
import numpy 
import vapor_utils
import vapor
def CTT(P,PB,T,QCLOUD,QICE):
	'''Calculate cloud-top temperature using WRF variables.
	Calling sequence: VAL=CTT(P,PB,T,QCLOUD,QICE)
	Where P,PB,T,QCLOUD,QICE are 3D WRF variables.
	(Replace QICE by 0 if not in data)
	Result VAL is a 2D variable.'''
#Copied (and adapted) from RIP4 fortran code cttcalc.f
#constants:
#iice should be 1 if QSNOW and QICE is present
	iice = (vapor.VariableExists(vapor.TIMESTEP,"QSNOW") and vapor.VariableExists(vapor.TIMESTEP,"QICE"))
	grav=9.81
	celkel=273.15 #celsius to kelvin difference
	c = 2.0/7.0
	abscoefi = 0.272
	abscoef = 0.145
#calculate TK:
	pf = P+PB
	tmk = (T+300.0)*numpy.power(pf*.00001,c)
	s = numpy.shape(P)	#size of the input array
	ss = [s[1],s[2]] # shape of 2d arrays
#initialize with value at bottom:
	WRF_CTT = tmk[0,:,:]-celkel
	opdepthd = numpy.zeros(ss,numpy.float32) #next value
	opdepthu = numpy.zeros(ss,numpy.float32) #previous value
#Sweep array from top to bottom, accumulating optical depth
	if (iice == 1):
		for k in range(s[0]-2,-1,-1): #start at top-1, end at bottom (k=0) accumulate opacity
			dp =100.*(pf[k,:,:]-pf[k+1,:,:]) 
			opdepthd=opdepthu+(abscoef*QCLOUD[k,:,:]+abscoefi*QICE[k,:,:])*dp/grav
			logArray= numpy.logical_and(opdepthd > 1.0,opdepthu <= 1.0)
		#identify level when opac=1 is reached
			WRF_CTT = numpy.where(logArray,tmk[k,:,:]-celkel,WRF_CTT[:,:])
			opdepthu = opdepthd
	else:
		for k in range(s[0]-2,-1,-1): #start at top-1, end at bottom (k=0) accumulate opacity
			dp =100.*(pf[k,:,:]-pf[k+1,:,:]) 
			logArray1 = tmk[k,:,:]<celkel
			opdepthd= numpy.where(logArray1,opdepthu+abscoefi*QCLOUD[k,:,:]*dp/grav,opdepthu+abscoef*QCLOUD[k,:,:]*dp/grav)
			logArray= numpy.logical_and(opdepthd > 1.0,opdepthu <= 1.0)
			#identify level when opac=1 is reached
			WRF_CTT = numpy.where(logArray,tmk[k,:,:]-celkel,WRF_CTT[:,:])
			opdepthu = opdepthd
	return WRF_CTT

def DBZ(P,PB,QRAIN,QGRAUP,QSNOW,T,QVAPOR,iliqskin=0,ivarint=0):
	''' Calculates 3D radar reflectivity based on WRF variables.
	Calling sequence: 
	VAL = DBZ(P,PB,QRAIN,QGRAUP,QSNOW,T,QVAPOR,iliqskin=0,ivarint=0)
	Where P,PB,QRAIN,QGRAUP,QSNOW,T,and QVAPOR are WRF 3D variables.
	Optional arguments iliqskin and ivarint default to 0
	if iliqskin=1, then frozen particles above freezing are
	assumed to scatter as a liquid particle.  If ivarint = 1 then
	intercept parameters are calculated based on Thompson, Rasmussen
	and Manning, as described in 2004 Monthly Weather Review.
	Result VAL is 3D variable on same grid as WRF variables.
	If QGRAUP or QSNOW are not available then replace them by 0.'''
#Copied from NCL/Fortran source code wrf_user_dbz.f
#Based on work by Mark Stoellinga, U. of Washington
#parameters ivarint and iliqskin defined as in original code, either 0 or 1.
#set to 0 by default.
#If iliqskin=1, frozen particles above freezing are assumed to scatter as a liquid particle
#If ivarint=0, the intercept parameters are assumed constant with values of 
# 8*10^6, 2*10^7, and 4*10^6, for rain, snow, and graupel respectively.
#If ivarint=1, intercept parameters are used as calculated in Thompson, Rasmussen, and Manning,
#2004 Monthly Weather Review, Vol 132, No. 2, pp.519-542
	c = 2.0/7.0
	R1=1.e-15
	RON = 8.e6
	RON2 = 1.e10
	SON = 2.e7
	GON = 5.37
	RON_MIN = 8.e6
	RON_QR0 = 0.0001
	RON_DELQR0 = 0.25*RON_QR0
	RON_CONST1R = (RON2-RON_MIN)*0.5
	RON_CONST2R = (RON2+RON_MIN)*0.5
	RN0_R = 8.e6
	RN0_S = 2.e7
	RN0_G = 4.e6
	GAMMA_SEVEN =720.
	RHOWAT = 1000.
	RHO_R = RHOWAT
	RHO_S = 100.
	RHO_G = 400.
	ALPHA = 0.224
	CELKEL = 273.15
	pi = 3.141592653589793
	RD = 287.04
	FACTOR_R = GAMMA_SEVEN*1.e18*numpy.power((1./(pi*RHO_R)),1.75)
	FACTOR_S = GAMMA_SEVEN*1.e18*numpy.power((1./(pi*RHO_S)),1.75)*numpy.power((RHO_S/RHOWAT),2.*ALPHA)
	FACTOR_G = GAMMA_SEVEN*1.e18*numpy.power((1./(pi*RHO_G)),1.75)*numpy.power((RHO_G/RHOWAT),2.*ALPHA)
#calculate Temp. in Kelvin
	PRESS = P+PB
	TK = (T+300.)*numpy.power(PRESS*.00001,c)
#Force Q arrays to be nonnegative:
	QVAPOR = numpy.maximum(QVAPOR,0.0)
	QSNOW = numpy.maximum(QSNOW,0.0)
	QRAIN = numpy.maximum(QRAIN,0.0)
	if (vapor.VariableExists(vapor.TIMESTEP,"QGRAUP")):
		QGRAUP = numpy.maximum(QGRAUP,0.0)
	else:	
#Avoid divide by zero if QGRAUP is not present:
		QGRAUP = 1.e-30 
	TestT = numpy.less(TK,CELKEL)
	QSNOW = numpy.where(TestT,QRAIN, QSNOW)
	QRAIN = numpy.where(TestT,0.0, QRAIN)
	VIRTUAL_T = TK*(0.622 + QVAPOR)/(0.622*(1.+QVAPOR))
	RHOAIR = PRESS/(RD*VIRTUAL_T)
	FACTORB_S = FACTOR_S
	FACTORB_G = FACTOR_G
	if (iliqskin == 1):
		FACTORB_S = numpy.float32(numpy.where(TestT,FACTOR_S,FACTOR_S/ALPHA))
		FACTORB_G = numpy.float32(numpy.where(TestT,FACTOR_G, FACTOR_G/ALPHA))
	if (ivarint == 1):
		TEMP_C = numpy.minimum(-.001, TK-CELKEL)
		SONV = numpy.minimum(2.e8,2.e6*numpy.exp(-.12*TEMP_C))
		TestG = numpy.less(R1, QGRAUP)
		GONV = numpy.where(TestG,2.38*numpy.power(pi*RHO_G/(RHOAIR*QGRAUP),0.92),GON)
		GONV = numpy.where(TestG,numpy.maximum(1.e4,numpy.minimum(GONV,GON)),GONV)
		GONV = RN0_G
		RONV = numpy.where(numpy.greater(QRAIN,R1),RON_CONST1R*numpy.tanh((RON_QR0-QRAIN)/RON_DELQR0) + RON_CONST2R,RON2)
	else:
		GONV = RN0_G
		RONV = RN0_R
		SONV = RN0_S
	Z_E = FACTOR_R*numpy.power(RHOAIR*QRAIN,1.75)/numpy.power(RONV,0.75) 
	Z_E = Z_E+FACTORB_S*numpy.power(RHOAIR*QSNOW,1.75)/numpy.power(SONV,0.75)
	Z_E = Z_E+FACTORB_G*numpy.power(RHOAIR*QGRAUP,1.75)/numpy.power(GONV,0.75)
	Z_E = numpy.maximum(Z_E, 0.001)

	WRF_DBZ = 10.0*numpy.log10(Z_E)
	return WRF_DBZ

def DBZ_MAX(P,PB,QRAIN,QGRAUP,QSNOW,T,QVAPOR,iliqskin=0,ivarint=0):
	''' Calculates 2D radar reflectivity based on WRF variables.
	Calling sequence: VAL = DBZ_MAX(P,PB,QRAIN,QGRAUP,QSNOW,T,QVAPOR)
	Where P,PB,QRAIN,QGRAUP,QSNOW,T,and QVAPOR are WRF 3D variables.
	Optional arguments iliqskin and ivarint default to 0, as
	described in help(DBZ)
	Result VAL is 2D variable on 2D grid of WRF variables.
	VAL is the maximum over a vertical column of DBZ.
	If QGRAUP or QSNOW are not available then replace them by 0.'''
	WRF_DBZ = DBZ(P,PB,QRAIN,QGRAUP,QSNOW,T,QVAPOR)
	WRF_DBZ_MAX = numpy.amax(WRF_DBZ,axis=0)
	return WRF_DBZ_MAX


def ETH(P,PB,T,QVAPOR):
	''' Program to calculate equivalent potential temperature using WRF
	variables P, PB, T, QVAPOR.
	Calling sequence:  WRF_ETH = ETH(P,PB,T,QVAPOR)
	Result WRF_ETH is a 3D variable on same grid as P,PB,T,QVAPOR.'''
#Copied from NCL/Fortran source code DEQTHECALC in eqthecalc.f
	c = 2.0/7.0
	EPS = 0.622
	GAMMA = 287.04/1004.0
	GAMMAMD = 0.608 -0.887
	TLCLC1 = 2840.0
	TLCLC2 = 3.5
	TLCLC3 = 4.805
	TLCLC4 = 55.0
	THTECON1 = 3376.0
	THTECON2 = 2.54
	THTECON3 = 0.81
	TH = T+300.0
#calculate Temp. in Kelvin
	PRESS = 0.01*(P+PB)
	TK = TH*numpy.power(PRESS*.001,c)
	Q = numpy.maximum(QVAPOR, 1.e-15)
	E = Q*PRESS/(EPS+Q)
	TLCL = TLCLC4+ TLCLC1/(numpy.log(numpy.power(TK,TLCLC2)/E)-TLCLC3)
	EXPNT = (THTECON1/TLCL - THTECON2)*Q*(1.0+THTECON3*Q)
	WRF_ETH = TK*numpy.power(1000.0/PRESS,GAMMA*(1.0+GAMMAMD*Q))*numpy.exp(EXPNT)
	return WRF_ETH

def PV(T,P,PB,U,V,ELEV,F):
	''' Routine that calculates potential vorticity using WRF variables.
	Calling sequence: WRF_PV = PV(T,P,PB,U,V,ELEV,F)
	Where T,P,PB,U,V are WRF 3D variables, F is WRF 2D variable.
	ELEV is the VAPOR ELEVATION variable (PH+PHB)/g
	Result is 3D variable WRF_PV.'''
# Based on wrf_pvo.f in NCL WRF library
	from vapor_utils import deriv_var_findiff
	PR = 0.01*(P+PB)
	DTHDP = deriv_var_findiff(T,PR,3)
	DTHDX = wrf_deriv_findiff(T,ELEV,1)
	DTHDY = wrf_deriv_findiff(T,ELEV,2)
	DUDP = deriv_var_findiff(U,PR,3)
	DVDP = deriv_var_findiff(V,PR,3)
	DUDY = wrf_deriv_findiff(U,ELEV,2)
	DVDX = wrf_deriv_findiff(V,ELEV,1)
	AVORT = DVDX - DUDY + F
	WRF_PVO = numpy.float32(-9.81*(DTHDP*AVORT-DVDP*DTHDX+DUDP*DTHDY)*10000.)
	return WRF_PVO

def RH(P,PB,T,QVAPOR):
	''' Calculation of relative humidity.
	Calling sequence WRF_RH = RH(P,PB,T,QVAPOR),
	where P,PB,T,QVAPOR are standard WRF 3D variables,
	result WRF_RH is 3D variable on same grid as inputs.'''
#Formula is from wrf_user.f
	c = 2.0/7.0
	SVP1 = 0.6112
	SVP2 = 17.67
	SVPT0 = 273.15
	SVP3 = 29.65
	EP_3 = 0.622
	TH = T+300.0
	PRESS = P+PB
	TK = TH*numpy.power(PRESS*.00001,c)
	ES = 10*SVP1*numpy.exp(SVP2*(TK-SVPT0)/(TK-SVP3))
	QVS = EP_3*ES/(0.01*PRESS - (1.-EP_3)*ES)
	WRF_RH = 100.0*numpy.maximum(numpy.minimum(QVAPOR/QVS,1.0),0)
	return WRF_RH


def SHEAR(U,V,P,PB,level1=200.,level2=850.):
	'''Program calculates horizontal wind shear
	Calling sequence: SHR = SHEAR(U,V,P,PB,level1,level2)
	where U and V are 3D wind velocity components, and
	result SHR is 3D wind shear.
	Shear is defined as the RMS difference between the horizontal 
	velocity interpolated to the specified pressure levels,
 	level1 and level2 (in millibars) which default to 200 and 850.'''
	from numpy import sqrt
	PR = 0.01*(P+PB) 
	uinterp1 = vapor_utils.interp3d(U,PR,level1)
	uinterp2 = vapor_utils.interp3d(U,PR,level2)
	vinterp1 = vapor_utils.interp3d(V,PR,level1)
	vinterp2 = vapor_utils.interp3d(V,PR,level2)
	result = (uinterp1-uinterp2)*(uinterp1-uinterp2)+(vinterp1-vinterp2)*(vinterp1-vinterp2)
	result = sqrt(result)
	return result 

def SLP(P,PB,T,QVAPOR,ELEVATION):
	'''Calculation of Sea-level pressure.
	Calling sequence:  WRF_SLP = SLP(P,PB,T,QVAPOR,ELEVATION)
	where P,PB,T,QVAPOR are WRF 3D variables and ELEVATION is 
	the VAPOR variable indicating the elevation in meters above sea level.
	Result is a 2D variable with same horizonal extents as input variables.''' 
#Copied (and adapted) from NCL fortran source code wrf_user.f
#constants:
	R=287.04
	G=9.81
	GAMMA=0.0065
	TC=273.16+17.05
	PCONST=10000
	c = 2.0/7.0
#calculate TK:
	TH = T+300.0
	PR = P+PB
	TK = (T+300.0)*numpy.power(PR*.00001,c)
#Find least z that is PCONST Pa above the surface
#Sweep array from bottom to top
	s = numpy.shape(P)	#size of the input array
	ss = [s[1],s[2]] # shape of 2d arrays
	WRF_SLP = numpy.empty(ss,numpy.float32)
	LEVEL = numpy.empty(ss,numpy.int32)
	# Ridiculous MM5 test:
	RIDTEST = numpy.empty(ss,numpy.int32)
	PLO = numpy.empty(ss, numpy.float32)
	ZLO = numpy.empty(ss,numpy.float32)
	TLO = numpy.empty(ss,numpy.float32)
	PHI = numpy.empty(ss,numpy.float32)
	ZHI = numpy.empty(ss,numpy.float32)
	THI = numpy.empty(ss,numpy.float32)
	LEVEL[:,:] = -1
	for K in range(s[0]):
		KHI = numpy.minimum(K+1, s[0]-1)
		LEVNEED = numpy.logical_and(numpy.less(LEVEL,0), numpy.less(PR[K,:,:] , PR[0,:,:] - PCONST))
		LEVEL[LEVNEED]=K
		PLO=numpy.where(LEVNEED,PR[K,:,:],PLO[:,:])
		TLO=numpy.where(LEVNEED,TK[K,:,:]*(1.+0.608*QVAPOR[K,:,:]), TLO[:,:])
		ZLO=numpy.where(LEVNEED,ELEVATION[K,:,:],ZLO[:,:])
		PHI=numpy.where(LEVNEED,PR[KHI,:,:],PHI[:,:])
		THI=numpy.where(LEVNEED,TK[KHI,:,:]*(1.+0.608*QVAPOR[KHI,:,:]), THI[:,:])
		ZHI=numpy.where(LEVNEED,ELEVATION[KHI,:,:],ZHI[:,:])
	P_AT_PCONST = PR[0,:,:]-PCONST
	T_AT_PCONST = THI - (THI-TLO)*numpy.log(P_AT_PCONST/PHI)*numpy.log(PLO/PHI)
	Z_AT_PCONST = ZHI - (ZHI-ZLO)*numpy.log(P_AT_PCONST/PHI)*numpy.log(PLO/PHI)
	T_SURF = T_AT_PCONST*numpy.power((PR[0,:,:]/P_AT_PCONST),(GAMMA*R/G))
	T_SEA_LEVEL = T_AT_PCONST + GAMMA*Z_AT_PCONST
	RIDTEST = numpy.logical_and(T_SURF <= TC, T_SEA_LEVEL >= TC)
	T_SEA_LEVEL = numpy.where(RIDTEST, TC, TC - .005*(T_SURF -TC)**2)
	Z_HALF_LOWEST=ELEVATION[0,:,:]
	WRF_SLP = 0.01*(PR[0,:,:]*numpy.exp(2.*G*Z_HALF_LOWEST/(R*(T_SEA_LEVEL+T_SURF))))
	return WRF_SLP

def TD(P,PB,QVAPOR):
	''' Calculation of dewpoint temperature based on WRF variables.
	Calling sequence: WRFTD = TD(P,PB,QVAPOR)
	where P,PB,QVAPOR are WRF 3D variables, and result WRFTD 
	is a 3D variable on the same grid.'''
#Let PR = 0.1*(P+PB) (pressure in hPa)
#and QV = MAX(QVAPOR,0)
#Where TDC = QV*PR/(0.622+QV)
# TDC = MAX(TDC,0.001)
#Formula is (243.5*log(TDC) - 440.8)/(19.48-log(TDC))
	QV = numpy.maximum(QVAPOR,0.0)
	TDC = 0.01*QV*(P+PB)/(0.622+QV)
	TDC = numpy.maximum(TDC,0.001)
	WRF_TD =(243.5*numpy.log(TDC) - 440.8)/(19.48 - numpy.log(TDC))
	return WRF_TD


def TK(P,PB,T):
	''' Calculation of temperature in degrees kelvin using WRF variables.
	Calling sequence: TMP = TK(P,PB,T)
	Where P,PB,T are WRF 3D variables, result TMP is a 3D variable
	indicating the temperature in degrees Kelvin.'''
#Formula is (T+300)*((P+PB)*10**(-5))**c, 
#Where c is 287/(7*287*.5) = 2/7
	c = 2.0/7.0
	TH = T+300.0
	WRF_TK = TH*numpy.power((P+PB)*.00001,c)
	return WRF_TK

def wrf_deriv_findiff(A,ELEV,dir):
	''' Operator that calculates the derivative of a WRF (or layered) variable A
	in the direction dir (user coordinates; in Python the direction is reversed) 
	ELEV is the ELEVATION variable. '''
	import vapor
	import vapor_utils
	if (dir == 3):
		return vapor_utils.deriv_var_findiff(A,ELEV,3)

	ext = (vapor.BOUNDS[3]-vapor.BOUNDS[0], vapor.BOUNDS[4]-vapor.BOUNDS[1],vapor.BOUNDS[5]-vapor.BOUNDS[2])
	usrmax = vapor.MapVoxToUser([vapor.BOUNDS[3],vapor.BOUNDS[4],vapor.BOUNDS[5]],vapor.REFINEMENT,vapor.LOD)
	usrmin = vapor.MapVoxToUser([vapor.BOUNDS[0],vapor.BOUNDS[1],vapor.BOUNDS[2]],vapor.REFINEMENT,vapor.LOD)
	dx = (usrmax[2]-usrmin[2])/ext[2]
	dy =  (usrmax[1]-usrmin[1])/ext[1]
	
	if (dir == 1):
		dadx_eta = vapor_utils.deriv_findiff(A,1,dx)
		dadz = vapor_utils.deriv_var_findiff(A,ELEV,3)
		dzdx_eta = vapor_utils.deriv_findiff(ELEV,1,dx)
		ans = dadx_eta -dadz*dzdx_eta
		return ans

	if (dir == 2):
	#	dA/dy is (dA/dy)_eta - (dA/dz)*(dZ/dy)_eta
		dady_eta = vapor_utils.deriv_findiff(A,2,dy)
		dadz = vapor_utils.deriv_var_findiff(A,ELEV,3)
		dzdy_eta = vapor_utils.deriv_findiff(ELEV,2,dy)
		ans = dady_eta - dadz*dzdy_eta
		return ans
	
def wrf_grad_findiff(A,ELEV):
	''' Operator that calculates the gradient of a scalar field 
	using 6th order finite differences on a WRF (layered) grid
	Calling sequence:  GRD = grad_findiff(A,ELEV)
	Where:
	A is a float32 array defining a scalar field.
	ELEV is the corresponding ELEVATION array
	Result GRD is a triple of 3 3-dimensional float3d arrays consisting of
	the gradient of A.'''
	
	aux1 = wrf_deriv_findiff(A,ELEV,3)   #x component of gradient (in python coords)
	aux2 = wrf_deriv_findiff(A,ELEV,2)
	aux3 = wrf_deriv_findiff(A,ELEV,1)
	return aux1,aux2,aux3 # return in user coordinate (x,y,z) order

# Calculate divergence for WRF (layered) grid
def wrf_div_findiff(A,B,C,ELEV):
	''' Operator that calculates the divergence of a vector field
	using 6th order finite differences.
	Calling sequence:  DIV = wrf_div_findiff(A,B,C,ELEV)
	Where:
	A, B, and C are 3-dimensional float32 arrays defining a vector field.
	A is the x-component, B is y, C is z (in user coordinates)
	ELEV is the ELEVATION variable
	Resulting DIV is a 3-dimensional float3d array consisting of
	the divergence of the triple (A,B,C).'''

	return wrf_deriv_findiff(A,ELEV,1)+wrf_deriv_findiff(B,ELEV,2)+wrf_deriv_findiff(C,ELEV,3)



def wrf_curl_findiff(A,B,C,ELEV):
	''' Operator that calculates the curl of a vector field 
	using 6th order finite differences, on a layered (e.g. WRF) grid.
	Calling sequence:  curlfield = wrf_curl_findiff(A,B,C,ELEV)
	Where:
	A,B,C are three 3-dimensional float32 arrays that define a
	vector field on a layered grid
	ELEV is the ELEVATION variable for the layered grid.  
	curlfield is a 3-tuple of 3-dimensional float32 arrays that is 
	returned by this operator.'''
	import vapor
	import vapor_utils	
	ext = (vapor.BOUNDS[3]-vapor.BOUNDS[0], vapor.BOUNDS[4]-vapor.BOUNDS[1],vapor.BOUNDS[5]-vapor.BOUNDS[2])
	usrmax = vapor.MapVoxToUser([vapor.BOUNDS[3],vapor.BOUNDS[4],vapor.BOUNDS[5]],vapor.REFINEMENT,vapor.LOD)
	usrmin = vapor.MapVoxToUser([vapor.BOUNDS[0],vapor.BOUNDS[1],vapor.BOUNDS[2]],vapor.REFINEMENT,vapor.LOD)
	dx =  (usrmax[2]-usrmin[2])/ext[2]	#user x coordinate, python z coordinate
	dy =  (usrmax[1]-usrmin[1])/ext[1]
	dz =  (usrmax[0]-usrmin[0])/ext[0]
	
#	x component is dC/dy - dB/dz [y,z are user coordinates]
#	dC/dy is calc by (dC/dy)_eta - (dC/dz)*(dZ/dy)_eta
#	where _eta indicates the derivative calculated in the WRF grid coordinates
#	as is performed by deriv_findiff.pro;
#	Z is ELEVATION and dC/dz is deriv wrt Elevation
#	dB/dz is deriv wrt ELEV

	dcdy_eta = vapor_utils.deriv_findiff(C, 2, dy)
	dcdz = vapor_utils.deriv_var_findiff(C,ELEV,3)
	dzdy_eta = vapor_utils.deriv_findiff(ELEV,2,dy)
	dbdz = vapor_utils.deriv_var_findiff(B,ELEV,3)

	outx = dcdy_eta - dcdz*dzdy_eta -dbdz

#	y component is dA/dz - dC/dx
#	dC/dx is (dC/dx)_eta -(dC/dz)*(dZ/dx)_eta)
	
	dadz = vapor_utils.deriv_var_findiff(A,ELEV,3)
	dcdx_eta = vapor_utils.deriv_findiff(C,1,dx)
	dzdx_eta = vapor_utils.deriv_findiff(ELEV,1,dx)

	outy = dadz - dcdx_eta + dcdz*dzdx_eta

#	z component is dB/dx - dA/dy
#	dB/dx is (dB/dx)_eta - (dB/dz)*(dZ/dx)_eta
#	dA/dy is (dA/dy)_eta - (dA/dz)*(dZ/dy)_eta
	
	dbdx_eta = vapor_utils.deriv_findiff(B,1,dx)
	dady_eta = vapor_utils.deriv_findiff(A,2,dy)

	outz = dbdx_eta - dbdz*dzdx_eta -dady_eta + dadz*dzdy_eta
	
	return outx, outy, outz     	#return results in user coordinate order


