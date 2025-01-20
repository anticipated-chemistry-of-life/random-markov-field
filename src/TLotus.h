#pragma once

#include "TStorageYVector.h"
#include "TTree.h"
#include "Types.h"
#include "stattools/ParametersObservations/TParameter.h"
#include "stattools/Priors/TPriorBase.h"
#include <cstddef>
#include <string>
#include <vector>

// For this class, we need to enforce that the first tree will always be the one of the species and
// the second tree will always be the one of the molecules. The rest of the trees, we won't care.
class TLotus : public stattools::prior::TBaseLikelihoodPrior<TypeLotus, NumDimLotus> {
public:
	// some type aliases, for better readability
	using BoxType = TLotus;
	using Base    = stattools::prior::TBaseLikelihoodPrior<TypeLotus, NumDimLotus>;
	using typename Base::Storage;
	using typename Base::UpdatedStorage;

	using TypeParamGamma = stattools::TParameter<SpecGamma, BoxType>;

private:
	// trees should be a const ref because we don't want to change the trees and don't want to copy them
	const std::vector<TTree> &_trees;

	// data
	TStorageYVector _L;
	std::vector<std::vector<size_t>> _occurrence_counters;

	std::vector<size_t> _dimensions_to_keep;
	std::vector<size_t> _dimensions_to_collapse;
	std::vector<size_t> _dimensions_lotus;
	size_t _ix_species;
	size_t _ix_molecules;

	// parameters gamma
	TypeParamGamma *_gamma = nullptr;

	// temporary values
	double _oldLL;
	double _curLL;

	// private functions
	double _calculate_probability_of_L_sm(bool x_sm, bool L_sm, size_t linear_index_in_L_space) const;
	bool _x_is_one(const std::vector<size_t> &index_in_Lotus) const;

	void _simulateUnderPrior(Storage *) override;

public:
	TLotus(const std::vector<TTree> &trees, const std::vector<size_t> &dimensions_to_collapse, TypeParamGamma *gamma);
	~TLotus() override = default;

	[[nodiscard]] std::string name() const override;
	void initialize() override;

	void load_from_file(const std::string &filename);

	double calculate_research_effort(size_t linear_index_in_L_space) const;
	double calculate_log_likelihood_of_L() const;

	double getSumLogPriorDensity(const Storage &) const override;

	[[nodiscard]] double calculateLLRatio(TypeParamGamma *, size_t Index, const Storage &);

	void updateTempVals(TypeParamGamma *, size_t Index, bool Accepted);

	void guessInitialValues() override;

	void set_x(bool state, size_t linear_index_in_L_space);
};
