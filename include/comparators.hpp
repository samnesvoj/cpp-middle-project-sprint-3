#pragma once

#include "book.hpp"

namespace bookdb::comp {

struct LessByAuthor {
    constexpr bool operator()(const Book &lhs, const Book &rhs) const noexcept { return lhs.author < rhs.author; }
};

struct LessByTitle {
    constexpr bool operator()(const Book &lhs, const Book &rhs) const noexcept { return lhs.title < rhs.title; }
};

struct LessByYear {
    constexpr bool operator()(const Book &lhs, const Book &rhs) const noexcept { return lhs.year < rhs.year; }
};

struct LessByGenre {
    constexpr bool operator()(const Book &lhs, const Book &rhs) const noexcept { return lhs.genre < rhs.genre; }
};

struct LessByRating {
    constexpr bool operator()(const Book &lhs, const Book &rhs) const noexcept { return lhs.rating < rhs.rating; }
};

struct LessByPopularity {
    constexpr bool operator()(const Book &lhs, const Book &rhs) const noexcept {
        return lhs.read_count < rhs.read_count;
    }
};

}  // namespace bookdb::comp
