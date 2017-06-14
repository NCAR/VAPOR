;
;   AddWRFCurl.pro
;
;   Utility to read three variables from a WRF VDF, calculate their curl
;   and put them back into the VDF.
;   All three variables and ELEVATION must be present at full resolution.
;
;   The .pro files wrf_curl_findiff.pro and deriv_findiff.pro  
;   and elev_deriv.pro must be in the
;   directory from which you started idl.
;
;   The vdf file is replaced.  The previous vdf file is saved, with
;   _saved appended to its name, in case of catastrophic failure
;
;   Arguments are:
;   vdffile = file path of the metadata file
;   varx, vary, varz = the 3 variables defining the file
;       whose magnitude is being calculated
;   curlx,curly,curlz = the names for the three components
;       of the curl being calculated
;   tsstart = the first (smallest) time step this is applied to
;     keyword args:
;   tsmax = the largest time step this is applied to
;   tsival = the increment between successive time steps  
;   mag = (keyword parameter) name of curl's magnitude (do not specify if you
;         don't want the
;         magnitude of the curl written)
;   onlymag = (keyword parameter) set this to anything if you only want the
;             curl's magnitude and not the actual vector field (note that you
;             must still supply names for curlx, curly, curlz)
;
;   Note: if you want to add the magnitude later, use AddMagVDF
;

;
;

PRO AddWRFCurl, vdffile,varx,vary,varz,curlx,curly,curlz,tsstart, $
		TSMAX=tsmax,TSIVAL=tsival,MAG=mag,ONLYMAG=onlymag

;   Make sure we're actually doing something
IF ( ~keyword_set(mag) && keyword_set(onlymag) ) THEN BEGIN
    print, "Neither curl nor curl's magnitude requested.  Nothing modified."
    RETURN
ENDIF


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
;   Need to make sure these are not in the list!
repeatvariables = 0
print, 'numvars = ',numvars
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

;
;   reset the varnames in mfd to the new value:
;
IF (repeatvariables EQ 0) THEN BEGIN
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

dfdx = vdc_bufreadcreate(mfd)
dfdy = vdc_bufreadcreate(mfd)
dfdz = vdc_bufreadcreate(mfd)
dfde = vdc_bufreadcreate(mfd)

;
;
;   Determine the dimensions of the x-variable at the full transformation
;   level.
;   This is used as the dimension of all the variables
;   Note. vdc_getdim() correctly handles dimension calucation for
;   volumes with non-power-of-two dimensions.
;

dim = vdc_getdim(dfdx, reflevel)

;
;   Create appropriately sized arrays to hold the source and result data
;

srcx = fltarr(dim[0],dim[1],dim[2])
srcy = fltarr(dim[0],dim[1],dim[2])
srcz = fltarr(dim[0],dim[1],dim[2])
srce = fltarr(dim[0],dim[1],dim[2])

dstx = fltarr(dim[0],dim[1],dim[2])
dsty = fltarr(dim[0],dim[1],dim[2])
dstz = fltarr(dim[0],dim[1],dim[2])

;
;   Prepare to read the indicated time step and variables
;

vdc_openvarread, dfdx, timestep, varx, reflevel
vdc_openvarread, dfdy, timestep, vary, reflevel
vdc_openvarread, dfdz, timestep, varz, reflevel
vdc_openvarread, dfde, timestep, 'ELEVATION', reflevel

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
    srcy[*,*,z] = slcy
    vdc_bufreadslice, dfdz, slcz
    srcz[*,*,z] = slcz
    vdc_bufreadslice, dfde, slce
    srce[*,*,z] = slce

    ;  Report every 100 reads:
    IF ((z MOD 100) EQ 0) THEN print,'reading slice ',z
ENDFOR
vdc_closevar, dfdx
vdc_bufreaddestroy, dfdx

vdc_closevar, dfdy
vdc_bufreaddestroy, dfdy

vdc_closevar, dfdz
vdc_bufreaddestroy, dfdz

vdc_closevar, dfde
vdc_bufreaddestroy, dfde
;  Now perform the curl on the data
wrf_curl_findiff,srcx,srcy,srcz,dstx,dsty,dstz,deltax,deltay,srce


;  If requested, write out the components of the curl:
IF (~keyword_set(onlymag) ) THEN BEGIN
    dfdcurlx = vdc_bufwritecreate(mfd)
    vdc_openvarwrite, dfdcurlx, timestep, curlx, reflevel
    dfdcurly = vdc_bufwritecreate(mfd)
    vdc_openvarwrite, dfdcurly, timestep, curly, reflevel
    dfdcurlz = vdc_bufwritecreate(mfd)
    vdc_openvarwrite, dfdcurlz, timestep, curlz, reflevel
    ; write the data one slice at a time
    FOR z = 0, dim[2]-1 DO BEGIN
        slcx = dstx[*,*,z]
        vdc_bufwriteslice,dfdcurlx, slcx
        slcy = dsty[*,*,z]
        vdc_bufwriteslice,dfdcurly, slcy
        slcz = dstz[*,*,z]
        vdc_bufwriteslice,dfdcurlz, slcz
        ;  Report every 100 writes:
        IF ((z MOD 100) EQ 0) THEN print,'writing slice ',z
    ENDFOR
    vdc_closevar, dfdcurlx
    vdc_bufwritedestroy, dfdcurlx
    vdc_closevar, dfdcurly
    vdc_bufwritedestroy, dfdcurly
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
;  Replace the vdf file with the new one, the first time through:
;
 
IF (timestep EQ tsstart AND repeatvariables NE newnum) THEN vdf_write,mfd,vdffile

print,'Curl completed of timestep ', timestep

timestep += tsival

ENDREP UNTIL (timestep GT tsmax)

vdf_destroy, mfd

END
