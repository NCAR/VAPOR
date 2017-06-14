;
;	AddMagVDF.pro
;
;	Utility to read three 3D variables from a VDF and calculate their magnitude
;	and put it back into the VDF.
;	All three variables must be present at full resolution.
;	This is performed one time-step at a time.
;	The vdf file is replaced.  The previous vdf file is saved, with
;	_saved appended to its name, in case of catastrophic failure 
;	 
;	Arguments are:
;	vdffile = file path of the metadata file
;	varx, vary, varz = the 3 3D variables defining the field
;		whose magnitude is being calculated
;	magvarname is the name for the field magnitude being calculated
;	tsstart specifies the first timestep for which the magnitude
;		is calculated.  If multiple timesteps are needed,
;	keyword variables:
;       tsmax = largest time step
;	tsival = increment between time steps	
;

PRO AddMagVDF,vdffile,varx,vary,varz,magvarname,tsstart,$
	TSMAX=tsmax,TSIVAL=tsival

;
;   Set up timesteps
;
timestep = tsstart
IF (~keyword_set(tsmax)) THEN tsmax = tsstart
IF (~keyword_set(tsival)) THEN tsival = 1


;
;	Start with the current metadata:
;

mfd = vdf_create(vdffile)

;
; 	save the current vdf file (in case we screw up)
;

savedvdffile = STRING(vdffile,'_saved')
vdf_write,mfd,savedvdffile

;
; 	Add the new variable name to the current variable names:
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
;  	Need to make sure this is not in the list!
isinvariables = 0
FOR I = 0, numvars-2 DO BEGIN
	newvarnames[I] = varnames[I]
	if (varnames[I] EQ magvarname) THEN isinvariables = 1
ENDFOR
	
IF (isinvariables EQ 0) THEN newvarnames[numvars-1] = magvarname ELSE newvarnames = varnames

print,'The 3D variable names in the vdf will be: ',newvarnames

;
;	reset the varnames in mfd to the new value:
;	and set the 2d vars if there are any
;

if (isinvariables EQ 0) THEN BEGIN
	if (nvarnames2dxy gt 0) THEN BEGIN
		vdf_setvarnames,mfd,[newvarnames,varnames2dxy]
		vdf_setvariables2dxy,mfd,varnames2dxy
	ENDIF ELSE BEGIN
		vdf_setvarnames,mfd,newvarnames
	ENDELSE
ENDIF

reflevel = vdf_getnumtransforms(mfd)
numtimesteps = vdf_getnumtimesteps(mfd)

REPEAT BEGIN

;
;	Create "Buffered Read" objects for each variable to read the data, passing the 
;	metadata object handle created by vdf_create() as an argument
;

dfdx = vdc_bufreadcreate(mfd)
dfdy = vdc_bufreadcreate(mfd)
dfdz = vdc_bufreadcreate(mfd)
dfdmag = vdc_bufwritecreate(mfd)

;
;
;	Determine the dimensions of the x-variable at the full transformation
;	level. 
;	This is used as the dimension of all the variables
;	Note. vdc_getdim() correctly handles dimension calucation for 
;	volumes with non-power-of-two dimensions. 
; 

dim = vdc_getdim(dfdx, reflevel)

;
;	Create appropriately sized arrays to hold the volume slices:
;

slicex = fltarr(dim[0], dim[1])
slicey = fltarr(dim[0], dim[1])
slicez = fltarr(dim[0], dim[1])
slicemag = fltarr(dim[0], dim[1])
slicemagsq = fltarr(dim[0], dim[1])


;
;	Prepare to read and write the indicated time step and variables
;

vdc_openvarread, dfdx, timestep, varx, reflevel
vdc_openvarread, dfdy, timestep, vary, reflevel
vdc_openvarread, dfdz, timestep, varz, reflevel
vdc_openvarwrite,dfdmag, timestep, magvarname, reflevel

;
;	Read the volume one slice at a time
;

FOR z = 0, dim[2]-1 DO BEGIN 
	vdc_bufreadslice, dfdx, slicex
	vdc_bufreadslice, dfdy, slicey
	vdc_bufreadslice, dfdz, slicez
	
	slicemagsq = slicex*slicex + slicey*slicey + slicez*slicez 
	slicemag = sqrt(slicemagsq) 
	
	vdc_bufwriteslice, dfdmag, slicemag

	; 	Report every 100 writes:

	IF ((Z MOD 100) EQ 0) THEN print,'wrote slice ',z

ENDFOR

vdc_closevar, dfdmag 
vdc_closevar, dfdx
vdc_closevar, dfdy
vdc_closevar, dfdz

vdc_bufwritedestroy, dfdmag
vdc_bufreaddestroy, dfdx
vdc_bufreaddestroy, dfdy
vdc_bufreaddestroy, dfdz


;
;  Replace the vdf file with the new one, the first time through:
;
IF (timestep EQ tsstart AND isinvariables EQ 0) THEN vdf_write,mfd,vdffile

print,'Magnitude calculated of timestep ', timestep

timestep += tsival

ENDREP UNTIL (timestep GT tsmax)
vdf_destroy, mfd

end
