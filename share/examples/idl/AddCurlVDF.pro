
;   AddCurlVDF.pro
;
;   Utility to read three variables from a VDF, calculate their curl
;   and optionally the curl's magnitude, and put them back into the VDF.
;   All three variables must be present at full resolution.
;
;   The .pro files curl_findiff.pro and deriv_findiff.pro must be in the
;   directory from which you started idl.
;
;   The vdf file is replaced.  The previous vdf file is saved, with
;   _saved appended to its name, in case of catastrophic failure
;
;   Arguments are:
;   vdffile = file path of the metadata file
;   varx, vary, varz = the 3 variables defining the field 
;       whose curl is being calculated
;   curlx,curly,curlz = the names for the three components
;       of the curl being calculated
;   tsstart = the time step to start with (or the only time step)
;   tsmax = (keyword parameter) the time step to stop with (don't specify
;           if you only want the one time step, tsstart)
;   tsival = (keyword parameter) the interval between time steps (will compute
;            info for tsstart, tsstart + tsival, etc.) (again, don't specify if
;            you only want the one time step, tsstart)
;   mag = (keyword parameter) name of curl's magnitude (do not specify if you
;         don't want the
;         magnitude of the curl written)
;   onlymag = (keyword parameter) set this to anything if you only want the 
;             curl's magnitude and not the actual vector field (note that you
;             must still supply names for curlx, curly, curlz)
;
;   Note: if you want to add the magnitude later, use AddMagVDF
;

PRO AddCurlVDF, vdffile,varx,vary,varz,curlx,curly,curlz,tsstart, $
                TSMAX=tsmax,TSIVAL=tsival,MAG=mag,ONLYMAG=onlymag

;   Make sure we're actually doing something
IF ( ~keyword_set(mag) && keyword_set(onlymag) ) THEN BEGIN
    print, "Neither curl nor curl's magnitude requested.  Nothing modified."
    RETURN
ENDIF


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
;   Add the new variable names to the current variable names:
;

;
;   How many variable names?
;
IF ( keyword_set(mag) && ~keyword_set(onlymag) ) THEN newnum = 4 
IF ( keyword_set(mag) && keyword_set(onlymag) ) THEN newnum = 1
IF ( ~keyword_set(mag) ) THEN newnum = 3

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
numvars = newnum + numvarsarray[1]
newvarnames = strarr(numvars)
;   Need to make sure the new var names are not in the list!
repeatvariables = 0
print, 'num 3D vars = ',numvars
FOR I = 0, numvars-newnum-1 DO BEGIN
    newvarnames[I] = varnames[I]
    IF ( ~keyword_set(onlymag) ) THEN BEGIN
        IF (varnames[I] EQ curlx) THEN repeatvariables = 1+repeatvariables
        IF (varnames[I] EQ curly) THEN repeatvariables = 1+repeatvariables
        IF (varnames[I] EQ curlz) THEN repeatvariables = 1+repeatvariables
    ENDIF
        IF ( keyword_set(mag) ) THEN BEGIN
        IF ( varnames [I] EQ mag ) THEN repeatvariables += 1
    ENDIF
ENDFOR

IF (repeatvariables GT 0 AND repeatvariables LT newnum) THEN BEGIN
    print, 'ERROR:  some but not all curl variable names exist already'
    STOP
ENDIF

IF (repeatvariables EQ 0) THEN BEGIN
    IF ( ~keyword_set(onlymag) ) THEN BEGIN
        newvarnames[numvars-newnum] = curlx
        newvarnames[numvars-newnum+1] = curly
        newvarnames[numvars-newnum+2] = curlz
    ENDIF
    IF ( keyword_set(mag) && ~keyword_set(onlymag) ) THEN $
        newvarnames[numvars-newnum+3] = mag
    IF ( keyword_set(onlymag) ) THEN newvarnames[numvars-newnum] = mag
ENDIF

IF (repeatvariables EQ newnum) THEN newvarnames = varnames


;
;   reset the varnames in mfd to the new value:
;   provided not all variables are repeated
;   Note that by default all variable names are 3D
;
if (repeatvariables NE newnum) THEN BEGIN
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

dstx = fltarr(dim[0],dim[1],dim[2])
dsty = fltarr(dim[0],dim[1],dim[2])
dstz = fltarr(dim[0],dim[1],dim[2])

;
;   Prepare to read the indicated time step and variables
;

vdc_openvarread, dfdx, timestep, varx, reflevel

;
;   Read the volume one slice at a time
;
slcx = fltarr(dim[0],dim[1])
slcy = fltarr(dim[0],dim[1])
slcz = fltarr(dim[0],dim[1])

;   Determine the grid spacing

