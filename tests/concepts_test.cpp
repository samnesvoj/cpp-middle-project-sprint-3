#include <deque>
#include <iterator>
#include <list>
#include <string>
#include <vector>

#include "comparators.hpp"
#include "concepts.hpp"
#include "heterogeneous_lookup.hpp"

using namespace bookdb;

static_assert(BookContainerLike<std::vector<Book>>);
static_assert(!BookContainerLike<std::deque<Book>>);
static_assert(!BookContainerLike<std::list<Book>>);
static_assert(!BookContainerLike<std::vector<int>>);

static_assert(BookIterator<std::vector<Book>::iterator>);
static_assert(BookIterator<std::vector<Book>::const_iterator>);
static_assert(BookIterator<std::list<Book>::iterator>);
static_assert(!BookIterator<std::vector<int>::iterator>);
static_assert(!BookIterator<std::back_insert_iterator<std::vector<Book>>>);

static_assert(BookSentinel<std::vector<Book>::iterator, std::vector<Book>::iterator>);
static_assert(BookSentinel<std::default_sentinel_t, std::counted_iterator<Book *>>);
static_assert(!BookSentinel<std::vector<int>::iterator, std::vector<int>::iterator>);

static_assert(BookPredicate<decltype([](const Book &book) { return book.year > 2000; })>);
static_assert(BookPredicate<bool (*)(const Book &)>);
static_assert(!BookPredicate<decltype([](int) { return true; })>);
static_assert(!BookPredicate<decltype([](const Book &) {})>);
static_assert(!BookPredicate<decltype([](Book &) { return true; })>);

static_assert(BookComparator<comp::LessByAuthor>);
static_assert(BookComparator<comp::LessByTitle>);
static_assert(BookComparator<comp::LessByYear>);
static_assert(BookComparator<comp::LessByGenre>);
static_assert(BookComparator<comp::LessByRating>);
static_assert(BookComparator<comp::LessByPopularity>);
static_assert(!BookComparator<TransparentStringLess>);
static_assert(std::sortable<std::vector<Book>::iterator, comp::LessByRating>);
