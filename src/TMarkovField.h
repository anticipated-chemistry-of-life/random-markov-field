//
// Created by madleina on 10.01.25.
//

#ifndef ACOL_TMARKOVFIELD_H
#define ACOL_TMARKOVFIELD_H

#include "TClique.h"
#include "TCurrentState.h"
#include "TTree.h"
#include "coretools/Main/TError.h"

//-----------------------------------
// TMarkovField
//-----------------------------------

class TLotus; // forward declaration

class TMarkovField {
private:
	// trees and Y
	std::vector<std::unique_ptr<TTree>> &_trees;
	TStorageYVector _Y;

	// stuff for updating Y
	size_t _K;
	size_t _num_outer_loops;
	std::vector<size_t> _num_leaves_per_dim_except_last;
	std::vector<TSheet> _sheets;
	TCurrentState _clique_last_dim;

	// fix values?
	bool _fix_Y = false;
	bool _fix_Z = false;

	// functions for updating Y
	void _update_sheets(bool first, const std::vector<size_t> &start_index_in_leaves_space,
	                    const std::vector<size_t> &previous_ix, size_t K_cur_sheet);
	void _fill_clique_along_last_dim(const std::vector<size_t> &start_index_in_leaves_space);
	void _calculate_log_prob_field(const std::vector<size_t> &index_in_leaves_space,
	                               std::array<coretools::TSumLogProbability, 2> &sum_log) const;
	bool _need_to_update_sheet(size_t sheet_ix, const std::vector<size_t> &start_index_in_leaves_space,
	                           const std::vector<size_t> &previous_ix) const;
	int _set_new_Y(bool new_state, const std::vector<size_t> &index_in_leaves_space,
	               std::vector<TStorageY> &linear_indices_in_Y_space_to_insert);
	void _update_counter_1_cliques(bool new_state, bool old_state, const std::vector<size_t> &index_in_leaves_space);

	void _simulate_Y();
	void _add_lotus_LL(const std::vector<size_t> &index_in_leaves_space, size_t leaf_index_last_dim,
	                   std::array<coretools::TSumLogProbability, 2> &sum_log, TLotus &lotus);
	void _prepare_lotus_LL(const std::vector<size_t> &start_index_in_leaves_space, size_t K_cur_sheet, TLotus &lotus);

	template<bool IsSimulation>
	int _update_Y(std::vector<size_t> index_in_leaves_space, size_t leaf_index_last_dim,
	              std::vector<TStorageY> &linear_indices_in_Y_space_to_insert, TLotus &lotus) {
		index_in_leaves_space.back() = leaf_index_last_dim;

		// prepare log probabilities for the two possible states
		std::array<coretools::TSumLogProbability, 2> sum_log;

		// calculate probabilities in Markov random field
		_calculate_log_prob_field(index_in_leaves_space, sum_log);

		// calculate log likelihood (lotus)
		if constexpr (!IsSimulation) { _add_lotus_LL(index_in_leaves_space, leaf_index_last_dim, sum_log, lotus); }

		// calculate log likelihood (virtual mass spec)...

		// sample state
		bool new_state = sample(sum_log);

		// update Y accordingly
		return _set_new_Y(new_state, index_in_leaves_space, linear_indices_in_Y_space_to_insert);
	}

	template<bool IsSimulation> void _update_all_Y(TLotus &lotus) {
		if (_fix_Y) { return; }

		// loop over sheets in last dimension
		for (size_t k = 0; k < _num_outer_loops; ++k) {
			const size_t start_ix_in_leaves_last_dim = k * _K; // 0, _K, 2*_K, ...

			// loop over all dimensions except last (linearized)
			size_t num_inner_loops = coretools::containerProduct(_num_leaves_per_dim_except_last);
			std::vector<size_t> previous_ix;
			for (size_t i = 0; i < num_inner_loops; ++i) {
				// get multi-dimensional index from linear coordinate and set the start of the last dimension
				auto start_index_in_leaves_space   = coretools::getSubscripts(i, _num_leaves_per_dim_except_last);
				start_index_in_leaves_space.back() = start_ix_in_leaves_last_dim;
				// calculate size of current sheet (make sure not to overshoot)
				const size_t K_cur_sheet =
				    std::min(_K, _trees.back()->get_number_of_leaves() - start_ix_in_leaves_last_dim);
				// update sheet(s), if necessary
				_update_sheets(i == 0, start_index_in_leaves_space, previous_ix, K_cur_sheet);

				// fill clique along last dimension
				start_index_in_leaves_space.back() = 0; // start from the beginning
				_fill_clique_along_last_dim(start_index_in_leaves_space);
				_prepare_lotus_LL(start_index_in_leaves_space, K_cur_sheet, lotus);

				// now loop along all leaves of the last dimension for updating (only K leaves for which we have
				// everything)
				const size_t end_ix_in_leaves_last_dim = start_ix_in_leaves_last_dim + K_cur_sheet;
				std::vector<std::vector<TStorageY>> linear_indices_in_Y_space_to_insert(NUMBER_OF_THREADS);
				int diff_counter_1_in_last_dim = 0;
#pragma omp parallel for num_threads(NUMBER_OF_THREADS) reduction(+ : diff_counter_1_in_last_dim)
				for (size_t j = start_ix_in_leaves_last_dim; j < end_ix_in_leaves_last_dim; ++j) {
					diff_counter_1_in_last_dim +=
					    _update_Y<IsSimulation>(start_index_in_leaves_space, j,
					                            linear_indices_in_Y_space_to_insert[omp_get_thread_num()], lotus);
				}

				// insert new 1-valued indices into Y
				// Note: indices of where Y is one in _sheets is not accurate anymore, but we don't use them, so it's ok
				_Y.insert_in_Y(linear_indices_in_Y_space_to_insert);
				_trees.back()
				    ->get_clique(start_index_in_leaves_space)
				    .update_counter_leaves_state_1(diff_counter_1_in_last_dim);

				previous_ix = start_index_in_leaves_space;
			}
		}
	}

	template<bool IsSimulation> void _update_all_Z() {
		if (_fix_Z) { return; }

		for (auto &_tree : _trees) { _tree->update_Z_and_mus_and_branch_lengths<IsSimulation>(_Y); }
	}

public:
	TMarkovField(size_t n_iterations, std::vector<std::unique_ptr<TTree>> &Trees);
	~TMarkovField() = default;

	// updates
	void update(TLotus &lotus);

	// simulation
	void simulate(TLotus &lotus);

	// get Y
	[[nodiscard]] const TStorageYVector &get_Y_vector() const;
	[[nodiscard]] const TStorageY &get_Y(size_t index) const;
	[[nodiscard]] size_t size_Y() const;
};

#endif // ACOL_TMARKOVFIELD_H
