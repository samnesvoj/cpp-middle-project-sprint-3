#pragma once

#include <cassert>
#include <concepts>
#include <format>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include "book.hpp"
#include "concepts.hpp"
#include "heterogeneous_lookup.hpp"

namespace bookdb {

// author в Book - string_view, поэтому строки авторов хранит сама база. Узлы unordered_set не двигаются
// при rehash, а при перемещении переходят новому владельцу, перепривязывать авторов нужно только при копировании.
// Менять author через GetBooks() и итераторы нельзя.
template <BookContainerLike BookContainer = std::vector<Book>>
class BookDatabase {
public:
    using value_type = Book;
    using size_type = std::ranges::range_size_t<BookContainer>;
    using difference_type = std::ranges::range_difference_t<BookContainer>;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = std::ranges::iterator_t<BookContainer>;
    using const_iterator = std::ranges::iterator_t<const BookContainer>;

    using AuthorContainer = std::unordered_set<std::string, TransparentStringHash, TransparentStringEqual>;

    BookDatabase() = default;

    BookDatabase(std::initializer_list<Book> books) {
        for (const Book &book : books) {
            PushBack(book);
        }
    }

    // книги скопировались вместе с view на чужие строки, переставляем на свои
    BookDatabase(const BookDatabase &other) : books_(other.books_), authors_(other.authors_) { RebindAuthors(); }

    BookDatabase(BookDatabase &&other) noexcept(std::is_nothrow_move_constructible_v<BookContainer> &&
                                                std::is_nothrow_move_constructible_v<AuthorContainer>)
        : books_(std::move(other.books_)), authors_(std::move(other.authors_)) {
        other.Clear();
    }

    BookDatabase &operator=(const BookDatabase &other) {
        if (this != &other) {
            *this = BookDatabase(other);
        }
        return *this;
    }

    BookDatabase &operator=(BookDatabase &&other) noexcept(std::is_nothrow_move_assignable_v<BookContainer> &&
                                                           std::is_nothrow_move_assignable_v<AuthorContainer>) {
        if (this != &other) {
            books_ = std::move(other.books_);
            authors_ = std::move(other.authors_);
            other.Clear();
        }
        return *this;
    }

    ~BookDatabase() = default;

    void Clear() noexcept {
        books_.clear();
        authors_.clear();
    }

    iterator begin() { return books_.begin(); }
    iterator end() { return books_.end(); }
    const_iterator begin() const { return books_.begin(); }
    const_iterator end() const { return books_.end(); }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    size_type size() const { return books_.size(); }
    [[nodiscard]] bool empty() const { return books_.empty(); }

    void PushBack(const Book &book) { EmplaceBack(book); }
    void PushBack(Book &&book) { EmplaceBack(std::move(book)); }

    template <typename... Args>
        requires std::constructible_from<Book, Args...>
    void EmplaceBack(Args &&...args) {
        books_.emplace_back(std::forward<Args>(args)...);
        try {
            Book &book = books_.back();
            book.author = InternAuthor(book.author);
        } catch (...) {
            books_.pop_back();
            throw;
        }
    }

    [[nodiscard]] std::span<const value_type> GetBooks() const { return {books_.data(), books_.size()}; }
    [[nodiscard]] std::span<value_type> GetBooks() { return {books_.data(), books_.size()}; }

    [[nodiscard]] const AuthorContainer &GetAuthors() { return authors_; }
    [[nodiscard]] const AuthorContainer &GetAuthors() const { return authors_; }

private:
    // find, а не emplace: так std::string создаётся только для нового автора
    std::string_view InternAuthor(std::string_view author) {
        auto it = authors_.find(author);
        if (it == authors_.end()) {
            it = authors_.emplace(author).first;
        }
        return *it;
    }

    void RebindAuthors() noexcept {
        for (Book &book : books_) {
            const auto it = authors_.find(book.author);
            assert(it != authors_.end());
            book.author = *it;
        }
    }

    BookContainer books_;
    AuthorContainer authors_;
};

}  // namespace bookdb

namespace std {
template <bookdb::BookContainerLike BookContainer>
struct formatter<bookdb::BookDatabase<BookContainer>> {
    template <typename FormatContext>
    auto format(const bookdb::BookDatabase<BookContainer> &db, FormatContext &fc) const {
        auto out = fc.out();
        out = format_to(out, "BookDatabase (size = {}): ", db.size());

        out = format_to(out, "Books:\n");
        for (const auto &book : db.GetBooks()) {
            out = format_to(out, "- {}\n", book);
        }

        out = format_to(out, "Authors:\n");
        for (const auto &author : db.GetAuthors()) {
            out = format_to(out, "- {}\n", author);
        }
        return out;
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};
}  // namespace std
