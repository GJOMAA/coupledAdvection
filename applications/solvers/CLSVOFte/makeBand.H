    const labelListList pcs = mesh.pointCells();
    const labelListList cps = mesh.cellPoints();
    scalarField band (mesh.V().size(),3);
//	const pointField& pts = mesh.points();
//    const cell& Cells = mesh.cells();


    forAll(alpha0,celli)
    {
        if (alpha0[celli]>1e-6 && alpha0[celli]<0.999999)
        {
            band[celli] = 0;
//            Info <<"cellI="<<celli<<endl;
            forAll(cps[celli],pi)
            {
                const label& cp = cps[celli][pi];
                const labelList& pc=pcs[cp];
                forAll(pc, i)
                {
//                    Info <<"pcI="<<pc[i]<<endl;
                    if(pc[i]!=celli)
                    {
                        const label& l= pc[i];
                        if( band[l] !=0 && band[l] !=1 )
                        {
                            if (alpha0[l]>1e-6 &&alpha0[l]<0.999999)
                                band[l] = 0;
                            else
                                band[l] = 1;
                        }
                        forAll(cps[l],ppi)
                        {
                            const label& cpp = cps[l][ppi];
                            const labelList& ppc = pcs[cpp];
                            forAll(ppc,ii)
                            {
                                if(ppc[ii]!=celli && ppc[ii]!=pc[i])
                                {
                                    const label& ll = ppc[ii];
                                    if (band[ll] == 3)
                                    {
                                        if (alpha0[ll]>1e-6 && alpha0[ll]<0.999999)
                                            band[ll] = 0;
                                        else
                                            band[ll] = 2;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