extents = VDF_GETEXTENTS(mfd)
deltax = (extents[3] - extents[0])/FLOAT(dim[0])
deltay = (extents[4] - extents[1])/FLOAT(dim[1])
deltaz = (extents[5] - extents[2])/FLOAT(dim[2])

FOR z = 0, dim[2]-1 DO BEGIN
    vdc_bufreadslice, dfdx, slcx

    ; copy to 3d array
    srcx[*,*,z] = slcx

    ;  Report every 100 reads:
    IF ((z MOD 100) EQ 0) THEN print,'reading x  slice ',z
ENDFOR
vdc_closevar, dfdx
vdc_bufreaddestroy, dfdx
vdc_openvarread, dfdy, timestep, vary, reflevel
FOR z = 0, dim[2]-1 DO BEGIN
    vdc_bufreadslice, dfdy, slcy
    ; copy to 3d array
    srcy[*,*,z] = slcy
    ;  Report every 100 reads:
    IF ((z MOD 100) EQ 0) THEN print,'reading y  slice ',z
ENDFOR

vdc_closevar, dfdy
vdc_bufreaddestroy, dfdy
vdc_openvarread, dfdz, timestep, varz, reflevel
FOR z = 0, dim[2]-1 DO BEGIN
    vdc_bufreadslice, dfdz, slcz
    ; copy to 3d array
    srcz[*,*,z] = slcz
    ;  Report every 100 reads:
    IF ((z MOD 100) EQ 0) THEN print,'reading z  slice ',z
ENDFOR

vdc_closevar, dfdz
vdc_bufreaddestroy, dfdz
;  Now perform the curl on the data
curl_findiff,srcx,srcy,srcz,dstx,dsty,dstz,deltax,deltay,deltaz

print,'performed the curl on ',varx,' ', vary,' ', varz


IF ( ~keyword_set(onlymag) ) THEN BEGIN

    dfdcurlx = vdc_bufwritecreate(mfd)
    vdc_openvarwrite, dfdcurlx, timestep, curlx, reflevel
    ; write the data one slice at a time
    FOR z = 0, dim[2]-1 DO BEGIN
        slcx = dstx[*,*,z]
        vdc_bufwriteslice,dfdcurlx, slcx
        ;  Report every 100 writes:
        IF ((z MOD 100) EQ 0) THEN print,'writing x slice ',z
    ENDFOR
    vdc_closevar, dfdcurlx
    vdc_bufwritedestroy, dfdcurlx

    dfdcurly = vdc_bufwritecreate(mfd)
    vdc_openvarwrite, dfdcurly, timestep, curly, reflevel
    FOR z = 0, dim[2]-1 DO BEGIN
        slcy = dsty[*,*,z]
        vdc_bufwriteslice,dfdcurly, slcy
        ;  Report every 100 writes:
        IF ((z MOD 100) EQ 0) THEN print,'writing y slice ',z
    ENDFOR
    vdc_closevar, dfdcurly
    vdc_bufwritedestroy, dfdcurly

    dfdcurlz = vdc_bufwritecreate(mfd)
    vdc_openvarwrite, dfdcurlz, timestep, curlz, reflevel
    FOR z = 0, dim[2]-1 DO BEGIN
        slcz = dstz[*,*,z]
        vdc_bufwriteslice,dfdcurlz, slcz
        ;  Report every 100 writes:
        IF ((z MOD 100) EQ 0) THEN print,'writing z slice ',z
    ENDFOR
    vdc_closevar, dfdcurlz
    vdc_bufwritedestroy, dfdcurlz

ENDIF

;
; If desired, compute and write the magnitude of the curl
;
IF ( keyword_set(mag) ) THEN BEGIN
    dfdmag = vdc_bufwritecreate(mfd)
    allMag = fltarr(dim[0],dim[1],dim[2])
    allMag = sqrt( dstx^2 + dsty^2 + dstz^2 )
    slcmag = fltarr(dim[0],dim[1])
    vdc_openvarwrite, dfdmag, timestep, mag, reflevel
    FOR z = 0, dim[2]-1 DO BEGIN
        slcmag = allMag[*,*,z]
        vdc_bufwriteslice, dfdmag, slcmag
        ;  Report every 100 writes
        IF ( (z MOD 100) EQ 0) THEN print, "Writing magnitude's z slice ", z
    ENDFOR
    vdc_closevar, dfdmag
    vdc_bufwritedestroy, dfdmag
ENDIF

;
;  First time, replace the vdf file with the new one
;

IF (timestep EQ tsstart AND repeatvariables NE newnum) THEN vdf_write,mfd,vdffile


print,'Curl completed'

timestep += tsival

ENDREP UNTIL ( timestep GT tsmax )

vdf_destroy, mfd

END
