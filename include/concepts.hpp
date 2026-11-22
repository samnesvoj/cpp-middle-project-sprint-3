#pragma once

#include <concepts>
#include <iterator>
#include <ranges>
#include <string_view>
#include <type_traits>

#include "book.hpp"

namespace bookdb {

// непрерывный, потому что BookDatabase отдаёт std::span<Book>
template <typename T>
concept BookContainerLike = std::ranges::contiguous_range<T> && std::ranges::sized_range<T> &&
                            std::same_as<std::ranges::range_value_t<T>, Book> && std::default_initializable<T> &&
                            std::copyable<T> && requires(T container, const Book &book) {
                                container.push_back(book);
                                container.emplace_back(book);
                                container.pop_back();
                                container.back();
                                { container.clear() } noexcept;
                            };

template <typename T>
concept BookIterator = std::forward_iterator<T> && std::same_as<std::iter_value_t<T>, Book> &&
                       std::is_lvalue_reference_v<std::iter_reference_t<T>>;

template <typename S, typename I>
concept BookSentinel = BookIterator<I> && std::sentinel_for<S, I>;

template <typename P>
concept BookPredicate = std::copy_constructible<P> && std::predicate<const P &, const Book &>;

template <typename C>
concept BookComparator = std::copy_constructible<C> && std::strict_weak_order<const C &, const Book &, const Book &>;

}  // namespace bookdb
