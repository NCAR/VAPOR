; $Id$
;
; NAME:
;       IMPREGION
;
; PURPOSE:
;		Import a volume subregion from VAPoR. The subregion must
;		first have been exported by VAPoR
;       endian or big endian format. 
;
; CALLING SEQUENCE:
;       IMPREGION
;
; KEYWORD PARAMETERS:
;       REFLEVEL[in]:    The desired refinement level. If this keyword is 
;			not set, the subregion will be imported at full resolution
;
;       VARNAME[in]:    The desired variable. If this keyword is 
;			not set, the default exported variable returned by
;			'vaporimport()' will be obtained
;
;       TIMESTEP[in]:    The desired time step. If this keyword is 
;			not set, the default exported time step returned by
;			'vaporimport()' will be obtained
;
;       STATEINFO[out]:    If this keyword is present the output of
;			the 'vaporimport()' function is returned by the keyword
;			argument, possibly modified by the VARNAME and TIMESTEP 
;			keyword. 
;
; OUTPUTS:
;       The region is returned
;
; COMMON BLOCKS:
;       None.
;
;-
function impregion, REFLEVEL=reflevel, STATEINFO=stateinfo, VARNAME=varname, TIMESTEP=timestep


on_error,2                      ;Return to caller if an error occurs
if keyword_set(reflevel) eq 0 then reflevel = -1

	; Get the state info exported from VAPoR that describes the 
	; region of interest. vaporimport() returns a structure with 
	; the following fields:
	;	VDFPATH		: Path to the .vdf file
	;	TIMESTEP	: Time step of the volume subregion of interest
	;	VARNAME		: Variable name of volume subregion of interest
	;	MINRANGE	: Minimum extents of region of interest in volume
	;				coordinates specified relative to the finest resolution
	;	MAXRANGE	: Maximum extents of region of interest in volume
	;				coordinates specified relative to the finest resolution
	stateinfo = vaporimport()

	if keyword_set(varname) then begin
		stateinfo.varname = varname
	endif else begin
		varname = stateinfo.varname
	endelse
	;if keyword_set(timestep) then begin
	if arg_present(timestep) then begin
		stateinfo.timestep = timestep
	endif else begin
		timestep = stateinfo.timestep
	endelse

	mfd = vdf_create(stateinfo.vdfpath)
	vars3d = vdf_getvariables3d(mfd)
	varIs2D = 1
	for i=0, n_elements(vars3d)-1 do begin
		if (vars3d[i] eq varname) then varIs2D = 0
	endfor


	;
	;   Create a "Buffered Read" object to read the data, passing the
	;   metadata object handle created by vdf_create() as an argument
	;
	if (varIs2D) then begin
		dfd = vdc_regreadcreate2d(mfd) 
	endif else begin
		dfd = vdc_regreadcreate(mfd) 
	endelse

	;
	; Transform coordinates from finest resolution to resolution
	; of interest. Note, the coordinates returned in 'stateinfo' 
	; are in voxel coordinates relative to the finest resolution level.
	; We first convert voxel coordinates to user coordinates, then 
	; convert user coordinates back to voxel coordinates, but at 
	; the requested refinement level. 
	;
	minu = vdc_mapvox2user(dfd, -1, timestep,stateinfo.minrange)
	min = vdc_mapuser2vox(dfd,reflevel,timestep,minu)

	maxu = vdc_mapvox2user(dfd, -1, timestep,stateinfo.maxrange)
	max = vdc_mapuser2vox(dfd,reflevel,timestep,maxu)


	; Create an array large enough to hold the volume subregion
	;
	if (varIs2D) then begin
		f = fltarr(max[0]-min[0]+1,max[1]-min[1]+1)
	endif else begin
		f = fltarr(max[0]-min[0]+1,max[1]-min[1]+1,max[2]-min[2]+1)
	endelse

	; Select the variable and time step we want to read
	;
	vdc_openvarread, dfd, timestep, varname, reflevel

	; Read the subregion
	;
	vdc_regread, dfd, min, max, f

	vdc_closevar, dfd

	vdc_regreaddestroy, dfd

	return, f

end
