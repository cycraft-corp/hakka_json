#include <hakka_json_array.hpp>
#include <hakka_json_int.hpp>
#include <hakka_json_string.hpp>

#include <gtest/gtest.h>

using namespace hakka;

// helper that dynamic cast JsonHandleCompact to correct type
template <typename T>
const T *h2t(const JsonHandleCompact &handle)
{
    auto view = handle.get_view();
    return std::get<const T*>(view);
}

template <typename T>
T *h2t_mut(JsonHandleCompact &handle)
{
    auto mut_ptr = handle.get_mut_ptr();
    return std::get<T*>(mut_ptr);
}

TEST(JsonArray, Create)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);
    ASSERT_EQ(json_array.get_type(), HakkaJsonType::HAKKA_JSON_ARRAY);
    ASSERT_EQ(h2t<JsonArrayCompact>(json_array)->dump(512).value(), "[]");
}

TEST(JsonArray, PushBackAndAt)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 0);

    auto json_int = JsonIntCompact::create(42);
    auto json_str = JsonStringCompact::create("Hello");

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_str), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 2);

    auto element0 = h2t_mut<JsonArrayCompact>(json_array)->at(0);
    ASSERT_TRUE(element0);
    ASSERT_EQ(element0.value().get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(std::get<int64_t>(h2t<JsonIntCompact>(element0.value())->get().value()), 42);

    auto element1 = h2t_mut<JsonArrayCompact>(json_array)->at(1);
    ASSERT_TRUE(element1);
    ASSERT_EQ(element1.value().get_type(), HakkaJsonType::HAKKA_JSON_STRING);
    ASSERT_EQ(std::get<std::string>(h2t<JsonStringCompact>(element1.value())->get().value()), "Hello");
}

TEST(JsonArray, Compare)
{
    auto json_array1 = JsonArrayCompact::create();
    auto json_array2 = JsonArrayCompact::create();
    auto json_array3 = JsonArrayCompact::create();

    ASSERT_TRUE(json_array1);
    ASSERT_TRUE(json_array2);
    ASSERT_TRUE(json_array3);

    auto json_int1 = JsonIntCompact::create(42);
    auto json_int2 = JsonIntCompact::create(42);
    auto json_int3 = JsonIntCompact::create(100);

    auto json_str1 = JsonStringCompact::create("Hello");
    auto json_str2 = JsonStringCompact::create("World");

    // Populate json_array1 and json_array2 identically
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(json_int1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(json_str1), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array2)->push_back(json_int2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array2)->push_back(json_str1), HAKKA_JSON_SUCCESS);

    // Populate json_array3 differently
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array3)->push_back(json_int3), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array3)->push_back(json_str2), HAKKA_JSON_SUCCESS);

    // Compare json_array1 and json_array2 (should be equal)
    auto result = h2t<JsonArrayCompact>(json_array1)->compare(json_array2);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result, 0);

    // Compare json_array1 and json_array3 (json_array1 < json_array3)
    result = h2t<JsonArrayCompact>(json_array1)->compare(json_array3);
    ASSERT_TRUE(result);
    ASSERT_LT(*result, 0);

    // Compare json_array3 and json_array1 (json_array3 > json_array1)
    result = h2t<JsonArrayCompact>(json_array3)->compare(json_array1);
    ASSERT_TRUE(result);
    ASSERT_GT(*result, 0);
}

// test hash
TEST(JsonArray, Hash)
{
    auto json_array1 = JsonArrayCompact::create();
    auto json_array2 = JsonArrayCompact::create();
    auto json_array3 = JsonArrayCompact::create();

    ASSERT_TRUE(json_array1);
    ASSERT_TRUE(json_array2);
    ASSERT_TRUE(json_array3);

    auto json_int1 = JsonIntCompact::create(42);
    auto json_int2 = JsonIntCompact::create(42);
    auto json_int3 = JsonIntCompact::create(100);

    auto json_str1 = JsonStringCompact::create("Hello");
    auto json_str2 = JsonStringCompact::create("World");

    // Populate json_array1 and json_array2 identically
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(json_int1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(json_str1), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array2)->push_back(json_int2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array2)->push_back(json_str1), HAKKA_JSON_SUCCESS);

    // Populate json_array3 differently
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array3)->push_back(json_int3), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array3)->push_back(json_str2), HAKKA_JSON_SUCCESS);

    // Hash json_array1 and json_array2 (should be equal)
    auto hash1 = h2t<JsonArrayCompact>(json_array1)->hash();
    auto hash2 = h2t<JsonArrayCompact>(json_array2)->hash();
    ASSERT_EQ(hash1, hash2);

    // Hash json_array1 and json_array3 (should be different)
    hash1 = h2t<JsonArrayCompact>(json_array1)->hash();
    auto hash3 = h2t<JsonArrayCompact>(json_array3)->hash();
    ASSERT_NE(hash1, hash3);
}

