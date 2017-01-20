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

#ifndef N2D2_SGDSOLVER_FRAME_H
#define N2D2_SGDSOLVER_FRAME_H

#include "Solver/SGDSolver.hpp"

namespace N2D2 {
template <class T> class SGDSolver_Frame : public SGDSolver<T> {
public:
    static std::shared_ptr<SGDSolver<T> > create()
    {
        return std::make_shared<SGDSolver_Frame<T> >();
    }

    SGDSolver_Frame();
    void
    update(Tensor4d<T>* data, Tensor4d<T>* diffData, unsigned int batchSize);
    void exportFreeParameters(const std::string& fileName) const;
    std::shared_ptr<SGDSolver_Frame<T> > clone() const
    {
        return std::shared_ptr<SGDSolver_Frame<T> >(doClone());
    }
    virtual ~SGDSolver_Frame() {};

protected:
    using SGDSolver<T>::mLearningRate;
    using SGDSolver<T>::mMomentum;
    using SGDSolver<T>::mDecay;
    using SGDSolver<T>::mPower;
    using SGDSolver<T>::mMaxIterations;
    using SGDSolver<T>::mLearningRatePolicy;
    using SGDSolver<T>::mLearningRateStepSize;
    using SGDSolver<T>::mLearningRateDecay;
    using SGDSolver<T>::mClamping;
    using SGDSolver<T>::mNbIterations;

    Tensor4d<T> mMomentumData;

private:
    virtual SGDSolver_Frame<T>* doClone() const
    {
        return new SGDSolver_Frame<T>(*this);
    }

    static Registrar<SGDSolver<T> > mRegistrar;
};
}

template <class T>
N2D2::SGDSolver_Frame<T>::SGDSolver_Frame()
    : SGDSolver<T>::SGDSolver()
{
    // ctor
}

template <class T>
void N2D2::SGDSolver_Frame<T>::update(Tensor4d<T>* data,
                                      Tensor4d<T>* diffData,
                                      unsigned int batchSize)
{
    if (mLearningRate == 0.0)
        return;

    T rate = mLearningRate;
    const unsigned int itFactor = mNbIterations / mLearningRateStepSize;

    if (mLearningRatePolicy == SGDSolver<T>::StepDecay) {
        rate *= std::pow(mLearningRateDecay, (double)itFactor);

        if (mNbIterations > 0 && (mNbIterations - batchSize)
                                 / mLearningRateStepSize != itFactor)
            std::cout << "Learning rate after " << mNbIterations
                      << " iteration(s): " << rate << std::endl;
    } else if (mLearningRatePolicy == SGDSolver<T>::ExponentialDecay)
        rate = mLearningRate * std::exp(-mLearningRateDecay * itFactor);
    else if (mLearningRatePolicy == SGDSolver<T>::InvTDecay)
        rate = mLearningRate / (1.0 + mLearningRateDecay * itFactor);
    else if (mLearningRatePolicy == SGDSolver<T>::PolyDecay) {
        const T power = mPower;
        const T maxIterations = mMaxIterations;
        rate = mLearningRate
               * std::pow(1.0 - (mNbIterations / maxIterations), power);
    }

    mNbIterations += batchSize;

    // Normalize in function of the batch size
    const T rateDiff = rate / (T)batchSize;
    const T momentum = mMomentum;
    const T decay = mDecay;

    if (momentum == 0.0f && decay == 0.0f) {
        // if outside the loop for better performance
        if (mClamping) {
            //#pragma omp parallel for
            for (int index = 0; index < (int)data->size(); ++index)
                (*data)(index) = Utils::clamp<T>(
                    (*data)(index) + rateDiff * (*diffData)(index), -1.0, 1.0);
        } else {
            //#pragma omp parallel for
            for (int index = 0; index < (int)data->size(); ++index)
                (*data)(index) += rateDiff * (*diffData)(index);
        }
    } else {
        if (mMomentumData.empty()) {
            mMomentumData.resize(
                data->dimX(), data->dimY(), data->dimZ(), data->dimB());
            mMomentumData.fill(0.0);
        }

#pragma omp parallel for if (mMomentumData.size() > 1024)
        for (int index = 0; index < (int)mMomentumData.size(); ++index) {
            // mMomentumData = mMomentumData*momentum
            mMomentumData(index) *= momentum;

            // mMomentumData = mMomentumData + diffData*mWeightsLearningRate
            mMomentumData(index) += rateDiff * (*diffData)(index);

            if (decay != 0.0f) {
                const T alpha = -decay * rate;

                // mMomentumData = mMomentumData - decay*rate*data
                for (int index = 0; index < (int)mMomentumData.size(); ++index)
                    mMomentumData(index) += alpha * (*data)(index);
            }

            // data = data + mMomentumData
            if (mClamping)
                (*data)(index) = Utils::clamp
                    <T>((*data)(index) + mMomentumData(index), -1.0, 1.0);
            else
                (*data)(index) += mMomentumData(index);
        }
    }
}

template <class T>
void N2D2::SGDSolver_Frame
    <T>::exportFreeParameters(const std::string& fileName) const
{
    float momentum = mMomentum;

    if (momentum != 0.0) {
        std::ofstream syn(fileName.c_str());

        if (!syn.good())
            throw std::runtime_error("Could not create synaptic file : "
                                     + fileName);

        for (std::vector<float>::const_iterator it = mMomentumData.begin();
             it != mMomentumData.end();
             ++it)
            syn << (*it) << " ";

        if (!syn.good())
            throw std::runtime_error("Error writing synaptic file: "
                                     + fileName);
    }
}

#endif // N2D2_SGDSOLVER_FRAME_H
