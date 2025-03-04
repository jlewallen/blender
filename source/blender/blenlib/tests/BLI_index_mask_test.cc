/* SPDX-License-Identifier: Apache-2.0 */

#include "BLI_index_mask.hh"
#include "testing/testing.h"

namespace blender::tests {

TEST(index_mask, DefaultConstructor)
{
  IndexMask mask;
  EXPECT_EQ(mask.min_array_size(), 0);
  EXPECT_EQ(mask.size(), 0);
}

TEST(index_mask, ArrayConstructor)
{
  [](IndexMask mask) {
    EXPECT_EQ(mask.size(), 4);
    EXPECT_EQ(mask.min_array_size(), 8);
    EXPECT_FALSE(mask.is_range());
    EXPECT_EQ(mask[0], 3);
    EXPECT_EQ(mask[1], 5);
    EXPECT_EQ(mask[2], 6);
    EXPECT_EQ(mask[3], 7);
  }({3, 5, 6, 7});
}

TEST(index_mask, RangeConstructor)
{
  IndexMask mask = IndexRange(3, 5);
  EXPECT_EQ(mask.size(), 5);
  EXPECT_EQ(mask.min_array_size(), 8);
  EXPECT_EQ(mask.last(), 7);
  EXPECT_TRUE(mask.is_range());
  EXPECT_EQ(mask.as_range().first(), 3);
  EXPECT_EQ(mask.as_range().last(), 7);
  Span<int64_t> indices = mask.indices();
  EXPECT_EQ(indices[0], 3);
  EXPECT_EQ(indices[1], 4);
  EXPECT_EQ(indices[2], 5);
}

TEST(index_mask, SliceAndOffset)
{
  Vector<int64_t> indices;
  {
    IndexMask mask{IndexRange(10)};
    IndexMask new_mask = mask.slice_and_offset(IndexRange(3, 5), indices);
    EXPECT_TRUE(new_mask.is_range());
    EXPECT_EQ(new_mask.size(), 5);
    EXPECT_EQ(new_mask[0], 0);
    EXPECT_EQ(new_mask[1], 1);
  }
  {
    Vector<int64_t> original_indices = {2, 3, 5, 7, 8, 9, 10};
    IndexMask mask{original_indices.as_span()};
    IndexMask new_mask = mask.slice_and_offset(IndexRange(1, 4), indices);
    EXPECT_FALSE(new_mask.is_range());
    EXPECT_EQ(new_mask.size(), 4);
    EXPECT_EQ(new_mask[0], 0);
    EXPECT_EQ(new_mask[1], 2);
    EXPECT_EQ(new_mask[2], 4);
    EXPECT_EQ(new_mask[3], 5);
  }
}

}  // namespace blender::tests
