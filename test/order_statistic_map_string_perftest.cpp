// Copyright (C) ETH Zurich
// Copyright (C) 2020 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// OrderStatisticMap performance test

#include "order_statistic_map/order_statistic_map.hpp"

#include <vector>
#include <random>
#include <string>
#include <map>

#include <benchmark/benchmark.h>

#define ARGS RangeMultiplier(4)->Range(64, 8 << 12)

const unsigned n_init = 100000;

using Key = std::string;
using Value = std::string;

std::vector<Key> keys;
std::vector<Value> vals;

template <class K, class V>
using PooledMap = std::map<K, V, std::less<K>, maplib::FixedSizeAllocator<std::pair<const K, V>>>;

void init() {
  static bool initialized = false;
  if (initialized)
    return;
  initialized = true;

  for (int i = 0; i < n_init; ++i) {
      keys.push_back("key " + std::to_string(i));
      vals.push_back("value " + std::to_string(i));
  }

  std::random_shuffle(keys.begin(), keys.end());
  std::random_shuffle(vals.begin(), vals.end());
}

template <template <class, class> class Map, bool pair>
static void performInsertRemoveTest(benchmark::State& state) {
  init();
  Map<Key, Value> map;
  for (int i = 0; i < state.range(0); ++i)
    map.insert({keys[i], vals[i]});

  unsigned idx_insert = state.range(0);
  unsigned idx_erase = 0;

  for (auto _ : state) {
    if constexpr (pair)
      map.insert({keys[idx_insert], vals[idx_insert]});
    else
      map.insert(keys[idx_insert], vals[idx_insert]);

    map.erase(keys[idx_erase]);

    ++idx_erase;
    ++idx_insert;
    if (idx_erase == keys.size())
      idx_erase = 0;
    if (idx_insert == keys.size())
      idx_insert = 0;
  }
}

static void BM_StdMapInsertErase(benchmark::State& state) {
  performInsertRemoveTest<std::map, true>(state);
}
BENCHMARK(BM_StdMapInsertErase)->ARGS;

static void BM_PooledStdMapInsertErase(benchmark::State& state) {
  performInsertRemoveTest<PooledMap, true>(state);
}
BENCHMARK(BM_PooledStdMapInsertErase)->ARGS;

static void BM_MyMapInsertErase(benchmark::State& state) {
  performInsertRemoveTest<maplib::OrderStatisticMap, false>(state);
}
BENCHMARK(BM_MyMapInsertErase)->ARGS;

template <template <class, class> class Map>
static void performFindTest(benchmark::State& state) {
  init();
  Map<Key, Value> map;
  for (int i = 0; i < state.range(0); ++i)
    map.insert({keys[i], vals[i]});

  unsigned idx = 0;

  for (auto _ : state) {
    benchmark::DoNotOptimize(map.count(keys[idx]));
    ++idx;
    if (idx == keys.size())
      idx = 0;
  }
}

static void BM_StdMapFind(benchmark::State& state) {
  performFindTest<std::map>(state);
}
BENCHMARK(BM_StdMapFind)->ARGS;

static void BM_PooledStdMapFind(benchmark::State& state) {
  performFindTest<PooledMap>(state);
}
BENCHMARK(BM_PooledStdMapFind)->ARGS;

static void BM_MyMapFind(benchmark::State& state) {
  performFindTest<maplib::OrderStatisticMap>(state);
}
BENCHMARK(BM_MyMapFind)->ARGS;