TEST(JsonArray, DumpSize)
{
    {
        auto json_array = JsonArrayCompact::create();
        ASSERT_TRUE(json_array);

        auto json_int = JsonIntCompact::create(42);
        auto json_str = JsonStringCompact::create("Hello");

        ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int), HAKKA_JSON_SUCCESS);
        ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_str), HAKKA_JSON_SUCCESS);

        auto size = h2t<JsonArrayCompact>(json_array)->dump_size();
        ASSERT_EQ(size, 13); // '[42, "Hello"]'.size() = 13
    }
    // nested
    {
        auto json_array = JsonArrayCompact::create();
        ASSERT_TRUE(json_array);

        auto json_int = JsonIntCompact::create(42);
        auto json_str = JsonStringCompact::create("Hello");

        auto nested_array = JsonArrayCompact::create();
        ASSERT_TRUE(nested_array);
        ASSERT_EQ(h2t_mut<JsonArrayCompact>(nested_array)->push_back(json_int), HAKKA_JSON_SUCCESS);
        ASSERT_EQ(h2t_mut<JsonArrayCompact>(nested_array)->push_back(json_str), HAKKA_JSON_SUCCESS);

        ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(nested_array), HAKKA_JSON_SUCCESS);

        auto size = h2t<JsonArrayCompact>(json_array)->dump_size();
        ASSERT_EQ(size, 15); // '[[42, "Hello"]]'.size() = 15
    }
    // empty
    {
        auto json_array = JsonArrayCompact::create();
        ASSERT_TRUE(json_array);

        auto size = h2t<JsonArrayCompact>(json_array)->dump_size();
        ASSERT_EQ(size, 2); // '[]'.size() = 2
    }
}

TEST(JsonArray, Dump)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    ASSERT_EQ(h2t<JsonArrayCompact>(json_array)->dump(512).value(), "[]");

    auto json_int = JsonIntCompact::create(42);
    auto json_str = JsonStringCompact::create("Hello");

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_str), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t<JsonArrayCompact>(json_array)->dump(512).value(), "[42, \"Hello\"]");
}

TEST(JsonArray, ToBytes)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_int = JsonIntCompact::create(42);
    auto json_str = JsonStringCompact::create("Hello");

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_str), HAKKA_JSON_SUCCESS);

    char buffer[512];
    uint32_t buffer_size = sizeof(buffer);
    auto result = h2t<JsonArrayCompact>(json_array)->to_bytes(buffer, &buffer_size);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);

    // Assuming to_bytes serializes to JSON string
    ASSERT_STREQ(buffer, "[42, \"Hello\"]");
    ASSERT_EQ(buffer_size, strlen("[42, \"Hello\"]"));
}

TEST(JsonArray, InvalidToBytes)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_int = JsonIntCompact::create(123456);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int), HAKKA_JSON_SUCCESS);

    char buffer[5]; // Insufficient buffer size
    uint32_t buffer_size = sizeof(buffer);
    auto result = h2t<JsonArrayCompact>(json_array)->to_bytes(buffer, &buffer_size);
    ASSERT_EQ(result, HAKKA_JSON_NOT_ENOUGH_MEMORY);
}

TEST(JsonArray, RefCount)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    ASSERT_EQ(h2t<JsonArrayCompact>(json_array)->inc_ref(), 2);
    ASSERT_EQ(h2t<JsonArrayCompact>(json_array)->inc_ref(), 3);
    ASSERT_EQ(h2t<JsonArrayCompact>(json_array)->dec_ref(), 2);
    ASSERT_EQ(h2t<JsonArrayCompact>(json_array)->dec_ref(), 1);
}

