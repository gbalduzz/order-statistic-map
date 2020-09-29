# maplib::SamplingMap
\#include<maplib/sampling_map.hpp>
```
template <class Key, class Value, class Weight, std::size_t chunk_size = 64>
class SamplingMap
```

Provides an associative container supporting insertion, erasure and search by key in O(log n).
Each key-value pair is associated with an integer or floting point weight. 
In addition the container sampling randomly or pseudo-randomly elements with a probability proportional to the weight.
 
Every iterator stays valid until it is explicitly erased or the container is destructed.

##Template Parameters

- `class Key`: Type of the keys. Each element in a map is uniquely identified by its key value.
Operator `<` must be defined on this type.
- `class Value`: type of the value associated with each key.
- `class Weight`: type of the weight proportional to the sampling probability.
- `std::size_t chunk_size` number of elements    


## Methods (partial)
```
  iterator findByKey(const std::size_t index) noexcept;
  const_iterator findByKey(const std::size_t index) const noexcept;
```
Returns an iterator to the key-value pair associated with the specified key. 
If the key is not present a null iterator is returned.

```
  template <class Rng> 
  iterator sample(Rng& rng) noexcept;
  template <class Rng> 
  const_iterator sample(Rng& rng) const noexcept;
```
Returns an iterator to a key-value pair sampled with probability proportional to its weight.
`std::uniform_int_distribution` or `std::uniform_real_distribution` must accept an argument of type 
Rng. Complexity: O(log n).

```
 Weight totalWeight() const noexcept
```
Returns the summed weight of all the container's entries.

## Iterators
The (const) iterator associated to `maplib::SamplingMap` provides the following useful method.

```
const std::pair<const Key, Value>* operator->() const 
std::pair<const Key, Value>* operator->()  
```
Arrow operator. Used to access the key-value pair

```
  Weight getWeight() const 
```
Returns the weight associated to the key-value pair

```
  void setWeight(Weight weight)  
```
Changes the weight associated to the key-value pair, updating its sampling probability.
