;	$Id$
;
;	This example shows how to read a single data volume
;	from a VDF data collection using a "Buffered Read" object. The entire
;	spatial domain of the volume is retrieved (as opposed to fetching 
;	a spatial subregion). However, the volume is extracted at 1/8th 
;	its native spatial resolution (1/2 resolution along each dimension).
;
;	Note: in order to run this example you must have first created
;	a VDF data collection by running either the WriteVDF.pro
;	or WriteTimeVaryVDF.pro example programs



;   A value of 0 indicates that the data should be read at the
;   coarsest resolution, a value of 1 indicates the next refinement
;   level, and so on
;   A value of -1 implies a the finest (native) resolution
;
reflevel = 1

;
;	Create a VDF metadata object from an existing metadata file. The
;	metadata file must already exist on disk, having been created from
;	one of the example programs that generates a .vdf file.
;
vdffile = 'test.vdf'
mfd = vdf_create(vdffile)


;
;	Create a "Buffered Read" object to read the data, passing the 
;	metadata object handle created by vdf_create() as an argument
;
dfd = vdc_bufreadcreate(mfd)

;
;	Determine the dimensions of the volume at the given transformation
;	level. 
;
;	Note. vdc_getdim() correctly handles dimension calucation for 
;	volumes with non-power-of-two dimensions. 
;
dim = vdc_getdim(dfd, reflevel)

;
;	Create an appropriately sized array to hold the volume
;
f = fltarr(dim)
slice = fltarr(dim[0], dim[1])


;
;	Prepare to read the indicated time step and variable
;
varnames = ['ml']
vdc_openvarread, dfd, 0, varnames[0], reflevel

;
;	Read the volume one slice at a time
;
for z = 0, dim[2]-1 do begin
	vdc_bufreadslice, dfd, slice
	
	; IDL won't let us read directly into a subscripted array - need 
	; to read into a 2D array and then copy to 3D :-(
	;
	f[*,*,z] = slice
endfor

;
; Close the currently opened variable/time-step. 
;
vdc_closevar, dfd


;
;	Destroy the "buffered read" data transformation object. 
;	We're done with it.
;
vdc_bufreaddestroy, dfd


vdf_destroy, mfd
end
