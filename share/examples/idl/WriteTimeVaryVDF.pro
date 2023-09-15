;	$Id$
;
;	This example shows how to create a time varying data set containing 
;	a single variable
;


; 
;	Dimensions of the data volumes - all volumes in a data set must be 
;	of the same dimension
;
dim = [64,64,64]

;
;
; The number of coarsened approximations to create. A value of 0
; indicates that the data should not be transformed, i.e. the
; data will only exist at it's native resolution. A value of 1
; indicates that a single coarsening should be applied, and so on.
;
num_levels = 1

;
;	Create a new VDF metadata object of the indicated dimension and 
;	transform
;	level. vdf_create() returns a handle for future operations on 
;	the metadata object.
;
mfd = vdf_create(dim,num_levels)

;
;	Set the maximum number of timesteps in the data set. Note, valid data 
;	set may contain less than the maximum number of time steps, 
;	but not more
;
timesteps = 10
vdf_setnumtimesteps, mfd,timesteps

;
;	Set the names of the variables the data set will contain. In this case,
;	only a single variable will be present, "ml"
;
varnames = ['ml']
vdf_setvarnames, mfd, varnames

;
;	Store the metadata object in a file for subsequent use
;
vdffile = 'test.vdf'
vdf_write, mfd, vdffile

;
;	Destroy the metadata object. We're done with it.
;
vdf_destroy, mfd


;
;	At this point we've defined all of the metadata associated with 
;	the test data set. Now we're ready to begin populating the data
;	set with actual data. Our data, in this case, will be a 
;	time-varying Marschner Lobb function sampled on a regular grid
;


;
;	Create a "buffered write" data transformation object. The data
;	transformation object will permit us to write (transform) raw
;	data into the data set. The metadata for the data volumes is
;	obtained from the metadata file we created previously. I.e.
;	'vdffile' must contain the path to a previously created .vdf
;	file. The 'vdf_bufwritecreate' returns a handle, 'dfd', for 
;	subsequent ;	operations.
;
dfd = vdc_bufwritecreate(vdffile)

;
;	Generate a time varying data, set one volume at a time, saving each
;	volume into the data collection
;
alpha = 0.05
for ts = 0, timesteps-1 do begin

	; Create a synthetic data volume
	;
	f = marschner_lobb(dim[0], dim[1], dim[2], ALPHA=alpha)
	alpha = alpha + 0.05

	; 
	; Prepare the data set for writing. We need to identify the time step
	; and the name of the variable that we wish to store
	;
	vdc_openvarwrite, dfd, ts, varnames[0]


	;
	; Write (transform) the volume to the data set one slice at a time
	;
	for z = 0, dim[2]-1 do begin
		vdc_bufwriteslice, dfd, f[*,*,z]
	endfor

	;
	; Close the currently opened variable/time-step. We're done writing
	; to it
	;
	vdc_closevar, dfd


endfor


;
;	Destroy the "buffered write" data transformation object. 
;	We're done with it.
;
vdc_bufwritedestroy, dfd


end
