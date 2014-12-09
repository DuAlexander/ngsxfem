#
# solve the Poisson equation -Delta u = f
#
# with boundary conditions
#      u = 0  on Gamma1
#  du/dn = 1  on Gamma2

# load geometry
geometry = square.in2d

# and mesh
#mesh = square.vol.gz
mesh = square_trigs2.vol.gz
#mesh = square_quad_coarse.vol.gz

shared = libngsxfem_spacetime
shared = libngsxfem_xfem
shared = libngsxfem_parabolic

define constant heapsize = 1e7

define constant zero = 0.0
define constant one = 1.0

define constant bneg = 1.0
define constant bpos = 5.0

define constant wx = 0.0
define constant wy = 0.0

define constant x0 = 0.4
define constant y0 = 0.4

define constant R = 0.16666666666

define coefficient bconvneg
(1.0/bneg*wx,1.0/bneg*wy),

define coefficient bconvpos
(1.0/bpos*wx,1.0/bpos*wy),

define coefficient binineg
(1.0*bneg),

define coefficient binipos
(0.0*bpos),
# (0.05*sin(15*x*x)*bpos),

define coefficient brhsneg
(0),

define coefficient brhspos
(0),

define coefficient bndneg
(0),

define coefficient bndpos
0,
#(bpos*0.1*sin(10*pi*z)*exp(-50*(x-z)*(x-z))),

define fespace fesh1
       -type=spacetimefes 
       -type_space=h1ho
       -order_space=1
       -order_time=1
       -dirichlet=[1,2]
#       -gaussradau
       # -dgjumps

define coefficient coef_lset
((x-z*(wx)-x0)*(x-z*(wx)-x0)+(y-z*(wy)-y0)*(y-z*(wy)-y0)-R*R),

define fespace fescomp
       -type=xh1fespace
       -spacetime 
       -type_space=h1ho
       -order_space=1
       -order_time=1
       -t0=0.0
       -t1=0.01
       -dirichlet=[1,2]
       -vmax=1.0
       -ref_space=0
       -ref_time=0
#       -gaussradau
       # -dgjumps

numproc informxfem npix 
        -xh1fespace=fescomp
        -coef_levelset=coef_lset

define fespace fesx
       -type=xfespace
       -spacetime
       -t0=0.0
       -t1=0.01
       #-levelset=(x-y+z-0.375)
       #-levelset=((x-0.1*z-0.4)*(x-0.1*z-0.4)+(y-0.1*z-0.4)*(y-0.1*z-0.4)-0.04)
       -vmax=0.1
       -ref_space=0
       -ref_time=0
#       -gaussradau
       # -dgjumps

define fespace fescl 
       -type=lsetcontfespace

define fespace fesnegpos
       -type=compound
       -spaces=[fesh1,fesh1,fescl]

numproc informxfem npix 
        -fespace=fesh1
        -xfespace=fesx
        -lsetcontfespace=fescl
        -coef_levelset=coef_lset

# define fespace fescomp2
#        -type=compound
#        -spaces=[fesh1,fesx]


define gridfunction u -fespace=fescomp
define gridfunction u_vis -fespace=fesnegpos

#define preconditioner c -type=spacetime -bilinearform=bftau -laterupdate -fespace=fescomp


numproc stx_solveinstat npsi 
        -initialneg=binineg
        -initialpos=binipos
        -beta_conv_neg=bconvneg
        -beta_conv_pos=bconvpos
        -beta_rhs_neg=brhsneg
        -beta_rhs_pos=brhspos
        -beta_ini_neg=binineg
        -beta_ini_pos=binipos
        -boundary_neg=bndneg
        -boundary_pos=bndpos
        -gf_vis=u_vis
        -gridfunction=u
        -solver=pardiso
        -fespace=fescomp
        -fespacevis=fesnegpos
        -dt=0.001
        -tend=2.0
        -userstepping
        -aneg=0.2
        -apos=0.1
        -bneg=1.0
        -bpos=1.0
        -lambda=20.0
        # -pause_after_step=1
        -solution_n=zero
        -solution_p=zero
        #-direct
        # -ghostpenalty
#        -delta=0.005
#      -minimal_stabilization

# define bilinearform evalx_past -fespace=fescomp -nonassemble
# stxvis_past one

define bilinearform evalx_future -fespace=fescomp -nonassemble
stxvis_future one

# define bilinearform evalx_neg -fespace=fesnegpos -nonassemble
# STtracefuture one -comp=1

# define bilinearform evalx_pos -fespace=fesnegpos -nonassemble
# STtracefuture one -comp=2

# numproc drawflux npdf_past -solution=u -bilinearform=evalx_past -label=u_past -applyd
numproc drawflux npdf_future -solution=u -bilinearform=evalx_future -label=u_future -applyd

# numproc drawflux npdf_past -solution=u_vis -bilinearform=evalx_neg -label=u_neg -applyd
# numproc drawflux npdf_future -solution=u_vis -bilinearform=evalx_pos -label=u_pos -applyd

# numproc visualization npviz 
#         -scalarfunction=u_future 
#         -subdivision=3
#         -nolineartexture        
        # -deformationscale=0.3 
        # -subdivision=2 
        # -minval=0.0 
        # -maxval=1.0

define bilinearform eval_negpos -fespace=fesnegpos -nonassemble
st_np_vis_future one

define bilinearform eval_negpos_past -fespace=fesnegpos -nonassemble
st_np_vis_past one

numproc drawflux npdf_np -solution=u_vis -bilinearform=eval_negpos -label=u_negpos -applyd
numproc drawflux npdf_np_past -solution=u_vis -bilinearform=eval_negpos_past -label=u_negpos_past -applyd

numproc visualization npvis -scalarfunction=u_negpos -nolineartexture -deformationscale=0.25 -subdivision=1 -minval=0.0 -maxval=1.0