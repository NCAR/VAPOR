;
; NAME:
;       WRF_CURL_FINDIFF
;
; PURPOSE:
;       Computes the curl of a vector field using sixth
;       order finite differences on WRF grid 
;	Uses an interpolation scheme described by Mark Stoellinga
;	to directly calculate the derivatives on the WRF grid.
;	The 6th order difference terms are due to Pablo Mininni
;
; CALLING SEQUENCE:
;       WRF_CURL_FINDIFF,INX,INY,INZ,OUTX,OUTY,OUTZ,DX,DY,ELEV
;
; PARAMETERS:
;       INX[in]:   3D array with the x component of the field
;       INY[in]:   3D array with the y component of the field
;       INZ[in]:   3D array with the z component of the field
;       OUTX[out]: 3D array with the x component of the curl
;       OUTY[out]: 3D array with the y component of the curl
;       OUTZ[out]: 3D array with the z component of the curl
;       DX[in]:    spatial step of the grid in the x direction
;       DY[in]:    spatial step of the grid in the y direction.
;	ELEV[in]:  elevation variable
;
;
; COMMON BLOCKS:
;       None
;
;-
PRO wrf_curl_findiff,inx,iny,inz,outx,outy,outz,dx,dy,elev

on_error,2                      ;return to caller if an error occurs

; x component is dW/dy - dV/dz 
; dW/dy is calc by (dW/dy)_eta - (dW/dz)*(dZ/dy)_eta
; where _eta indicates the derivative calculated in the WRF grid coordinates
; as is performed by deriv_findiff.pro
; and Z is the ELEVATION.
deriv_findiff, inz, aux1, 2, dy
elev_deriv, inz, aux2, elev
deriv_findiff, elev, aux3, 2, dy  
;  dV/dz:
elev_deriv, iny, aux4, elev
outx = aux1 - aux2*aux3 - aux4

; y component is dU/dz - dW/dx 
;  dU/dz:
elev_deriv, inx, aux4, elev
; dW/dx is calc by (dW/dx)_eta - (dW/dz)*(dZ/dx)_eta
deriv_findiff, inz, aux1, 1, dx
elev_deriv, inz, aux2, elev
deriv_findiff, elev, aux3, 1, dx  

outy = aux4 - aux1 + aux2*aux3

; z component of curl is dV/dx - dU/dy
; dV/dx is (dV/dx)_eta - (dV/dz)*(dZ/dx)_eta
deriv_findiff, iny, aux1, 1, dx
elev_deriv, iny, aux2, elev
deriv_findiff, elev, aux3, 1, dx  
; dU/dy is (dU/dy)_eta - (dU/dz)*(dZ/dy)_eta
deriv_findiff, inx, aux4, 2, dy
elev_deriv, inx, aux5, elev
deriv_findiff, elev, aux6, 2, dy  
outz = aux1 - aux2*aux3 - aux4 + aux5*aux6


end
