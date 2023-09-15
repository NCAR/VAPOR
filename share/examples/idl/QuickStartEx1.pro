; $Id$
;
;

;
;   This example is referenced by the VAPOR Quick Start guide. It
;   demonstrates VAPOR's interaction with IDL by importing a data
;   subregion from VAPOR, calculating the Z component of vorticity,
;   and exporting the new quantity back to VAPOR.
;

;
; Import a region of data previously exported by vaporgui.
; In this case, the components of the velocity field, scaled by ro
;
ru = impregion(STATEINFO=stateinfo, VARNAME='ru')
rv = impregion(STATEINFO=stateinfo, VARNAME='rv')
rw = impregion(STATEINFO=stateinfo, VARNAME='rw')
ro = impregion(STATEINFO=stateinfo, VARNAME='ro')

ru = ru/ro
rv = rv/ro
rw = rw/ro

dim = size(ru, /DIMENSIONS)

;
; Compute vorticity
;
x=findgen(dim[0])
y=findgen(dim[1])
dvu=fltarr(dim[0], dim[1], dim[2])
omz=fltarr(dim[0], dim[1], dim[2])
for k=0,dim[2]-1 do begin
    tmp=reform(ru(*,*,k))
;    dvu(*,*,k)=myderiv(x,tmp,1,1)
    omz(*,*,k)=myderiv(y,tmp,2,1)
    tmp=reform(rv(*,*,k))
;    dvu(*,*,k)=dvu(*,*,k)+myderiv(y,tmp,2,1)
    omz(*,*,k)=myderiv(x,tmp,1,1)-omz(*,*,k)
endfor

;
; Finally, export the region to a new VDC, named 'QuickStartEx1.vdf'.
; The new variable we have created will be named 'omz'. The new VDC maybe
; either merged into a running vaporgui session from whence the original
; data came, or may simply be loaded as a new data set
;
;  Note there's an arithmetic error we can ignore...

tmpvdf = 'QuickStartEx1.vdf'
varname = 'omz'
expregion, tmpvdf, stateinfo, varname, omz

end
