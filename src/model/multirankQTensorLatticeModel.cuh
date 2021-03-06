#ifndef multirankQTensorLatticeModel_CUH
#define multirankQTensorLatticeModel_CUH

#include "std_include.h"
#include "indexer.h"
#include <cuda_runtime.h>
#include "curand_kernel.h"

/*! \file multirankQTensorLatticeModel.cuh */

/** @addtogroup modelKernels model Kernels
 * @{
 * \brief CUDA kernels and callers for model classes
 */

bool gpu_copyReceivingBuffer(int *type,
                            dVec *position,
                            int *iBuf,
                            scalar *dBuf,
                            int N,
                            int maxIndex,
                            int blockSize = 512);
bool gpu_prepareSendingBuffer(int *type,
                            dVec *position,
                            int *iBuf,
                            scalar *dBuf,
                            int3 latticeSites,
                            Index3D latticeIndex,
                            int maxIndex,
                            int blockSize = 512);

/** @} */ //end of group declaration
#endif
