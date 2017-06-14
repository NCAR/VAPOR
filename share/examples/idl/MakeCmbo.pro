;
;   MakeCombo.pro
;
;   Utility to calculate a combination of 3D variables in a vdf,
;   and put it back into the VDF.
;   i.e. calculate A*U/W+B*V/T+C/S, where U, V, W, T, and S are 3D variables and
;   A, B, and C are constants.  
;   All variables involved must be present at full resolution,
;
;   The vdf file is replaced.  The previous vdf file is saved, with
;   _saved appended to its name, in case of catastrophic failure
;
;   Arguments are:
;   vdffile = file path of the metadata file
;   uVar, vVar, wVar, tVar, sVar = (keyword parameters) the 5 3D variable names.
;       To add a constant, omit S and set C to your constant
;   resVar = name for the combination
;   A,B,C = coefficients
;   tsstart = first time step for which to calculate the combination.  If tsmax
;       and tsival are not specified, then only tsstart is computed
;   tsmax = (keyword parameter) last time step for which to compute combination
;   tsival = (keyword parameter) interval between desired time steps
;

PRO MakeCmbo, vdffile,U=uVar,V=vVar,W=wVar,T=tVar,S=sVar,A,B,C, $
              resVar,tsstart,TSMAX=tsmax,TSIVAL=tsival

;
;   Define some logical variables for conciseness
;
gotU = keyword_set(uVar)
gotW = keyword_set(wVar)
gotV = keyword_set(vVar)
gotT = keyword_set(tVar)
gotS = keyword_set(sVar)

;
;   Make sure we're using at least one variable
;
IF ( ~( gotU || gotW || gotV || gotT || gotS ) ) THEN BEGIN
    print, "No variables added and no files modified"
    return
ENDIF

;
;   What to do if timestep keywords aren't defined
;
IF ( ~keyword_set(tsmax) ) THEN tsmax = tsstart
IF ( ~keyword_set(tsival) ) THEN tsival = 1

;
;   Initialize timestep
;
timestep = tsstart

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
;   Add the new variable name to the current variable names, if it's
;   not already there
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
;   Need to make sure this is not in the list!
isinvariables = 0
FOR I = 0, numvars-2 DO BEGIN
    newvarnames[I] = varnames[I]
    if (varnames[I] EQ resVar) THEN isinvariables = 1
ENDFOR

IF (isinvariables EQ 0) THEN newvarnames[numvars-1] = resVar ELSE newvarnames = varnames

print,'The 3D variable names in the vdf will be: ',newvarnames

;
;   reset the varnames in mfd to the new value:
;
if (isinvariables EQ 0) THEN BEGIN
	if(nvarnames2dxy gt 0) THEN BEGIN
		vdf_setvarnames,mfd,[newvarnames,varnames2dxy]
		vdf_setvariables2dxy,mfd,varnames2dxy
	ENDIF ELSE BEGIN
		vdf_setvarnames,mfd,newvarnames
	ENDELSE
ENDIF

reflevel = vdf_getnumtransforms(mfd)
numtimesteps = vdf_getnumtimesteps(mfd)

;
;   Begin loop that iterates over time steps
;
REPEAT BEGIN

print, "Working on time step ", timestep

;
;   Create "Buffered Read" objects for each variable to read the data, passing the
;   metadata object handle created by vdf_create() as an argument
;

IF ( gotU ) then dfdu = vdc_bufreadcreate(mfd)
IF ( gotW ) then dfdw = vdc_bufreadcreate(mfd)
IF ( gotV ) then dfdv = vdc_bufreadcreate(mfd)
IF ( gotT ) then dfdt = vdc_bufreadcreate(mfd)
IF ( gotS ) then dfds = vdc_bufreadcreate(mfd)

dfdres = vdc_bufwritecreate(mfd)


;
;
;   Determine the dimensions of one of the variables at the full transformation
;   level.
;   This is used as the dimension of all the variables
;   Note. vdc_getdim() correctly handles dimension calucation for
;   volumes with non-power-of-two dimensions.
;
IF ( gotU ) THEN $
    dim = vdc_getdim(dfdu, reflevel) ELSE $
