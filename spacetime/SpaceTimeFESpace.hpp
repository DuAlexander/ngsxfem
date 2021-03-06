#ifndef FILE_SPACETIMEFESPACE_HPP
#define FILE_SPACETIMEFESPACE_HPP

// SpaceTimeFESpace based on:

/*********************************************************************/
/* File:   myFESpace.hpp                                             */
/* Author: Joachim Schoeberl                                         */
/* Date:   26. Apr. 2009                                             */
/*********************************************************************/

#include <comp.hpp>    // provides FESpace, ...
#include <h1lofe.hpp>

#include <fem.hpp>

namespace ngcomp
{

  class SpaceTimeFESpace : public FESpace
  {
    int ndof;
    FESpace* Vh;
    shared_ptr<FESpace> Vh_ptr;
    ScalarFiniteElement<1>* tfe;
    double time;
    bool override_time = false;

  public:

    SpaceTimeFESpace (shared_ptr<MeshAccess> ama, shared_ptr<FESpace> aVh, shared_ptr<ScalarFiniteElement<1>> atfe, const Flags & flags);

    //virtual FESpace get_V() const { return *Vh; }
    // FESpace* GetSpaceFESpace() { return Vh; }

    // destructor
    virtual ~SpaceTimeFESpace ();

    virtual string GetClassName () const
    {
      return "SpaceTimeFESpace";
    }


    virtual void Update(LocalHeap & lh);
    virtual size_t GetNDof () const { return ndof; }
    
    virtual void GetDofNrs (ElementId ei, Array<DofId> & dnums) const;
    virtual FiniteElement & GetFE (ElementId ei, Allocator & alloc) const;
    FiniteElement* GetTimeFE() { return tfe; }

    // For debugging
    void SetTime(double a) {time = a; override_time = true;}
    void SetOverrideTime(bool a) {override_time = a;}
    // Provide Info for Python
    int order_time() const
    {    
      NodalTimeFE* time_FE = dynamic_cast< NodalTimeFE*>(tfe);
      return time_FE->order_time();
    }

    Array<double>& TimeFE_nodes() const
    { 
      NodalTimeFE* time_FE = dynamic_cast< NodalTimeFE*>(tfe);
      return time_FE->GetNodes();
    }

    bool IsTimeNodeActive(int i) const
    {
      NodalTimeFE* time_FE = dynamic_cast< NodalTimeFE*>(tfe);
      return time_FE->IsNodeActive(i);
    }

    template<typename SCAL>
    void RestrictGFInTime(shared_ptr<GridFunction> st_GF, double time, shared_ptr<GridFunction> s_GF);
    shared_ptr<GridFunction> CreateRestrictedGF( shared_ptr<GridFunction> st_GF, double time);
    void InterpolateToP1(shared_ptr<CoefficientFunction> st_CF, shared_ptr<CoefficientFunction> tref, double t, double dt, shared_ptr<GridFunction> st_GF);

  };

}    

#endif
