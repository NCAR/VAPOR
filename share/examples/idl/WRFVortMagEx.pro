;

;
;   This example demonstrates the use of VAPOR with IDL
;   on a WRF dataset, by importing a data
;   subregion from VAPOR, calculating the magnitude of vorticity,
;   and exporting the new variable (VMag) back to VAPOR.
;
; Import a region of data previously exported by vaporgui.
; In this case, the components of the velocity field
; and the ELEVATION variable
;
U = impregion(STATEINFO=stateinfo, VARNAME='U')
V = impregion(STATEINFO=stateinfo, VARNAME='V')
W = impregion(STATEINFO=stateinfo, VARNAME='W')
ELEV = impregion(STATEINFO=stateinfo, VARNAME='ELEVATION')


mfd = vdf_create(stateinfo.VDFPATH)
extents = VDF_GETEXTENTS(mfd)
dfd = vdc_bufreadcreate(mfd)
dims = vdc_getdim(dfd, -1)

; Calculate deltax, deltay based on horizontal size of full data
deltax =  (extents[3] - extents[0])/float(dims[0])
deltay =  (extents[4] - extents[1])/float(dims[1])


;
; Compute vorticity magnitude, results go in resx,resy,resz
;
;print, 'deltas: ', deltax, deltay

print, 'input minmax ', min(U),max(U),min(V),max(V),min(W),max(W)

wrf_curl_findiff,U,V,W,resx,resy,resz,deltax,deltay,ELEV

; 
; Now, calculate and export vorticity magnitude 
;
vMag = sqrt( resx^2 + resy^2 + resz^2 )

minval = Min(vmag)
maxval = Max(vmag)

;print,'min,max: ',minval, maxval

tmpvdf = 'VMag.vdf'
expregion, tmpvdf, stateinfo, 'VMag', vMag 
end