TEST(JsonArray, RefCountAlive)
{
    JsonHandleCompact array_out_handle;
    {
        auto json_handle = JsonArrayCompact::create(); // Ref count = 1
        ASSERT_TRUE(json_handle);
        array_out_handle = json_handle;       // Copy constructor, Ref count = 2
        ASSERT_EQ(h2t<JsonArrayCompact>(json_handle)->inc_ref(), 3); // Ref count = 3
    } // json_handle goes out of scope, Ref count = 2

    // Check if the object is still alive
    ASSERT_TRUE(array_out_handle.get_type() == HakkaJsonType::HAKKA_JSON_ARRAY);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(array_out_handle)->length(), 0);

    ASSERT_EQ(h2t<JsonArrayCompact>(array_out_handle)->dec_ref(), 1); // Ref count = 1

    // Check if the object is still alive
    ASSERT_TRUE(array_out_handle.get_type() == HakkaJsonType::HAKKA_JSON_ARRAY);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(array_out_handle)->length(), 0);
} // Release the object correctly

TEST(JsonArray, SetAndGet)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_int = JsonIntCompact::create(42);
    auto json_str = JsonStringCompact::create("Hello");

    // Initially empty, setting at index 0 should fail
    auto result = h2t_mut<JsonArrayCompact>(json_array)->set(0, json_int);
    ASSERT_EQ(result, HAKKA_JSON_INDEX_OUT_OF_BOUNDS);

    // Push back elements and set
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_str), HAKKA_JSON_SUCCESS);

    auto json_new_int = JsonIntCompact::create(100);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->set(1, json_new_int), HAKKA_JSON_SUCCESS);

    auto element = h2t_mut<JsonArrayCompact>(json_array)->at(1);
    ASSERT_TRUE(element);
    ASSERT_EQ(element.value().get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(h2t<JsonIntCompact>(element.value())->get().value(), PrimitiveType(100));
}

TEST(JsonArray, Remove)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_int = JsonIntCompact::create(42);
    auto json_str = JsonStringCompact::create("Hello");

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_str), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 2);

    // Remove element at index 0
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->remove(0), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 1);

    auto element = h2t_mut<JsonArrayCompact>(json_array)->at(0);
    ASSERT_TRUE(element);
    ASSERT_EQ(element.value().get_type(), HakkaJsonType::HAKKA_JSON_STRING);
    ASSERT_EQ(h2t<JsonStringCompact>(element.value())->get().value(), PrimitiveType("Hello"));

    // Attempt to remove out of bounds
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->remove(5), HAKKA_JSON_INDEX_OUT_OF_BOUNDS);
}

TEST(JsonArray, Clear)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_int = JsonIntCompact::create(42);
    auto json_str = JsonStringCompact::create("Hello");

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_str), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 2);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->clear(), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 0);
    ASSERT_EQ(h2t<JsonArrayCompact>(json_array)->dump(512).value(), "[]");
}

TEST(JsonArray, PushBackInvalidHandle)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    JsonHandleCompact invalid_handle; // Null handle
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(invalid_handle), HAKKA_JSON_INVALID_ARGUMENT);
}

TEST(JsonArray, Extend)
{
    auto json_array1 = JsonArrayCompact::create();
    auto json_array2 = JsonArrayCompact::create();

    ASSERT_TRUE(json_array1);
    ASSERT_TRUE(json_array2);

    auto json_int1 = JsonIntCompact::create(1);
    auto json_int2 = JsonIntCompact::create(2);
    auto json_int3 = JsonIntCompact::create(3);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(json_int1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(json_int2), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array2)->push_back(json_int3), HAKKA_JSON_SUCCESS);

    // Extend json_array1 with json_array2
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->extend(json_array2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->length(), 3);

    auto element2 = h2t_mut<JsonArrayCompact>(json_array1)->at(2);
    ASSERT_TRUE(element2);
    ASSERT_EQ(element2.value().get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(h2t<JsonIntCompact>(element2.value())->get().value(), PrimitiveType(3));
}

TEST(JsonArray, Count)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_int1 = JsonIntCompact::create(42);
    auto json_int2 = JsonIntCompact::create(42);
    auto json_int3 = JsonIntCompact::create(100);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int3), HAKKA_JSON_SUCCESS);

    uint32_t count = 0;
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->count(json_int1, &count), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(count, 2);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->count(json_int3, &count), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(count, 1);

    auto json_str = JsonStringCompact::create("Hello");
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->count(json_str, &count), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(count, 0);
}

TEST(JsonArray, Index)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_int1 = JsonIntCompact::create(1);
    auto json_int2 = JsonIntCompact::create(2);
    auto json_int3 = JsonIntCompact::create(3);
    auto json_int4 = JsonIntCompact::create(4);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int3), HAKKA_JSON_SUCCESS);

    uint32_t index = 0;
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->index(json_int2, 0, 3, &index), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(index, 1);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->index(json_int4, 0, 3, &index), HAKKA_JSON_KEY_NOT_FOUND);
}

