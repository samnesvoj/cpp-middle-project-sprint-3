#pragma once

#include <algorithm>
#include <array>
#include <format>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace bookdb {

enum class Genre { Fiction, NonFiction, SciFi, Biography, Mystery, Unknown };

namespace detail {
inline constexpr std::array<std::pair<std::string_view, Genre>, 5> kGenreNames{{
    {"Fiction", Genre::Fiction},
    {"NonFiction", Genre::NonFiction},
    {"SciFi", Genre::SciFi},
    {"Biography", Genre::Biography},
    {"Mystery", Genre::Mystery},
}};
}  // namespace detail

// Регистр важен, незнакомая строка даёт Genre::Unknown.
[[nodiscard]] constexpr Genre GenreFromString(std::string_view s) {
    const auto it = std::ranges::find(detail::kGenreNames, s, &std::pair<std::string_view, Genre>::first);
    return it != detail::kGenreNames.end() ? it->second : Genre::Unknown;
}

struct Book {
    // string_view для экономии памяти, чтобы ссылаться на оригинальную строку, хранящуюся в другом контейнере
    std::string_view author;
    std::string title;

    int year;
    Genre genre;
    double rating;
    int read_count;

    constexpr Book(std::string title, std::string_view author, int year, Genre genre, double rating, int read_count)
        : author(author), title(std::move(title)), year(year), genre(genre), rating(rating), read_count(read_count) {}

    constexpr Book(std::string title, std::string_view author, int year, std::string_view genre, double rating,
                   int read_count)
        : Book(std::move(title), author, year, GenreFromString(genre), rating, read_count) {}
};

using BookRefList = std::vector<std::reference_wrapper<const Book>>;
}  // namespace bookdb

namespace std {
template <>
struct formatter<bookdb::Genre, char> {
    template <typename FormatContext>
    auto format(const bookdb::Genre g, FormatContext &fc) const {
        std::string genre_str;

        // clang-format off
        using bookdb::Genre;
        switch (g) {
            case Genre::Fiction:    genre_str = "Fiction"; break;
            case Genre::Mystery:    genre_str = "Mystery"; break;
            case Genre::NonFiction: genre_str = "NonFiction"; break;
            case Genre::SciFi:      genre_str = "SciFi"; break;
            case Genre::Biography:  genre_str = "Biography"; break;
            case Genre::Unknown:    genre_str = "Unknown"; break;
            default:
                throw logic_error{"Unsupported bookdb::Genre"};
            }
        // clang-format on
        return format_to(fc.out(), "{}", genre_str);
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};

template <>
struct formatter<bookdb::Book, char> {
    template <typename FormatContext>
    auto format(const bookdb::Book &book, FormatContext &fc) const {
        return format_to(fc.out(), "\"{}\" by {} ({}), {}, rating {}, read {} times", book.title, book.author,
                         book.year, book.genre, book.rating, book.read_count);
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};

}  // namespace std
