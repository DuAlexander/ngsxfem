from math import pi
# ngsolve stuff
from ngsolve import *

from xfem.utils import *

h = specialcf.mesh_size

# 3D: sphere configuration
def Make3DProblem_PureDiffusion():
    from netgen.csg import CSGeometry, OrthoBrick, Pnt
    cube = CSGeometry()
    cube.Add (OrthoBrick(Pnt(-1.41,-1.41,-1.41), Pnt(1.41,1.41,1.41)))
    # mesh = Mesh (cube.GenerateMesh(maxh=0.5, quad_dominated=False))
    mesh = Mesh (cube.GenerateMesh(maxh=1, quad_dominated=False))
    mesh.Refine()
    
    a = 1
    c = 0.0

    problem = {"Diffusion" : a,
               "Convection" : None,
               "Reaction" : c,
               "Source" : (sin(pi*z)*(a*pi*pi*(1-z*z)+c)+a*cos(pi*z)*2*pi*z),
               "SourceMeanValue" : 0.0,
               "Solution" : sin(pi*z),
               "GradSolution" : CoefficientFunction((pi*cos(pi*z)*(-x*z),pi*cos(pi*z)*(-y*z),pi*cos(pi*z)*(1-z*z))),
               "VolumeStabilization" : a/h+c*h,
               "Levelset" : sqrt(x*x+y*y+z*z)-1,
               "GradLevelset" : CoefficientFunction((x,y,z)),
               "Lambda" : 10,
               "Iterative" : True,
               "Order" : 2,
               "Mesh" : mesh,
               "StaticCondensation" : False,
               "HDG": False,
               # "checkDGpattern" : True,
               # "checkCGpattern" : True,
               # "checkCGGPpattern" : True,
               # "checkHDGpattern" : True,
    }
    return problem;

# 3D: sphere configuration
def Make3DProblem_DiffusionReactionSphere():
    from netgen.csg import CSGeometry, OrthoBrick, Pnt
    cube = CSGeometry()
    cube.Add (OrthoBrick(Pnt(-1.41,-1.41,-1.41), Pnt(1.41,1.41,1.41)))
    # mesh = Mesh (cube.GenerateMesh(maxh=0.5, quad_dominated=False))
    mesh = Mesh (cube.GenerateMesh(maxh=1, quad_dominated=False))
    mesh.Refine()
    
    a = 1
    c = 1
    
    problem = {"Diffusion" : a,
               "Convection" : None,
               "Reaction" : c,
               "Source" : (sin(pi*z)*(a*pi*pi*(1-z*z)+c)+a*cos(pi*z)*2*pi*z),
               "Solution" : sin(pi*z),
               "GradSolution" : CoefficientFunction((pi*cos(pi*z)*(-x*z),pi*cos(pi*z)*(-y*z),pi*cos(pi*z)*(1-z*z))),
               "VolumeStabilization" : a/h+c*h,
               "Levelset" : LevelsetExamples["sphere"],
               "GradLevelset" : CoefficientFunction((x,y,z)),
               "Lambda" : 10,
               "Iterative" : False,
               "Order" : 2,
               "Mesh" : mesh,
               "StaticCondensation" : False,
               "HDG": False,
               # "checkDGpattern" : True,
               # "checkCGpattern" : True,
               # "checkCGGPpattern" : True,
               # "checkHDGpattern" : True,
    }
    return problem;

def Make3DProblem_Convection():
    from netgen.csg import CSGeometry, OrthoBrick, Pnt
    cube = CSGeometry()
    cube.Add (OrthoBrick(Pnt(-1.41,-1.41,-1.41), Pnt(1.41,1.41,1.41)))
    # mesh = Mesh (cube.GenerateMesh(maxh=0.5, quad_dominated=False))
    mesh = Mesh (cube.GenerateMesh(maxh=1, quad_dominated=False))
    mesh.Refine()

    eps = 0.025
    c = 1
    problem = {"Diffusion" : 0.0,
               "Convection" : CoefficientFunction((-y*sqrt(1-z*z),x*sqrt(1-z*z),0)),
               "Reaction" : c,
               # "Source" : (12 * eps*sqrt(eps)*x*y*z/(eps+4*y*y) + 16 *eps*sqrt(eps)*(1-z*z)*x*y*z/((eps+4*z*z)*(eps+4*z*z)) + (6*eps*x*y+sqrt(x*x+y*y)*(x*x-y*y)+x*y)*atan(2*z/sqrt(eps))),
               "Source" : (sqrt(x*x+y*y)*(x*x-y*y)+x*y)*atan(2*z/sqrt(eps)) ,
               "Solution" : x*y*atan(2*z/sqrt(eps)),
               "GradSolution" : None,
               "VolumeStabilization" : 1+eps/h+c*h,
               "Levelset" : sqrt(x*x+y*y+z*z)-1,
               "GradLevelset" : CoefficientFunction((x,y,z)),
               "Lambda" : 10,
               "Iterative" : False,
               "Order" : 2,
               "Mesh" : mesh,
               "StaticCondensation" : True,
               "HDG": True,
               # "checkDGpattern" : True,
               # "checkHDGpattern" : True,
               # "checkCGpattern" : True,
               # "checkCGGPpattern" : True
    }
    return problem;


