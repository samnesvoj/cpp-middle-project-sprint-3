#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "book_database.hpp"
#include "comparators.hpp"
#include "filters.hpp"
#include "statsistics.hpp"

using namespace bookdb;

namespace {
using Db = BookDatabase<>;

void ExpectAuthorsOwnedBy(const Db &db) {
    for (const Book &book : db) {
        const auto it = db.GetAuthors().find(book.author);
        ASSERT_NE(it, db.GetAuthors().end());
        EXPECT_EQ(book.author.data(), it->data());
    }
}

Db MakeDb() {
    Db db;
    db.EmplaceBack("1984", "George Orwell", 1949, Genre::SciFi, 4.0, 190);
    db.EmplaceBack("Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143);
    db.EmplaceBack("Emma", "Jane Austen", 1815, Genre::Fiction, 4.1, 80);
    return db;
}
}  // namespace

TEST(BookDatabaseTest, EmptyAndAliases) {
    static_assert(std::ranges::contiguous_range<Db>);
    static_assert(std::same_as<Db::value_type, Book>);
    static_assert(std::same_as<Db::size_type, std::size_t>);
    static_assert(std::same_as<Db::difference_type, std::ptrdiff_t>);
    static_assert(std::same_as<Db::reference, Book &>);
    static_assert(std::same_as<Db::const_reference, const Book &>);
    static_assert(std::same_as<Db::pointer, Book *>);
    static_assert(std::same_as<Db::const_pointer, const Book *>);
    static_assert(std::same_as<Db::iterator, std::vector<Book>::iterator>);
    static_assert(std::same_as<Db::const_iterator, std::vector<Book>::const_iterator>);
    static_assert(std::is_nothrow_move_constructible_v<Db> && std::is_nothrow_move_assignable_v<Db>);

    const Db db;
    EXPECT_TRUE(db.empty());
    EXPECT_EQ(db.size(), 0U);
    EXPECT_EQ(db.begin(), db.end());
    EXPECT_TRUE(db.GetBooks().empty());
    EXPECT_TRUE(db.GetAuthors().empty());
}

TEST(BookDatabaseTest, PushBack) {
    const Book source{"Dune", "Frank Herbert", 1965, Genre::SciFi, 4.6, 77};
    std::string long_title(100, 'T');
    const char *title_data = long_title.data();

    Db db;
    db.PushBack(source);
    db.PushBack(Book{std::move(long_title), "Jane Austen", 1815, Genre::Fiction, 4.1, 80});

    ASSERT_EQ(db.size(), 2U);
    EXPECT_EQ(source.title, "Dune");
    EXPECT_EQ(db.GetBooks()[0].title, "Dune");
    EXPECT_EQ(db.GetBooks()[1].title.data(), title_data);
    ExpectAuthorsOwnedBy(db);

    for (int i = 0; i < 100; ++i) {
        db.PushBack(db.GetBooks().front());
    }
    EXPECT_EQ(db.size(), 102U);
    ExpectAuthorsOwnedBy(db);
}

TEST(BookDatabaseTest, EmplaceAndInitializerList) {
    struct ThrowingTitle {
        operator std::string() const { throw std::runtime_error("title"); }
    };

    Db db = MakeDb();
    db.EmplaceBack("Dune", "Frank Herbert", 1965, "SciFi", 4.6, 77);
    ASSERT_EQ(db.size(), 4U);
    EXPECT_EQ(db.GetBooks()[3].genre, Genre::SciFi);

    EXPECT_THROW(db.EmplaceBack(ThrowingTitle{}, "New Author", 2000, Genre::Fiction, 1.0, 1), std::runtime_error);
    EXPECT_EQ(db.size(), 4U);
    EXPECT_FALSE(db.GetAuthors().contains(std::string_view{"New Author"}));

    const Db listed{
        Book{"A", "One", 2000, Genre::Fiction, 1.0, 1},
        Book{"B", "Two", 2001, Genre::Mystery, 2.0, 2},
        Book{"C", "One", 2002, Genre::SciFi, 3.0, 3},
    };
    EXPECT_EQ(listed.size(), 3U);
    EXPECT_EQ(listed.GetAuthors().size(), 2U);
    ExpectAuthorsOwnedBy(listed);
    EXPECT_TRUE(Db{}.empty());
}

TEST(BookDatabaseTest, AuthorsAndStringView) {
    const Db db = MakeDb();
    EXPECT_EQ(db.GetAuthors().size(), 2U);
    EXPECT_EQ(db.GetBooks()[0].author.data(), db.GetBooks()[1].author.data());
    ExpectAuthorsOwnedBy(db);

    Db other;
    {
        std::string author(60, 'a');
        other.EmplaceBack("T", author, 2000, Genre::Fiction, 3.0, 1);
        author.assign(60, '#');
    }
    EXPECT_EQ(other.GetBooks()[0].author, std::string(60, 'a'));

    Db target;
    target.PushBack(db.GetBooks()[2]);
    EXPECT_NE(target.GetBooks()[0].author.data(), db.GetBooks()[2].author.data());
    ExpectAuthorsOwnedBy(target);
}

TEST(BookDatabaseTest, IteratorsAndAccess) {
    Db db = MakeDb();
    static_assert(std::same_as<decltype(db.GetBooks()), std::span<Book>>);
    static_assert(std::same_as<decltype(std::as_const(db).GetBooks()), std::span<const Book>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*db.cbegin())>>);

    EXPECT_EQ(std::distance(db.begin(), db.end()), 3);
    EXPECT_EQ(std::distance(db.cbegin(), db.cend()), 3);
    EXPECT_EQ(&*db.begin(), db.GetBooks().data());
    EXPECT_EQ(&db.GetAuthors(), &std::as_const(db).GetAuthors());

