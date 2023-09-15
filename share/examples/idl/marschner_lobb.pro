;	$Id$
;
; Evaluate the 3D marschner lobb function over a grid at the specified
; resolution. The sampled function is returned.
;

function marschner_lobb, nx, ny, nz, ALPHAFLAG = ALPHA, FMFLAG=FM

	if (n_elements(ALPHA) eq 0) then ALPHA = 0.25
	if (n_elements(FM) eq 0) then FM = 6.0

	rarr = fltarr(nx,ny,nz)
	zarr = fltarr(nx,ny,nz)
	PI = 3.14159265358979323846

	for z = 0, nz-1 do begin
		for y = 0, ny-1 do begin
			yc = (y / float(ny-1)) - 0.5
			for x = 0, nx-1 do begin
				xc = (x / float(nx-1)) - 0.5
				rarr[x,y,z] = sqrt(xc*xc + yc*yc)
				zarr[x,y,z] = (z / float(nz-1)) - 0.5
			endfor
		endfor
	endfor

	pr = cos(2*PI*FM*cos(PI*rarr/2.0))
	f = (1.0 - sin(PI*zarr/2.0) + (ALPHA * (1.0 + pr))) / (2*(1+ALPHA))
				
	return, f
end
