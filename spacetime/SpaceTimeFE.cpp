/*********************************************************************/
/* File:   SpaceTimeFE.cpp (based on mylittlengsolve)                */
/* Author: Janosch Preuss                                            */
/* Date:   June 2017                                                 */
/*********************************************************************/

#include <fem.hpp>
#include "SpaceTimeFE.hpp"


namespace ngfem
{


   SpaceTimeFE :: SpaceTimeFE (ScalarFiniteElement<2>* s_FE, ScalarFiniteElement<1>* t_FE, bool aoverride_time, double atime)
    /*
      Call constructor for base class:
      number of dofs is (dofs in space) * (Dofs in time), maximal order is order
     */
      : ScalarFiniteElement<2> ((s_FE->GetNDof())*(t_FE->GetNDof()), s_FE->Order())
    {

        sFE = s_FE;
        tFE = t_FE;
        time = atime;
        override_time = aoverride_time;
    }

    void SpaceTimeFE :: CalcShape (const IntegrationPoint & ip,
                                    BareSliceVector<> shape) const
    {

       if (tFE->Order() == 0)
          sFE->CalcShape(ip,shape);
       else
       {

            Vector<> time_shape(tFE->GetNDof());
            IntegrationPoint z(override_time ? time : ip(2));
            tFE->CalcShape(z,time_shape);

            Vector<> space_shape(sFE->GetNDof());
            sFE->CalcShape(ip,space_shape);

            // define shape functions
            int ii = 0;
            for(int j=0; j<tFE->GetNDof(); j++) {
              for(int i=0; i<sFE->GetNDof(); i++) {
                shape(ii++) = space_shape(i)*time_shape(j);
              }
            }
       }
     }

    void SpaceTimeFE :: CalcDShape (const IntegrationPoint & ip,
                                    BareSliceMatrix<> dshape) const

    {
      // matrix of derivatives:

         if (tFE->Order() == 0)
            sFE->CalcDShape(ip,dshape);
         else {

            Vector<> time_shape(tFE->GetNDof());
            IntegrationPoint z(override_time ? time : ip(2));
            tFE->CalcShape(z,time_shape);

            Matrix<double> space_dshape(sFE->GetNDof(),2);
            sFE->CalcDShape(ip,space_dshape);


            int ii = 0;
            for(int j = 0; j < tFE->GetNDof(); j++) {
                for(int i=0; i< sFE->GetNDof(); i++) {
                    dshape(ii,0) = space_dshape(i,0)*time_shape(j);
                    dshape(ii,1) = space_dshape(i,1)*time_shape(j);
                    ii++;
                }
            }
         }

    }

    // for time derivatives

    void SpaceTimeFE :: CalcDtShape (const IntegrationPoint & ip,
                                     BareSliceVector<> dshape) const

    {
        // matrix of derivatives:

           Matrix<double> time_dshape(tFE->GetNDof(),1);
           IntegrationPoint z(override_time ? time : ip(2));
           tFE->CalcDShape(z,time_dshape);

           Vector<> space_shape(sFE->GetNDof());
           sFE->CalcShape(ip,space_shape);

           int ii = 0;
           for(int j = 0; j < tFE->GetNDof(); j++) {
              for(int i=0; i< sFE->GetNDof(); i++) {
                 dshape(ii++) = space_shape(i)*time_dshape(j,0);
              }
           }

    }


    NodalTimeFE :: NodalTimeFE (int order, bool askip_first_node, bool aonly_first_node)
        : ScalarFiniteElement<1> (askip_first_node ? order : (aonly_first_node ? 1 : order + 1), order), 
        skip_first_node(askip_first_node), only_first_node(aonly_first_node)
      {
         k_t = order;
         CalcInterpolationPoints ();
      }


      void NodalTimeFE :: CalcShape (const IntegrationPoint & ip,
                                     BareSliceVector<> shape) const
      {
         AutoDiff<1> adx (ip(0), 0);
         int begin = skip_first_node ? 1 : 0;
         int end = only_first_node ? 1 : ndof+begin;
         int cnt = 0;
         for(int i = begin; i < end; i++) {
             shape(cnt++) = Lagrange_Pol (adx, i).Value() ;
         }
      }


      void NodalTimeFE :: CalcDShape (const IntegrationPoint & ip,
                                      BareSliceMatrix<> dshape) const
      {
         AutoDiff<1> adx (ip(0), 0);
         int begin = skip_first_node ? 1 : 0;
         int end = only_first_node ? 1 : ndof+begin;
         int cnt = 0;
         for(int i = begin; i < end; i++) {
             dshape(cnt++,0) = Lagrange_Pol(adx, i).DValue(0);
          }
      }

      void NodalTimeFE :: CalcInterpolationPoints ()
      {
         nodes.SetSize(order+1);
         switch (order)
         {
          // Gauss-Lobatto integration points (Spectral FE)
          case 0 : nodes[0] = 0.0;  break;
          case 1 : nodes[0] = 0.0; nodes[1] = 1.0;  break;
          case 2 : nodes[0] = 0.0; nodes[1] = 0.5; nodes[2] = 1.0;  break;
          case 3 : nodes[0] = 0.0; nodes[1] = 0.5*(1.0-1.0/sqrt(5.0));
                   nodes[2] = 0.5*(1.0+1.0/sqrt(5.0)); nodes[3] = 1.0;  break;
          case 4 : nodes[0] = 0.0; nodes[1] = 0.5*(1.0-sqrt(3.0/7.0)); nodes[2] = 0.5;
                   nodes[3] = 0.5*(1.0+sqrt(3.0/7.0)); nodes[4] = 1.0;  break;
          case 5 : nodes[0] = 0.0;
                   nodes[1] = 0.5*(1.0 - sqrt(1.0/3.0 + 2.0*sqrt(7.0)/21.0));
                   nodes[2] = 0.5*(1.0 - sqrt(1.0/3.0 - 2.0*sqrt(7.0)/21.0));
                   nodes[3] = 0.5*(1.0 + sqrt(1.0/3.0 - 2.0*sqrt(7.0)/21.0));
                   nodes[4] = 0.5*(1.0 + sqrt(1.0/3.0 + 2.0*sqrt(7.0)/21.0));
                   nodes[5] = 1.0;  break;
          default : throw Exception("Requested TimeFE not implemented yet.");
         }
      }


}
