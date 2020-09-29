# maplib::OrderStatisticSet
\#include<maplib/order_statistic_set.hpp>
```
template <class Key, std::size_t chunk_size = 64>
class OrderStatisticset; 
```

Provides a container equivalent to a `maplib::OrderStatisticMap` with no value stored. 
 
 The interface is mostly identical to `std::set`, with the addition of  `findByIndex`.
 References stays valid until the referenced key  is explicitly erased or the container is destructed.

##Template Parameters

- `class Key`: Type of the keys. Each element in a set is uniquely identified by its key value.
Operator `<` must be defined on this type.
- `std::size_t chunk_size` number of elements    

## Methods (partial)
```
  const Key& findByIndex(const std::size_t index) const noexcept;
```
Returns a reference to the index-th lowest key present in the container. The validity of index
is tested only in debug mode.
