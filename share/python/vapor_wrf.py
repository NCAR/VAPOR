''' vapor_wrf module includes following WRF-based utilities:
	ETH - equivalent potential temperature
	RH - relative humidity
	SHEAR - horizontal wind shear
	SLP - sea-level pressure (2D)
	TD - dewpoint temperature
	TK - temperature in degrees Kelvin.'''
	
import numpy 
import vapor_utils

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
	U=vapor_utils.StaggeredToUnstaggeredGrid(U,2) 
	V=vapor_utils.StaggeredToUnstaggeredGrid(V,1) 
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
