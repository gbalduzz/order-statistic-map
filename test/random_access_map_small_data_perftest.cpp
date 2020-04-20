// Copyright (C) ETH Zurich
// Copyright (C) 2020 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// RandomAccessMap performance test

#include "random_access_map/random_access_map.hpp"

#include <vector>
#include <random>
#include <string>
#include <map>
#include <unordered_map>

#include <benchmark/benchmark.h>

#define ARGS RangeMultiplier(4)->Range(64, 8<<12)

const unsigned n_init = 50000;
const unsigned n_test = 10;

using Key = int;
using Value = int;

std::vector<Key> keys;
std::vector<Value> vals;

void init() {
  static bool initialized = false;
  if (initialized)
    return;
  initialized = true;

  for (int i = 0; i < n_init + n_test; ++i) {
    keys.push_back(i);
    vals.push_back(i);
  }

  std::random_shuffle(keys.begin(), keys.end());
  std::random_shuffle(vals.begin(), vals.end());
}

template <template <class, class> class Map>
static void performInsertRemoveTest(benchmark::State& state) {
  init();
  Map<Key, Value> map;
  for (int i = 0; i < state.range(0); ++i)
    map.insert({keys[i], vals[i]});

  for (auto _ : state) {
    for (int i = n_init; i < n_init + n_test; ++i)
      map.insert({keys[i], vals[i]});
    for (int i = n_init; i < n_init + n_test; ++i)
      map.erase(keys[i]);
  }
}

static void BM_StdMapInsertErase(benchmark::State& state) {
  performInsertRemoveTest<std::map>(state);
}
BENCHMARK(BM_StdMapInsertErase)->ARGS;

static void BM_StdUnorderedMapInsertErase(benchmark::State& state) {
  performInsertRemoveTest<std::unordered_map>(state);
}
BENCHMARK(BM_StdUnorderedMapInsertErase)->ARGS;

static void BM_MyMapInsertErase(benchmark::State& state) {
  performInsertRemoveTest<ramlib::RandomAccessMap>(state);
}
BENCHMARK(BM_MyMapInsertErase)->ARGS;

template <template <class, class> class Map>
static void performFindTest(benchmark::State& state) {
  init();
  Map<Key, Value> map;
  for (int i = 0; i < state.range(0); ++i)
    map.insert({keys[i], vals[i]});

  for (auto _ : state) {
    for (int i = 0; i < n_test; ++i)
      benchmark::DoNotOptimize(map.count(keys[i]));
  }
}

static void BM_StdMapFind(benchmark::State& state) {
  performFindTest<std::map>(state);
}
BENCHMARK(BM_StdMapFind)->ARGS;

static void BM_StdUnorderedMapFind(benchmark::State& state) {
  performFindTest<std::unordered_map>(state);
}
BENCHMARK(BM_StdUnorderedMapFind)->ARGS;

static void BM_MyMapFind(benchmark::State& state) {
  performFindTest<ramlib::RandomAccessMap>(state);
}
BENCHMARK(BM_MyMapFind)->ARGS;
