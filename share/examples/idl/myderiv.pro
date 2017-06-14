;
;	$Id$;
;

FUNCTION myderiv,x,fx,n,nonperiod
;
;
      nx=n_elements(x)
      s=size(fx)
      ndim=s(0)
;
;  Centered differences
;
if (ndim eq 1) then begin
        dfx=-1.*(shift(fx,1)-shift(fx,-1))/(x(2)-x(0))
	if (nonperiod ne 0) then begin
		lf=0
		rt=s(1)-1
		dfx(lf)=(-3.0d00*fx(lf)+4.0d00*fx(lf+1)-fx(lf+2))/(x(2)-x(0))
		dfx(rt)=(fx(rt-2)-4.0d00*fx(rt-1)+3.0d00*fx(rt))/(x(2)-x(0))
	endif
endif
if (ndim eq 2) then begin
      if(n eq 1) then begin
        dfx=-1.*(shift(fx,1,0)-shift(fx,-1,0))/(x(2)-x(0))
        if (nonperiod ne 0) then begin
                lf=0
                rt=s(1)-1
                dfx(lf,*)=(-3.0d00*fx(lf,*)+4.0d00*fx(lf+1,*)-fx(lf+2,*))/(x(2)-x(0))
                dfx(rt,*)=(fx(rt-2,*)-4.0d00*fx(rt-1,*)+3.0d00*fx(rt,*))/(x(2)-x(0))
        endif
      endif
      if(n eq 2) then begin
        dfx=-1.*(shift(fx,0,1)-shift(fx,0,-1))/(x(2)-x(0))
	if (nonperiod ne 0) then begin
                lf=0
                rt=s(2)-1
                dfx(*,lf)=(-3.0d00*fx(*,lf)+4.0d00*fx(*,lf+1)-fx(*,lf+2))/(x(2)-x(0))
                dfx(*,rt)=(fx(*,rt-2)-4.0d00*fx(*,rt-1)+3.0d00*fx(*,rt))/(x(2)-x(0))
        endif
      endif
endif
if (ndim eq 3) then begin
      if(n eq 1) then begin
        dfx=-1.*(shift(fx,1,0,0)-shift(fx,-1,0,0))/(x(2)-x(0))
	if (nonperiod ne 0) then begin
                lf=0
                rt=s(1)-1
                dfx(lf,*,*)=(-3.0d00*fx(lf,*,*)+4.0d00*fx(lf+1,*,*)-fx(lf+2,*,*))/(x(2)-x(0))
                dfx(rt,*,*)=(fx(rt-2,*,*)-4.0d00*fx(rt-1,*,*)+3.0d00*fx(rt,*,*))/(x(2)-x(0))
        endif
      endif
      if(n eq 2) then begin
        dfx=-1.*(shift(fx,0,1,0)-shift(fx,0,-1,0))/(x(2)-x(0))
	if (nonperiod ne 0) then begin
                lf=0
                rt=s(2)-1
                dfx(*,lf,*)=(-3.0d00*fx(*,lf,*)+4.0d00*fx(*,lf+1,*)-fx(*,lf+2,*))/(x(2)-x(0))
                dfx(*,rt,*)=(fx(*,rt-2,*)-4.0d00*fx(*,rt-1,*)+3.0d00*fx(*,rt,*))/(x(2)-x(0))
        endif
      endif
      if(n eq 3) then begin
        dfx=-1.*(shift(fx,0,0,1)-shift(fx,0,0,-1))/(x(2)-x(0))
	if (nonperiod ne 0) then begin
                lf=0
                rt=s(3)-1
                dfx(*,*,lf)=(-3.0d00*fx(*,*,lf)+4.0d00*fx(*,*,lf+1)-fx(*,*,lf+2))/(x(2)-x(0))
                dfx(*,*,rt)=(fx(*,*,rt-2)-4.0d00*fx(*,*,rt-1)+3.0d00*fx(*,*,rt))/(x(2)-x(0))
        endif
      endif
endif
;
      return,dfx
      end
