
#include "TStorageYVector.h"
#include "coretools/algorithms.h"
#include "smart_binary_search.h"
#include "gtest/gtest.h"
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

// unit test for smart binary search for the different smart functions
TEST(Binary_search, easy_function) {
	// create the engine
	std::random_device rd{};
	std::mt19937 gen{rd()};

	double p = 0.01;
	std::bernoulli_distribution d(p);

	// We create a TStorageYVector object
	std::vector<size_t> dimensions{4000, 6000};
	TStorageYVector Y(1000, dimensions);

	// We fill randomly the TStorageYVector where the values are between 0 and 100*100
	for (size_t i = 0; i < coretools::containerProduct(dimensions); i++) {
		auto sample = d(gen);
		if (sample) { Y.insert_one(i); }
	}

	std::vector<long long> vec_duration_1;
	std::vector<long long> vec_duration_2;
	for (size_t j = 0; j < dimensions[0]; ++j) {
		std::vector<size_t> multi_index_start{j, 0};
		auto t1 = std::chrono::high_resolution_clock::now();
		auto [res, where_you_in_Y, index_in_Y] =
		    fill_current_state_easy(Y, multi_index_start, dimensions[dimensions.size() - 1]);
		auto t2 = std::chrono::high_resolution_clock::now();
		std::vector<bool> truth;
		truth.clear();
		auto linear_index = Y.get_linear_index_in_container_space(multi_index_start);
		auto t3           = std::chrono::high_resolution_clock::now();
		for (size_t i = 0; i < dimensions[dimensions.size() - 1]; ++i) {
			auto [is_one, index] = Y.binary_search((linear_index + i));
			truth.push_back(is_one);
		}
		auto t4        = std::chrono::high_resolution_clock::now();
		auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
		auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count();
		vec_duration_1.push_back(duration1);
		vec_duration_2.push_back(duration2);

		EXPECT_EQ(res, truth);
	}
	std::cout << "Average duration for fill_current_state_easy: "
	          << std::accumulate(vec_duration_1.begin(), vec_duration_1.end(), 0) / vec_duration_1.size() << " ms"
	          << std::endl;
	std::cout << "Average duration for standard binary search: "
	          << std::accumulate(vec_duration_2.begin(), vec_duration_2.end(), 0) / vec_duration_2.size() << " ms"
	          << std::endl;
}

TEST(Binary_search, hard_function) {
	// create the engine
	std::random_device rd{};
	std::mt19937 gen{rd()};

	double p = 0.001;
	std::bernoulli_distribution d(p);

	// We create a TStorageYVector object
	std::vector<size_t> dimensions{10000, 2000};
	TStorageYVector Y(1000, dimensions);

	// We fill randomly the TStorageYVector where the values are between 0 and 100*100
	for (size_t i = 0; i < coretools::containerProduct(dimensions); i++) {
		auto sample = d(gen);
		if (sample) { Y.insert_one(i); }
	}

	std::vector<long long> vec_duration_1; // we create a vector to store the duration of each iteration
	std::vector<long long> vec_duration_2;
	for (size_t j = 0; j < dimensions[1]; ++j) {
		std::vector<size_t> multi_index_start{0, j};
		auto t1                                = std::chrono::high_resolution_clock::now();
		auto [res, where_you_in_Y, index_in_Y] = fill_current_state_hard(
		    Y, dimensions[0], multi_index_start, dimensions[1], coretools::containerProduct(dimensions));
		auto t2 = std::chrono::high_resolution_clock::now();
		std::vector<bool> truth;
		truth.clear();
		auto linear_index = Y.get_linear_index_in_Y_space(multi_index_start);
		auto t3           = std::chrono::high_resolution_clock::now();
		for (size_t i = 0; i < dimensions[0]; ++i) {
			auto [truth_res, index] = Y.binary_search((linear_index + i * dimensions[1]));
			truth.push_back(truth_res);
		}
		auto t4 = std::chrono::high_resolution_clock::now();
		EXPECT_EQ(res, truth);

		auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
		auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count();
		vec_duration_1.push_back(duration1);
		vec_duration_2.push_back(duration2);
	}
	std::cout << "Average duration for fill_current_state_hard: "
	          << std::accumulate(vec_duration_1.begin(), vec_duration_1.end(), 0) / vec_duration_1.size() << " ms"
	          << std::endl;
	std::cout << "Average duration for standard binary search: "
	          << std::accumulate(vec_duration_2.begin(), vec_duration_2.end(), 0) / vec_duration_2.size() << " ms"
	          << std::endl;
}
