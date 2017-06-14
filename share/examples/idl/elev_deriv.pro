; NAME:
;      ELEV_DERIV 
;
; PURPOSE:
;       Computes first order z-derivatives using 6 order differences 
;       differences in WRF grids.  The z-derivative is based on
;	the ratio of differences in the field variable to differences
;	in the ELEVATION variable, both in the z-coordinate.  
;       No provision is made for a non-increasing ELEVATION variable...
;	that could result in divide by zero.
;	The ELEV variable must have the same dimensions as IN
;
; CALLING SEQUENCE:
;       ELEV_DERIV,ELEV,IN,OUT
;
; PARAMETERS:
;       IN[in]:   3D array of field variable
;       OUT[out]: 3D array with the z derivative
;       ELEV[in]: 3D array of elevations (z-coordinates)
;       
; COMMON BLOCKS:
;       None
;
;-
PRO elev_deriv,in,out,elev 

on_error,2                      ;return to caller if an error occurs
s = size(in)                    ;size of the input array
                       ;output has the same size than input
out = fltarr(s(1),s(2),s(3))


   for i = 0,2 do begin
       out(*,*,i) = (-11*in(*,*,i)+18*in(*,*,i+1)-9*in(*,*,i+2)+2*in(*,*,i+3))$
       	/(-11*elev(*,*,i)+18*elev(*,*,i+1)-9*elev(*,*,i+2)+2*elev(*,*,i+3))
                    ;forward differences near the first boundary
   endfor
   for i = 3,s(3)-4 do begin
       out(*,*,i) = (-in(*,*,i-3)+9*in(*,*,i-2)-45*in(*,*,i-1)+45*in(*,*,i+1)$
                    -9*in(*,*,i+2)+in(*,*,i+3))$
       	/(-elev(*,*,i-3)+9*elev(*,*,i-2)-45*elev(*,*,i-1)+45*elev(*,*,i+1)$
                    -9*elev(*,*,i+2)+elev(*,*,i+3)) ;centered differences
   endfor
   for i = s(3)-3,s(3)-1 do begin
       out(*,*,i) = (-2*in(*,*,i-3)+9*in(*,*,i-2)-18*in(*,*,i-1)+11*in(*,*,i))$
       	/(-2*elev(*,*,i-3)+9*elev(*,*,i-2)-18*elev(*,*,i-1)+11*elev(*,*,i))
                    ;backward differences near the second boundary
   endfor

end
