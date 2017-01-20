/*
    (C) Copyright 2015 CEA LIST. All Rights Reserved.
    Contributor(s): Olivier BICHLER (olivier.bichler@cea.fr)
                    David BRIAND (david.briand@cea.fr)

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

#ifndef DEEPNET_CUDNN_H
#define DEEPNET_CUDNN_H

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>

#include <common_cuda.h>
#include <cublas_v2.h>
#include <cuda.h>
#include <cudnn.h>

#include <iterator>
#include <vector>

#include <string>

#include <algorithm> // std::sort
#include <cmath>
#include <cstdlib>
#include <dirent.h>
#include <iomanip>
#include <stdexcept>

#include "Random.hpp"
#include "n2d2_cudnn_kernels.hpp"
#include "typedefs.h"
#include "utils.h"

void getFilesList(const std::string dir, std::vector<std::string>& files);

void envRead(const std::string& fileName,
             unsigned int size,
             unsigned int channelsHeight,
             unsigned int channelsWidth,
             DATA_T* data,
             unsigned int outputsSize,
             int32_t* outputTargets);

struct oclProfiling {
    std::string name;
    double processTime;
};

struct oclHandleStruct {
    std::vector<double> events;
    std::vector<oclProfiling> profiling;
};
extern oclHandleStruct oclHandles;

/************************************CONVOLUTION*************************************************/
/************************************************************************************************/
void setConvolution(unsigned int batchSize,
                    unsigned int nbChannels,
                    unsigned int channelsHeight,
                    unsigned int channelsWidth,
                    unsigned int paddingY,
                    unsigned int paddingX,
                    unsigned int strideY,
                    unsigned int strideX,
                    unsigned int subSampleY,
                    unsigned int subSampleX,
                    cudnnHandle_t& context_handle,
                    cudnnTensorFormat_t context_tensorFormat,
                    cudnnDataType_t context_dataType,
                    cudnnTensorDescriptor_t inputsTensor,
                    cudnnTensorDescriptor_t outputsTensor,
                    ActivationFunction_T func,
                    cudnnConvolutionFwdAlgo_t algo,
                    size_t sizeInBytes,
                    void* workSpace,
                    unsigned int nbOutputs,
                    unsigned int outputOffset,
                    unsigned int kernelHeight,
                    unsigned int kernelWidth,
                    cudnnTensorDescriptor_t biasDesc,
                    cudnnFilterDescriptor_t filterDesc,
                    cudnnConvolutionDescriptor_t convDesc);

void convcell(cudnnHandle_t& context_handle,
              ActivationFunction_T func,
              cudnnConvolutionFwdAlgo_t algo,
              void* workSpace,
              size_t sizeInBytes,
              cudnnTensorDescriptor_t inputsTensor,
              DATA_T* inputs_data,
              unsigned int outputOffset,
              int noBias,
              cudnnTensorDescriptor_t outputsTensor,
              DATA_T** outputs_data,
              cudnnTensorDescriptor_t biasDesc,
              DATA_T* bias_data,
              cudnnFilterDescriptor_t filterDesc,
              cudnnConvolutionDescriptor_t convDesc,
              DATA_T* weights_data);
/************************************BATCHNORM***************************************************/
/************************************************************************************************/
void setBatchnorm(unsigned int batchSize,
                  unsigned int nbChannels,
                  unsigned int channelsHeight,
                  unsigned int channelsWidth,
                  cudnnTensorDescriptor_t scaleDesc,
                  cudnnTensorFormat_t context_tensorFormat,
                  cudnnDataType_t context_dataType,
                  cudnnTensorDescriptor_t inputsTensor,
                  cudnnTensorDescriptor_t outputsTensor);

void batchnormcell(cudnnHandle_t& context_handle,
                   unsigned int batchSize,
                   unsigned int nbChannels,
                   unsigned int channelsHeight,
                   unsigned int channelsWidth,
                   cudnnTensorDescriptor_t inputsTensor,
                   DATA_T* inputs_data,
                   DATA_T* scale,
                   cudnnTensorDescriptor_t scaleDesc,
                   DATA_T* bias,
                   DATA_T* mean,
                   DATA_T* variance,
                   DATA_T epsilon,
                   unsigned int outputOffset,
                   cudnnTensorDescriptor_t outputsTensor,
                   DATA_T** outputs_data,
                   ActivationFunction_T func);