TEST(JsonArray, Pop)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_int1 = JsonIntCompact::create(1);
    auto json_int2 = JsonIntCompact::create(2);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int2), HAKKA_JSON_SUCCESS);

    JsonHandleCompact popped;
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->pop(1, &popped), HAKKA_JSON_SUCCESS);
    ASSERT_TRUE(popped);
    ASSERT_EQ(popped.get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(h2t<JsonIntCompact>(popped)->get().value(), PrimitiveType(2));
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 1);

    // Attempt to pop out of bounds
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->pop(5, &popped), HAKKA_JSON_INDEX_OUT_OF_BOUNDS);
}

TEST(JsonArray, Reverse)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_int1 = JsonIntCompact::create(1);
    auto json_int2 = JsonIntCompact::create(2);
    auto json_int3 = JsonIntCompact::create(3);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int3), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->reverse(), HAKKA_JSON_SUCCESS);

    auto element0 = h2t_mut<JsonArrayCompact>(json_array)->at(0);
    auto element1 = h2t_mut<JsonArrayCompact>(json_array)->at(1);
    auto element2 = h2t_mut<JsonArrayCompact>(json_array)->at(2);

    ASSERT_EQ(h2t<JsonIntCompact>(element0.value())->get().value(), PrimitiveType(3));
    ASSERT_EQ(h2t<JsonIntCompact>(element1.value())->get().value(), PrimitiveType(2));
    ASSERT_EQ(h2t<JsonIntCompact>(element2.value())->get().value(), PrimitiveType(1));
}

TEST(JsonArray, GetSlice)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    // Populate array with [1, 2, 3, 4, 5]
    for (int i = 1; i <= 5; ++i)
    {
        auto json_int = JsonIntCompact::create(static_cast<int64_t>(i));
        ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int), HAKKA_JSON_SUCCESS);
    }

    // Get slice [2, 4)
    auto slice = h2t_mut<JsonArrayCompact>(json_array)->get_slice(1, 4, 1);
    ASSERT_TRUE(slice);
    ASSERT_EQ(slice.value().get_type(), HakkaJsonType::HAKKA_JSON_ARRAY);

    auto sliced_array = h2t<JsonArrayCompact>(slice.value());
    ASSERT_EQ(sliced_array->length(), 3);
    ASSERT_EQ(h2t<JsonIntCompact>(sliced_array->at(0).value())->get().value(), PrimitiveType(2));
    ASSERT_EQ(h2t<JsonIntCompact>(sliced_array->at(1).value())->get().value(), PrimitiveType(3));
    ASSERT_EQ(h2t<JsonIntCompact>(sliced_array->at(2).value())->get().value(), PrimitiveType(4));
}

TEST(JsonArray, SetSlice)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    // Populate array with [1, 2, 3, 4, 5]
    for (int i = 1; i <= 5; ++i)
    {
        auto json_int = JsonIntCompact::create(static_cast<int64_t>(i));
        ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int), HAKKA_JSON_SUCCESS);
    }

    // Create a slice [6, 7]
    auto slice = JsonArrayCompact::create();
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(slice)->push_back(JsonIntCompact::create(6)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(slice)->push_back(JsonIntCompact::create(7)), HAKKA_JSON_SUCCESS);

    // Set slice [1:3] to [6,7], resulting in [1,6,7,4,5]
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->set_slice(1, 3, 1, slice), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->at(0).value().get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(h2t<JsonIntCompact>(h2t_mut<JsonArrayCompact>(json_array)->at(0).value())->get().value(), PrimitiveType(1));

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->at(1).value().get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(h2t<JsonIntCompact>(h2t_mut<JsonArrayCompact>(json_array)->at(1).value())->get().value(), PrimitiveType(6));

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->at(2).value().get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(h2t<JsonIntCompact>(h2t_mut<JsonArrayCompact>(json_array)->at(2).value())->get().value(), PrimitiveType(7));

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->at(3).value().get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(h2t<JsonIntCompact>(h2t_mut<JsonArrayCompact>(json_array)->at(3).value())->get().value(), PrimitiveType(4));

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->at(4).value().get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(h2t<JsonIntCompact>(h2t_mut<JsonArrayCompact>(json_array)->at(4).value())->get().value(), PrimitiveType(5));
}

