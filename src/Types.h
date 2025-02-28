//
// Created by madleina on 22.10.24.
//

#ifndef ACOL_TYPES_H
#define ACOL_TYPES_H

#include "coretools/Types/commonWeakTypes.h"
#include "stattools/ParametersObservations/THash.h"
#include "stattools/ParametersObservations/TObservation.h"
#include "stattools/ParametersObservations/spec.h"
#include "stattools/Priors/TPriorExponential.h"
#include "stattools/Priors/TPriorUniform.h"
#include <stdint.h>

// Parameter types
using TypeGamma               = coretools::Positive;
using TypeMu                  = coretools::StrictlyPositive;
using TypeBinnedBranchLengths = coretools::UnsignedInt8WithMax<0>;

// Gamma
using PriorOnGamma = stattools::prior::TUniformFixed<TypeGamma>;
using SpecGamma    = stattools::ParamSpec<TypeGamma, stattools::name("gamma"), PriorOnGamma>;

// Mu
using PriorOnMu = stattools::prior::TExponentialFixed<TypeMu>;
using SpecMu_0  = stattools::ParamSpec<TypeMu, stattools::name("mu_0"), PriorOnMu, stattools::EnforceUniqueHash<false>>;
using SpecMu_1  = stattools::ParamSpec<TypeMu, stattools::name("mu_1"), PriorOnMu, stattools::EnforceUniqueHash<false>>;

// binned branch lengths
using PriorOnBinnedBranches = stattools::prior::TUniformFixed<TypeBinnedBranchLengths>;
using SpecBinnedBranches    = stattools::ParamSpec<TypeBinnedBranchLengths, stattools::name("bin_branch"),
                                                   PriorOnBinnedBranches, stattools::EnforceUniqueHash<false>>;

// Markov Field
using TypeMarkovField                     = coretools::Boolean;
constexpr static size_t NumDimMarkovField = 1; // note: only for stattools, actually not known at compile time

// Observation: Lotus
template<bool SimpleErrorModel> class TLotus; // forward declaration to avoid circular inclusion
using TypeLotus                     = coretools::Boolean;
constexpr static size_t NumDimLotus = 2;
using StorageLotus                  = coretools::TMultiDimensionalStorage<TypeLotus, NumDimLotus>;

// Type for calculating the number of 1's per clique
using TypeCounter1 = uint32_t;

#endif // ACOL_TYPES_H
