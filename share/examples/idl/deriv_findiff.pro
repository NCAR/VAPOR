; $Id$
;
; NAME:
;       DERIV_FINDIFF
;
; PURPOSE:
;       Computes first order derivatives using sixth order finite
;       differences in regular Cartesian grids
;
; CALLING SEQUENCE:
;       DERIV_FINDIFF,IN,OUT,DIR,DX
;
; PARAMETERS:
;       IN[in]:   3D array with the field component
;       OUT[out]: 3D array with the spatial derivative
;       DIR[in]:  direction (1,2, or 3) of the the derivative
;       DX[in]:   spatial step of the grid in the direction DIR
;       
; COMMON BLOCKS:
;       None
;
;-
PRO deriv_findiff,in,out,dir,dx

on_error,2                      ;return to caller if an error occurs
s = size(in)                    ;size of the input array
out = in                        ;output has the same size than input

if dir eq 1 then begin          ;derivative for dir=1
   for i = 0,2 do begin
       out(i,*,*) = (-11*in(i,*,*)+18*in(i+1,*,*)-9*in(i+2,*,*)+2*in(i+3,*,*))$
                    /(6*dx)     ;forward differences near the first boundary
   endfor
   for i = 3,s(dir)-4 do begin
       out(i,*,*) = (-in(i-3,*,*)+9*in(i-2,*,*)-45*in(i-1,*,*)+45*in(i+1,*,*) $
                    -9*in(i+2,*,*)+in(i+3,*,*))/(60*dx) ;centered differences
   endfor
   for i = s(dir)-3,s(dir)-1 do begin
       out(i,*,*) = (-2*in(i-3,*,*)+9*in(i-2,*,*)-18*in(i-1,*,*)+11*in(i,*,*))$
                    /(6*dx)     ;backward differences near the second boundary
   endfor
endif

if dir eq 2 then begin          ;derivative for dir=2
   for i = 0,2 do begin
       out(*,i,*) = (-11*in(*,i,*)+18*in(*,i+1,*)-9*in(*,i+2,*)+2*in(*,i+3,*))$
                    /(6*dx)     ;forward differences near the first boundary
   endfor
   for i = 3,s(dir)-4 do begin
       out(*,i,*) = (-in(*,i-3,*)+9*in(*,i-2,*)-45*in(*,i-1,*)+45*in(*,i+1,*) $
                    -9*in(*,i+2,*)+in(*,i+3,*))/(60*dx) ;centered differences
   endfor
   for i = s(dir)-3,s(dir)-1 do begin
       out(*,i,*) = (-2*in(*,i-3,*)+9*in(*,i-2,*)-18*in(*,i-1,*)+11*in(*,i,*))$
                    /(6*dx)     ;backward differences near the second boundary
   endfor
endif

if dir eq 3 then begin          ;derivative for dir=3
   for i = 0,2 do begin
       out(*,*,i) = (-11*in(*,*,i)+18*in(*,*,i+1)-9*in(*,*,i+2)+2*in(*,*,i+3))$
                    /(6*dx)     ;forward differences near the first boundary
   endfor
   for i = 3,s(dir)-4 do begin
       out(*,*,i) = (-in(*,*,i-3)+9*in(*,*,i-2)-45*in(*,*,i-1)+45*in(*,*,i+1)$
                    -9*in(*,*,i+2)+in(*,*,i+3))/(60*dx) ;centered differences
   endfor
   for i = s(dir)-3,s(dir)-1 do begin
       out(*,*,i) = (-2*in(*,*,i-3)+9*in(*,*,i-2)-18*in(*,*,i-1)+11*in(*,*,i))$
                    /(6*dx)     ;backward differences near the second boundary
   endfor
endif

end
