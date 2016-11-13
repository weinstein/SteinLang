#include <functional>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "util/one_of.h"
#include "util/optional.h"

namespace util {

template <typename ContainerTy,
          typename Eq = std::equal_to<typename ContainerTy::value_type>>
class SequenceDiffer {
 public:
  typedef typename ContainerTy::const_iterator Iterator;
  typedef typename ContainerTy::value_type Value;

  struct Addition {
    Iterator insert_pos;
    Iterator data;
  };

  struct Deletion {
    Iterator data;
    Iterator remove_pos;
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

  std::vector<Modification> Diff(const ContainerTy& lhs, const ContainerTy& rhs);

 private:
  struct ModSeqResult {
    util::Optional<Modification> head;
    const ModSeqResult* tail = nullptr;
    std::size_t len = 0;
  };

  static std::tuple<std::size_t, std::size_t> ToCacheKey(
      const Iterator& lhs_begin, const Iterator& lhs_end,
      const Iterator& rhs_begin, const Iterator& rhs_end) {
    return std::make_tuple(lhs_end - lhs_begin, rhs_end - rhs_begin);
  }

  ModSeqResult Diff(const Iterator& lhs_begin,
                    const Iterator& lhs_end,
                    const Iterator& rhs_begin,
                    const Iterator& rhs_end);

  const ModSeqResult* Recurse(const Iterator& lhs_begin,
                              const Iterator& lhs_end,
                              const Iterator& rhs_begin,
                              const Iterator& rhs_end);

  ModSeqResult MakeModification(const Iterator& lhs_begin,
                                const Iterator& lhs_end,
                                const Iterator& rhs_begin,
                                const Iterator& rhs_end,
                                util::Optional<Modification> m);

  ModSeqResult MakeAddition(const Iterator& lhs_begin,
                            const Iterator& lhs_end,
                            const Iterator& rhs_begin,
                            const Iterator& rhs_end) {
    return MakeModification(lhs_begin, lhs_end, rhs_begin, rhs_end,
                            {Addition{lhs_begin, rhs_begin}});
  }

  ModSeqResult MakeDeletion(const Iterator& lhs_begin,
                            const Iterator& lhs_end,
                            const Iterator& rhs_begin,
                            const Iterator& rhs_end) {
    return MakeModification(lhs_begin, lhs_end, rhs_begin, rhs_end,
                            {Deletion{lhs_begin, rhs_begin}});
  }

  ModSeqResult MakeNoModification(const Iterator& lhs_begin,
                                  const Iterator& lhs_end,
                                  const Iterator& rhs_begin,
                                  const Iterator& rhs_end) {
    return MakeModification(lhs_begin, lhs_end, rhs_begin, rhs_end,
                            util::EmptyOptional());
  }

  struct SeqCacheKeyHasher {
    std::size_t operator()(
        const std::tuple<std::size_t, std::size_t>& x) const {
      auto hasher = std::hash<std::size_t>{};
      return hasher(std::get<0>(x)) ^ hasher(std::get<1>(x));
    }
  };

  const Eq eq_ = Eq();

  std::unordered_map<std::tuple<std::size_t, std::size_t>, ModSeqResult,
                     SeqCacheKeyHasher>
      cached_seqs_;
};

template <typename ContainerTy,
          typename Eq = std::equal_to<typename ContainerTy::value_type>>
std::vector<typename SequenceDiffer<ContainerTy, Eq>::Modification> DiffSeqs(
    const ContainerTy& lhs, const ContainerTy& rhs) {
  return SequenceDiffer<ContainerTy, Eq>().Diff(lhs, rhs);
}

template <typename ContainerTy, typename Eq>
const typename SequenceDiffer<ContainerTy, Eq>::ModSeqResult*
SequenceDiffer<ContainerTy, Eq>::Recurse(const Iterator& lhs_begin,
                                         const Iterator& lhs_end,
                                         const Iterator& rhs_begin,
                                         const Iterator& rhs_end) {
  const auto& key = ToCacheKey(lhs_begin, lhs_end, rhs_begin, rhs_end);
  auto it = cached_seqs_.find(key);
  if (it == cached_seqs_.end()) {
    it = cached_seqs_.emplace(key, Diff(lhs_begin, lhs_end, rhs_begin, rhs_end))
             .first;
  }
  return &(it->second);
}

template <typename ContainerTy, typename Eq>
typename SequenceDiffer<ContainerTy, Eq>::ModSeqResult
SequenceDiffer<ContainerTy, Eq>::MakeModification(
    const Iterator& lhs_begin, const Iterator& lhs_end,
    const Iterator& rhs_begin, const Iterator& rhs_end,
    util::Optional<Modification> m) {
  std::size_t len = 0;
  const ModSeqResult* tail;
  if (m.is_present()) {
    tail = m.value().is_addition()
               ? Recurse(lhs_begin, lhs_end, rhs_begin + 1, rhs_end)
               : Recurse(lhs_begin + 1, lhs_end, rhs_begin, rhs_end);
    len = 1 + tail->len;
  } else {
    tail = Recurse(lhs_begin + 1, lhs_end, rhs_begin + 1, rhs_end);
    len = tail->len;
  }
  return ModSeqResult{std::move(m), tail, len};
}

template <typename ContainerTy, typename Eq>
std::vector<typename SequenceDiffer<ContainerTy, Eq>::Modification>
SequenceDiffer<ContainerTy, Eq>::Diff(const ContainerTy& lhs,
                                      const ContainerTy& rhs) {
  cached_seqs_.clear();
  const ModSeqResult* result_list =
      Recurse(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
  std::vector<Modification> result;
  while (result_list != nullptr) {
    if (result_list->head.is_present()) {
      result.push_back(result_list->head.value());
    }
    result_list = result_list->tail;
  }
  return result;
}

template <typename ContainerTy, typename Eq>
typename SequenceDiffer<ContainerTy, Eq>::ModSeqResult
SequenceDiffer<ContainerTy, Eq>::Diff(const Iterator& lhs_begin,
                                      const Iterator& lhs_end,
                                      const Iterator& rhs_begin,
                                      const Iterator& rhs_end) {
  util::Optional<ModSeqResult> result;
  if (lhs_begin < lhs_end) {
    result = MakeDeletion(lhs_begin, lhs_end, rhs_begin, rhs_end);
  }
  if (rhs_begin < rhs_end) {
    ModSeqResult add_rhs = MakeAddition(lhs_begin, lhs_end, rhs_begin, rhs_end);
    if (!result.is_present() || add_rhs.len < result.value().len) {
      result = std::move(add_rhs);
    }
  }
  if (lhs_begin < lhs_end && rhs_begin < rhs_end &&
      eq_(*lhs_begin, *rhs_begin)) {
    ModSeqResult no_mod =
        MakeNoModification(lhs_begin, lhs_end, rhs_begin, rhs_end);
    if (!result.is_present() || no_mod.len < result.value().len) {
      result = std::move(no_mod);
    }
  }
  if (result.is_present()) {
    return std::move(result.mutable_value());
  }
  return ModSeqResult{util::EmptyOptional(), nullptr, 0};
}

}  // namespace util