TEST(JsonArray, MultiplyArray)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_int = JsonIntCompact::create(42);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->multiply(3), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 3);
    for (size_t i = 0; i < h2t_mut<JsonArrayCompact>(json_array)->length(); ++i)
    {
        auto element = h2t_mut<JsonArrayCompact>(json_array)->at(i);
        ASSERT_TRUE(element);
        ASSERT_EQ(element.value().get_type(), HakkaJsonType::HAKKA_JSON_INT);
        ASSERT_EQ(h2t<JsonIntCompact>(element.value())->get().value(), PrimitiveType(42));
    }
}

TEST(JsonArray, Insert)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_int1 = JsonIntCompact::create(1);
    auto json_int2 = JsonIntCompact::create(2);
    auto json_int3 = JsonIntCompact::create(3);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int3), HAKKA_JSON_SUCCESS);

    // Insert json_int2 at index 1
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->insert(1, json_int2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 3);

    auto element1 = h2t_mut<JsonArrayCompact>(json_array)->at(1);
    ASSERT_TRUE(element1);
    ASSERT_EQ(h2t<JsonIntCompact>(element1.value())->get().value(), PrimitiveType(2));

    // check the size after insert
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 3);

    // Attempt to insert out of bounds
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->insert(5, json_int2), HAKKA_JSON_INDEX_OUT_OF_BOUNDS);
}

TEST(JsonArray, Erase)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_int1 = JsonIntCompact::create(1);
    auto json_int2 = JsonIntCompact::create(2);
    auto json_int3 = JsonIntCompact::create(3);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int3), HAKKA_JSON_SUCCESS);

    // Erase element at index 1
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->erase(1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 2);

    auto element1 = h2t_mut<JsonArrayCompact>(json_array)->at(1);
    ASSERT_TRUE(element1);
    ASSERT_EQ(h2t<JsonIntCompact>(element1.value())->get().value(), PrimitiveType(3));

    // Attempt to erase out of bounds
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->erase(5), HAKKA_JSON_INDEX_OUT_OF_BOUNDS);
}

TEST(JsonArray, DeepNesting)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    // Create a deeply nested array structure
    const int depth = 10;
    auto current_array = json_array;
    for (int i = 0; i < depth; ++i)
    {
        auto new_array = JsonArrayCompact::create();
        ASSERT_EQ(h2t_mut<JsonArrayCompact>(current_array)->push_back(new_array), HAKKA_JSON_SUCCESS);
        current_array = new_array;
    }

    // Verify the structure
    current_array = json_array;
    for (int i = 0; i < depth; ++i)
    {
        ASSERT_EQ(h2t_mut<JsonArrayCompact>(current_array)->length(), 1);
        auto element = h2t_mut<JsonArrayCompact>(current_array)->at(0);
        ASSERT_TRUE(element);
        ASSERT_EQ(element.value().get_type(), HakkaJsonType::HAKKA_JSON_ARRAY);
        current_array = element.value();
    }

    // dump it and print to stdout
    auto dump_result = h2t<JsonArrayCompact>(json_array)->dump(512);
    ASSERT_TRUE(dump_result);
    std::cout << dump_result.value() << std::endl;
}

// test the load function
TEST(JsonObject, Load)
{
    std::string json_str = R"([
        {
            "key1": 42,
            "key2": "value2",
            "key3": {
                "nestedKey1": [1, 2, 3],
                "nestedKey2": {
                    "deepKey": "deepValue"
                }
            }
        },
        {
            "key1": 100,
            "key2": "value3",
            "key3": {
                "nestedKey1": [4, 5, 6],
                "nestedKey2": {
                    "deepKey": "deepValue2"
                }
            }
        }
    ])";

    std::string dump_outed;
    {
        auto json_array = JsonArrayCompact::loads(json_str);
        ASSERT_TRUE(json_array);

        // Verify the structure
        ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array.value())->length(), 2);

        auto element1 = h2t_mut<JsonArrayCompact>(json_array.value())->at(0);
        ASSERT_TRUE(element1);
        ASSERT_EQ(element1.value().get_type(), HakkaJsonType::HAKKA_JSON_OBJECT);

        auto element2 = h2t_mut<JsonArrayCompact>(json_array.value())->at(1);
        ASSERT_TRUE(element2);
        ASSERT_EQ(element2.value().get_type(), HakkaJsonType::HAKKA_JSON_OBJECT);

        // dump it and print to stdout
        auto dump_result = h2t<JsonArrayCompact>(json_array.value())->dump(512);
        ASSERT_TRUE(dump_result);
        dump_outed = dump_result.value();
        std::cout << dump_outed << std::endl;
    }

    // dumped string could be load back
    {
        auto json_array = JsonArrayCompact::loads(dump_outed);
        ASSERT_TRUE(json_array);

        // Verify the structure
        ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array.value())->length(), 2);

        auto element1 = h2t_mut<JsonArrayCompact>(json_array.value())->at(0);
        ASSERT_TRUE(element1);
        ASSERT_EQ(element1.value().get_type(), HakkaJsonType::HAKKA_JSON_OBJECT);

        auto element2 = h2t_mut<JsonArrayCompact>(json_array.value())->at(1);
        ASSERT_TRUE(element2);
        ASSERT_EQ(element2.value().get_type(), HakkaJsonType::HAKKA_JSON_OBJECT);
    }
}

