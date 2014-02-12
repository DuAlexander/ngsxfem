/*********************************************************************/
/* File:   npxtest.cpp                                               */
/* Author: Christoph Lehrenfeld                                      */
/* Date:   2. Feb. 2014                                              */
/*********************************************************************/


/*
 */

#include <solve.hpp>
#include "xintegration.hpp"
#include "../common/spacetimefespace.hpp"

using namespace ngsolve;
using namespace xintegration;

typedef std::pair<double,double> TimeInterval;

// /// A space-time integration point 
// template <int DIMS, int DIMR, typename SCAL = double>
// class SpaceTimeMappedIntegrationPoint : public MappedIntegrationPoint<DIMS,DIMR,SCAL>
// {
// protected:
//     /// reference time 
//     double tref; 
//     /// time interval
//     TimeInterval  ti;
// public:
//     ///
//     SpaceTimeMappedIntegrationPoint (const IntegrationPoint & aip,
//                                      const ElementTransformation & aeltrans,
//                                      const double & atref,
//                                      const TimeInterval & ati)
//         : MappedIntegrationPoint<DIMS,DIMR,SCAL>(aip,aeltrans), tref(atref), ti(ati)  { ; }
//     ///
//     double GetTime() const { return (1.0-tref) * ti.first + tref * ti.second; }
// };

template <ELEMENT_TYPE ET, int DIM>
ScalarFiniteElement<DIM> * GetSpaceFE (const Ngs_Element & ngel, int order, LocalHeap & lh)
{
    L2HighOrderFE<ET> * hofe =  new (lh) L2HighOrderFE<ET> ();
    
    hofe -> SetVertexNumbers (ngel.vertices);
    hofe -> SetOrder (order);
    hofe -> ComputeNDof();
    
    return hofe;
}

/* ---------------------------------------- 
   numproc
   ---------------------------------------- */
template<int D>
class NumProcTestXFEM : public NumProc
{
protected:
    bool isspacetime;
    
    int order_space;
    int order_time;

    int num_int_ref_space;
    int num_int_ref_time;

    GridFunction * gf_lset;
public:
    
    NumProcTestXFEM (PDE & apde, const Flags & flags)
        : NumProc (apde)
    {
        cout << " \n\nNumProcTestXFEM - constructor start \n\n " << endl;

		gf_lset = pde.GetGridFunction (flags.GetStringFlag ("levelset", "lset"));
        isspacetime = flags.GetDefineFlag("spacetime");
        num_int_ref_space = (int) flags.GetNumFlag("num_int_ref_space",0);
        num_int_ref_time = (int) flags.GetNumFlag("num_int_ref_time",0);
        const FESpace & fes = gf_lset->GetFESpace();
        if (isspacetime)
        {
            const SpaceTimeFESpace & fes_st = dynamic_cast< const SpaceTimeFESpace & > (gf_lset->GetFESpace());
            order_space = 2*fes_st.OrderSpace();
            order_time = 2*fes_st.OrderTime();
        }
        else
        {
            order_time = 0;
            order_space = 2 * fes.GetOrder();
        }

        cout << " \n\nNumProcTestXFEM - constructor end \n\n " << endl;
    }
  
    ~NumProcTestXFEM()
    {
    }

    virtual string GetClassName () const
    {
        return "NumProcTestXFEM";
    }
  

