#pragma once

/// from ngxfem
#include "../xfem/xFESpace.hpp"

using namespace ngsolve;
// using namespace cutinfo;

namespace ngcomp
{

  class SFESpace : public FESpace
  {
  protected:
    int ndof=0;
    int order;
    BitArray activeelem;
    shared_ptr<CoefficientFunction> coef_lset = NULL;
    Array<int> firstdof_of_el;
    Array<Mat<2>> cuts_on_el;
  public:
    SFESpace (shared_ptr<MeshAccess> ama,
              shared_ptr<CoefficientFunction> a_coef_lset,
              int aorder,
              const Flags & flags);

    virtual ~SFESpace(){};

    // a name for our new fe-space
    virtual string GetClassName () const
    {
      return "SFESpace ( experimental and 2D )";
    }

    /// update element coloring
    virtual void FinalizeUpdate(LocalHeap & lh)
    {
      if ( coef_lset == NULL )
      {
        cout << " no lset, FinalizeUpdate postponed " << endl;
        return;
      }
      FESpace::FinalizeUpdate (lh);
    }

    virtual size_t GetNDof () const { return ndof; }

    virtual void GetDofNrs (ElementId ei, Array<int> & dnums) const;
    // virtual void UpdateCouplingDofArray();
    virtual bool DefinedOn (ElementId id) const
    {
      if (activeelem.Size() == 0)
        return false;
      if (id.IsBoundary())
        return false;
      else
        return activeelem.Test(id.Nr());
    }

    bool IsElementCut(int elnr) const { return activeelem.Test(elnr); }
    const BitArray & CutElements() const { return activeelem; }

    virtual void Update(LocalHeap & lh);


    virtual FiniteElement & GetFE (ElementId ei, Allocator & alloc) const;

  };

}
