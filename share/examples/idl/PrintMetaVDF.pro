;	$Id$
;
;	This example shows how to extact various bits of metadata information
;	from a from a VDF data collection.
;
;	Note: in order to run this example you must have first created
;	a VDF data collection by running either the WriteVDF.pro
;	or WriteTimeVaryVDF.pro example programs


;
;	Create a VDF metadata object from an existing metadata file. The
;	metadata file must already exist on disk, having been created from
;	one of the example programs that generates a .vdf file.
;
vdffile = 'test.vdf'
mfd = vdf_create(vdffile)


print, 'Block size = ', vdf_getblocksize(mfd)
bs = vdf_getblocksize(mfd)

print, 'Dimension = ', vdf_getdimension(mfd)

print, 'Num Filter Coefficients = ', vdf_getfiltercoef(mfd)

print, 'Num Lifting Coefficients = ', vdf_getliftingcoef(mfd)

print, 'Num Wavelet Transforms = ', vdf_getnumtransforms(mfd)

print, 'Grid Type = ', vdf_getgridtype(mfd)

print, 'Coordinate Type = ', vdf_getcoordtype(mfd)

print, 'Volume extents = ', vdf_getextents(mfd)

print, 'Num Time Steps = ', vdf_getnumtimesteps(mfd)

print, 'Variable Names = ', vdf_getvarnames(mfd)

nvarnames3d = 0
if (n_elements(vdf_getvariables3d(mfd)) ne 0) then begin
	varnames3d = vdf_getvariables3d(mfd)
	nvarnames3d = n_elements(varnames3d)
	print, '3D Variable Names = ', varnames3d
endif

nvarnames2dxy = 0
if (n_elements(vdf_getvariables2dxy(mfd)) ne 0) then begin
	varnames2dxy = vdf_getvariables2dxy(mfd)
	nvarnames2dxy = n_elements(varnames2dxy)
	print, '2DXY Variable Names = ', varnames2dxy
endif

print, 'Global comment = ', vdf_getcomment(mfd)


timesteps = vdf_getnumtimesteps(mfd)

for ts = 0, timesteps[0]-1 do begin
	print, 'Time step ', ts, ' metadata'
	print, '	User Time = ', vdf_gettusertime(mfd, ts)
	if (n_elements(vdf_gettxcoords(mfd, ts)) ne 0) then print, '	XCoords = ', vdf_gettxcoords(mfd, ts)
	if (n_elements(vdf_gettycoords(mfd, ts)) ne 0) then print, '	YCoords = ', vdf_gettycoords(mfd, ts)
	if (n_elements(vdf_gettzcoords(mfd, ts)) ne 0) then print, '	ZCoords = ', vdf_gettzcoords(mfd, ts)
	if (n_elements(vdf_gettcomment(mfd, ts)) ne 0) then print, '	Comment = ', vdf_gettcomment(mfd, ts)

	for v = 0, nvarnames3d-1 do begin
		print, '	3D Variable ', varnames3d[v], ':'
		if (n_elements(vdf_getvcomment(mfd, ts, varnames3d[v])) ne 0) then print, '	Comment = ', vdf_getvcomment(mfd, ts, varnames3d[v])

	endfor

	for v = 0, nvarnames2dxy-1 do begin
		print, '	2D Variable ', varnames2dxy[v], ':'
		if (n_elements(vdf_getvcomment(mfd, ts, varnames2dxy[v])) ne 0) then print, '	Comment = ', vdf_getvcomment(mfd, ts, varnames2dxy[v])

	endfor
endfor


vdf_destroy, mfd

end
