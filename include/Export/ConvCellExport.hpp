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

#ifndef N2D2_CONVCELLEXPORT_H
#define N2D2_CONVCELLEXPORT_H

#include "CellExport.hpp"
#include "Cell/ConvCell.hpp"

#ifdef WIN32
// For static library
#pragma comment(                                                               \
    linker,                                                                    \
    "/include:?mRegistrar@CPP_ConvCellExport@N2D2@@0U?$Registrar@VConvCellExport@N2D2@@@2@A")
#pragma comment(                                                               \
    linker,                                                                    \
    "/include:?mRegistrar@CPP_Cuda_ConvCellExport@N2D2@@0U?$Registrar@VConvCellExport@N2D2@@@2@A")
#pragma comment(                                                               \
    linker,                                                                    \
    "/include:?mRegistrar@CPP_OpenCL_ConvCellExport@N2D2@@0U?$Registrar@VConvCellExport@N2D2@@@2@A")
#pragma comment(                                                               \
    linker,                                                                    \
    "/include:?mRegistrar@CPP_cuDNN_ConvCellExport@N2D2@@0U?$Registrar@VConvCellExport@N2D2@@@2@A")
#pragma comment(                                                               \
    linker,                                                                    \
    "/include:?mRegistrar@C_ConvCellExport@N2D2@@0U?$Registrar@VConvCellExport@N2D2@@@2@A")
#pragma comment(                                                               \
    linker,                                                                    \
    "/include:?mRegistrar@C_HLS_ConvCellExport@N2D2@@0U?$Registrar@VConvCellExport@N2D2@@@2@A")
#pragma comment(                                                               \
    linker,                                                                    \
    "/include:?mRegistrar@SC_Spike_ConvCellExport@N2D2@@0U?$Registrar@VConvCellExport@N2D2@@@2@A")
#endif

namespace N2D2 {
/**
 * Base class for methods for the ConvCell type for any export type
 * ConvCell, ANY EXPORT
*/
class ConvCellExport : public CellExport {
public:
    typedef std::function
        <void(ConvCell& cell, const std::string&)> RegistryCreate_T;

    static RegistryMap_T& registry()
    {
        static RegistryMap_T rMap;
        return rMap;
    }

    static void
    generate(Cell& cell, const std::string& dirName, const std::string& type);

private:
    static Registrar<CellExport> mRegistrar;
};
}

#endif // N2D2_CONVCELLEXPORT_H
