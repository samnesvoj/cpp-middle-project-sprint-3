#include <gtest/gtest.h>

#include <flat_set>
#include <memory>

#include "comparators.hpp"
#include "statsistics.hpp"

using namespace bookdb;

namespace {
using Db = BookDatabase<>;

Db MakeDb() {
    Db db;
    db.EmplaceBack("1984", "George Orwell", 1949, Genre::SciFi, 4.0, 190);
    db.EmplaceBack("Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143);
    db.EmplaceBack("Emma", "Jane Austen", 1815, Genre::Fiction, 4.2, 80);
    db.EmplaceBack("Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.6, 98);
    return db;
}
}  // namespace

TEST(StatisticsTest, AuthorHistogram) {
    auto db = std::make_unique<Db>(MakeDb());
    const auto histogram = buildAuthorHistogramFlat(*db);
    const auto reversed = buildAuthorHistogramFlat(*db, std::greater<>{});
    const auto empty = buildAuthorHistogramFlat(Db{});
    db.reset();

    ASSERT_EQ(histogram.size(), 3U);
    EXPECT_EQ(histogram.at("George Orwell"), 2U);
    EXPECT_EQ(histogram.at("Jane Austen"), 1U);
    EXPECT_EQ(histogram.begin()->first, "Aldous Huxley");
    EXPECT_EQ(reversed.begin()->first, "Jane Austen");
    EXPECT_TRUE(empty.empty());
}

TEST(StatisticsTest, GenreRatings) {
    Db db = MakeDb();
    const auto ratings = calculateGenreRatings(db.begin(), db.end());
    ASSERT_EQ(ratings.size(), 2U);
    EXPECT_NEAR(ratings.at(Genre::SciFi), 4.3, 1e-12);
    EXPECT_NEAR(ratings.at(Genre::Fiction), 4.3, 1e-12);

    const auto part = calculateGenreRatings(db.begin(), db.begin() + 2);
    EXPECT_DOUBLE_EQ(part.at(Genre::SciFi), 4.0);
    EXPECT_DOUBLE_EQ(part.at(Genre::Fiction), 4.4);

    EXPECT_TRUE(calculateGenreRatings(db.begin(), db.begin()).empty());
}

TEST(StatisticsTest, AverageRating) {
    EXPECT_NEAR(calculateAverageRating(MakeDb()), 4.3, 1e-12);
    EXPECT_EQ(calculateAverageRating(Db{}), 0.0);
}

TEST(StatisticsTest, RandomSample) {
    const Db db = MakeDb();
    const auto sample = sampleRandomBooks(db, 3);
    ASSERT_EQ(sample.size(), 3U);

    std::flat_set<const Book *> unique;
    for (const Book &book : sample) {
        unique.insert(&book);
        EXPECT_TRUE(&book >= db.GetBooks().data() && &book < db.GetBooks().data() + db.size());
    }
    EXPECT_EQ(unique.size(), 3U);

    EXPECT_TRUE(sampleRandomBooks(db, 0).empty());
    EXPECT_EQ(sampleRandomBooks(db, 4).size(), 4U);
    EXPECT_EQ(sampleRandomBooks(db, 100).size(), 4U);
    EXPECT_TRUE(sampleRandomBooks(Db{}, 5).empty());
}

TEST(StatisticsTest, TopN) {
    Db db = MakeDb();
    const auto top = getTopNBy(db, 2, comp::LessByRating{});
    ASSERT_EQ(top.size(), 2U);
    EXPECT_EQ(top[0].get().title, "Brave New World");
    EXPECT_EQ(top[1].get().title, "Animal Farm");

    EXPECT_EQ(getTopNBy(db, 1, comp::LessByPopularity{})[0].get().title, "1984");
    EXPECT_TRUE(getTopNBy(db, 0, comp::LessByRating{}).empty());
    EXPECT_EQ(getTopNBy(db, db.size(), comp::LessByRating{}).size(), 4U);
    EXPECT_EQ(getTopNBy(db, 100, comp::LessByRating{}).size(), 4U);
    for (const Book &book : db) {
        EXPECT_EQ(book.author.data(), db.GetAuthors().find(book.author)->data());
    }

    Db empty;
    EXPECT_TRUE(getTopNBy(empty, 3, comp::LessByRating{}).empty());

    Db ties;
    ties.EmplaceBack("A", "X", 2000, Genre::Fiction, 5.0, 1);
    ties.EmplaceBack("B", "X", 2000, Genre::Fiction, 5.0, 2);
    ties.EmplaceBack("C", "X", 2000, Genre::Fiction, 1.0, 3);
    const auto best = getTopNBy(ties, 2, comp::LessByRating{});
    EXPECT_EQ(best[0].get().rating, 5.0);
    EXPECT_EQ(best[1].get().rating, 5.0);
}
