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

#include "GradientCheck.hpp"

N2D2::GradientCheck::GradientCheck(double epsilon, double maxError)
    : mEpsilon(epsilon), mMaxError(maxError)
{
    // ctor
}

void N2D2::GradientCheck::initialize(Interface<Float_T>& inputs,
                                     Tensor4d<Float_T>& outputs,
                                     Tensor4d<Float_T>& diffInputs,
                                     PropagateType propagate,
                                     BackPropagateType backPropagate)
{
    mOutputs = &outputs;
    mDiffInputs = &diffInputs;
    mPropagate = propagate;

    // Initialize input values
    for (std::vector<Tensor4d<Float_T>*>::iterator itTensor = inputs.begin(),
                                                   itTensorEnd = inputs.end();
         itTensor != itTensorEnd;
         ++itTensor) {
        Tensor4d<Float_T>* input = (*itTensor);

        for (unsigned int index = 0; index < input->size(); ++index)
            (*input)(index) = Random::randUniform(-1.0, 1.0);

        input->synchronizeHToD();
    }

    propagate(false);
    outputs.synchronizeDToH();

#pragma omp parallel for if (outputs.size() > 32)
    for (int index = 0; index < (int)outputs.size(); ++index)
        diffInputs(index) = 1.0 - outputs(index);

    diffInputs.synchronizeHToD();
    backPropagate();
}

void N2D2::GradientCheck::check(const std::string& tensorName,
                                Tensor4d<Float_T>& tensor,
                                Tensor4d<Float_T>& diffTensor,
                                Tensor4d<std::vector<unsigned int> >* control)
{
    double cumulativeError = 0.0;
    unsigned int nbGradients = 0;

    tensor.synchronizeDToH();
    diffTensor.synchronizeDToH();

    for (unsigned int b = 0; b < tensor.dimB(); ++b) {
        for (unsigned int z = 0; z < tensor.dimZ(); ++z) {
            for (unsigned int y = 0; y < tensor.dimY(); ++y) {
                for (unsigned int x = 0; x < tensor.dimX(); ++x) {
                    const Float_T value = tensor(x, y, z, b);

                    // Compute approx. gradient
                    tensor(x, y, z, b) = value + mEpsilon;
                    tensor.synchronizeHToD(x, y, z, b, 1);
                    mPropagate(false);

                    Tensor4d<std::vector<unsigned int> > controlData;

                    if (control != NULL)
                        controlData.data() = control->data();

                    double approxGradient = cost();

                    tensor(x, y, z, b) = value - mEpsilon;
                    tensor.synchronizeHToD(x, y, z, b, 1);
                    mPropagate(false);

                    approxGradient = (approxGradient - cost())
                                     / (2.0 * mEpsilon);

                    // Computed gradient
                    tensor(x, y, z, b) = value;
                    tensor.synchronizeHToD(x, y, z, b, 1);

                    const Float_T gradient = -diffTensor(x, y, z, b);
                    const double error = std::fabs(gradient - approxGradient);

                    if (control != NULL && error >= mMaxError
                        && controlData.data() != control->data()) {
                        // A discontinuity is possible if the maximum input is >
                        // "input" but < "input + epsilon"
                        // If this is the case, the checking for this input is
                        // discarded (this should occurs very rarely)
                        // Note: discontinuity check is only needed if error >=
                        // maxError (optimization)
                        std::cout << "Gradient: possible discontinuity for "
                                  << tensorName << " @ (" << x << ", " << y
                                  << ", " << z << ", " << b << ") -- ignoring"
                                  << std::endl;

                        // Continue AFTER initial input value is restored
                        continue;
                    }

                    cumulativeError += error;
                    ++nbGradients;

                    if (error >= mMaxError) {
                        std::cout << "Gradient check error for \"" << tensorName
                                  << "\""
                                  << " @ (" << x << ", " << y << ", " << z
                                  << ", " << b << ")\n"
                                  << std::setprecision(std::numeric_limits
                                                       <double>::digits10 + 1)
                                  << "  Computed = " << gradient
                                  << "\n"
                                     "  Approximated = " << approxGradient
                                  << "\n"
                                     "  Error = " << error
                                  << " > max. error = " << mMaxError
                                  << std::endl;

                        throw std::runtime_error("Gradient check failed!");
                    }
                }
            }
        }
    }

    std::cout << "Gradient check for \"" << tensorName
              << "\" PASSED! (avg error = "
              << (cumulativeError / ((nbGradients == 0) ? 1.0 : nbGradients))
              << ")" << std::endl;
}

N2D2::GradientCheck::~GradientCheck()
{
    mDiffInputs->fill(0.0);
}

double N2D2::GradientCheck::cost() const
{
    mOutputs->synchronizeDToH();
    const int outputSize = mOutputs->size();

    double cost = 0.0;

#pragma omp parallel for reduction(+ : cost)
    for (int index = 0; index < outputSize; ++index) {
        const double error = 1.0 - (*mOutputs)(index);
        cost += error * error;
    }

    return (1.0 / 2.0) * cost;
}
