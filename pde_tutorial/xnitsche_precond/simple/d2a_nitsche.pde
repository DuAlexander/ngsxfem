
# load geometry
geometry = d1a_nitsche.in2d

# and mesh
mesh = d2a_nitsche_reg2.vol.gz
#mesh = d2a_nitsche_unstr2.vol.gz

shared = libngsxfem_xfem

define constant heapsize = 1e8

define constant zero = 0.0
define constant one = 1.0
define constant two = 2.0

#sliver case
# define constant eps1 = (1.0/exp(log(2)*23))

# define constant x0 = (0.5+eps1)
# define constant y0 = (0.5+eps1)

#shifting case
define constant shift = (0.0/100000)

define constant x0 = (0.5)
define constant y0 = (0.5)

define constant bneg = 2.0
define constant bpos = 1.0

define constant aneg = 3.0
define constant apos = 2.0

define constant abneg = (aneg*bneg)
define constant abpos = (apos*bpos)

define constant lambda = 2

define constant R = 0.05

define constant d = 0.2

define coefficient lset
(
 (        
  ((x-x0+d) < 0) 
   *
   (        
    ((y-y0+d) < 0) 
     * (sqrt((x-x0+d)*(x-x0+d)+(y-y0+d)*(y-y0+d))-R)
    +
    ((y-y0+d) > 0) * ((y-y0-d) < 0) 
     * (x0-x-d-R) 
    +
    ((y-y0-d) > 0) 
     * (sqrt((x-x0+d)*(x-x0+d)+(y-y0-d)*(y-y0-d))-R)
   )
  +
  ((x-x0+d) > 0) * ((x-x0-d) < 0) 
   *
   (        
    ((y-y0+d) < 0) 
     * (y0-y-d-R) 
    +
    ((y-y0+d) > 0) * ((y-y0-d) < 0) 
     * (-R) 
    +
    ((y-y0-d) > 0) 
     * (y-y0-d-R) 
   )
  +
  ((x-x0-d) > 0) 
   *
   (        
    ((y-y0+d) < 0) 
     * (sqrt((x-x0-d)*(x-x0-d)+(y-y0+d)*(y-y0+d))-R)
    +
    ((y-y0+d) > 0) * ((y-y0-d) < 0) 
     * (x-x0-d-R) 
    +
    ((y-y0-d) > 0) 
     * (sqrt((x-x0-d)*(x-x0-d)+(y-y0-d)*(y-y0-d))-R)
   )
 )
)




define fespace fesl2
       -type=l2ho -order=2

define gridfunction ulset -fespace=fesl2

numproc setvalues npslset -gridfunction=ulset -coefficient=lset #-boundary

define fespace fescomp
       -type=xh1fespace
       -order=1
       -dirichlet=[1,2]
       -ref_space=3
#       -dgjumps

numproc informxfem npix 
        -xh1fespace=fescomp
        -coef_levelset=lset

define gridfunction u -fespace=fescomp

define linearform f -fespace=fescomp # -print
xsource bneg zero

define constant small = 0.025

define bilinearform a -fespace=fescomp #-eliminate_internal -keep_internal -symmetric -linearform=f # -printelmat -print
#xmass one one
#xmass small small
xlaplace abneg abpos
#xnitsche_minstab aneg apos bneg bpos
#xnitsche_minstab_alphabeta aneg apos bneg bpos
#xnitsche_halfhalf aneg apos bneg bpos lambda
xnitsche_hansbo aneg apos bneg bpos lambda
#lo_ghostpenalty aneg apos small

numproc setvaluesx npsvx -gridfunction=u -coefficient_neg=two -coefficient_pos=zero -boundary -print

define preconditioner c -type=local -bilinearform=a -lapacktest #-block
# define preconditioner c -type=direct -bilinearform=a -test
#define preconditioner c -type=bddc -bilinearform=a -test -block

##define preconditioner c -type=multigrid -bilinearform=a -test #-smoother=block

numproc bvp npbvp -gridfunction=u -bilinearform=a -linearform=f -solver=cg -preconditioner=c -maxsteps=1000 -prec=1e-6 # -print

#numproc calccond npcc -bilinearform=a -inverse=pardiso # -gridfunction=u -printmatrix #-symmetric

numproc visualization npviz -scalarfunction=ulset -deformationscale=1 -minval=0 -maxval=0 #-comp=0

numproc xdifference npxd 
        -solution=u 
        -solution_n=zero
        -solution_p=zero
        -levelset=lset
        -interorder=2
        -henryweight_n=1
        -henryweight_p=1

#numproc xoutput npxo -solution=u -solution_n=two -solution_p=one -subdivision=0

numproc quit npq -option