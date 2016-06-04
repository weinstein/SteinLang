#ifndef UTIL_MAP_UTIL_H_
#define UTIL_MAP_UTIL_H_

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

#endif  // UTIL_MAP_UTIL_H_
