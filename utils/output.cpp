#include "output.hpp"

#include<stdlib.h>

namespace ngfem
{

  void MakeTikzHeader(ofstream & outf_tikz)
  {
    outf_tikz << "\\documentclass{standalone}" << endl;

    outf_tikz << "\\usepackage{pgfplots}" << endl;
    outf_tikz << "\\usetikzlibrary{shapes,arrows,snakes,calendar,matrix,backgrounds,folding,calc,positioning,spy}" << endl;
    outf_tikz << "\\usepackage{tikz,tikz-3dplot}" << endl;
    // outf_tikz << "\\usepackage{tikz}" << endl;
    // outf_tikz << "\\usepackage{tikz-3dplot}" << endl;
    outf_tikz << "\\begin{document}" << endl;
    outf_tikz << "\\tdplotsetmaincoords{60}{120}" << endl;
    outf_tikz << "\\begin{tikzpicture}[scale=100,tdplot_main_coords,spy using outlines={white, circle, magnification=4, size=25 * 3 *\\myscale, connect spies, transform shape}]" << endl;
    outf_tikz << "\\def\\mycolorstring{black}" << endl;
    outf_tikz << "\\def\\mydrawopacity{0.5}" << endl;
    outf_tikz << "\\def\\myfillopacity{1.0}" << endl;
  }

  void MakeTikzFooter(ofstream & outf_tikz)
  {
    outf_tikz << "\\end{tikzpicture}" << endl;
    outf_tikz << "\\end{document}" << endl;
  }

  void MakeSketchHeader(ofstream & outf_sketch)
  {
    outf_sketch << "def tetrasandlines {\n";
  }

  void MakeSketchFooter(ofstream & outf_sketch, Flags & flags)
  {
    string viewpoint(flags.GetStringFlag("viewpoint","(10.0,10.0,10.0)"));
    string lookat(flags.GetStringFlag("lookat","(0.0,0.0,0.0)"));

    outf_sketch << "}\n"
                << "def viewpoint " << viewpoint << "\n"
                << "def lookat " << lookat << "\n"
                << "put{ scale(2) then view((viewpoint),(lookat))}{\n"
                << "  {tetrasandlines}\n"
                << "}\n"
                << "special |\\tikzstyle{transparent cone} = [fill=blue!20,fill opacity=0.8]|[lay=under]\n"
                << "global { language tikz }\n";
  }


  template<int D>
  bool OnBound( Vec<D> p )
  {
    bool ret;
    // std::cout << " p = " << p << std::endl;
    if ( 
      (p[0] < 1e-14)
      ||(p[1] < 1e-14)
      ||((1-p[0]-p[1]) < 1e-14)
      )
      ret = true;
    else
      ret = false;

    // if (ret)
    //   std::cout << " true " << std::endl;
    // else
    //   std::cout << " false " << std::endl;

    return ret;
  }

