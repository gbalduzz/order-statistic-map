// Copyright (C) ETH Zurich
// Copyright (C) 2020 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// Provides a map with O(log n) insertion, removal and random access, i.e. access of the value
// relative to the i-th lowest key.
// Useful for a random selection in an ordered list with variable size.
// Implemented as an augmented red-black tree.

#pragma once

#include "sampling_map.hpp"

namespace maplib {

// Precondition: elements of type Key have full order.
template <class Key, class Weight, std::size_t chunk_size = 64>
class SamplingSet {
private:
  struct Null {
    Null() = default;
  };

public:
  using Node = details::WeightedNode<Key, Null, Weight>;
  using const_iterator = SamplingMapIterator<Node, true>;

  SamplingSet() = default;
  SamplingSet(const std::initializer_list<std::pair<Key, Weight>>& list);
  SamplingSet(const std::vector<std::pair<Key, Weight>>& linearized);

  SamplingSet(const SamplingSet& rhs) = default;
  SamplingSet(SamplingSet&& rhs) = default;

  SamplingSet& operator=(const SamplingSet& rhs) = default;
  SamplingSet& operator=(SamplingSet&& rhs) = default;

  auto begin() const noexcept {
    return map_.begin();
  };
  auto end() const noexcept {
    return map_.end();
  };

  auto begin() noexcept {
    return map_.begin();
  };
  auto end() noexcept {
    return map_.end();
  };

  // Insert new key. Returns false if the key is already present.
  bool insert(const Key& key, const Weight& weight) noexcept;
  bool insert(const std::pair<Key, Weight>& values) noexcept {
    return insert(values.first, values.second);
  }

  // Remove the node relative to key.
  // Returns: true if the key is found and removed. False if no operation is performed.
  bool erase(const Key& key) noexcept;

  // Returns true if a stored key compares equal to the argument.
  bool contains(const Key& key) const noexcept {
    return map_.contains(key);
  }
  bool count(const Key& key) const noexcept {
    return contains(key);
  }

  // Returns an iterator relative to a node sampled with probability proportional to its weight.
  template <class Rng>
  const Key& sample(Rng& rng) const;

  // Sample the node that satisfies: weight(left subtree) <= `position` <
  //                                 weight(left subtree) + weight(node)
  // If the chosen position is outside [0, totalWeight()] returns the null iterator.
  // If the weight is a floating point number, a weight of totalWeight() will result in the last
  // entry, otherwise it results in the null iterator.
  const Key& sample(Weight position) const;

  // Sample from a value scaled in [0, 1].
  const Key& sampleScaled(double position) const {
    return sample(position * totalWeight());
  }

  // Returns an array of ordered keys and value pairs.
  std::vector<std::pair<Key, Weight>> linearize() const noexcept;

  std::size_t size() const noexcept {
    return map_.size();
  }

  Weight totalWeight() const noexcept {
    return map_.totalWeight();
  }

  // For testing purposes.
  bool checkConsistency() const noexcept {
    return map_.checkConsistency();
  }

private:
  SamplingMap<Key, Null, Weight> map_;
};

template <class Key, class Weight, std::size_t chunk_size>
SamplingSet<Key, Weight, chunk_size>::SamplingSet(
    const std::initializer_list<std::pair<Key, Weight>>& list) {
  for (const auto& elem : list)
    insert(elem);
}

template <class Key, class Weight, std::size_t chunk_size>
SamplingSet<Key, Weight, chunk_size>::SamplingSet(const std::vector<std::pair<Key, Weight>>& linearized) {
  for (const auto& elem : linearized)
    insert(elem);
}

template <class Key, class Weight, std::size_t chunk_size>
bool SamplingSet<Key, Weight, chunk_size>::insert(const Key& key, const Weight& weight) noexcept {
  auto [it, inserted] = map_.insert(key, {}, weight);
  return inserted;
}

template <class Key, class Weight, std::size_t chunk_size>
bool SamplingSet<Key, Weight, chunk_size>::erase(const Key& key) noexcept {
  return map_.erase(key);
}

template <class Key, class Weight, std::size_t chunk_size>
template <class Rng>
const Key& SamplingSet<Key, Weight, chunk_size>::sample(Rng& rng) const {
  auto it = map_.sample(rng);
  if (it == map_.end())
    throw(std::out_of_range("Sampling out of the set range."));
  return it->first;
}

template <class Key, class Weight, std::size_t chunk_size>
const Key& SamplingSet<Key, Weight, chunk_size>::sample(const Weight position) const {
  auto it = map_.sample(position);
  if (it == map_.end())
    throw(std::out_of_range("Sampling out of the set range."));
  return it->first;
}

template <class Key, class Weight, std::size_t chunk_size>
std::vector<std::pair<Key, Weight>> SamplingSet<Key, Weight, chunk_size>::linearize() const noexcept {
  std::vector<std::pair<Key, Weight>> result;
  result.reserve(size());

  for (auto it = begin(); it != end(); ++it)
    result.emplace_back(it->first, it.getWeight());

  return result;
}

}  // namespace maplib
