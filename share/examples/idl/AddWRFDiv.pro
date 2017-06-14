
;   AddWRFDiv.pro
;
;   Utility to read three variables from a WRF VDF, calculate their divergence 
;   and put it back into the VDF.
;   All three variables must be present at full resolution.
;
;   The .pro files deriv_findiff.pro and wrf_div_findiff.pro 
;   and elev_deriv.pro must be in the
;   directory from which you started idl.
;
;   The vdf file is replaced.  The previous vdf file is saved, with
;   _saved appended to its name, in case of catastrophic failure
;
;   Arguments are:
;   vdffile = file path of the metadata file
;   varx, vary, varz = the 3 variables defining the field 
;       whose divergence is being calculated
;   divVar = the name for the divergence variable being calculated 
;   tsstart = the time step to start with (or the only time step)
;   tsmax = (keyword parameter) the time step to stop with (don't specify
;           if you only want the one time step, tsstart)
;   tsival = (keyword parameter) the interval between time steps (will compute
;            info for tsstart, tsstart + tsival, etc.) (again, don't specify if
;            you only want the one time step, tsstart)
;

PRO AddWRFDiv, vdffile,varx,vary,varz,divvar,tsstart, $
                TSMAX=tsmax,TSIVAL=tsival


;   
;   Variable timestep now functions as a switch and initializer
;
timestep = tsstart
IF ( ~keyword_set(tsmax) ) THEN tsmax=tsstart
IF ( ~keyword_set(tsival) ) THEN tsival=1

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
;

;
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
;   Need to make sure the new var name is not in the list!
isinvariables = 0
FOR I = 0, numvars-2 DO BEGIN
        newvarnames[I] = varnames[I]
        if (varnames[I] EQ divvar) THEN isinvariables = 1
ENDFOR

IF (isinvariables EQ 0) THEN newvarnames[numvars-1] = divvar ELSE newvarnames = varnames

print,'The 3D variable names in the vdf will be: ',newvarnames

;
;   reset the varnames in mfd to the new value:
;   provided not all variables are repeated
;   Note that by default all variable names are 3D
;
if (isinvariables EQ 0) THEN BEGIN
	if (nvarnames2dxy gt 0) THEN BEGIN
		vdf_setvarnames,mfd,[newvarnames,varnames2dxy]
		vdf_setvariables2DXY,mfd,varnames2dxy
	ENDIF ELSE BEGIN
		vdf_setvarnames,mfd,newvarnames
	ENDELSE
ENDIF

reflevel = vdf_getnumtransforms(mfd)

;
;   Begin loop that iterates over time steps
;
REPEAT BEGIN

print, "Working on time step ", timestep

;
;   Create "Buffered Read" objects for each variable to read the data, passing the
;   metadata object handle created by vdf_create() as an argument
;

dfdx = vdc_bufreadcreate(mfd)
dfdy = vdc_bufreadcreate(mfd)
dfdz = vdc_bufreadcreate(mfd)
dfde = vdc_bufreadcreate(mfd)
dfddiv = vdc_bufwritecreate(mfd)

;
;
;   Determine the dimensions of the x-variable at the full transformation
;   level.
;   This is used as the dimension of all the variables
;

dim = vdc_getdim(dfdx, reflevel)

;
;   Create appropriately sized arrays to hold the source and result data
;

srcx = fltarr(dim[0],dim[1],dim[2])
srcy = fltarr(dim[0],dim[1],dim[2])
srcz = fltarr(dim[0],dim[1],dim[2])
srce = fltarr(dim[0],dim[1],dim[2])

divarray = fltarr(dim[0],dim[1],dim[2])

;
;   Prepare to read the indicated time step and variables
;

vdc_openvarread, dfdx, timestep, varx, reflevel
vdc_openvarread, dfdy, timestep, vary, reflevel
vdc_openvarread, dfdz, timestep, varz, reflevel
vdc_openvarread, dfde, timestep, 'ELEVATION', reflevel
vdc_openvarwrite, dfddiv, timestep, divvar, reflevel

;
;   Read the volume one slice at a time
;
slcx = fltarr(dim[0],dim[1])
slcy = fltarr(dim[0],dim[1])
slcz = fltarr(dim[0],dim[1])
slce = fltarr(dim[0],dim[1])

;   Determine the grid spacing

extents = VDF_GETEXTENTS(mfd)
deltax = (extents[3] - extents[0])/FLOAT(dim[0])
deltay = (extents[4] - extents[1])/FLOAT(dim[1])

FOR z = 0, dim[2]-1 DO BEGIN
    vdc_bufreadslice, dfdx, slcx
    ; copy to 3d array
    srcx[*,*,z] = slcx
    vdc_bufreadslice, dfdy, slcy
    ; copy to 3d array
    srcy[*,*,z] = slcy
    vdc_bufreadslice, dfdz, slcz
    ; copy to 3d array
    srcz[*,*,z] = slcz
    vdc_bufreadslice, dfde, slce
    ; copy to 3d array
    srce[*,*,z] = slce

    ;  Report every 100 reads:
    IF ((z MOD 100) EQ 0) THEN print,'reading slice ',z
ENDFOR
vdc_closevar, dfdx
vdc_closevar, dfdy
vdc_closevar, dfdz
vdc_closevar, dfde
vdc_bufreaddestroy, dfdx
vdc_bufreaddestroy, dfdy
vdc_bufreaddestroy, dfdz
vdc_bufreaddestroy, dfde

;  Now perform the divergence on the data
wrf_div_findiff,srcx,srcy,srcz,divarray,deltax,deltay,srce

print,'performed the divergence on ',varx,' ', vary,' ', varz

; write the data one slice at a time
FOR z = 0, dim[2]-1 DO BEGIN
    slc = divarray[*,*,z]
    vdc_bufwriteslice,dfddiv, slc
    ;  Report every 100 writes:
    IF ((z MOD 100) EQ 0) THEN print,'writing x slice ',z
ENDFOR
vdc_closevar, dfddiv
vdc_bufwritedestroy, dfddiv

;
;  First time, replace the vdf file with the new one
;

IF (timestep EQ tsstart AND isinvariables EQ 0) THEN vdf_write,mfd,vdffile

print,'Divergence completed'

timestep += tsival

ENDREP UNTIL ( timestep GT tsmax )

vdf_destroy, mfd

END