  template<int D>
  void DoSpecialOutput (shared_ptr<GridFunction> gfu, 
                        SolutionCoefficients<D> & solcoef, 
                        int subdivision, 
                        Flags & flags,
                        LocalHeap & lh)
  {
    ofstream outf_gnuplot("special.output.gnuplot");
    ofstream outf_tikz("special.output.tikz");
    ofstream outf_sketch("special.output.sketch");

    bool showerror = flags.GetDefineFlag("showerror");

    int cnt_sketch = 1;
    double scalex_sketch = flags.GetNumFlag("scalex",10.0);
    double scaley_sketch = flags.GetNumFlag("scaley",10.0);
    double scalez_sketch = flags.GetNumFlag("scalez",10.0);

    MakeTikzHeader(outf_tikz);
    MakeSketchHeader(outf_sketch);
    
    outf_tikz.precision(12);
    outf_tikz << std::fixed;

    string fineedgenegcolor (flags.GetStringFlag("fineedgenegcolor","orange")); //("black");
    string fineedgenegstyle (flags.GetStringFlag("fineedgenegstyle","dashed")); //("black");
    string edgenegcolor (flags.GetStringFlag("edgenegcolor","white!20!black")); //("black");
    string edgenegstyle (flags.GetStringFlag("edgenegstyle","solid")); //("black");
    // string negcolor ("green!60!black");
    string negcolor (flags.GetStringFlag("negcolor","white!20!black"));

    string fineedgeposcolor (flags.GetStringFlag("fineedgeposcolor","orange")); //("black");
    string fineedgeposstyle (flags.GetStringFlag("fineedgeposstyle","thin")); //("black");
    string edgeposcolor (flags.GetStringFlag("edgeposcolor","white!70!black")); // ("black");
    string edgeposstyle (flags.GetStringFlag("edgeposstyle","solid")); //("black");
    // string poscolor ("blue!60!black");
    string poscolor (flags.GetStringFlag("poscolor","white!70!black"));

    double fineedgenegopacity = flags.GetNumFlag("fineedgenegopacity",1.0);
    double fineedgeposopacity = flags.GetNumFlag("fineedgeposopacity",0.5);
    double fillnegopacity = flags.GetNumFlag("fillnegopacity",1.0);
    double fillposopacity = flags.GetNumFlag("fillposopacity",0.5);
    double drawnegopacity = flags.GetNumFlag("drawnegopacity",1.0);
    double drawposopacity = flags.GetNumFlag("drawposopacity",0.0);

    double myeps = flags.GetNumFlag("overlapeps",0.0); //0.01


    const double time = 0.0;
    // cout << " CalcXError at time = " << time << endl;
    Array<int> dnums;
    // int activeels = 0;
    shared_ptr<MeshAccess> ma (gfu->GetFESpace()->GetMeshAccess());

    DOMAIN_TYPE plotdt = POS;
    for (plotdt=POS; plotdt<IF; plotdt=(DOMAIN_TYPE)((int)plotdt+1))
    {
      Array<Vec<D> > edges(0);
      Array<Vec<D> > medges(0);
      Array<double > edges_val(0);


      for (int elnr = 0; elnr < ma->GetNE(); ++elnr)
      {
        HeapReset hr(lh);
        gfu -> GetFESpace()->GetDofNrs (elnr, dnums);
        const int size = dnums.Size();

        FlatVector<double> elvec (size, lh);
        gfu -> GetVector().GetIndirect (dnums, elvec);   

        ElementTransformation & eltrans = ma->GetTrafo(elnr,false,lh);

        ELEMENT_TYPE eltype = eltrans.GetElementType();
      
        const FiniteElement & base_fel = gfu -> GetFESpace()->GetFE(elnr,lh);
        const CompoundFiniteElement & cfel = 
          dynamic_cast<const CompoundFiniteElement&> (base_fel);

        const XFiniteElement * xfe = NULL;
        const XDummyFE * dummfe = NULL;
        const ScalarFiniteElement<D> * scafe = NULL;
        const ScalarSpaceTimeFiniteElement<D> * scastfe = NULL;

        for (int j = 0; j < cfel.GetNComponents(); ++j)
        {
          if (xfe==NULL)
            xfe = dynamic_cast<const XFiniteElement* >(&cfel[j]);
          if (dummfe==NULL)
            dummfe = dynamic_cast<const XDummyFE* >(&cfel[j]);
          if (scafe==NULL)
            scafe = dynamic_cast<const ScalarFiniteElement<D>* >(&cfel[j]);
          if (scastfe==NULL)
            scastfe = dynamic_cast<const ScalarSpaceTimeFiniteElement<D>* >(&cfel[j]);
        }

        bool spacetime = scastfe != NULL;

        if (spacetime)
          throw Exception(" no spacetime tested yet");

        int ndof_x = xfe!=NULL ? xfe->GetNDof() : 0;
        int ndof = spacetime ? scastfe->GetNDof() : scafe->GetNDof();
        int ndof_total = ndof+ndof_x;
        FlatVector<> shape_total(ndof_total,lh);
        FlatVector<> shape(ndof,&shape_total(0));
        FlatVector<> shapex(ndof_x,&shape_total(ndof));

        FlatMatrixFixWidth<D> dshape_total(ndof_total,lh);
        FlatMatrixFixWidth<D> dshape(ndof,&dshape_total(0,0));
        FlatMatrixFixWidth<D> dshapex(ndof_x,&dshape_total(ndof,0));
        if (xfe)
        {

          const FlatXLocalGeometryInformation & fxgeom( spacetime ? 
                                                        xfe->GetFlatLocalGeometryUpTrace() : 
                                                        xfe->GetFlatLocalGeometry());
          const ScalarFieldEvaluator & lset_eval = *(fxgeom.lset);
          Array< Simplex<D> * > draw_simplices_neg(0);
          Array< Simplex<D> * > draw_simplices_pos(0);
          CompositeQuadratureRule<D> * cquad = new CompositeQuadratureRule<D>() ;
          XLocalGeometryInformation * xgeom = XLocalGeometryInformation::Create(eltype, ET_POINT, lset_eval, 
                                                                                *cquad, lh, 
                                                                                0,0, 
                                                                                subdivision,0);

          xgeom->SetSimplexArrays(draw_simplices_neg, draw_simplices_pos);
          xgeom->MakeQuadRule();
          DOMAIN_TYPE dt = POS;
          for (dt=POS; dt<IF; dt=(DOMAIN_TYPE)((int)dt+1))
          {

            if (plotdt != dt) continue;
            // std::cout << " draw_simplices.Size() = " << draw_simplices.Size() << std::endl;
          
            Array<IntegrationPoint> ips(0);
          
            Array< Simplex<D> * > & draw_simplices( dt==POS ? draw_simplices_pos : draw_simplices_neg);
          
            for (int i = 0; i < draw_simplices.Size(); ++i)
            {

              bool edgeonbound[3];
              // int inner_cnt = 3;
              {
                if (dt == NEG)
                  outf_tikz << "\\draw ["
                            << edgenegcolor << ", " 
                            << "fill=" << negcolor 
                            << ", fill opacity=" << fillnegopacity 
                            << ", draw opacity=" << drawposopacity
                            << "] ";
                else
                  outf_tikz << "\\draw [" 
                            << edgeposcolor << ", "
                            << "fill=" << poscolor 
                            << ", fill opacity=" << fillposopacity 
                            << ", draw opacity=" << drawposopacity
                            << "] ";
              }

              Vec<D> lastp (0.0);
              Vec<D> mlastp (0.0);
              double lastval = 0.0;

              Vec<D> center = 0.0;

              for (int kk = 0; kk < 3; ++kk)
              {
                 center += 1.0/3.0 * *(draw_simplices[i]->p[kk]);
              }
              for (int j = 0; j < D+2; ++j)
              {
                int jj = j<D+1 ? j : 0; 
                // const Vec<D> & p = *(draw_simplices[i]->p[jj]);
                const Vec<D> p = (1.0+myeps) * *(draw_simplices[i]->p[jj]) - myeps * center ;
                IntegrationPoint ip(p);
                MappedIntegrationPoint<D,D> mip(ip, eltrans);
                DimMappedIntegrationPoint<D+1> mipp(ip, eltrans);
                mipp.Point().Range(0,D) = mip.GetPoint();
                mipp.Point()[D] = time;



                //     double solval = dt == POS ? solcoef.GetSolutionPos().Evaluate(mipp) : solcoef.GetSolutionNeg().Evaluate(mipp);

                //     Vec<D> soldval;
                //     if (solcoef.HasSolutionDNeg() && solcoef.HasSolutionDPos())
                //     {
                //       if (dt == POS)
                //         solcoef.GetSolutionDPos().Evaluate(mipp,soldval);
                //       else
                //         solcoef.GetSolutionDNeg().Evaluate(mipp,soldval);
                //     }
                //     else
                //     {
                //       if (dt == POS)
                //         CalcDxShapeOfCoeff<D>(&(solcoef.GetSolutionPos()),mip,time,soldval,lh);
                //       else
                //         CalcDxShapeOfCoeff<D>(&(solcoef.GetSolutionNeg()),mip,time,soldval,lh);
                //     }

                if (!spacetime)
                {
                  shape = scafe->GetShape(mip.IP(), lh);
                  scafe->CalcMappedDShape(mip, dshape);
                }
                else
                {
                  scastfe->CalcShapeSpaceTime(mip.IP(), 1.0, shape, lh);
                  scastfe->CalcMappedDxShapeSpaceTime(mip, 1.0, dshape, lh);
                }

                shapex = shape;
                dshapex = dshape;

                for (int l = 0; l < ndof_x; ++l)
                {
                  if (xfe->GetSignsOfDof()[l] != dt)
                  {
                    shapex(l) = 0.0;
                    dshapex.Row(l) = 0.0;
                  }
                }

                double discval = InnerProduct(shape_total,elvec);

                if (showerror)
                {
                  double solval = dt == POS ? solcoef.GetSolutionPos().Evaluate(mip) : solcoef.GetSolutionNeg().Evaluate(mip);
                  discval = abs(discval-solval);
                }


                if(j != D+1)
                {
                  outf_sketch << "  def p" << cnt_sketch++
                              << "(" << mip.GetPoint()(1) * scalex_sketch 
                              << "," << discval * scalez_sketch 
                              << "," << mip.GetPoint()(0) * scaley_sketch << ")\n";
                  // inner_cnt--;
                  // if (OnBound(p)) 
                  //   inner_on_bound[inner_cnt] = true;
                  // else
                  //   inner_on_bound[inner_cnt] = false;
                }


                if(j != D+1)
                  outf_tikz << "(" << mip.GetPoint()(0) << "," << mip.GetPoint()(1) << "," << discval << ") -- ";
                outf_gnuplot << mip.GetPoint() << "\t" << discval << endl;

                //     Vec<D> discdval = Trans(dshape_total) * elvec;
                //     Vec<D> diffdval = soldval - discdval;
                //     double diffdsqr = L2Norm2(diffdval);

                //     double fac = mip.GetWeight();
                //     if (dt == POS)
                //     {
                //       l2diff_p += b_pos*fac*sqr(discval-solval);
                //       h1diff_p += b_pos*fac*diffdsqr;
                //     }
                //     else
                //     {
                //       l2diff_n += b_neg*fac*sqr(discval-solval);
                //       h1diff_n += b_neg*fac*diffdsqr;
                //     }

                if (j>0)
                {
                  Vec<D> tmp = 0.5 * lastp + 0.5 * p;
                  if (OnBound(p) && OnBound(lastp) && OnBound(tmp))
                  {
                    edges.Append(lastp);
                    edges.Append(p);
                    medges.Append(mlastp);
                    medges.Append(mip.GetPoint());
                    edges_val.Append(lastval);
                    edges_val.Append(discval);
                    edgeonbound[j-1] = true;
                  }
                  else
                    edgeonbound[j-1] = false;
                }
                lastp = p;
                mlastp = mip.GetPoint();
                lastval = discval;
              } //simplex
              outf_gnuplot << endl << endl;
              outf_tikz << " cycle;" << endl;
              string & color = dt == NEG ? negcolor : poscolor;
              string & edgecolor = dt == NEG ? fineedgenegcolor : fineedgeposcolor;
              string & fineedgestyle = dt == NEG ? fineedgenegstyle : fineedgeposstyle;
              double fillopacity = dt == NEG ? fillnegopacity : fillposopacity;
              double drawopacity = dt == NEG ? fineedgenegopacity : fineedgeposopacity;
              outf_sketch << "polygon[" //line style = dashed"
                          << "style=none"  // << "red" // << color 
                          << ",color=" << edgecolor // << "red" // << color 
                          << ",fill=" << color 
                          << ",cull=false,fill opacity=" << fillopacity
                          << ",draw opacity=" << 0 //*0.25
                          << "]"
                          << "(p" << cnt_sketch-3 << ")"
                          << "(p" << cnt_sketch-2 << ")"
                          << "(p" << cnt_sketch-1 << ")\n";

              if (!edgeonbound[0])
                outf_sketch << "line[" //line style = dashed"
                            << "style=" << fineedgestyle // << "red" // << color 
                            << ",color=" << edgecolor // << "red" // << color 
                            << ",draw opacity=" << drawopacity //*0.25
                            << "]"
                            << "(p" << cnt_sketch-3 << ")"
                            << "(p" << cnt_sketch-2 << ")\n";
              if (!edgeonbound[1])
                outf_sketch << "line[" //line style = dashed"
                            << "style=" << fineedgestyle // << "red" // << color 
                            << ",color=" << edgecolor // << "red" // << color 
                            << ",draw opacity=" << drawopacity //*0.25
                            << "]"
                            << "(p" << cnt_sketch-2 << ")"
                            << "(p" << cnt_sketch-1 << ")\n";
              if (!edgeonbound[2])
                outf_sketch << "line[" //line style = dashed"
                            << "style=" << fineedgestyle // << "red" // << color 
                            << ",color=" << edgecolor // << "red" // << color 
                            << ",draw opacity=" << drawopacity //*0.25
                            << "]"
                            << "(p" << cnt_sketch-3 << ")"
                            << "(p" << cnt_sketch-1 << ")\n";
             
            }
          } // dt

          // if (spacetime)
          // {

          // }
          // else
          // {

          //   FlatVector<> jump(ndof_total,lh);

          //   const FlatXLocalGeometryInformation & xgeom(xfe->GetFlatLocalGeometry());
          //   const FlatCompositeQuadratureRule<D> & fcompr(xgeom.GetCompositeRule<D>());
          //   const FlatQuadratureRuleCoDim1<D> & fquad(fcompr.GetInterfaceRule());

          //   for (int i = 0; i < fquad.Size(); ++i)
          //   {
          //     IntegrationPoint ip(&fquad.points(i,0),0.0);
          //     MappedIntegrationPoint<D,D> mip(ip, eltrans);
      
          //     Mat<D,D> Finv = mip.GetJacobianInverse();
          //     const double absdet = mip.GetMeasure();

          //     Vec<D> nref = fquad.normals.Row(i);
          //     Vec<D> normal = absdet * Trans(Finv) * nref ;
          //     double len = L2Norm(normal);
          //     normal /= len;

          //     const double weight = fquad.weights(i) * len; 
        
          //     shape = scafe->GetShape(mip.IP(), lh);
          //     jump.Range(0,ndof) = (b_pos-b_neg) * shape;
          //     jump.Range(ndof,ndof_total) = shape;

          //     for (int l = 0; l < ndof_x; ++l)
          //     {
          //       if (xfe->GetSignsOfDof()[l] == NEG)
          //         jump(ndof+l) *= -b_neg;
          //       else
          //         jump(ndof+l) *= b_pos;
          //     }
            
          //     const double jumpval = InnerProduct(jump,elvec);
          //     ifjumpl2 += weight * sqr(jumpval);
          //   }
          // }
          xgeom->ClearArrays();
        } // is xfe 
        else
        {
          DOMAIN_TYPE dt = dummfe->GetDomainType();

          if (plotdt != dt) continue;

          Array<IntegrationPoint*> ips(4);
          IntegrationPoint ip1(0.0);
          ip1(0) = 1.0;
          IntegrationPoint ip2(0.0);
          ip2(1) = 1.0;
          IntegrationPoint ip3(0.0);
          ip3(2) = 1.0;
        
          ips[0] = &ip1;
          ips[1] = &ip2;
          ips[2] = &ip3;
          ips[3] = &ip1;
        

          if (dt == NEG)
            outf_tikz << "\\draw [" << edgenegcolor 
                      << ", fill=" << negcolor 
                      << ", fill opacity=" << fillnegopacity 
                      << ", draw opacity=" << drawnegopacity 
                      << "] ";
          else
            outf_tikz << "\\draw [" << edgeposcolor 
                      << ", fill=" << poscolor 
                      << ", fill opacity=" << fillposopacity 
                      << ", draw opacity=" << drawposopacity 
                      << "] ";

          // IntegrationRule pir = SelectIntegrationRule (eltrans.GetElementType(), intorder);
          for (int i = 0 ; i < ips.Size(); i++)
          {
            MappedIntegrationPoint<D,D> mip(*ips[i], eltrans);
            DimMappedIntegrationPoint<D+1> mipp(*ips[i], eltrans);
            mipp.Point().Range(0,D) = mip.GetPoint();
            mipp.Point()[D] = time;

            // double solval = dt == POS ? solcoef.GetSolutionPos().Evaluate(mipp) : solcoef.GetSolutionNeg().Evaluate(mipp);
            // Vec<D> soldval;
            // if (solcoef.HasSolutionDNeg() && solcoef.HasSolutionDPos())
            // {
            //   if (dt == POS)
            //     solcoef.GetSolutionDPos().Evaluate(mipp,soldval);
            //   else
            //     solcoef.GetSolutionDNeg().Evaluate(mipp,soldval);
            // }
            // else
            // {
            //   if (dt == POS)
            //     CalcDxShapeOfCoeff<D>(&(solcoef.GetSolutionPos()),mip,time,soldval,lh);
            //   else
            //     CalcDxShapeOfCoeff<D>(&(solcoef.GetSolutionNeg()),mip,time,soldval,lh);
            // }
          
            if (!spacetime)
            {
              shape = scafe->GetShape(mip.IP(), lh);
              scafe->CalcMappedDShape(mip, dshape);
            }
            else
            {
              scastfe->CalcShapeSpaceTime(mip.IP(), 1.0, shape, lh);
              scastfe->CalcMappedDxShapeSpaceTime(mip, 1.0, dshape, lh);
            }

            double discval = InnerProduct(shape,elvec);

            if (showerror)
            {
              double solval = dt == POS ? solcoef.GetSolutionPos().Evaluate(mip) : solcoef.GetSolutionNeg().Evaluate(mip);
              discval = abs(discval-solval);
            }

            if(i != ips.Size()-1)
            {
              outf_sketch << "  def p" << cnt_sketch++
                          << "(" << mip.GetPoint()(1) * scalex_sketch 
                          << "," << discval * scalez_sketch
                          << "," << mip.GetPoint()(0) * scaley_sketch  << ")\n";
            }

            if(i != ips.Size()-1)
              outf_tikz << "(" << mip.GetPoint()(0) << "," << mip.GetPoint()(1) << "," << discval << ") -- ";
            outf_gnuplot << mip.GetPoint() << "\t" << discval << endl;
            //   Vec<D> discdval = Trans(dshape) * elvec;
            //   Vec<D> diffdval = soldval - discdval;
            //   double diffdsqr = L2Norm2(diffdval);

            //   double fac = mip.GetWeight();
            //   if (dt == POS)
            //   {
            //     l2diff_p += b_pos*fac*sqr(discval-solval);
            //     h1diff_p += b_pos*fac*diffdsqr;
            //   }
            //   else
            //   {
            //     l2diff_n += b_neg*fac*sqr(discval-solval);
            //     h1diff_n += b_neg*fac*diffdsqr;
            //   }
          }
          // if (dt == NEG)
            

          string & color = dt == NEG ? negcolor : poscolor;
          string & edgecolor = dt == NEG ? edgenegcolor : edgeposcolor;
          string & edgestyle = dt == NEG ? edgenegstyle : edgeposstyle;
          double fillopacity = dt == NEG ? fillnegopacity : fillposopacity;
          double drawopacity = dt == NEG ? drawnegopacity : drawposopacity;
          outf_sketch << "polygon["
                      << "color=" << edgecolor 
                      << ",style=" << edgestyle 
                      << ",fill=" << color 
                      << ",cull=false,fill opacity=" << fillopacity
                      << ",draw opacity=" << drawopacity << "]"
                      << "(p" << cnt_sketch-3 << ")"
                      << "(p" << cnt_sketch-2 << ")"
                      << "(p" << cnt_sketch-1 << ")\n";

          outf_tikz << " cycle;" << endl;
          outf_gnuplot << endl << endl;
        }
      }


      string & color = plotdt == NEG ? negcolor : poscolor;
      string & edgecolor = plotdt == NEG ? edgenegcolor : edgeposcolor;
      // double fillopacity = plotdt == NEG ? fillnegopacity : fillposopacity;
      double drawopacity = plotdt == NEG ? drawnegopacity : drawposopacity;
      string & edgestyle = plotdt == NEG ? edgenegstyle : edgeposstyle;

      for (int i = 0; i < edges.Size(); i+=2)
      {
        outf_tikz << "\\draw [" 
                  << color
                  << "] " 
                  << "(" << medges[i](0) << ", " << medges[i](1) << ", " << edges_val[i] << ")" 
                  << " -- " 
                  << "(" << medges[i+1](0) << ", " << medges[i+1](1) << ", " << edges_val[i+1] << ")" 
                  << ";";
                
        outf_sketch << "  def p" << cnt_sketch++
                    << "( " << medges[i](1) * scalex_sketch 
                    << ", " << edges_val[i] * scalez_sketch
                    << ", " << medges[i](0) * scaley_sketch 
                    << ")\n";
        outf_sketch << "  def p" << cnt_sketch++
                    << "( " << medges[i+1](1) * scalex_sketch 
                    << ", " << edges_val[i+1] * scalez_sketch
                    << ", " << medges[i+1](0) * scaley_sketch 
                    << ")\n";
        outf_sketch << " line [draw=" << edgecolor << ",style="<< edgestyle <<",draw opacity=" << drawopacity << "]" 
                    << "(p" << cnt_sketch-2 << ")"
                    << "(p" << cnt_sketch-1 << ")\n";
                
        //line style=thin
      }

    }
    MakeTikzFooter(outf_tikz);
    MakeSketchFooter(outf_sketch, flags);

    // system("pdflatex special.output.tikz");
  }

