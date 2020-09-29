# maplib::OrderStatisticMap
\#include<maplib/order_statistic_map.hpp>
```
template <class Key, class Value, std::size_t chunk_size = 64>
class OrderStatisticMap; 
```

Provides an associative container supporting insertion, erasure and search on O(log n). In addition
 search of the element associated to the k-th largest key is supported in O(log n).
 
 The interface is mostly identical to `std::map`, with the exception of `find` being replaced by 
 `findByKey` and `findByIndex`, and `operator[]` being removed due to the ambiguity in the access 
 method.
 
 Every iterator stays valid until it is explicitly erased or the container is destructed.

##Template Parameters

- `class Key`: Type of the keys. Each element in a map is uniquely identified by its key value.
Operator `<` must be defined on this type.
- `class Value`: type of the value associated with each key.
- `std::size_t chunk_size` number of elements    


## Methods (partial)
```
  iterator findByKey(const std::size_t index) noexcept;
  const_iterator findByKey(const std::size_t index) const noexcept;
```
Returns an iterator to the key-value pair associated with the specified key. 
If the key is not present a null iterator is returned.

```
  iterator findByIndex(const std::size_t index) noexcept;
  const_iterator findByIndex(const std::size_t index) const noexcept;
```
Returns an iterator to the key-value pair associated with the index-th lowest key present in the 
container. The validity of `index` is tested only in debug mode.