TEST(JsonObject, Load2)
{
    const char json_str[] = R"(["value1", "value2", "value3", "value4", "value5"])";
    auto json_array = JsonArrayCompact::loads(std::string_view(json_str, sizeof(json_str) - 1));
    ASSERT_TRUE(json_array);

    // Verify the structure
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array.value())->length(), 5);

    for (int i = 0; i < 5; ++i)
    {
        auto element = h2t_mut<JsonArrayCompact>(json_array.value())->at(i);
        ASSERT_TRUE(element);
        ASSERT_EQ(element.value().get_type(), HakkaJsonType::HAKKA_JSON_STRING);
        ASSERT_EQ(h2t<JsonStringCompact>(element.value())->get().value(), PrimitiveType("value" + std::to_string(i + 1)));
    }
}

// iterator test
TEST(JsonArray, Iterator)
{
    {
        auto json_array = JsonArrayCompact::create();
        ASSERT_TRUE(json_array);

        auto json_int1 = JsonIntCompact::create(1);
        auto json_int2 = JsonIntCompact::create(2);
        auto json_int3 = JsonIntCompact::create(3);

        ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int1), HAKKA_JSON_SUCCESS);
        ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int2), HAKKA_JSON_SUCCESS);
        ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int3), HAKKA_JSON_SUCCESS);

        // Iterate over the array
        int i = 1;
        for (auto it = h2t_mut<JsonArrayCompact>(json_array)->begin(); it != h2t_mut<JsonArrayCompact>(json_array)->end(); ++it)
        {
            ASSERT_TRUE(*it);
            ASSERT_EQ((*it).get_type(), HakkaJsonType::HAKKA_JSON_INT);
            ASSERT_EQ(h2t<JsonIntCompact>(*it)->get().value(), PrimitiveType(i));
            ++i;
        }
    }

    // test random access
    {
        auto json_array = JsonArrayCompact::create();
        ASSERT_TRUE(json_array);

        auto json_int1 = JsonIntCompact::create(1);
        auto json_int2 = JsonIntCompact::create(2);
        auto json_int3 = JsonIntCompact::create(3);

        ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int1), HAKKA_JSON_SUCCESS);
        ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int2), HAKKA_JSON_SUCCESS);
        ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_int3), HAKKA_JSON_SUCCESS);

        // Iterate over the array
        int i = 1;
        for (auto it = h2t_mut<JsonArrayCompact>(json_array)->begin(); it != h2t_mut<JsonArrayCompact>(json_array)->end(); ++it)
        {
            ASSERT_TRUE(*it);
            ASSERT_EQ((*it).get_type(), HakkaJsonType::HAKKA_JSON_INT);
            ASSERT_EQ(h2t<JsonIntCompact>(*it)->get().value(), PrimitiveType(i));
            ++i;
        }

        // Random access
        auto it = h2t_mut<JsonArrayCompact>(json_array)->begin();
        ASSERT_TRUE(it != h2t_mut<JsonArrayCompact>(json_array)->end());
        ASSERT_EQ((*it).get_type(), HakkaJsonType::HAKKA_JSON_INT);
        ASSERT_EQ(h2t<JsonIntCompact>(*it)->get().value(), PrimitiveType(1));

        it += 2;
        ASSERT_TRUE(it != h2t_mut<JsonArrayCompact>(json_array)->end());
        ASSERT_EQ((*it).get_type(), HakkaJsonType::HAKKA_JSON_INT);
        ASSERT_EQ(h2t<JsonIntCompact>(*it)->get().value(), PrimitiveType(3));

        it -= 1;
        ASSERT_TRUE(it != h2t_mut<JsonArrayCompact>(json_array)->end());
        ASSERT_EQ((*it).get_type(), HakkaJsonType::HAKKA_JSON_INT);
        ASSERT_EQ(h2t<JsonIntCompact>(*it)->get().value(), PrimitiveType(2));

        it += 1;
        ASSERT_TRUE(it != h2t_mut<JsonArrayCompact>(json_array)->end()); // 3

        it += 1;
        ASSERT_TRUE(it == h2t_mut<JsonArrayCompact>(json_array)->end());

        // out of bounds would set to the bound
        it += 1;
        ASSERT_TRUE(it == h2t_mut<JsonArrayCompact>(json_array)->end());
    }
}

