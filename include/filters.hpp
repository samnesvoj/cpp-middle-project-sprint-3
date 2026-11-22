#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

#include "book.hpp"
#include "concepts.hpp"

namespace bookdb {

constexpr auto YearBetween(int start, int end) {
    return [start, end](const Book &book) { return start <= book.year && book.year <= end; };
}

// >=, как в подписи примера в main.cpp
constexpr auto RatingAbove(double min_rating) {
    return [min_rating](const Book &book) { return book.rating >= min_rating; };
}

constexpr auto GenreIs(Genre genre) {
    return [genre](const Book &book) { return book.genre == genre; };
}

template <BookPredicate... Preds>
constexpr auto all_of(Preds... preds) {
    return
        [... preds = std::move(preds)]([[maybe_unused]] const Book &book) { return (... && std::invoke(preds, book)); };
}

template <BookPredicate... Preds>
constexpr auto any_of(Preds... preds) {
    return
        [... preds = std::move(preds)]([[maybe_unused]] const Book &book) { return (... || std::invoke(preds, book)); };
}

template <BookIterator It, BookSentinel<It> Sent, BookPredicate Pred>
auto filterBooks(It first, Sent last, Pred pred) {
    BookRefList result;
    std::ranges::copy_if(first, last, std::back_inserter(result), std::move(pred));
    return result;
}

}  // namespace bookdb
