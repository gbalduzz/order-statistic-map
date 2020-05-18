// Copyright (C) ETH Zurich
// Copyright (C) 2020 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// Test for the SamplingSet class.

#include "order_statistic_map/sampling_set.hpp"

#include <set>
#include <random>
#include <string>

#include "gtest/gtest.h"

TEST(SamplingSetTest, SamplingInt) {
  maplib::SamplingSet<int, unsigned> set_int{{0, 1}, {1, 2}, {2, 1}};
  EXPECT_EQ(4, set_int.totalWeight());

  auto expected = [](int sample) { return sample < 1 ? 0 : sample < 3 ? 1 : 2; };

  std::ranlux24_base rng1(0);
  std::ranlux24_base rng2(0);

  std::uniform_int_distribution<unsigned> distro(0, 3);

  for (int i = 0; i < 20; ++i) {
    const unsigned scaled = distro(rng1);
    const auto expected_idx = expected(scaled);

    EXPECT_EQ(set_int.sample(rng2), expected_idx);
  }

  const auto total_weight = set_int.totalWeight();
  for (int i = 0; i < 20; ++i) {
    const unsigned scaled = distro(rng1);
    const auto expected_idx = expected(scaled);

    EXPECT_EQ(set_int.sample(rng2), expected_idx);
    EXPECT_EQ(set_int.sample(scaled), expected_idx);
    EXPECT_EQ(set_int.sampleScaled(double(scaled) / total_weight), expected_idx);
  }

  // Out of range
  EXPECT_THROW(set_int.sample(total_weight), std::out_of_range);
}

TEST(SamplingSetTest, SamplingFloat) {
  maplib::SamplingSet<std::string, float> set_float{{"a", 1.5}, {"b", 0}, {"c", 2}};
  EXPECT_EQ(3.5, set_float.totalWeight());

  std::ranlux24_base rng1(0);
  std::ranlux24_base rng2(0);

  std::uniform_real_distribution<float> distro(0, 3.5);

  const auto total_weight = set_float.totalWeight();
  for (int i = 0; i < 20; ++i) {
    const auto scaled = distro(rng1);
    const auto expected = scaled < 1.5 ? "a" : "c";

    EXPECT_EQ(set_float.sample(rng2), expected);
    EXPECT_EQ(set_float.sample(scaled), expected);
    EXPECT_EQ(set_float.sampleScaled(scaled / total_weight), expected);
  }

  // At the edge of the boundary.
  EXPECT_NO_THROW(set_float.sample(total_weight));
  EXPECT_THROW(set_float.sample(total_weight * (1. + 5 * std::numeric_limits<float>::epsilon())),
               std::out_of_range);

  // Test empty set.
  maplib::SamplingSet<int, double> empty;
  EXPECT_EQ(0, empty.totalWeight());
  EXPECT_THROW(empty.sample(rng1), std::out_of_range);
}

// Manually test insertion, erasure, and retrieval.
TEST(SamplingSetTest, InsertFindErase) {
  maplib::SamplingSet<std::string, int> set;
  // Set is empty
  EXPECT_FALSE(set.erase("foo"));

  set.insert("foo", 1);
  set.insert("bar", 1);
  EXPECT_EQ(2, set.size());

  EXPECT_TRUE(set.count("foo"));
  EXPECT_TRUE(set.count("bar"));
  EXPECT_FALSE(set.count("baz"));

  EXPECT_TRUE(set.erase("bar"));
  EXPECT_TRUE(set.erase("foo"));

  // Set is now empty
  EXPECT_EQ(0, set.size());
}

TEST(SamplingSetTest, Assignment) {
  maplib::SamplingSet<int, int> set1{{1, 1}, {-1, 2}, {42, 1}};
  maplib::SamplingSet<int, int> set2;
  maplib::SamplingSet<int, int> set3;

  set2 = set1;
  EXPECT_EQ(set1.linearize(), set2.linearize());

  set3 = std::move(set1);
  EXPECT_EQ(set2.linearize(), set3.linearize());
}
