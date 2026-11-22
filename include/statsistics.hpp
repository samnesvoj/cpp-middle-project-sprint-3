#pragma once

#include <algorithm>
#include <flat_map>
#include <functional>
#include <iterator>
#include <random>
#include <ranges>
#include <string>
#include <string_view>

#include "book_database.hpp"

namespace bookdb {

// Компаратор прозрачный, чтобы искать по string_view без создания std::string.
template <BookContainerLike T, typename Comparator = TransparentStringLess>
    requires std::strict_weak_order<Comparator, std::string_view, std::string_view> &&
             requires { typename Comparator::is_transparent; }
auto buildAuthorHistogramFlat(const BookDatabase<T> &cont, Comparator comp = {}) {
    std::flat_map<std::string, std::size_t, Comparator> histogram(std::move(comp));
    for (const Book &book : cont) {
        const auto it = histogram.lower_bound(book.author);
        if (it != histogram.end() && !histogram.key_comp()(book.author, it->first)) {
            ++it->second;
        } else {
            histogram.emplace_hint(it, std::string(book.author), 1);
        }
    }
    return histogram;
}

template <BookIterator It, BookSentinel<It> Sent>
auto calculateGenreRatings(It first, Sent last) {
    struct Total {
        double sum = 0.0;
        std::size_t count = 0;
    };

    std::flat_map<Genre, Total> totals;
    std::ranges::for_each(first, last, [&totals](const Book &book) {
        Total &total = totals.try_emplace(book.genre).first->second;
        total.sum += book.rating;
        ++total.count;
    });

    std::flat_map<Genre, double> averages;
    for (const auto &[genre, total] : totals) {
        averages.emplace_hint(averages.end(), genre, total.sum / static_cast<double>(total.count));
    }
    return averages;
}

template <BookContainerLike T>
double calculateAverageRating(const BookDatabase<T> &books) {
    if (books.empty()) {
        return 0.0;
    }
    const double sum = std::ranges::fold_left(books | std::views::transform(&Book::rating), 0.0, std::plus<>{});
    return sum / static_cast<double>(books.size());
}

template <BookContainerLike T>
auto sampleRandomBooks(const BookDatabase<T> &cont, size_t num) {
    const std::size_t count = std::min(num, cont.size());

    BookRefList sample;
    if (count == 0) {
        return sample;
    }
    sample.reserve(count);

    thread_local std::mt19937 engine{std::random_device{}()};
    std::ranges::sample(cont, std::back_inserter(sample), static_cast<std::ptrdiff_t>(count), engine);
    return sample;
}

// Лучшие по comp: для LessByRating это книги с наибольшим рейтингом. Ссылки живут до следующего изменения базы.
template <BookContainerLike T, BookComparator Comparator>
auto getTopNBy(BookDatabase<T> &cont, size_t n, Comparator comp) {
    const std::size_t count = std::min(n, cont.size());
    if (count == 0) {
        return BookRefList{};
    }

    const auto middle = cont.begin() + static_cast<std::ptrdiff_t>(count);
    std::partial_sort(cont.begin(), middle, cont.end(),
                      [&comp](const Book &lhs, const Book &rhs) { return comp(rhs, lhs); });
    return BookRefList(cont.begin(), middle);
}

}  // namespace bookdb
