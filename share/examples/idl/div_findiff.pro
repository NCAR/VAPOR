; $Id$
;
; NAME:
;       DIV_FINDIFF
;
; PURPOSE:
;       Computes the divergence of a vector field using sixth
;       order finite differences in regular Cartesian grids
;
; CALLING SEQUENCE:
;       DIV_FINDIFF,INX,INY,INZ,OUT,DX,DY,DZ
;
; PARAMETERS:
;       INX[in]:   3D array with the x component of the field
;       INY[in]:   3D array with the y component of the field
;       INZ[in]:   3D array with the z component of the field
;       OUT[out]:  3D array with the divergence
;       DX[in]:    spatial step of the grid in the x direction
;       DY[in]:    spatial step of the grid in the y direction.
;       DZ[in]:    spatial step of the grid in the z direction.
;
;
; COMMON BLOCKS:
;       None
;
;-
PRO div_findiff,inx,iny,inz,out,dx,dy,dz

on_error,2                      ;return to caller if an error occurs

deriv_findiff,inx,aux1,1,dx       ;x component of the div 
deriv_findiff,iny,aux2,2,dy       ;y component of the div 
deriv_findiff,inz,aux3,3,dz       ;z component of the div 

out = aux1+aux1+aux3 

end
