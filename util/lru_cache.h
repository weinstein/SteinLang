#ifndef UTIL_LRU_CACHE_H_
#define UTIL_LRU_CACHE_H_

#include <list>
#include <unordered_map>
#include <utility>

namespace steinlang {

// A cache which stores at most max_size entries, evicting the least recently
// used entries when full.
// The stored value types can be any moveable type.
// Objects stored in the cache are only destructed when evicted (or when the
// LRUCache is destroyed) which allows for behavior upon cache eviction to be
// extended by using RAII-style value types that do work in destructors.
template <typename K, typename V>
class LRUCache {
 public:

  typedef std::pair<const K, V> value_type;

  LRUCache(size_t max_size) : max_size_(max_size) {}
  virtual ~LRUCache() = default;

  LRUCache(const LRUCache&) = delete;
  LRUCache& operator=(const LRUCache&) = delete;

  size_t size() const { return data_.size(); }

  value_type& FindWithDefault(value_type x) {
    auto map_it = data_pos_.find(x.first);
    if (map_it != data_pos_.end()) {
      data_.splice(data_.begin(), data_, map_it->second);
      return data_.front();
    } else {
      // Evict until there's room.
      while (size() >= max_size_) {
        Evict(data_.back());
      }
      // Emplace new item at the front of data_ and return it.
      auto list_it = data_.emplace(data_.begin(), std::move(x));
      data_pos_.insert({list_it->first, list_it});
      return *list_it;
    }
  }

  template <typename... Args>
  V& FindOrEmplace(const K& key, Args&&... args) {
    return FindWithDefault({key, V{std::forward<Args>(args)...}}).second;
  }

 private:
  void Evict(const value_type& x) {
    const K& key = x.first;
    auto map_it = data_pos_.find(key);
    auto list_it = map_it->second;
    data_.erase(list_it);
    data_pos_.erase(map_it);
  }

  const size_t max_size_;
  std::unordered_map<K, typename std::list<value_type>::iterator> data_pos_;
  std::list<value_type> data_;
};

}  // namespace steinlang

#endif  // UTIL_LRU_CACHE_H_
