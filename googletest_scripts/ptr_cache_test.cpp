#include "gtest/gtest.h"

#include "ptr_cache.hpp" // Only include the module that's tested

namespace {

struct MyObj {
  int i = 0, j = 1; 
};

// Besides correct insertion/query/eviction behaviors the `ptr_cache` has to be
// deleting objects correctly upon eviction. However, memory errors are not 
// something GoogleTest can detect. As a result, one needs to run this
// executable in valgrind to make sure that there are no memory errors:
// valgrind ./googletest_scripts/ptr_cache_test

// Test the cache when queries don't count as a use.
// It needs to insert and evict correctly.
TEST(ptr_cache_query_false, insert_eviction)
{
  VAPoR::ptr_cache<int, MyObj, 3, false> cache;
  const auto* p = cache.query(1);
  EXPECT_EQ(p, nullptr);

  cache.insert(1, new MyObj{1, 100});
  cache.insert(2, new MyObj{2, 200});
  cache.insert(3, new MyObj{3, 300});

  // Make sure that we have all 3 objects
  p = cache.query(1);
  EXPECT_NE(p, nullptr); // not nullptr
  EXPECT_EQ(p->j, 100);
  p = cache.query(2);
  EXPECT_NE(p, nullptr);
  EXPECT_EQ(p->j, 200);
  p = cache.query(3);
  EXPECT_NE(p, nullptr);
  EXPECT_EQ(p->j, 300);
  p = cache.query(4);
  EXPECT_EQ(p, nullptr);

  // Insert a new object; make sure that "1" is evicted.
  cache.insert(4, new MyObj{4, 400});
  p = cache.query(4);
  EXPECT_NE(p, nullptr);
  EXPECT_EQ(p->j, 400);
  p = cache.query(1);
  EXPECT_EQ(p, nullptr);

  // Do a query on "2"
  p = cache.query(2);

  // Insert another object; make sure that "2" is evicted.
  cache.insert(5, new MyObj{5, 500});
  p = cache.query(5);
  EXPECT_NE(p, nullptr);
  EXPECT_EQ(p->j, 500);
  p = cache.query(2);
  EXPECT_EQ(p, nullptr);
}


// Test the cache when queries count as a use.
TEST(ptr_cache_query_true, insert_eviction)
{
  VAPoR::ptr_cache<int, MyObj, 3, true> cache;

  cache.insert(1, new MyObj{1, 100});
  cache.insert(2, new MyObj{2, 200});
  cache.insert(3, new MyObj{3, 300});

  // Make sure that we have all 3 objects
  const auto* p = cache.query(1);
  EXPECT_NE(p, nullptr); // not nullptr
  EXPECT_EQ(p->j, 100);
  p = cache.query(2);
  EXPECT_NE(p, nullptr);
  EXPECT_EQ(p->j, 200);
  p = cache.query(3);
  EXPECT_NE(p, nullptr);
  EXPECT_EQ(p->j, 300);
  p = cache.query(4);
  EXPECT_EQ(p, nullptr);

  // Insert a new object; make sure that "1" is evicted.
  cache.insert(4, new MyObj{4, 400});
  p = cache.query(4);
  EXPECT_NE(p, nullptr);
  EXPECT_EQ(p->j, 400);
  p = cache.query(1);
  EXPECT_EQ(p, nullptr);

  // Do a query on "2", then insert, it should be "3" that's evicted.
  p = cache.query(2);
  cache.insert(5, new MyObj{5, 500});
  p = cache.query(5);
  EXPECT_NE(p, nullptr);
  EXPECT_EQ(p->j, 500);
  p = cache.query(3);
  EXPECT_EQ(p, nullptr);

  // Do another query on "2", then insert, it should be "4" that's evicted.
  p = cache.query(2);
  cache.insert(6, new MyObj{6, 600});
  p = cache.query(6);
  EXPECT_NE(p, nullptr);
  EXPECT_EQ(p->j, 600);
  p = cache.query(4);
  EXPECT_EQ(p, nullptr);
}

} // End of namespace
