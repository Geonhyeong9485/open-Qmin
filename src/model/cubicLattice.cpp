#include "cubicLattice.h"
#include "cubicLattice.cuh"
#include "functions.h"
/*! \file cubicLattice.cpp */

cubicLattice::cubicLattice(int l, bool _slice, bool _useGPU)
    {
    useGPU=_useGPU;
    latticeIndex = Index3D(l);
    sliceSites = _slice;
    N = l*l*l;
    L=l;
    Box = make_shared<periodicBoundaryConditions>(L);
    selfForceCompute = false;
    initializeNSites();
    normalizeSpins = true;
    };

cubicLattice::cubicLattice(int lx, int ly, int lz, bool _slice, bool _useGPU)
    {
    useGPU=_useGPU;
    int3 idx;
    idx.x=lx;idx.y=ly;idx.z=lz;
    latticeIndex = Index3D(idx);
    sliceSites = _slice;
    N = lx*ly*lz;
    L=lx;
    Box = make_shared<periodicBoundaryConditions>(L);//DONT USE
    selfForceCompute = false;
    initializeNSites();
    normalizeSpins = true;
    };

void cubicLattice::initializeNSites()
    {
    positions.resize(N);
    types.resize(N);
    forces.resize(N);
    //temporary?
    masses.resize(N);
    velocities.resize(N);

    vector<dVec> zeroes(N,make_dVec(0.0));
    vector<int> zeroInts(N,0);
    vector<scalar> unities(N,1.0);
    fillGPUArrayWithVector(zeroInts,types);
    fillGPUArrayWithVector(unities,masses);
    fillGPUArrayWithVector(zeroes,positions);
    fillGPUArrayWithVector(zeroes,forces);
    fillGPUArrayWithVector(zeroes,velocities);
    };

void cubicLattice::moveParticles(GPUArray<dVec> &displacements, scalar scale)
    {
    if(!useGPU)
        {//cpu branch
        ArrayHandle<dVec> h_disp(displacements, access_location::host,access_mode::read);
        ArrayHandle<dVec> h_pos(positions);
        #include "ompParallelLoopDirective.h"
        for(int pp = 0; pp < N; ++pp)
            {
            h_pos.data[pp] += scale*h_disp.data[pp];
            if(normalizeSpins)
                {
                scalar nrm = norm(h_pos.data[pp]);
                h_pos.data[pp] = (1/nrm)*h_pos.data[pp];
                }
            }
        }
    else
        {//gpu branch
        ArrayHandle<dVec> d_disp(displacements,access_location::device,access_mode::readwrite);
        ArrayHandle<dVec> d_pos(positions,access_location::device,access_mode::readwrite);
        gpu_update_spins(d_disp.data,d_pos.data,scale,N,normalizeSpins);
        };

    };

void cubicLattice::setSpinsRandomly(noiseSource &noise)
    {
    if(!useGPU)
        {
        ArrayHandle<dVec> pos(positions);
        #include "ompParallelLoopDirective.h"
        for(int pp = 0; pp < N; ++pp)
            {
            for (int dd = 0; dd < DIMENSION; ++dd)
                pos.data[pp][dd] = noise.getRealNormal();
            scalar lambda = dot(pos.data[pp],pos.data[pp]);
            pos.data[pp] = (1/sqrt(lambda))*pos.data[pp];
            };
        }
    else
        {
        ArrayHandle<dVec> pos(positions,access_location::device,access_mode::overwrite);
        int blockSize = 128;
        int nBlocks = N/blockSize+1;
        noise.initialize(nBlocks);
        noise.initializeGPURNGs();
        ArrayHandle<curandState> d_curandRNGs(noise.RNGs,access_location::device,access_mode::readwrite);
        gpu_set_random_spins(pos.data,d_curandRNGs.data, blockSize,nBlocks,N);
        }
    };

int cubicLattice::latticeSiteToLinearIndex(const int3 &target)
    {
    if(sliceSites)
        {
            /*
        int xp = wrap(target.x,L);
        int yp = wrap(target.y-target.x,L);
        int zp = wrap(target.z-target.y+target.x,L);
        return latticeIndex(xp,yp,zp);
        */
            return latticeIndex(target);
        }
    else
        {
        return latticeIndex(target);
        }
    }

int cubicLattice::getNeighbors(int target, vector<int> &neighbors, int &neighs)
    {
    neighs = 6;
    if(neighbors.size()!=neighs) neighbors.resize(neighs);
    if(!sliceSites)
        {
        int3 position = latticeIndex.inverseIndex(target);
        if(position.x >0 && position.x < latticeIndex.sizes.x-1)
            {
            neighbors[0] = latticeIndex(position.x-1,position.y,position.z);
            neighbors[1] = latticeIndex(position.x+1,position.y,position.z);
            }
        else if(position.x ==0)
            {
            neighbors[0] = latticeIndex(latticeIndex.sizes.x-1,position.y,position.z);
            neighbors[1] = latticeIndex(1,position.y,position.z);
            }
        else
            {
            neighbors[0] = latticeIndex(latticeIndex.sizes.x-2,position.y,position.z);
            neighbors[1] = latticeIndex(0,position.y,position.z);
            };
        if(position.y >0 && position.y < latticeIndex.sizes.y-1)
            {
            neighbors[2] = latticeIndex(position.x,position.y-1,position.z);
            neighbors[3] = latticeIndex(position.x,position.y+1,position.z);
            }
        else if(position.y ==0)
            {
            neighbors[2] = latticeIndex(position.x,latticeIndex.sizes.y-1,position.z);
            neighbors[3] = latticeIndex(position.x,1 ,position.z);
            }
        else
            {
            neighbors[2] = latticeIndex(position.x,latticeIndex.sizes.y-2,position.z);
            neighbors[3] = latticeIndex(position.x,0,position.z);
            };
        if(position.z >0 && position.z < latticeIndex.sizes.z-1)
            {
            neighbors[4] = latticeIndex(position.x,position.y,position.z-1);
            neighbors[5] = latticeIndex(position.x,position.y,position.z+1);
            }
        else if(position.z ==0)
            {
            neighbors[4] = latticeIndex(position.x,position.y,latticeIndex.sizes.z-1);
            neighbors[5] = latticeIndex(position.x,position.y,1);
            }
        else
            {
            neighbors[4] = latticeIndex(position.x,position.y,latticeIndex.sizes.z-2);
            neighbors[5] = latticeIndex(position.x,position.y,0);
            };
        return target;
        };
    return target; //nope
    };

void cubicLattice::createBoundaryObject(vector<int> &latticeSites, boundaryType _type, scalar Param1, scalar Param2)
    {
    growGPUArray(boundaries,1);
    ArrayHandle<boundaryObject> boundaryObjs(boundaries);
    boundaryObject bound(_type,Param1,Param2);
    boundaryObjs.data[boundaries.getNumElements()-1] = bound;

    //set all sites in the boundary to the correct type
    int j = boundaries.getNumElements();
    ArrayHandle<int> t(types);
    for (int ii = 0; ii < latticeSites.size();++ii)
        {
        t.data[latticeSites[ii]] = j;
        };

    int neighNum;
    vector<int> neighbors;
    //set all neighbors of boundary sites to type -1
    for (int ii = 0; ii < latticeSites.size();++ii)
        {
        int currentIndex = getNeighbors(latticeSites[ii],neighbors,neighNum);
        for (int nn = 0; nn < neighbors.size(); ++nn)
            if(t.data[neighbors[nn]] < 1)
                t.data[neighbors[nn]] = -1;
        };
    printf("there are now %i boundary objects known to the configuration\n",boundaries.getNumElements());
    };