IF ( gotW ) THEN $
    dim = vdc_getdim(dfdw, reflevel) ELSE $
IF ( gotV ) THEN $
    dim = vdc_getdim(dfdv, reflevel) ELSE $
IF ( gotT ) THEN $
    dim = vdc_getdim(dfdt, reflevel) ELSE $
IF ( gotS ) THEN $
    dim = vdc_getdim(dfds, reflevel) ELSE $
    BEGIN
    print, "No variables present, early error detection failed"
    return
    ENDELSE

;
;   Create appropriately sized arrays to hold the volume slices:
;
IF ( gotU ) THEN sliceu = fltarr(dim[0], dim[1])
IF ( gotW ) THEN slicew = fltarr(dim[0], dim[1])
IF ( gotV ) THEN slicev = fltarr(dim[0], dim[1])
IF ( gotT ) THEN slicet = fltarr(dim[0], dim[1])
IF ( gotS ) THEN slices = fltarr(dim[0], dim[1])

sliceres = fltarr(dim[0], dim[1])



;
;   Prepare to read and write the indicated time step and variables
;
IF ( gotU ) THEN vdc_openvarread, dfdu, timestep, uVar, reflevel
IF ( gotW ) THEN vdc_openvarread, dfdw, timestep, wVar, reflevel
IF ( gotV ) THEN vdc_openvarread, dfdv, timestep, vVar, reflevel
IF ( gotT ) THEN vdc_openvarread, dfdt, timestep, tVar, reflevel
IF ( gotS ) THEN vdc_openvarread, dfds, timestep, sVar, reflevel
vdc_openvarwrite,dfdres, timestep, resVar, reflevel

;
;   Read the volume one slice at a time
;

FOR z = 0, dim[2]-1 DO BEGIN
    IF ( gotU ) THEN BEGIN
        vdc_bufreadslice, dfdu, sliceu
        sliceres = A*sliceu
        IF ( gotW ) THEN BEGIN
            vdc_bufreadslice, dfdw, slicew
            sliceres /= slicew
        ENDIF
    ENDIF ELSE BEGIN
        IF ( gotW ) THEN BEGIN
            vdc_bufreadslice, dfdw, slicew
            sliceres = A/slicew
        ENDIF
    ENDELSE

    IF ( gotV ) THEN BEGIN
        vdc_bufreadslice, dfdv, slicev
        IF ( gotT ) THEN BEGIN
            vdc_bufreadslice, dfdt, slicet
            sliceres += B*slicev/slicet
        ENDIF ELSE sliceres += B*slicev
    ENDIF ELSE BEGIN
        IF ( gotT ) THEN BEGIN
            vdc_bufreadslice, dfdt, slicet
            sliceres += B/slicet
        ENDIF
    ENDELSE

    IF ( gotS ) THEN BEGIN
        vdc_bufreadslice, dfds, slices
        sliceres += C/slices
    ENDIF ELSE sliceres += C

    vdc_bufwriteslice, dfdres, sliceres

    ;  Report every 100 writes:

    IF ((Z MOD 100) EQ 0) THEN print,'wrote slice ',z

ENDFOR

vdc_closevar, dfdres
IF ( gotU ) THEN BEGIN
    vdc_closevar, dfdu
    vdc_bufreaddestroy, dfdu
ENDIF
IF ( gotW ) THEN BEGIN
    vdc_closevar, dfdw
    vdc_bufreaddestroy, dfdw
ENDIF
IF ( gotV ) THEN BEGIN
    vdc_closevar, dfdv
    vdc_bufreaddestroy, dfdv
ENDIF
IF ( gotT ) THEN BEGIN
    vdc_closevar, dfdt
    vdc_bufreaddestroy, dfdt
ENDIF
IF ( gotS ) THEN BEGIN
    vdc_closevar, dfds
    vdc_bufreaddestroy, dfds
ENDIF

vdc_bufwritedestroy, dfdres


;
;  Replace the vdf file with the new one
;

vdf_write,mfd,vdffile
;vdf_destroy, mfd
print,dim[2],' slices written to output file'

timestep += tsival

ENDREP UNTIL ( timestep GT tsmax )

vdf_destroy, mfd

END

