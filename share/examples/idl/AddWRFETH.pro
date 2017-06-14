;
;   AddWRFETH.pro
;
;   Utility to calculate Equivalent Potential Temperature
;   and put it back into the VDF, as the variable named ETH_.
;   The VDC must have TK_, QVAPOR, P, and PB already
;   with the usual meanings in a WRF dataset
;   Formula was provided by Yongsheng Chen
;
;   The vdf file is replaced.  The previous vdf file is saved, with
;   _saved appended to its name, in case of catastrophic failure
;
;   Arguments are:
;   vdffile = file path of the metadata file
;   tsstart = the first (smallest) time step this is applied to
;     keyword args:
;   tsmax = the largest time step this is applied to
;   tsival = the increment between successive time steps  
;

PRO AddWRFETH, vdffile,tsstart, $
		TSMAX=tsmax,TSIVAL=tsival


;
;   Set up timesteps
;
timestep = tsstart
IF (~keyword_set(tsmax)) THEN tsmax = tsstart
IF (~keyword_set(tsival)) THEN tsival = 1


;
;   Start with the current metadata:
;

mfd = vdf_create(vdffile)

;
;   save the current vdf file (in case we screw up)
;

savedvdffile = STRING(vdffile,'_saved')
vdf_write,mfd,savedvdffile

;
;   Add the new variable name to the current variable names:
;   But need to know if it's already there
;   How many variable names?
;

nvarnames3d = 0
if (n_elements(vdf_getvariables3d(mfd)) ne 0) then begin
        varnames = vdf_getvariables3d(mfd)
        nvarnames3d = n_elements(varnames)
endif

nvarnames2dxy = 0
if (n_elements(vdf_getvariables2dxy(mfd)) ne 0) then begin
        varnames2dxy = vdf_getvariables2dxy(mfd)
        nvarnames2dxy = n_elements(varnames2dxy)
endif

numvarsarray = size(varnames)
numvars = 1 + numvarsarray[1]
newvarnames = strarr(numvars)
;   Need to make sure these are not in the list!
repeatvariables = 0

FOR I = 0, numvars-2 DO BEGIN
    newvarnames[I] = varnames[I]
    IF (varnames[I] EQ 'ETH_') THEN repeatvariables = 1+repeatvariables
ENDFOR

IF (repeatvariables EQ 0) THEN newvarnames[numvars-1] = 'ETH_' 

;
;   reset the varnames in mfd to the new value:
;   unless it's already in the vdf
;
if (repeatvariables EQ 0) THEN BEGIN
	if(nvarnames2dxy gt 0) THEN BEGIN 
		vdf_setvarnames,mfd,[newvarnames,varnames2dxy]
		vdf_setvariables2dXY,mfd,varnames2dxy
	ENDIF ELSE BEGIN
		vdf_setvarnames,mfd,newvarnames
	ENDELSE
ENDIF

reflevel = vdf_getnumtransforms(mfd)

REPEAT BEGIN

print, "Working on time step ", timestep

;
;   Create "Buffered Read" objects for each variable to read the data, passing the
;   metadata object handle created by vdf_create() as an argument
;

dfdP = vdc_bufreadcreate(mfd)
dfdPB = vdc_bufreadcreate(mfd)
dfdQV = vdc_bufreadcreate(mfd)
dfdTK = vdc_bufreadcreate(mfd)
dfdETH = vdc_bufwritecreate(mfd)

;
;
;   Determine the dimensions of the P-variable at the full transformation
;   level.
;   This is used as the dimension of all the variables
;

dim = vdc_getdim(dfdP, reflevel)

;
;   Prepare to read the indicated time step and variables
;

vdc_openvarread, dfdP, timestep, 'P', reflevel
vdc_openvarread, dfdPB, timestep, 'PB', reflevel
vdc_openvarread, dfdQV, timestep, 'QVAPOR', reflevel
vdc_openvarread, dfdTK, timestep, 'TK_', reflevel
vdc_openvarwrite, dfdETH, timestep, 'ETH_', reflevel


;
;   Read the volume one slice at a time
;

slcP = fltarr(dim[0],dim[1])
slcPB = fltarr(dim[0],dim[1])
slcQV = fltarr(dim[0],dim[1])
slcTK = fltarr(dim[0],dim[1])

;   Determine the grid spacing

extents = VDF_GETEXTENTS(mfd)

; establish some constants:
eps = 0.622
gamm = 287.04/1004.
rgasmd=0.608
cpmd=0.887
gammamd=rgasmd-cpmd
tlclc1=2840.
tlclc2=3.5
tlclc3=4.805
tlclc4=55.
thtecon1=3376. 
thtecon2=2.54
thtecon3=.81

; process the data one slice at a time:

maxslcETH = 0.0
minslcETH = 10000000.
FOR z = 0, dim[2]-1 DO BEGIN
    vdc_bufreadslice, dfdP, slcP
    vdc_bufreadslice, dfdPB, slcPB
    vdc_bufreadslice, dfdQV, slcQV
    vdc_bufreadslice, dfdTK, slcTK
    PR = (slcP + slcPB)*0.01
    
    Q = slcQV > 1.e-15
    E = Q*PR/(eps+Q)
     
    tlcl = tlclc1/(alog(slcTK^tlclc2/E)-tlclc3)+tlclc4
    slcETH = slcTK*(1000./PR)^(gamm*(1.+gammamd*Q))*EXP((thtecon1/tlcl-thtecon2)*Q*(1.+thtecon3*Q))
    ;  Report every 20 slices
    IF ((z MOD 20) EQ 0) THEN print,'reading slice ',z
    vdc_bufwriteslice, dfdETH, slcETH
ENDFOR
vdc_closevar, dfdP
vdc_bufreaddestroy, dfdP
vdc_closevar, dfdPB
vdc_bufreaddestroy, dfdPB
vdc_closevar, dfdQV
vdc_bufreaddestroy, dfdQV
vdc_closevar, dfdTK
vdc_bufreaddestroy, dfdTK
vdc_closevar, dfdETH
vdc_bufwritedestroy, dfdETH

;
;  Replace the vdf file with the new one, the first time through:
;
 
IF (timestep EQ tsstart AND repeatvariables EQ 0) THEN vdf_write,mfd,vdffile

print,'ETH_ completed of timestep ', timestep

timestep += tsival

ENDREP UNTIL (timestep GT tsmax)

vdf_destroy, mfd

END
