#ifndef MAP_UTIL_H__
#define MAP_UTIL_H__

namespace steinlang {

template <typename ContainerTy>
bool ContainsKey(const ContainerTy& container,
                 const typename ContainerTy::key_type& key) {
  return container.count(key);
}

template <typename MapTy>
const typename MapTy::mapped_type& FindOrDie(
    const MapTy& m, const typename MapTy::key_type& key) {
  return m.find(key)->second;
}

}  // namespace steinlang

#endif  // MAP_UTIL_H__
