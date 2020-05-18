// Copyright (C) ETH Zurich
// Copyright (C) 2020 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// OrderStatisticMap performance test

#include "order_statistic_map/sampling_map.hpp"

#include <vector>
#include <random>

#include <benchmark/benchmark.h>

const unsigned n_init = 10000;
const unsigned n_test = 10;

using Key = int;
using Value = int;

std::vector<Key> keys;
std::vector<Value> vals;
std::vector<float> weights;

void init() {
  static bool initialized = false;
  if (initialized)
    return;
  initialized = true;

  for (int i = 0; i < n_init + n_test; ++i) {
    keys.push_back(i);
    vals.push_back(i);
    weights.push_back(i + 0.5);
  }

  std::random_shuffle(keys.begin(), keys.end());
  std::random_shuffle(vals.begin(), vals.end());
  std::random_shuffle(weights.begin(), weights.end());
}

static void BM_SamplingMapInsertErase(benchmark::State& state) {
  init();
  maplib::SamplingMap<Key, Value, float> map;
  for (int i = 0; i < state.range(0); ++i)
    map.insert(keys[i], vals[i], weights[i]);

  for (auto _ : state) {
    for (int i = n_init; i < n_init + n_test; ++i)
      map.insert(keys[i], vals[i], weights[i]);
    for (int i = n_init; i < n_init + n_test; ++i)
      map.erase(keys[i]);
  }
}
BENCHMARK(BM_SamplingMapInsertErase)->Arg(100)->Arg(1000)->Arg(n_init);

static void BM_SamplingMapSample(benchmark::State& state) {
  init();
  maplib::SamplingMap<Key, Value, float> map;
  for (int i = 0; i < state.range(0); ++i)
    map.insert(keys[i], vals[i], weights[i]);

  std::vector<maplib::SamplingMap<Key, Value, float>::iterator> findings(n_test);
  std::ranlux24_base rng(0);

  for (auto _ : state) {
    for (int i = 0; i < n_test; ++i)
      findings[i] = map.sample(rng);
  }
}
BENCHMARK(BM_SamplingMapSample)->Arg(100)->Arg(1000)->Arg(n_init);
