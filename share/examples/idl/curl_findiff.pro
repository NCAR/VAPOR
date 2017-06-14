; $Id$
;
; NAME:
;       CURL_FINDIFF
;
; PURPOSE:
;       Computes the curl of a vector field using sixth
;       order finite differences in regular Cartesian grids
;
; CALLING SEQUENCE:
;       CURL_FINDIFF,INX,INY,INZ,OUTX,OUTY,OUTZ,DX,DY,DZ
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
;       DZ[in]:    spatial step of the grid in the z direction.
;
;
; COMMON BLOCKS:
;       None
;
;-
PRO curl_findiff,inx,iny,inz,outx,outy,outz,dx,dy,dz

on_error,2                      ;return to caller if an error occurs

deriv_findiff,inz,aux1,2,dy       ;x component of the curl
deriv_findiff,iny,aux2,3,dz
outx = aux1-aux2

deriv_findiff,inx,aux1,3,dz       ;y component of the curl
deriv_findiff,inz,aux2,1,dx
outy = aux1-aux2

deriv_findiff,iny,aux1,1,dx       ;z component of the curl
deriv_findiff,inx,aux2,2,dy
outz = aux1-aux2

end
