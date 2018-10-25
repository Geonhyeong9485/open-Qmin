#ifndef landauDeGennesLC_H
#define landauDeGennesLC_H

#include "baseLatticeForce.h"
#include "landauDeGennesLCBoundary.h"
/*! \file landauDeGennesLC.h */

enum class distortionEnergyType {oneConstant,twoConstant,threeConstant};

//!A landau-de gennes  q-tensor framework force computer...currently working with the one-constant approximation for the distortion term
class landauDeGennesLC : public baseLatticeForce
    {
    public:

        landauDeGennesLC(scalar _A, scalar _B, scalar _C, scalar _L1);
        landauDeGennesLC(scalar _A, scalar _B, scalar _C, scalar _L1, scalar _L2,scalar _L3orWavenumber, distortionEnergyType _type);

        //!set up a few basic things (common force tuners, number of energy components, etc.)
        void baseInitialization();
        //!The model setting creates an additional data structure to help with 2- or 3- constant approximation
        virtual void setModel(shared_ptr<cubicLattice> _model);
        //select the force routing based on the number of elastic constants
        virtual void computeForceGPU(GPUArray<dVec> &forces,bool zeroOutForce = true)
            {
            switch (numberOfConstants)
                {
                case distortionEnergyType::oneConstant :
                    computeForceOneConstantGPU(forces,zeroOutForce);
                    break;
                case distortionEnergyType::twoConstant :
                    computeForceTwoConstantGPU(forces,zeroOutForce);
                    break;
                case distortionEnergyType::threeConstant :
                    computeForceThreeConstantGPU(forces,zeroOutForce);
                    break;
                };
            if(lattice->boundaries.getNumElements() >0)
                {
                computeBoundaryForcesGPU(forces,false);
                };
            };
        virtual void computeForceCPU(GPUArray<dVec> &forces,bool zeroOutForce = true)
            {
            switch (numberOfConstants)
                {
                case distortionEnergyType::oneConstant :
                    computeForceOneConstantCPU(forces,zeroOutForce);
                    break;
                case distortionEnergyType::twoConstant :
                    computeForceTwoConstantCPU(forces,zeroOutForce);
                    break;
                case distortionEnergyType::threeConstant :
                    computeForceThreeConstantCPU(forces,zeroOutForce);
                    break;
                };
            if(lattice->boundaries.getNumElements() >0)
                {
                computeBoundaryForcesCPU(forces,false);
                };
            };

        //!Precompute the first derivatives at all of the LC Sites
        virtual void computeFirstDerivatives();

        virtual void computeBoundaryForcesCPU(GPUArray<dVec> &forces,bool zeroOutForce);
        virtual void computeBoundaryForcesGPU(GPUArray<dVec> &forces,bool zeroOutForce);

        virtual void computeForceOneConstantCPU(GPUArray<dVec> &forces,bool zeroOutForce);
        virtual void computeForceOneConstantGPU(GPUArray<dVec> &forces,bool zeroOutForce);

        virtual void computeForceTwoConstantCPU(GPUArray<dVec> &forces,bool zeroOutForce);
        virtual void computeForceTwoConstantGPU(GPUArray<dVec> &forces,bool zeroOutForce);

        virtual void computeForceThreeConstantCPU(GPUArray<dVec> &forces,bool zeroOutForce);
        virtual void computeForceThreeConstantGPU(GPUArray<dVec> &forces,bool zeroOutForce);

        virtual void computeEnergyCPU();
        virtual void computeEnergyGPU(){computeEnergyCPU();};//NOT DONE YET;

        //!A vector storing the components of energy (phase,distortion,anchoring)
        vector<scalar> energyComponents;

    protected:
        //!constants, etc.
        scalar A;
        scalar B;
        scalar C;
        scalar L1;
        scalar L2;
        scalar L3;
        scalar q0;

        distortionEnergyType numberOfConstants;
        //!for 2- and 3- constant approximations, the force calculation is helped by first pre-computing first derivatives
        GPUArray<cubicLatticeDerivativeVector> forceCalculationAssist;
        /*
        The layout for forceCalculationAssist is the following... forceCalculationAssist[i] is site i, and the
        derivates are laid out like
        {d (dVec[0])/dx, d (dVec[1])/dx, ... , d (dVec[0])/dy, ...d (dVec[0])/dz,...d (dVec[DIMENSION-1])/dz}
        */

        //!performance for the first derivative calculation
        shared_ptr<kernelTuner> forceAssistTuner;
        //!performance for the boundary force kernel
        shared_ptr<kernelTuner> boundaryForceTuner;
    };

#endif
