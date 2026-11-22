#include <gtest/gtest.h>

#include <list>
#include <string>
#include <vector>

#include "book_database.hpp"
#include "filters.hpp"

using namespace bookdb;

namespace {
const Book kOld{"Emma", "Jane Austen", 1815, Genre::Fiction, 4.1, 80};
const Book kClassic{"1984", "George Orwell", 1949, Genre::SciFi, 4.5, 190};
const Book kModern{"Neuromancer", "William Gibson", 1984, Genre::SciFi, 3.9, 120};

std::vector<std::string> Titles(const BookRefList &books) {
    std::vector<std::string> titles;
    for (const Book &book : books) {
        titles.push_back(book.title);
    }
    return titles;
}
}  // namespace

TEST(FilterTest, PredicateFactories) {
    constexpr auto between = YearBetween(1949, 1984);
    EXPECT_FALSE(between(kOld));
    EXPECT_TRUE(between(kClassic));
    EXPECT_TRUE(between(kModern));
    EXPECT_FALSE(YearBetween(1950, 1983)(kClassic));
    EXPECT_FALSE(YearBetween(2000, 1900)(kClassic));

    EXPECT_TRUE(RatingAbove(4.5)(kClassic));
    EXPECT_TRUE(RatingAbove(4.0)(kOld));
    EXPECT_FALSE(RatingAbove(4.0)(kModern));

    EXPECT_TRUE(GenreIs(Genre::SciFi)(kClassic));
    EXPECT_FALSE(GenreIs(Genre::SciFi)(kOld));
    EXPECT_FALSE(GenreIs(Genre::Unknown)(kOld));
}

TEST(FilterTest, CompositionAndFiltering) {
    constexpr auto both = all_of(GenreIs(Genre::SciFi), RatingAbove(4.0));
    EXPECT_TRUE(both(kClassic));
    EXPECT_FALSE(both(kModern));
    EXPECT_FALSE(both(kOld));

    const auto either = any_of(YearBetween(1800, 1900), RatingAbove(4.5), GenreIs(Genre::Mystery));
    EXPECT_TRUE(either(kOld));
    EXPECT_TRUE(either(kClassic));
    EXPECT_FALSE(either(kModern));

    EXPECT_TRUE(all_of()(kOld));
    EXPECT_FALSE(any_of()(kOld));
    static_assert(
        BookPredicate<decltype(any_of(all_of(YearBetween(1900, 1999), RatingAbove(4.5)), GenreIs(Genre::Fiction)))>);

    BookDatabase<> db{kOld, kClassic, kModern};
    const auto sci_fi = filterBooks(db.begin(), db.end(), GenreIs(Genre::SciFi));
    EXPECT_EQ(Titles(sci_fi), (std::vector<std::string>{"1984", "Neuromancer"}));
    EXPECT_EQ(&sci_fi[0].get(), &db.GetBooks()[1]);

    const auto from_second = filterBooks(db.cbegin() + 1, db.cend(), all_of());
    EXPECT_EQ(Titles(from_second), (std::vector<std::string>{"1984", "Neuromancer"}));
    EXPECT_TRUE(filterBooks(db.begin(), db.end(), GenreIs(Genre::Biography)).empty());
    EXPECT_TRUE(filterBooks(db.begin(), db.begin(), all_of()).empty());

    std::list<Book> list{kOld, kClassic, kModern};
    EXPECT_EQ(Titles(filterBooks(list.begin(), list.end(), RatingAbove(4.1))),
              (std::vector<std::string>{"Emma", "1984"}));
}
