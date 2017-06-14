;
;   MakeLinCmb.pro
;
;   Utility to calculate a linear combination of 3D variables in a vdf,
;   and put it back into the VDF.
;   i.e. calculate A*U+B*V+C, where U and V are 3D variables and
;   A, B, and C are constants.  If B is 0, V is ignored.
;   U must be present at full resolution,
;   If B != 0, then V must be present at full resolution
;
;   This is performed one time-step at a time.
;   The vdf file is replaced.  The previous vdf file is saved, with
;   _saved appended to its name, in case of catastrophic failure
;
;   Arguments are:
;   vdffile = file path of the metadata file
;   uVar, vVar  the 2 variable names (of U and V)
;   resVar is the name for the linear combination
;   A,B,C are the coefficients
;   timestep specifies the (only) timestep for which the lin comb
;     is calculated.  If multiple timesteps are needed,
;     the calculation must be repeated for each of them.
;

PRO MakeLinCmb,vdffile,uVar,vVar,A,B,C,resVar,timestep

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
;   Create "Buffered Read" objects for each variable to read the data, passing the
;   metadata object handle created by vdf_create() as an argument
;

dfdu = vdc_bufreadcreate(mfd)
if ( B NE 0) then dfdv = vdc_bufreadcreate(mfd)
dfdres = vdc_bufwritecreate(mfd)

;
;
;   Determine the dimensions of the u-variable at the full transformation
;   level.
;   This is used as the dimension of all the variables
;   Note. vdc_getdim() correctly handles dimension calucation for
;   volumes with non-power-of-two dimensions.
;

dim = vdc_getdim(dfdu, reflevel)

;
;   Create appropriately sized arrays to hold the volume slices:
;

sliceu = fltarr(dim[0], dim[1])
IF ( B NE 0) then slicev = fltarr(dim[0], dim[1])
sliceres = fltarr(dim[0], dim[1])



;
;   Prepare to read and write the indicated time step and variables
;

vdc_openvarread, dfdu, timestep, uVar, reflevel
IF ( B NE 0) then vdc_openvarread, dfdv, timestep, vVar, reflevel
vdc_openvarwrite,dfdres, timestep, resVar, reflevel

;
;   Read the volume one slice at a time
;

FOR z = 0, dim[2]-1 DO BEGIN
    vdc_bufreadslice, dfdu, sliceu
    IF ( B NE 0) then vdc_bufreadslice, dfdv, slicev

    sliceres = sliceu*A + C
    IF ( B NE 0) then sliceres = sliceres + slicev*B

    vdc_bufwriteslice, dfdres, sliceres

    ;  Report every 100 writes:

    IF ((Z MOD 100) EQ 0) THEN print,'wrote slice ',z

ENDFOR

vdc_closevar, dfdres
vdc_closevar, dfdu
IF ( B NE 0) then vdc_closevar, dfdv


vdc_bufwritedestroy, dfdres
vdc_bufreaddestroy, dfdu
IF ( B NE 0) then vdc_bufreaddestroy, dfdv

;
;  Replace the vdf file with the new one
;

vdf_write,mfd,vdffile
vdf_destroy, mfd
print,dim[2],' slices written to output file'
end
