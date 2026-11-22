#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <unordered_set>

#include "heterogeneous_lookup.hpp"

using namespace bookdb;

TEST(LookupTest, TransparentSearch) {
    const std::unordered_set<std::string, TransparentStringHash, TransparentStringEqual> authors{"Jane Austen",
                                                                                                 "J.R.R. Tolkien"};

    const auto it = authors.find(std::string_view{"Jane Austen"});
    ASSERT_NE(it, authors.end());
    EXPECT_EQ(*it, "Jane Austen");
    EXPECT_NE(authors.find("J.R.R. Tolkien"), authors.end());
    EXPECT_NE(authors.find(std::string{"Jane Austen"}), authors.end());
    EXPECT_EQ(authors.find(std::string_view{"Charles Dickens"}), authors.end());
    EXPECT_EQ(authors.find("jane austen"), authors.end());
    EXPECT_EQ(authors.size(), 2U);

    const TransparentStringHash hash;
    const std::string owned = "George Orwell";
    EXPECT_EQ(hash(owned), hash(std::string_view{owned}));
    EXPECT_EQ(hash(owned), hash("George Orwell"));

    EXPECT_TRUE(TransparentStringEqual{}(owned, "George Orwell"));
    EXPECT_TRUE(TransparentStringLess{}("a", std::string_view{"b"}));
}
