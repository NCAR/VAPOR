;
; NAME:
;       WRF_DIV_FINDIFF
;
; PURPOSE:
;       Computes the divergence of a vector field using sixth
;       order finite differences on WRF grid 
;	Uses an interpolation scheme described by Mark Stoellinga
;	to directly calculate the derivatives on the WRF grid.
;	The 6th order difference terms are due to Pablo Mininni
;
; CALLING SEQUENCE:
;       WRF_DIV_FINDIFF,INX,INY,INZ,OUT,DX,DY,ELEV
;
; PARAMETERS:
;       INX[in]:   3D array with the x component of the field
;       INY[in]:   3D array with the y component of the field
;       INZ[in]:   3D array with the z component of the field
;       OUT[out]:  3D array with the divergence 
;       DX[in]:    spatial step of the grid in the x direction
;       DY[in]:    spatial step of the grid in the y direction.
;	ELEV[in]:  elevation variable
;
;
; COMMON BLOCKS:
;       None
;
;-
PRO wrf_div_findiff,inx,iny,inz,out,dx,dy,elev

on_error,2                      ;return to caller if an error occurs

; calculate dU/dx as (dU/dx)_eta - (dU/dz)*(dZ/dx)_eta
; where _eta indicates the derivative calculated in the WRF grid coordinates
; as is performed by deriv_findiff.pro
deriv_findiff, inx, aux1, 1, dx
elev_deriv, inx, aux2, elev
deriv_findiff, elev, aux3, 1, dx  
out = aux1 - aux2*aux3

; calculate dV/dy as (dV/dy)_eta - (dV/dz)*(dZ/dy)_eta
; where _eta indicates the derivative calculated in the WRF grid coordinates
; as is performed by deriv_findiff.pro
deriv_findiff, iny, aux1, 2, dy
elev_deriv, iny, aux2, elev
deriv_findiff, elev, aux3, 2, dy  
out = out + aux1 - aux2*aux3

; dW/dz:
elev_deriv, inz, aux1, elev

out = out + aux1

end
