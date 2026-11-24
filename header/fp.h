#ifndef FP_H
#define FP_H

#include <algorithm>
#include <iterator>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

namespace fp {

template <class Seq, class Pred>
Seq filter(const Seq& source, Pred keep) {
    Seq result;
    std::copy_if(source.begin(), source.end(), std::back_inserter(result), keep);
    return result;
}

template <class Seq, class Fn>
auto map(const Seq& source, Fn transform) {
    using Out = std::decay_t<std::invoke_result_t<Fn, const typename Seq::value_type&>>;
    std::vector<Out> result;
    result.reserve(source.size());
    std::transform(source.begin(), source.end(), std::back_inserter(result), transform);
    return result;
}

template <class Seq, class T, class Op>
T reduce(const Seq& source, T initial, Op combine) {
    return std::accumulate(source.begin(), source.end(), std::move(initial), combine);
}

}

#endif
