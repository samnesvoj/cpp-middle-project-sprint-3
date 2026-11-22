#include <gtest/gtest.h>

#include <format>
#include <string>

#include "book.hpp"

using namespace bookdb;

static_assert(GenreFromString("Fiction") == Genre::Fiction);
static_assert(GenreFromString("SciFi") == Genre::SciFi);
static_assert(GenreFromString("fiction") == Genre::Unknown);
static_assert(GenreFromString("") == Genre::Unknown);

static_assert([] {
    const Book book{"1984", "George Orwell", 1949, "SciFi", 4.0, 190};
    return book.genre == Genre::SciFi && book.year == 1949 && book.author == "George Orwell";
}());

TEST(BookTest, ConstructorsAndGenre) {
    for (const auto &[name, genre] : {std::pair{"Fiction", Genre::Fiction},
                                      {"NonFiction", Genre::NonFiction},
                                      {"SciFi", Genre::SciFi},
                                      {"Biography", Genre::Biography},
                                      {"Mystery", Genre::Mystery},
                                      {"Unknown", Genre::Unknown},
                                      {"Poetry", Genre::Unknown}}) {
        EXPECT_EQ(GenreFromString(name), genre) << name;
    }

    const Book from_enum{"1984", "George Orwell", 1949, Genre::SciFi, 4.0, 190};
    EXPECT_EQ(from_enum.title, "1984");
    EXPECT_EQ(from_enum.author, "George Orwell");
    EXPECT_EQ(from_enum.year, 1949);
    EXPECT_EQ(from_enum.genre, Genre::SciFi);
    EXPECT_DOUBLE_EQ(from_enum.rating, 4.0);
    EXPECT_EQ(from_enum.read_count, 190);

    const Book from_string{"Dune", "Frank Herbert", 1965, "SciFi", 4.6, 77};
    EXPECT_EQ(from_string.genre, Genre::SciFi);
    EXPECT_EQ(from_string.title, "Dune");
    EXPECT_EQ(Book("T", "A", 2000, "Nonsense", 3.0, 0).genre, Genre::Unknown);
}

TEST(BookTest, Formatting) {
    EXPECT_EQ(std::format("{}", Genre::NonFiction), "NonFiction");

    const Book book{"1984", "George Orwell", 1949, Genre::SciFi, 4.5, 190};
    EXPECT_EQ(std::format("{}", book), "\"1984\" by George Orwell (1949), SciFi, rating 4.5, read 190 times");
}