  template void DoSpecialOutput<2>(shared_ptr<GridFunction> gfu, SolutionCoefficients<2> & solcoef, 
                                   int subdivision, Flags & flags, LocalHeap & lh);
  template void DoSpecialOutput<3>(shared_ptr<GridFunction> gfu, SolutionCoefficients<3> & solcoef, 
                                   int subdivision, Flags & flags,LocalHeap & lh);


  void OutputMeshOnly (shared_ptr<MeshAccess> ma, LocalHeap & lh)
  {
    // ofstream outf_gnuplot("special.output.gnuplot");
    ofstream outf_tikz("mesh.output.tikz");
    // ofstream outf_sketch("special.output.sketch");

    // int cnt_sketch = 1;
    // double scalex_sketch = 10.0;
    // double scaley_sketch = 10.0;
    // double scalez_sketch = 10.0;

    MakeTikzHeader(outf_tikz);
    // MakeSketchHeader(outf_sketch);
    
    outf_tikz.precision(12);
    outf_tikz << std::fixed;

    for (int elnr = 0; elnr < ma->GetNE(); ++elnr)
    {
      HeapReset hr(lh);
      ElementTransformation & eltrans = ma->GetTrafo(elnr,false,lh);
      // ELEMENT_TYPE eltype = eltrans.GetElementType();
      
      IntegrationPoint ip1(0.0,0.0);
      IntegrationPoint ip2(1.0,0.0);
      IntegrationPoint ip3(0.0,1.0);
      MappedIntegrationPoint<2,2> mip1(ip1, eltrans);
      MappedIntegrationPoint<2,2> mip2(ip2, eltrans);
      MappedIntegrationPoint<2,2> mip3(ip3, eltrans);

      Vec<2> p1 = mip1.GetPoint();
      Vec<2> p2 = mip2.GetPoint();
      Vec<2> p3 = mip3.GetPoint();
      
      outf_tikz << "\\draw [\\mycolorstring, fill opacity=\\myfillopacity, draw opacity=\\mydrawopacity]" 
                << " (" << p1(0) << ", " << p1(1) << ")"
                << " -- "
                << " (" << p2(0) << ", " << p2(1) << ")"
                << " -- "
                << " (" << p3(0) << ", " << p3(1) << ")"
                << " -- cycle; \n";
    }
    MakeTikzFooter(outf_tikz);
    // MakeSketchFooter(outf_sketch);

    // system("pdflatex special.output.tikz");
  }


}
