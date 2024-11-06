//
// Created by VISANI Marco on 18.10.2024.
//

#ifndef TSTORAGEZVECTOR_H
#define TSTORAGEZVECTOR_H
#include "TStorageZ.h"
#include "coretools/algorithms.h"
#include <cassert>
#include <vector>

class TStorageZVector {
private:
	std::vector<size_t> _dimensions;
	std::vector<TStorageZ> _vec;

public:
	using value_type  = uint32_t;
	TStorageZVector() = default;
	TStorageZVector(const std::vector<size_t> &dimensions) : _dimensions(dimensions) {
		// TODO : NOTE that dimensions correspond to the number of internal nodes in each dimension !!!
	}

	[[nodiscard]] bool is_one(const uint32_t index_in_Z) const { return _vec[index_in_Z].is_one(); };

	void set_to_one(uint32_t coordinate) {
		auto [found, index] = binary_search(coordinate);
		if (found) {
			_vec[index].set_state(true);
		} else {
			_vec.insert(_vec.begin() + index, TStorageZ((int32_t)coordinate));
		}
	}

	void set_to_zero(uint32_t coordinate) {
		auto [found, index] = binary_search(coordinate);
		if (found) { _vec[index].set_state(false); }
	}

	void remove_zeros() {
		_vec.erase(std::remove_if(_vec.begin(), _vec.end(), [](const TStorageZ &storage) { return !storage.is_one(); }),
		           _vec.end());
	}
	size_t size() const { return _vec.size(); }
	auto begin() const { return _vec.begin(); }
	auto end() const { return _vec.end(); }

	uint64_t get_coordinate(const std::vector<size_t> &multi_dim_index) const {
		return coretools::getLinearIndex(multi_dim_index, _dimensions);
	}

	[[nodiscard]] std::pair<bool, size_t> binary_search(uint32_t coordinate) const {

		// lower_bound return the first element that is not less than the value
		auto it = std::lower_bound(_vec.begin(), _vec.end(), coordinate);

		// if our coordinate is bigger than the biggest element in the vector
		// we say that we haven't found our element and that if we want to
		// insert it, we should insert it at the end of the vector
		if (it == _vec.end()) { return {false, _vec.size()}; }

		// else our coordinate is in the range of the coordinates in the vector
		// meaning that if we haven't found it, we will insert it at that position
		// to keep the vector sorted
		size_t index = std::distance(_vec.begin(), it);
		if (*it != coordinate) { return {false, index}; }

		// if we found the coordinate we return the index and true
		return {true, index};
	};

	[[nodiscard]] std::pair<bool, size_t> binary_search(const std::vector<size_t> &multi_dim_index) const {
		return binary_search(get_coordinate(multi_dim_index));
	}
};

#endif // TSTORAGEZVECTOR_H