    const auto it = std::find_if(db.begin(), db.end(), [](const Book &b) { return b.author == "Jane Austen"; });
    ASSERT_NE(it, db.end());
    EXPECT_EQ(it->title, "Emma");
    EXPECT_TRUE(std::all_of(db.cbegin(), db.cend(), [](const Book &b) { return b.rating > 0; }));

    int reads = 0;
    std::for_each(db.begin(), db.end(), [&reads](const Book &b) { reads += b.read_count; });
    EXPECT_EQ(reads, 413);

    std::sort(db.begin(), db.end(), comp::LessByYear{});
    EXPECT_EQ(db.GetBooks()[0].title, "Emma");
    std::sort(db.begin(), db.end(), comp::LessByAuthor{});
    EXPECT_EQ(db.GetBooks()[2].author, "Jane Austen");
    std::sort(db.begin(), db.end(), comp::LessByPopularity{});
    EXPECT_EQ(db.GetBooks()[2].title, "1984");
    ExpectAuthorsOwnedBy(db);

    db.Clear();
    EXPECT_TRUE(db.empty());
    EXPECT_TRUE(db.GetAuthors().empty());
    db.EmplaceBack("New", "New Author", 2024, Genre::Mystery, 3.0, 1);
    ExpectAuthorsOwnedBy(db);
}

TEST(BookDatabaseTest, CopyAndMove) {
    auto original = std::make_unique<Db>(MakeDb());
    const Db copy{*original};
    ExpectAuthorsOwnedBy(copy);
    EXPECT_NE(copy.GetBooks()[0].author.data(), original->GetBooks()[0].author.data());
    original.reset();
    EXPECT_EQ(copy.GetBooks()[2].author, "Jane Austen");
    ExpectAuthorsOwnedBy(copy);

    Db assigned;
    assigned.EmplaceBack("Old", "Old Author", 1999, Genre::Fiction, 1.0, 1);
    Db source = MakeDb();
    assigned = source;
    source.Clear();
    EXPECT_EQ(assigned.size(), 3U);
    EXPECT_FALSE(assigned.GetAuthors().contains(std::string_view{"Old Author"}));
    ExpectAuthorsOwnedBy(assigned);
    const Db &alias = assigned;
    assigned = alias;
    ExpectAuthorsOwnedBy(assigned);

    Db moved_from = MakeDb();
    const std::string_view orwell = moved_from.GetBooks()[0].author;
    Db moved{std::move(moved_from)};
    EXPECT_EQ(moved.GetBooks()[0].author.data(), orwell.data());
    EXPECT_TRUE(moved_from.empty());
    EXPECT_TRUE(moved_from.GetAuthors().empty());
    ExpectAuthorsOwnedBy(moved);

    Db move_target;
    move_target.EmplaceBack("Old", "Old Author", 1999, Genre::Fiction, 1.0, 1);
    move_target = std::move(moved);
    EXPECT_EQ(move_target.size(), 3U);
    EXPECT_FALSE(move_target.GetAuthors().contains(std::string_view{"Old Author"}));
    EXPECT_TRUE(moved.empty());
    ExpectAuthorsOwnedBy(move_target);

    std::vector<Db> dbs;
    for (int i = 0; i < 20; ++i) {
        dbs.push_back(MakeDb());
    }
    for (const Db &db : dbs) {
        ExpectAuthorsOwnedBy(db);
    }
}

TEST(BookDatabaseTest, LargeDataset) {
    constexpr int kBooks = 5000;
    constexpr int kAuthors = 1200;

    Db db;
    db.EmplaceBack("First", "Al", 2000, Genre::Fiction, 1.0, 0);
    const std::string_view first_author = db.GetBooks()[0].author;
    for (int i = 1; i < kBooks; ++i) {
        db.EmplaceBack(std::format("Title {}", i), std::format("Author number {} with a long name", i % kAuthors),
                       1900 + i % 120, static_cast<Genre>(i % 5), 1.0 + i % 5, i);
    }

    ASSERT_EQ(db.size(), static_cast<std::size_t>(kBooks));
    EXPECT_EQ(db.GetAuthors().size(), static_cast<std::size_t>(kAuthors) + 1);
    EXPECT_EQ(first_author, "Al");
    EXPECT_EQ(first_author.data(), db.GetBooks()[0].author.data());
    ExpectAuthorsOwnedBy(db);

    const auto histogram = buildAuthorHistogramFlat(db);
    std::size_t counted = 0;
    for (const auto &[author, count] : histogram) {
        counted += count;
    }
    EXPECT_EQ(histogram.size(), db.GetAuthors().size());
    EXPECT_EQ(counted, db.size());

    double sum = 0;
    for (const Book &book : db) {
        sum += book.rating;
    }
    EXPECT_NEAR(calculateAverageRating(db), sum / kBooks, 1e-9);

    const auto top = getTopNBy(db, 100, comp::LessByPopularity{});
    ASSERT_EQ(top.size(), 100U);
    EXPECT_EQ(top.front().get().read_count, kBooks - 1);
    EXPECT_EQ(top.back().get().read_count, kBooks - 100);
    ExpectAuthorsOwnedBy(db);

    EXPECT_EQ(sampleRandomBooks(db, 500).size(), 500U);
    const auto filtered = filterBooks(db.begin(), db.end(), YearBetween(1900, 1909));
    EXPECT_EQ(filtered.size(), static_cast<std::size_t>(std::ranges::count_if(db, YearBetween(1900, 1909))));

    const Db copy{db};
    db.Clear();
    EXPECT_EQ(copy.size(), static_cast<std::size_t>(kBooks));
    ExpectAuthorsOwnedBy(copy);
}