def Make3DProblem():
    return Make3DProblem_Diffusion()
    # return Make3DProblem_Convection()

# torus with parameters as in 'Grande, Reusken, A higher order finite element method for partial differential euqations on surface, SINUM, 2016'
def Make3DProblem_Torus():
    from netgen.csg import CSGeometry, OrthoBrick, Pnt
    cube = CSGeometry()
    cube.Add (BoundingBoxes["torus"])
    # mesh = Mesh (cube.GenerateMesh(maxh=0.5, quad_dominated=False))
    mesh = Mesh (cube.GenerateMesh(maxh=5, quad_dominated=False))
    mesh.Refine()
    mesh.Refine()
    
    a = 1
    c = 0
    def atan2(y,x):
        return IfPos(x,atan(y/x),IfPos(y,pi+atan(y/x),-pi+atan(y/x)))

    R = 1.0
    r = 0.6
    theta = atan2(z,sqrt(x*x+y*y)-R)
    phi = atan2(y,x)

    dudphi = 3.0*cos(3.0*phi)*cos(3*theta+phi)-sin(3*phi)*sin(3*theta+phi)
    dudthe = -3.0*sin(3*phi)*sin(3*theta+phi)
    dphidx = -y/(x*x+y*y)
    dphidy = x/(x*x+y*y)
    dphidz = CoefficientFunction(0.0)
    dthedx = -x*z/(r*r*sqrt(x*x+y*y))
    dthedy = -y*z/(r*r*sqrt(x*x+y*y))
    dthedz = (sqrt(x*x+y*y)-R)/(r*r)
    
    problem = {"Diffusion" : a,
               "Convection" : None,
               "Reaction" : c,
               "Source" : (1.0/(r*r)*(9.0*sin(3.0*phi)*cos(3.0*theta+phi)) - 1.0/((R + r*cos(theta))*(R + r*cos(theta))) * (-10.0*sin(3.0*phi)*cos(3.0*theta+phi)-6.0*cos(3*phi)*sin(3.0*theta+phi))
               - (1.0/(r*(R+r*cos(theta))))*(3.0*sin(theta)*sin(3.0*phi)*sin(3.0*theta+phi))).Compile(),
               "Solution" : sin(3.0*phi)*cos(3*theta+phi),
               "GradSolution" : CoefficientFunction(( (dudphi*dphidx+dudthe*dthedx).Compile(),
                                                      (dudphi*dphidy+dudthe*dthedy).Compile(),
                                                      (dudphi*dphidz+dudthe*dthedz).Compile())),
               "VolumeStabilization" : a/h+c*h,
               "Levelset" : LevelsetExamples["torus"],
               "GradLevelset" : CoefficientFunction((x,y,z)),
               "Lambda" : 10,
               "Iterative" : True,
               "Order" : 2,
               "Mesh" : mesh,
               "StaticCondensation" : True,
               "HDG": False,
               # "checkDGpattern" : True,
               # "checkCGpattern" : True,
               # "checkCGGPpattern" : True,
               # "checkHDGpattern" : True,
    }
    return problem;

# 3D: sphere configuration
def MakeSPPProblem():
    from netgen.csg import CSGeometry, OrthoBrick, Pnt
    cube = CSGeometry()
    cube.Add (OrthoBrick(Pnt(-1.41,-1.41,-1.41), Pnt(1.41,1.41,1.41)))
    # mesh = Mesh (cube.GenerateMesh(maxh=0.5, quad_dominated=False))
    mesh = Mesh (cube.GenerateMesh(maxh=1, quad_dominated=False))
    mesh.Refine()
    
    a = 1
    c = 0.0

    problem = {"Diffusion" : a,
               "Convection" : None,
               "Reaction" : 0.0,
               "Source" : (sin(pi*z)*(a*pi*pi*(1-z*z)+c)+a*cos(pi*z)*2*pi*z),
               "SourceMeanValue" : 0.0,
               "Solution" : sin(pi*z),
               "GradSolution" : CoefficientFunction((pi*cos(pi*z)*(-x*z),pi*cos(pi*z)*(-y*z),pi*cos(pi*z)*(1-z*z))),
               "VolumeStabilization" : None, #a/h+c*h,
               "Levelset" : sqrt(x*x+y*y+z*z)-1,
               "GradLevelset" : CoefficientFunction((x,y,z)),
               "Lambda" : 10,
               "Iterative" : True,
               "Order" : 2,
               "Mesh" : mesh,
               "StaticCondensation" : False,
               "HDG": False,
               # "checkDGpattern" : True,
               # "checkCGpattern" : True,
               # "checkCGGPpattern" : True,
               # "checkHDGpattern" : True,
    }
    return problem;