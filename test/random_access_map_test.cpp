// Copyright (C) 2020 Giovanni Balduzzi
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// Bidirectional iterator for the RandomAccessMap class

#include "random_access_map/random_access_map.hpp"

#include <map>
#include <random>
#include <string>

#include "gtest/gtest.h"

// Manually test insertion, erasure, and retrieval.
TEST(RandomAccessMapTest, InsertFindErase) {
  ramlib::RandomAccessMap<std::string, int> map;
  // Map is empty
  EXPECT_FALSE(map.erase("foo"));

  map.insert("foo", 2);
  map.insert("bar", 1);
  EXPECT_EQ(2, map.size());

  EXPECT_EQ(2, map.findByKey("foo")->second);
  EXPECT_EQ(1, map.findByKey("bar")->second);
  EXPECT_EQ("foo", map.findByIndex(1)->first);  // foo > bar.
  EXPECT_EQ(2, map.findByIndex(1)->second);

  EXPECT_EQ("bar", map.findByIndex(0)->first);
  EXPECT_EQ(1, map.findByIndex(0)->second);

  EXPECT_EQ(map.findByKey("baz"), map.end());
  EXPECT_THROW(map.findByIndex(2), std::out_of_range);

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
  auto [it_baz, success] = map.insert("baz", 3);
  EXPECT_TRUE(success);
  EXPECT_EQ(1, map.size());
  EXPECT_EQ(3, (*map.findByKey("baz")).second);
  EXPECT_EQ(3, (*map.findByIndex(0)).second);

  // Change iterator value
  it_baz->second = 5;
  EXPECT_EQ(5, map.findByKey("baz")->second);

  auto [it2, success2] = map.insert("baz", 6);
  EXPECT_FALSE(success2);
  EXPECT_EQ(it_baz, it2);
  EXPECT_EQ(6, it2->second);
}

// Perform the test with a number of randomly inserted and removed values.
TEST(RandomAccessMapTest, LinearizeAndRandomAccess) {
  ramlib::RandomAccessMap<int, int> my_map;
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

    my_map.insert(keys[i], val);
    ASSERT_TRUE(my_map.checkConsistency());
  }

  // Remove random keys.
  std::mt19937_64 rng(0);
  for (int i = 0; i < n_removals; ++i) {
    const std::size_t key_idx = std::uniform_int_distribution<std::size_t>(0, keys.size() - 1)(rng);
    const auto key = keys.at(key_idx);
    keys.erase(keys.begin() + key_idx);

    std_map.erase(key);

    my_map.erase(key);
    ASSERT_TRUE(my_map.checkConsistency());
  }

  auto linearized = my_map.linearize();

  ASSERT_EQ(std_map.size(), my_map.size());

  std::size_t idx = 0;
  for (auto it : std_map) {
    const auto my_it = my_map.findByIndex(idx);

    EXPECT_EQ(it.first, linearized[idx].first);
    EXPECT_EQ(it.second, linearized[idx].second);

    // Test random accessor.
    EXPECT_EQ(it.first, my_it->first);
    EXPECT_EQ(it.second, my_it->second);

    ++idx;
  }
}

TEST(RandomAccessMapTest, Assignment) {
  ramlib::RandomAccessMap<int, double> map1{{1, 0.5}, {-1, 3.14}, {42, -273.15}};
  ramlib::RandomAccessMap<int, double> map2;
  ramlib::RandomAccessMap<int, double> map3;

  map2 = map1;
  EXPECT_EQ(map1.linearize(), map2.linearize());

  map3 = std::move(map1);
  EXPECT_EQ(map2.linearize(), map3.linearize());

  ramlib::RandomAccessMap<int, double> map4(map3.linearize());
  EXPECT_EQ(map4.linearize(), map3.linearize());
}
