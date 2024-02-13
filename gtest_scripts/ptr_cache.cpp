#include "gtest/gtest.h"

#include "../include/vapor/ptr_cache.hpp"

#include <string>

namespace {

TEST(cache, QueryFalse)
{
  using Cache3 = VAPoR::ptr_cache<int, std::string, 3, false>;
  Cache3 cache;

  auto* obj1 = new std::string("BigString1");
  auto* obj2 = new std::string("BigString2");
  auto* obj3 = new std::string("BigString3");
  auto* obj4 = new std::string("BigString4");

  cache.insert(1, obj1);
  EXPECT_EQ(cache.query(1), obj1);

  cache.insert(2, obj2);
  cache.insert(3, obj3);
  cache.insert(4, obj4);

  EXPECT_EQ(cache.query(1), nullptr);
  EXPECT_EQ(cache.query(4), obj4);
  EXPECT_EQ(cache.query(3), obj3);
  EXPECT_EQ(cache.query(2), obj2);

  auto* obj5 = new std::string("BigString5");
  cache.insert(5, obj5);
  EXPECT_EQ(cache.query(2), nullptr);
}

TEST(cache, QueryTrue)
{
  using Cache3 = VAPoR::ptr_cache<int, std::string, 3, true>;
  Cache3 cache;

  auto* obj1 = new std::string("BigString1");
  auto* obj2 = new std::string("BigString2");
  auto* obj3 = new std::string("BigString3");
  auto* obj4 = new std::string("BigString4");

  cache.insert(1, obj1);
  EXPECT_EQ(cache.query(1), obj1);

  cache.insert(2, obj2);
  cache.insert(3, obj3);
  cache.insert(4, obj4);

  EXPECT_EQ(cache.query(1), nullptr);
  EXPECT_EQ(cache.query(4), obj4);
  EXPECT_EQ(cache.query(3), obj3);
  EXPECT_EQ(cache.query(2), obj2);

  auto* obj5 = new std::string("BigString5");
  cache.insert(5, obj5);
  EXPECT_EQ(cache.query(4), nullptr);
  EXPECT_EQ(cache.query(2), obj2);
}

}

