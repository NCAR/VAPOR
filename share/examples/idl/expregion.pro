;
; NAME:
;       EXPREGION
;
; PURPOSE:
;		Export a volume subregion to VAPoR. This procedure is a convenience
;		function for exporting a subvolume to VAPOR. In essense, it simply
;		creates a mini VDC with the same basic attributes (dimension, 
;		user extents, num transforms, etc) as an existing "parent" VDC. 
;		The new, mini VDC may be merged with the parent VDC by the
;		vapor gui.
;
;		N.B. This procedure assumes that the region to be exported is sampled
;		at the native (highest) resolution of the grid.
;
; CALLING SEQUENCE:
;       EXPREGION, VDF, STATEINFO, VARNAMES, ARRAY0 [, ARRAY1 [, ARRAY2...[, ARRAY5]]]
;
; KEYWORD PARAMETERS:
;
;		APPEND:		A boolean indicating whether exported variables should
;					be written to an existing .vdf file named by VDF. If not
;					set a new .vdf file will be created
;
;       COORD:		A three-element array containing the starting XYZ 
;					coordinates (in voxels) of the region
;					to be exported relative to the parent VDC. The
;					default is to use the MINRANGE structure member of 
;					the STATEINFO paramater
;
;       NUMTS:		If set specifies the number of timesteps to allow
;					for in the .vdf file specified by VDF. This keyword
;					and the APPEND keyword are mutually exclusive.
;
;		TS:			If set specifies the time set offset for the
;					exported variable. The default is 0
;
;		VAR2DXY:	If set the variables 2D arrays in the XY plane
;					
;
; INPUTS:
;
;		VDF:		Path name of the vdf file that will describe
;					the region
;
;
;		STATEINFO:	A stateinfo structure returned by vaporimport() (the
;					parent VDC)
;
;       VARNAMES:	The names of the variables that are being exported. 
;					The number of ARRAY arguments must agree with the
;					number of names contained in VARNAMES
;
;		ARRAY0:		First 3D array containig the region to export
;		ARRAY1:		Second 3D array containig the region to export
;		ARRAY2:		Third 3D array containig the region to export
;		ARRAY3:		Fourth 3D array containig the region to export
;		ARRAY4:		Fifth 3D array containig the region to export
;		ARRAY5:		Sixth 3D array containig the region to export
;
; OUTPUTS:
;
; COMMON BLOCKS:
;       None.
;
;-
pro expregion, vdf, stateinfo, varnames, array0, array1, array2, array3, array4, array5, COORD=coord, APPEND=append, NUMTS=numts, TS=ts, VAR2DXY=var2dxy


	on_error,2                      ;Return to caller if an error occurs
	if keyword_set(coord) eq 0 then coord = stateinfo.minrange
	if keyword_set(numts) eq 0 then numts = 1
	if keyword_set(ts) eq 0 then ts = 0
	if keyword_set(append) eq 0 then append = 0
	if keyword_set(var2dxy) eq 0 then var2dxy = 0
	varnamesvec = [varnames]	; Ensure varnames is an array


	;
	; Open the .vdf file describing the parent VDC
	;
	mfd = vdf_create(stateinfo.vdfpath)

	;
	; Extract the relevant attributes from the parent VDC
	;
	nfiltercoef = vdf_getfiltercoef(mfd)
	nliftingcoef = vdf_getliftingcoef(mfd)
	bs = vdf_getblocksize(mfd)
	nxforms = vdf_getnumtransforms(mfd)
	dim = vdf_getdimension(mfd)

	vdf_destroy, mfd

	;
	; If the append keyword is set create a new, "mini" VDC with the 
	; same basic attributes of the parente VDC. Otherwise the .vdf file 
	; referenced by 'vdf' must already exist.
	;
	if (append eq 0) then begin
		mfd = vdf_create(dim, nxforms, BS=bs, NFILTERCOEF=nfiltercoef, NLIFTINGCOEF=nliftingcoef)

		;
		; Add the new variable to the mini VDC
		;
		vdf_setvarnames, mfd, varnamesvec
		if (var2dxy eq 1)  then begin
			vdf_setvariables2dxy, mfd, varnamesvec
		endif
		vdf_setnumtimesteps, mfd, numts

		vdf_write, mfd, vdf
		vdf_destroy, mfd
	endif


	;
	; Now write the arrays to the new VDC
	;
	if (var2dxy eq 1)  then begin
		dfd = vdc_regwritecreate2d(vdf)
	endif else begin
		dfd = vdc_regwritecreate(vdf)
	endelse

	;
	; Brain damaged code for handling optional number of function
	; parameters
	;
	if (n_elements(array0) ne 0) and (n_elements(varnamesvec) ge 1) then begin
		vdc_openvarwrite, dfd, ts, varnamesvec[0], -1
		vdc_regwrite, dfd, array0, coord
		vdc_closevar, dfd
	endif
	if (n_elements(array1) ne 0) and (n_elements(varnamesvec) ge 2) then begin
		vdc_openvarwrite, dfd, ts, varnamesvec[1], -1
		vdc_regwrite, dfd, array1, coord
		vdc_closevar, dfd
	endif
	if (n_elements(array2) ne 0) and (n_elements(varnamesvec) ge 3) then begin
		vdc_openvarwrite, dfd, ts, varnamesvec[2], -1
		vdc_regwrite, dfd, array2, coord
		vdc_closevar, dfd
	endif
	if (n_elements(array3) ne 0) and (n_elements(varnamesvec) ge 4) then begin
		vdc_openvarwrite, dfd, ts, varnamesvec[3], -1
		vdc_regwrite, dfd, array3, coord
		vdc_closevar, dfd
	endif
	if (n_elements(array4) ne 0) and (n_elements(varnamesvec) ge 5) then begin
		vdc_openvarwrite, dfd, ts, varnamesvec[4], -1
		vdc_regwrite, dfd, array4, coord
		vdc_closevar, dfd
	endif
	if (n_elements(array5) ne 0) and (n_elements(varnamesvec) ge 6) then begin
		vdc_openvarwrite, dfd, ts, varnamesvec[5], -1
		vdc_regwrite, dfd, array5, coord
		vdc_closevar, dfd
	endif


	vdc_regwritedestroy, dfd

end