    virtual void Do (LocalHeap & lh)
    {
        HeapReset hr(lh);

        Array<int> els_of_dt(3);
        els_of_dt = 0.0;

        Array<double> meas_of_dt(3);
        meas_of_dt = 0.0;

        for (int elnr = 0; elnr < ma.GetNE(); ++elnr)
        {
            HeapReset hr(lh);
            // Ngs_Element ngel = ma.GetElement(elnr);

            ElementTransformation & eltrans = ma.GetTrafo (ElementId(VOL,elnr), lh);
            ELEMENT_TYPE et_space = eltrans.GetElementType();
            ELEMENT_TYPE et_time = isspacetime ? ET_SEGM : ET_POINT;
            
            IntegrationPoint ip(0.0);
            MappedIntegrationPoint<D,D> mip(ip,eltrans);
            const double absdet = mip.GetJacobiDet();

            const FESpace & fes = gf_lset->GetFESpace();
            const FiniteElement & fel = fes.GetFE(elnr, lh);
            Array<int> dnums;
            fes.GetDofNrs(elnr,dnums);
            
            FlatVector<> linvec(dnums.Size(),lh);
            gf_lset->GetVector().GetIndirect(dnums,linvec);

            if( et_space == ET_TRIG && et_time == ET_SEGM)
            {
                const ScalarSpaceTimeFiniteElement<2> &  fel_st 
                    = dynamic_cast<const ScalarSpaceTimeFiniteElement<2> & >(fel);
                ScalarSpaceTimeFEEvaluator<2> lset_eval(fel_st, linvec, lh);

                PointContainer<3> pc;
                CompositeQuadratureRule<3> compositerule;
                NumericalIntegrationStrategy<ET_TRIG,ET_SEGM> numint(lset_eval, pc,
                                                                     compositerule, 
                                                                     lh,
                                                                     order_space, order_time,
                                                                     num_int_ref_space, 
                                                                     num_int_ref_time);
                numint.SetVerticesSpace();
                numint.SetVerticesTime();

                els_of_dt[numint.MakeQuadRule()]++;
                
                for (int i = 0; i < compositerule.quadrule_pos.Size(); ++i)
                    meas_of_dt[POS] += absdet * compositerule.quadrule_pos.weights[i];
                for (int i = 0; i < compositerule.quadrule_neg.Size(); ++i)
                    meas_of_dt[NEG] += absdet * compositerule.quadrule_neg.weights[i];
                for (int i = 0; i < compositerule.quadrule_if.Size(); ++i)
                    meas_of_dt[IF] += compositerule.quadrule_if.weights[i];
                
            }
            else if ( et_space == ET_TET && et_time == ET_SEGM)
            {
                const ScalarSpaceTimeFiniteElement<3> &  fel_st 
                    = dynamic_cast<const ScalarSpaceTimeFiniteElement<3> & >(fel);
                ScalarSpaceTimeFEEvaluator<3> lset_eval(fel_st, linvec, lh);

                PointContainer<4> pc;
                CompositeQuadratureRule<4> compositerule;
                NumericalIntegrationStrategy<ET_TET,ET_SEGM> numint(lset_eval, pc,
                                                                    compositerule, 
                                                                    lh,
                                                                    order_space, order_time,
                                                                    num_int_ref_space, 
                                                                    num_int_ref_time);
                numint.SetVerticesSpace();
                numint.SetVerticesTime();

                els_of_dt[numint.MakeQuadRule()]++;

                for (int i = 0; i < compositerule.quadrule_pos.Size(); ++i)
                    meas_of_dt[POS] += compositerule.quadrule_pos.weights[i];
                for (int i = 0; i < compositerule.quadrule_neg.Size(); ++i)
                    meas_of_dt[NEG] += compositerule.quadrule_neg.weights[i];
                for (int i = 0; i < compositerule.quadrule_if.Size(); ++i)
                    meas_of_dt[IF] += compositerule.quadrule_if.weights[i];

            }
            
        }

        cout << " pos elements : " << els_of_dt[POS] << endl;
        cout << " neg elements : " << els_of_dt[NEG] << endl;
        cout << " cut elements : " << els_of_dt[IF] << endl;
        cout << " pos measure : " << meas_of_dt[POS] << endl;
        cout << " neg measure : " << meas_of_dt[NEG] << endl;
        cout << " cut measure : " << meas_of_dt[IF] << endl;

    }
};

static RegisterNumProc<NumProcTestXFEM<2> > npinittestxfem2d("testxfem");
static RegisterNumProc<NumProcTestXFEM<3> > npinittestxfem3d("testxfem");