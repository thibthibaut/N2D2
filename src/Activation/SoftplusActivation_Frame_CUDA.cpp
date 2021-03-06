/*
    (C) Copyright 2016 CEA LIST. All Rights Reserved.
    Contributor(s): Olivier BICHLER (olivier.bichler@cea.fr)

    This software is governed by the CeCILL-C license under French law and
    abiding by the rules of distribution of free software.  You can  use,
    modify and/ or redistribute the software under the terms of the CeCILL-C
    license as circulated by CEA, CNRS and INRIA at the following URL
    "http://www.cecill.info".

    As a counterpart to the access to the source code and  rights to copy,
    modify and redistribute granted by the license, users are provided only
    with a limited warranty  and the software's author,  the holder of the
    economic rights,  and the successive licensors  have only  limited
    liability.

    The fact that you are presently reading this means that you have had
    knowledge of the CeCILL-C license and that you accept its terms.
*/

#ifdef CUDA

#include "Activation/SoftplusActivation_Frame_CUDA.hpp"

template <>
N2D2::Registrar<N2D2::SoftplusActivation<N2D2::Float_T> >
N2D2::SoftplusActivation_Frame_CUDA
    <N2D2::Float_T>::mRegistrar(N2D2::SoftplusActivation_Frame_CUDA
                                <N2D2::Float_T>::create,
                                "Frame_CUDA",
                                "Transcode_CUDA",
                                "CSpike_CUDA",
                                NULL);

namespace N2D2 {
template <>
void SoftplusActivation_Frame_CUDA<float>::propagate(Tensor<float>* data)
{
    CudaTensor<float>* cudaData = static_cast<CudaTensor<float>*>(data);
    cudaSSoftplus_propagate(cudaData->getDevicePtr(), cudaData->size());
}

template <>
void SoftplusActivation_Frame_CUDA<double>::propagate(Tensor<double>* data)
{
    CudaTensor<double>* cudaData = static_cast<CudaTensor<double>*>(data);
    cudaDSoftplus_propagate(cudaData->getDevicePtr(), cudaData->size());
}

template <>
void SoftplusActivation_Frame_CUDA
    <float>::backPropagate(Tensor<float>* data, Tensor<float>* diffData)
{
    CudaTensor<float>* cudaData = static_cast<CudaTensor<float>*>(data);
    CudaTensor<float>* cudaDiffData = static_cast
        <CudaTensor<float>*>(diffData);
    cudaSSoftplus_backPropagate(cudaData->getDevicePtr(),
                                cudaDiffData->getDevicePtr(),
                                cudaData->size());
}

template <>
void SoftplusActivation_Frame_CUDA
    <double>::backPropagate(Tensor<double>* data, Tensor<double>* diffData)
{
    CudaTensor<double>* cudaData = static_cast<CudaTensor<double>*>(data);
    CudaTensor<double>* cudaDiffData = static_cast
        <CudaTensor<double>*>(diffData);
    cudaDSoftplus_backPropagate(cudaData->getDevicePtr(),
                                cudaDiffData->getDevicePtr(),
                                cudaData->size());
}
}

#endif