// Test removing from beginning and verify no holes
TEST(JsonArray, RemoveFromBeginningNoHoles)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    // Create array [42, 43, 44]
    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonIntCompact::create(43);
    auto json_value3 = JsonIntCompact::create(44);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_value2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_value3), HAKKA_JSON_SUCCESS);

    // Verify initial state - 42 is at index 0
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 3);
    ASSERT_EQ(std::get<int64_t>(h2t<JsonIntCompact>(h2t_mut<JsonArrayCompact>(json_array)->at(0).value())->get().value()), 42);

    // Remove element at index 0
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->remove(0), HAKKA_JSON_SUCCESS);

    // Verify no holes - elements should shift
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 2);
    ASSERT_EQ(std::get<int64_t>(h2t<JsonIntCompact>(h2t_mut<JsonArrayCompact>(json_array)->at(0).value())->get().value()), 43);
    ASSERT_EQ(std::get<int64_t>(h2t<JsonIntCompact>(h2t_mut<JsonArrayCompact>(json_array)->at(1).value())->get().value()), 44);

    // Verify elements are accessible
    auto result0 = h2t_mut<JsonArrayCompact>(json_array)->at(0);
    ASSERT_TRUE(result0);
    ASSERT_EQ(std::get<int64_t>(h2t<JsonIntCompact>(result0.value())->get().value()), 43);

    auto result1 = h2t_mut<JsonArrayCompact>(json_array)->at(1);
    ASSERT_TRUE(result1);
    ASSERT_EQ(std::get<int64_t>(h2t<JsonIntCompact>(result1.value())->get().value()), 44);
}

// Test removing from middle and verify no holes
TEST(JsonArray, RemoveFromMiddleNoHoles)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonIntCompact::create(43);
    auto json_value3 = JsonIntCompact::create(44);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_value2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_value3), HAKKA_JSON_SUCCESS);

    // Remove element at index 1 (middle)
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->remove(1), HAKKA_JSON_SUCCESS);

    // Verify no holes - element at index 2 should shift to index 1
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 2);
    ASSERT_EQ(std::get<int64_t>(h2t<JsonIntCompact>(h2t_mut<JsonArrayCompact>(json_array)->at(0).value())->get().value()), 42);
    ASSERT_EQ(std::get<int64_t>(h2t<JsonIntCompact>(h2t_mut<JsonArrayCompact>(json_array)->at(1).value())->get().value()), 44);
}

// Test comparison after removal from beginning
TEST(JsonArray, CompareAfterRemovalFromBeginning)
{
    // Create array1: [42, 43, 44]
    auto json_array1 = JsonArrayCompact::create();
    ASSERT_TRUE(json_array1);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(JsonIntCompact::create(42)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(JsonIntCompact::create(43)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(JsonIntCompact::create(44)), HAKKA_JSON_SUCCESS);

    // Remove first element from array1 -> [43, 44]
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->remove(0), HAKKA_JSON_SUCCESS);

    // Create array2 directly as [43, 44]
    auto json_array2 = JsonArrayCompact::create();
    ASSERT_TRUE(json_array2);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array2)->push_back(JsonIntCompact::create(43)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array2)->push_back(JsonIntCompact::create(44)), HAKKA_JSON_SUCCESS);

    // Compare: should be equal because no holes, elements shifted
    auto compare_result = h2t<JsonArrayCompact>(json_array1)->compare(json_array2);
    ASSERT_TRUE(compare_result);
    ASSERT_EQ(*compare_result, 0) << "Arrays should be equal after removal (no holes)";

    // Verify dump output is identical
    auto dump1 = h2t<JsonArrayCompact>(json_array1)->dump(512).value();
    auto dump2 = h2t<JsonArrayCompact>(json_array2)->dump(512).value();
    ASSERT_EQ(dump1, dump2);
}

