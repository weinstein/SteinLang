#include <deque>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "util/one_of.h"

namespace util {

template <typename It>
class SequenceDiffer {
 public:
  struct Addition {
    It insert_pos;
    It data;
  };
  struct Deletion {
    It delete_pos;
  };
  struct Modification {
    Modification(Addition a) : data(std::move(a)) {}
    Modification(Deletion d) : data(std::move(d)) {}

    bool is_addition() const { return data.is_first(); }
    const Addition& addition() const { return data.first(); }

    bool is_deletion() const { return data.is_second(); }
    const Deletion& deletion() const { return data.second(); }

    util::OneOf<Addition, Deletion> data;
  };

  template <typename T>
  std::deque<Modification> Diff(const T& lhs, const T& rhs) {
    cached_seqs_.clear();
    return Diff(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
  }

 private:
  std::deque<Modification> Diff(const It& lhs_begin, const It& lhs_end,
                                const It& rhs_begin, const It& rhs_end);

  std::deque<Modification> Recurse(const It& lhs_begin, const It& lhs_end,
                                   const It& rhs_begin, const It& rhs_end);

  static std::tuple<std::size_t, std::size_t> ToCacheKey(const It& lhs_begin,
                                                         const It& lhs_end,
                                                         const It& rhs_begin,
                                                         const It& rhs_end) {
    return std::make_tuple(lhs_end - lhs_begin, rhs_end - rhs_begin);
  }

  struct SeqCacheKeyHasher {
    std::size_t operator()(
        const std::tuple<std::size_t, std::size_t>& x) const {
      auto hasher = std::hash<std::size_t>{};
      return hasher(std::get<0>(x)) ^ hasher(std::get<1>(x));
    }
  };

  std::unordered_map<
      std::tuple<std::size_t, std::size_t>,
      std::deque<Modification>,
      SeqCacheKeyHasher>
      cached_seqs_;
};

template <typename T>
std::deque<typename SequenceDiffer<typename T::const_iterator>::Modification> Diff(
    const T& lhs, const T& rhs) {
  return SequenceDiffer<typename T::const_iterator>().Diff(lhs, rhs);
}

template <typename It>
std::deque<typename SequenceDiffer<It>::Modification> SequenceDiffer<It>::Recurse(
    const It& lhs_begin, const It& lhs_end, const It& rhs_begin,
    const It& rhs_end) {
  const auto& key = ToCacheKey(lhs_begin, lhs_end, rhs_begin, rhs_end);
  auto it = cached_seqs_.find(key);
  if (it != cached_seqs_.end()) {
    return it->second;
  }
  return cached_seqs_[key] = Diff(lhs_begin, lhs_end, rhs_begin, rhs_end);
}

template <typename It>
std::deque<typename SequenceDiffer<It>::Modification> SequenceDiffer<It>::Diff(
    const It& lhs_begin, const It& lhs_end, const It& rhs_begin,
    const It& rhs_end) {
  if (lhs_begin < lhs_end && rhs_begin < rhs_end) {
    std::deque<Modification> remove_lhs =
        Recurse(lhs_begin + 1, lhs_end, rhs_begin, rhs_end);
    remove_lhs.push_front(Modification{Deletion{lhs_begin}});
    std::deque<Modification> add_rhs =
        Recurse(lhs_begin, lhs_end, rhs_begin + 1, rhs_end);
    add_rhs.push_front(Modification{Addition{lhs_begin, rhs_begin}});
    auto result = remove_lhs.size() < add_rhs.size() ? std::move(remove_lhs)
                                                     : std::move(add_rhs);

    if (*lhs_begin == *rhs_begin) {
      std::deque<Modification> subseq =
          Recurse(lhs_begin + 1, lhs_end, rhs_begin + 1, rhs_end);
      if (subseq.size() < result.size()) {
        result = std::move(subseq);
      }
    }
    return std::move(result);
  }

  std::deque<Modification> result;
  for (It i = rhs_begin; i != rhs_end; ++i) {
    result.push_back(Modification{Addition{lhs_end, i}});
  }
  for (It i = lhs_begin; i != lhs_end; ++i) {
    result.push_back(Modification{Deletion{i}});
  }
  return result;
}

}  // namespace util
