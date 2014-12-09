
# load geometry
geometry = square.in2d

# and mesh
#mesh = square.vol.gz
#mesh = square_trigs_2x2.vol.gz
mesh = square_trigs.vol.gz
#mesh = square_quad_coarse.vol.gz

shared = libngsxfem_xfem

define constant heapsize = 1e8

define constant zero = 0.0
define constant one = 1.0

define constant two = 2.0

define constant small = 0.5

define constant x0 = 0.5
define constant y0 = 0.5

define constant bneg = 1.0
define constant bpos = 1.0

define constant aneg = 0.2
define constant apos = 0.5

define constant abneg = (aneg*bneg)
define constant abpos = (apos*bpos)

define constant lambda = 2

define constant R = 0.33333333

# define coefficient lset
# ((x-x0)*(x-x0)+(y-y0)*(y-y0)-R*R),       

define coefficient lset
# ( x - 0.5 )
#( sqrt(0.25*(x-x0)*(x-x0)+(y-y0)*(y-y0))  - R),       
(
 8*(x-x0)*(x-x0)+64*(y-y0)*(y-y0)*(y-y0)*(y-y0)-8*(y-y0)*(y-y0)*(1-(y-y0)*(y-y0))-0.1
)


define fespace fes
       -type=h1ho
       -order=1
      -dirichlet=[1,2]
#       -dgjumps

define fespace fescomp
        -type=fictdom2fes
       -dirichlet=[1,2]

numproc informfictdomfes npix 
        -fictdomfes=fescomp
        -pair
        -fespace=fes
        -coef_levelset=lset


define gridfunction u -fespace=fescomp

define linearform f -fespace=fescomp #-print
fictxsource bneg bpos

 define bilinearform a -fespace=fescomp -print  -printelmat
 #-eliminate_internal -keep_internal -symmetric -linearform=f # -printelmat -print
#fictxmass one one 
fictxlaplace abneg abpos 
fictxnitsche aneg apos bneg bpos lambda
# #xmass small small
# xlaplace abneg abpos
# #xnitsche_minstab_hansbo aneg apos bneg bpos
# xnitsche_hansbo aneg apos bneg bpos lambda
# #lo_ghostpenalty aneg apos one

# numproc setvaluesx npsvx -gridfunction=u -coefficient_neg=two -coefficient_pos=one -boundary -print

define preconditioner c -type=local -bilinearform=a -test -block
#define preconditioner c -type=direct -bilinearform=a #-test
# define preconditioner c -type=bddc -bilinearform=a -test -block

# ##define preconditioner c -type=multigrid -bilinearform=a -test #-smoother=block

numproc bvp npbvp -gridfunction=u -bilinearform=a -linearform=f -solver=cg -preconditioner=c -maxsteps=1000 -prec=1e-6 -print

# define bilinearform avis1 -fespace=fescomp -nonassemble
# fictvis one -comp=1

# define bilinearform avis2 -fespace=fescomp -nonassemble
# fictvis one -comp=2

# numproc drawflux npdf1 -bilinearform=avis1 -solution=u -label=uneg
# numproc drawflux npdf2 -bilinearform=avis2 -solution=u -label=upos


# define coefficient veczero
# (0,0),

# numproc calccond npcc -bilinearform=a -inverse=pardiso #-symmetric

# numproc xdifference npxd 
#         -solution=u 
#         -solution_n=two
#         -solution_p=one
#         -derivative_n=veczero
#         -derivative_p=veczero
#         -levelset=lset
#         -interorder=2
#         -henryweight_n=1.0
#         -henryweight_p=1.0

# numproc xoutput npxo -solution=u -solution_n=two -solution_p=one -subdivision=0

# numproc visualization npviz -scalarfunction=u #-comp=0