// Test comparison after removal from middle
TEST(JsonArray, CompareAfterRemovalFromMiddle)
{
    // Create array1: [42, 43, 44]
    auto json_array1 = JsonArrayCompact::create();
    ASSERT_TRUE(json_array1);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(JsonIntCompact::create(42)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(JsonIntCompact::create(43)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(JsonIntCompact::create(44)), HAKKA_JSON_SUCCESS);

    // Remove middle element from array1 -> [42, 44]
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->remove(1), HAKKA_JSON_SUCCESS);

    // Create array2 directly as [42, 44]
    auto json_array2 = JsonArrayCompact::create();
    ASSERT_TRUE(json_array2);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array2)->push_back(JsonIntCompact::create(42)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array2)->push_back(JsonIntCompact::create(44)), HAKKA_JSON_SUCCESS);

    // Compare: should be equal
    auto compare_result = h2t<JsonArrayCompact>(json_array1)->compare(json_array2);
    ASSERT_TRUE(compare_result);
    ASSERT_EQ(*compare_result, 0);
}

// Test multiple removals and comparison
TEST(JsonArray, MultipleRemovalsAndComparison)
{
    // Create array1: [1, 2, 3, 4, 5]
    auto json_array1 = JsonArrayCompact::create();
    ASSERT_TRUE(json_array1);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(JsonIntCompact::create(1)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(JsonIntCompact::create(2)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(JsonIntCompact::create(3)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(JsonIntCompact::create(4)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->push_back(JsonIntCompact::create(5)), HAKKA_JSON_SUCCESS);

    // Remove 1, 3, 5 (indices shift after each removal)
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->remove(0), HAKKA_JSON_SUCCESS); // Remove 1 -> [2,3,4,5]
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->remove(1), HAKKA_JSON_SUCCESS); // Remove 3 -> [2,4,5]
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->remove(2), HAKKA_JSON_SUCCESS); // Remove 5 -> [2,4]

    // Create array2 directly as [2, 4]
    auto json_array2 = JsonArrayCompact::create();
    ASSERT_TRUE(json_array2);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array2)->push_back(JsonIntCompact::create(2)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array2)->push_back(JsonIntCompact::create(4)), HAKKA_JSON_SUCCESS);

    // Verify length
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array1)->length(), 2);

    // Compare
    auto compare_result = h2t<JsonArrayCompact>(json_array1)->compare(json_array2);
    ASSERT_TRUE(compare_result);
    ASSERT_EQ(*compare_result, 0);

    // Verify iteration works correctly after multiple removals
    auto it = h2t_mut<JsonArrayCompact>(json_array1)->begin();
    auto end = h2t_mut<JsonArrayCompact>(json_array1)->end();

    ASSERT_TRUE(it != end);
    ASSERT_EQ(std::get<int64_t>(h2t<JsonIntCompact>(*it)->get().value()), 2);

    ++it;
    ASSERT_TRUE(it != end);
    ASSERT_EQ(std::get<int64_t>(h2t<JsonIntCompact>(*it)->get().value()), 4);

    ++it;
    ASSERT_TRUE(it == end);
}

// Test removing from end (pop_back scenario)
TEST(JsonArray, RemoveFromEndNoHoles)
{
    auto json_array = JsonArrayCompact::create();
    ASSERT_TRUE(json_array);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonIntCompact::create(43);
    auto json_value3 = JsonIntCompact::create(44);

    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_value2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->push_back(json_value3), HAKKA_JSON_SUCCESS);

    // Remove last element
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->remove(2), HAKKA_JSON_SUCCESS);

    // Verify no holes
    ASSERT_EQ(h2t_mut<JsonArrayCompact>(json_array)->length(), 2);
    ASSERT_EQ(std::get<int64_t>(h2t<JsonIntCompact>(h2t_mut<JsonArrayCompact>(json_array)->at(0).value())->get().value()), 42);
    ASSERT_EQ(std::get<int64_t>(h2t<JsonIntCompact>(h2t_mut<JsonArrayCompact>(json_array)->at(1).value())->get().value()), 43);

    // Verify out-of-bounds check still works
    auto result = h2t_mut<JsonArrayCompact>(json_array)->at(2);
    ASSERT_FALSE(result);
    ASSERT_EQ(result.error(), HAKKA_JSON_INDEX_OUT_OF_BOUNDS);
}