/************************************POOLING*****************************************************/
/************************************************************************************************/
void setPooling(unsigned int batchSize,
                unsigned int nbChannels,
                unsigned int channelsHeight,
                unsigned int channelsWidth,
                unsigned int strideY,
                unsigned int strideX,
                unsigned int outputHeight,
                unsigned int outputWidth,
                cudnnTensorFormat_t context_tensorFormat,
                cudnnDataType_t context_dataType,
                cudnnTensorDescriptor_t inputsTensor,
                cudnnTensorDescriptor_t outputsTensor,
                Pooling_T func,
                cudnnPoolingMode_t& cudnnPooling,
                unsigned int nbOutputs,
                unsigned int outputOffset,
                unsigned int poolHeight,
                unsigned int poolWidth,
                cudnnPoolingDescriptor_t mapping);

void poolcell(cudnnHandle_t& context_handle,
              cudnnTensorDescriptor_t inputsTensor,
              DATA_T* inputs_data,
              unsigned int outputOffset,
              cudnnTensorDescriptor_t outputsTensor,
              DATA_T** outputs_data,
              cudnnPoolingDescriptor_t mapping);
/************************************FMP*****************************************************/
/************************************************************************************************/
void setFmp();

void fmpcell(cudnnHandle_t& context_handle,
             unsigned int batchSize,
             unsigned int nbChannels,
             unsigned int channelsHeight,
             unsigned int channelsWidth,
             unsigned int* gridx,
             unsigned int* gridy,
             const bool overlapping,
             const DATA_T* inputs_data,
             unsigned int nbOutputs_,
             unsigned int outputsHeight,
             unsigned int outputsWidth,
             unsigned int nbOutputs,
             unsigned int outputOffset,
             DATA_T* outputs_data);

/************************************FULLY
 * CONNECTED*********************************************/
/************************************************************************************************/
void setFc(unsigned int batchSize,
           unsigned int nbChannels,
           unsigned int channelsHeight,
           unsigned int channelsWidth,
           cudnnHandle_t& context_handle,
           cudnnTensorFormat_t context_tensorFormat,
           cudnnDataType_t context_dataType,
           cudnnTensorDescriptor_t inputsTensor,
           cudnnTensorDescriptor_t outputsTensor,
           ActivationFunction_T func,
           unsigned int nbOutputs,
           cudnnTensorDescriptor_t biasDesc);

void fullyConnected(unsigned int batchSize,
                    unsigned int nbChannels,
                    cudnnHandle_t& context_handle,
                    cublasHandle_t& context_cublasHandle,
                    ActivationFunction_T func,
                    cudnnTensorDescriptor_t inputsTensor,
                    DATA_T* inputs_data,
                    cudnnTensorDescriptor_t outputsTensor,
                    unsigned int nbOutputs,
                    unsigned int outputOffset,
                    int noBias,
                    DATA_T** outputs_data,
                    cudnnTensorDescriptor_t biasDesc,
                    DATA_T* bias_data,
                    DATA_T* ones_vec_data,
                    DATA_T* weights_data);

void setSoftmax(unsigned int batchSize,
                unsigned int nbChannels,
                unsigned int channelsHeight,
                unsigned int channelsWidth,
                cudnnTensorFormat_t context_tensorFormat,
                cudnnDataType_t context_dataType,
                cudnnTensorDescriptor_t inputsTensor,
                cudnnTensorDescriptor_t outputsTensor);

void softmax(cudnnHandle_t& context_handle,
             cudnnTensorDescriptor_t inputsTensor,
             DATA_T* inputs_data,
             cudnnTensorDescriptor_t outputsTensor,
             DATA_T** outputs_data);

void output_generation(unsigned int batchSize,
                       unsigned int nbOutputs,
                       DATA_T* dataIn,
                       uint32_t* outputEstimated);

void spatial_output_generation(unsigned int batchSize,
                               unsigned int nbOutputs,
                               unsigned int outputsHeight,
                               unsigned int outputsWidth,
                               DATA_T* outputsData,
                               uint32_t* outputEstimated);

void fmpcell_propagate_generateRegions(unsigned int* grid,
                                       unsigned int sizeIn,
                                       unsigned int sizeOut);

void confusion_print(unsigned int nbOutputs, unsigned int* confusion);

void dumpMem(int size, DATA_T* data, std::string fileName);

#endif // DEEPNET_CUDNN_H
