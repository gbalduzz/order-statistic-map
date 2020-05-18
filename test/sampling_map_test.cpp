// Copyright (C) ETH Zurich
// Copyright (C) 2020 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// Test for the SamplingMap class.

#include "order_statistic_map/sampling_map.hpp"

#include <map>
#include <random>
#include <string>

#include "gtest/gtest.h"

TEST(OrderStatisticMapTest, SamplingInt) {
  maplib::SamplingMap<int, int, unsigned> map_int{{0, 0, 1}, {1, 0, 2}, {2, 0, 1}};
  EXPECT_EQ(4, map_int.totalWeight());

  std::ranlux24_base rng1(0);
  std::ranlux24_base rng2(0);

  std::uniform_int_distribution<unsigned> distro(0, 3);

  for (int i = 0; i < 20; ++i) {
    const unsigned scaled = distro(rng1);
    const auto expected_idx = scaled < 1 ? 0 : scaled < 3 ? 1 : 2;

    EXPECT_EQ(map_int.sample(rng2)->first, expected_idx);
  }

  // Update weight.
  map_int.findByKey(0).setWeight(3);
  EXPECT_EQ(6, map_int.totalWeight());
  distro = std::uniform_int_distribution<unsigned>(0, 5);

  const auto total_weight = map_int.totalWeight();
  for (int i = 0; i < 20; ++i) {
    const unsigned scaled = distro(rng1);
    const auto expected_idx = scaled < 3 ? 0 : scaled < 5 ? 1 : 2;

    EXPECT_EQ(map_int.sample(rng2)->first, expected_idx);
    EXPECT_EQ(map_int.sample(scaled)->first, expected_idx);
    EXPECT_EQ(map_int.sampleScaled(double(scaled) / total_weight)->first, expected_idx);
  }

  // Out of range
  EXPECT_FALSE(map_int.sample(total_weight));
}

TEST(OrderStatisticMapTest, SamplingFloat) {
  maplib::SamplingMap<std::string, int, float> map_float{{"a", 0, 1.5}, {"b", 0, 0}, {"c", 0, 2}};
  EXPECT_EQ(3.5, map_float.totalWeight());

  std::ranlux24_base rng1(0);
  std::ranlux24_base rng2(0);

  std::uniform_real_distribution<float> distro(0, 3.5);

  const auto total_weight = map_float.totalWeight();
  for (int i = 0; i < 20; ++i) {
    const auto scaled = distro(rng1);
    const auto expected = scaled < 1.5 ? "a" : "c";

    EXPECT_EQ(map_float.sample(rng2)->first, expected);
    EXPECT_EQ(map_float.sample(scaled)->first, expected);
    EXPECT_EQ(map_float.sampleScaled(scaled / total_weight)->first, expected);
  }

  // At the edge of the boundary.
  EXPECT_TRUE(map_float.sample(total_weight));
  EXPECT_FALSE(map_float.sample(total_weight * (1. + 5 * std::numeric_limits<float>::epsilon())));

  // Test empty map.
  maplib::SamplingMap<int, std::string, double> empty;
  EXPECT_EQ(0, empty.totalWeight());
  EXPECT_FALSE(empty.sample(rng1));
}

//// Manually test insertion, erasure, and retrieval.
TEST(OrderStatisticMapTest, InsertFindErase) {
  maplib::SamplingMap<std::string, int, unsigned> map;
  // Map is empty
  EXPECT_FALSE(map.erase("foo"));

  map.insert("foo", 2, 1);
  map.insert("bar", 1, 1);
  EXPECT_EQ(2, map.size());

  EXPECT_EQ(2, map.findByKey("foo")->second);
  EXPECT_EQ(1, map.findByKey("bar")->second);

  EXPECT_EQ(map.findByKey("baz"), map.end());

  // Change value.
  auto it_bar = map.findByKey("bar");
  ASSERT_TRUE(it_bar);
  it_bar->second = -4;
  EXPECT_EQ(-4, map.findByKey("bar")->second);

  // Erase by iterator
  map.erase(it_bar);
  ASSERT_TRUE(map.checkConsistency());
  // Erase by key.
  EXPECT_TRUE(map.erase("foo"));

  // Map is now empty
  EXPECT_EQ(0, map.size());

  // Test insertion after root has been deleted and test insert return value.
  auto [it_baz, success] = map.insert("baz", 3, 3);
  EXPECT_TRUE(success);
  EXPECT_EQ(1, map.size());
  EXPECT_EQ(3, (*map.findByKey("baz")).second);

  // Change iterator value
  it_baz->second = 5;
  EXPECT_EQ(5, map.findByKey("baz")->second);

  it_baz.setWeight(124);
  EXPECT_EQ(it_baz.getWeight(), map.findByKey("baz").getWeight());
  ASSERT_TRUE(map.checkConsistency());

  auto [it2, success2] = map.insert("baz", 6, 2);
  EXPECT_FALSE(success2);
  EXPECT_EQ(it_baz, it2);
  EXPECT_EQ(6, it2->second);
}

// Perform the test with a number of randomly inserted and removed values.
TEST(OrderStatisticMapTest, InsertRemoveConsistancy) {
  maplib::SamplingMap<int, int, double> my_map;
  std::map<int, int> std_map;

  const int n_insertions = 100;
  const int n_removals = 75;

  // Prepare a shuffled list of unique keys.
  std::vector<int> keys(n_insertions);
  std::iota(keys.begin(), keys.end(), 0);
  std::random_shuffle(keys.begin(), keys.end());

  for (int i = 0; i < n_insertions; ++i) {
    const int val = i;
    std_map[keys[i]] = val;

    my_map.insert(keys[i], val, i + 0.5);
    ASSERT_TRUE(my_map.checkConsistency());
  }

  // Remove random keys.
  std::mt19937_64 rng(0);
  for (int i = 0; i < n_removals; ++i) {
    const std::size_t key_idx = std::uniform_int_distribution<std::size_t>(0, keys.size() - 1)(rng);
    const auto key = keys.at(key_idx);
    keys.erase(keys.begin() + key_idx);

    std_map.erase(key);

    EXPECT_TRUE(my_map.erase(key));
    ASSERT_TRUE(my_map.checkConsistency());
  }

  auto linearized = my_map.linearize();

  ASSERT_EQ(std_map.size(), my_map.size());

  std::size_t idx = 0;
  for (auto it : std_map) {
    EXPECT_EQ(it.first, std::get<0>(linearized[idx]));
    EXPECT_EQ(it.second, std::get<1>(linearized[idx]));

    ++idx;
  }
}

TEST(OrderStatisticMapTest, Assignment) {
  maplib::SamplingMap<int, double, int> map1{{1, 0.5, 1}, {-1, 3.14, 2}, {42, -273.15, 1}};
  maplib::SamplingMap<int, double, int> map2;
  maplib::SamplingMap<int, double, int> map3;

  map2 = map1;
  EXPECT_EQ(map1.linearize(), map2.linearize());

  map3 = std::move(map1);
  EXPECT_EQ(map2.linearize(), map3.linearize());
}
