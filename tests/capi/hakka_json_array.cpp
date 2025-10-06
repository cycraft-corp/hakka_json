#include <gtest/gtest.h>

#include <hakka_json_object.h>
#include <hakka_json_array.h>
#include <hakka_json_primitive.h>

#include <cstring>
#include <string>

TEST(HakkaJsonArrayTest, CreateAndDumpEmptyArray)
{
    HakkaHandle array;
    ASSERT_EQ(CreateHakkaArray(&array), HAKKA_JSON_SUCCESS);
    ASSERT_NE(array, 0);

    uint8_t buffer[256] = {0};
    uint64_t buffer_size = sizeof(buffer);
    ASSERT_EQ(DumpHakkaArray(array, 10, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "[]");

    // Clean up
    HakkaRelease(&array);
}

TEST(HakkaJsonArrayTest, LoadAndDumpArray)
{
    const char json_str[] = R"([42, "value2"])";
    HakkaHandle array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &array, 10), HAKKA_JSON_SUCCESS);
    ASSERT_NE(array, 0);

    uint8_t buffer[256] = {0};
    uint64_t buffer_size = sizeof(buffer);
    ASSERT_EQ(DumpHakkaArray(array, 10, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), json_str);

    // Clean up
    HakkaRelease(&array);
}

TEST(HakkaJsonArrayTest, SetAndGetInt)
{
    HakkaHandle array;
    ASSERT_EQ(CreateHakkaArray(&array), HAKKA_JSON_SUCCESS);
    ASSERT_NE(array, 0);

    HakkaHandle int_value;
    ASSERT_EQ(CreateHakkaInt(&int_value, 42), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(PushBackHakkaArray(array, int_value), HAKKA_JSON_SUCCESS);

    HakkaHandle value_get = 0;
    ASSERT_EQ(GetHakkaArrayObject(array, 0, &value_get), HAKKA_JSON_SUCCESS);

    int64_t value = 0;
    ASSERT_EQ(GetHakkaInt(value_get, &value), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(value, 42);

    // Clean up
    HakkaRelease(&array);
    HakkaRelease(&int_value);
    HakkaRelease(&value_get);
}

TEST(HakkaJsonArrayTest, SetAndGetString)
{
    HakkaHandle array;
    ASSERT_EQ(CreateHakkaArray(&array), HAKKA_JSON_SUCCESS);
    ASSERT_NE(array, 0);

    HakkaHandle string_value;
    ASSERT_EQ(CreateHakkaString(&string_value, reinterpret_cast<const uint8_t *>("value2"), 6), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(PushBackHakkaArray(array, string_value), HAKKA_JSON_SUCCESS);

    HakkaHandle new_string_value;
    ASSERT_EQ(CreateHakkaString(&new_string_value, reinterpret_cast<const uint8_t *>("new_value"), 10), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(SetHakkaArray(array, 0, new_string_value), HAKKA_JSON_SUCCESS);

    HakkaHandle value_get = 0;
    ASSERT_EQ(GetHakkaArrayObject(array, 0, &value_get), HAKKA_JSON_SUCCESS);

    uint8_t buffer[256] = {0};
    uint32_t buffer_size = sizeof(buffer);
    ASSERT_EQ(GetHakkaString(value_get, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "new_value");

    // Clean up
    HakkaRelease(&array);
    HakkaRelease(&string_value);
    HakkaRelease(&new_string_value);
    HakkaRelease(&value_get);
}

TEST(HakkaJsonArrayTest, SetAndGetSlice)
{
    HakkaHandle array;
    ASSERT_EQ(CreateHakkaArray(&array), HAKKA_JSON_SUCCESS);
    ASSERT_NE(array, 0);

    HakkaHandle int_value;
    ASSERT_EQ(CreateHakkaInt(&int_value, 42), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(PushBackHakkaArray(array, int_value), HAKKA_JSON_SUCCESS);

    HakkaHandle string_value;
    ASSERT_EQ(CreateHakkaString(&string_value, reinterpret_cast<const uint8_t *>("value2"), 6), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(PushBackHakkaArray(array, string_value), HAKKA_JSON_SUCCESS);

    HakkaHandle slice = 0;
    ASSERT_EQ(GetHakkaArraySlice(array, 0, 2, 1, &slice), HAKKA_JSON_SUCCESS);

    uint32_t size = 0;
    ASSERT_EQ(GetHakkaArraySize(slice, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 2);

    // slice is a new array, we check all the values
    HakkaHandle value_get = 0;
    ASSERT_EQ(GetHakkaArrayObject(slice, 0, &value_get), HAKKA_JSON_SUCCESS);

    int64_t int_value_get = 0;
    ASSERT_EQ(GetHakkaInt(value_get, &int_value_get), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(int_value_get, 42);

    HakkaHandle value_get_1 = 0;
    ASSERT_EQ(GetHakkaArrayObject(slice, 1, &value_get_1), HAKKA_JSON_SUCCESS);

    uint8_t buffer[256] = {0};
    uint32_t buffer_size = sizeof(buffer);
    ASSERT_EQ(GetHakkaString(value_get_1, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "value2");

    // Clean up
    HakkaRelease(&array);
    HakkaRelease(&int_value);
    HakkaRelease(&string_value);
    HakkaRelease(&slice);
    HakkaRelease(&value_get);
    HakkaRelease(&value_get_1);
}

TEST(HakkaJsonArrayTest, SetSlice)
{
    // load a list of json string to array object
    const char json_str[] = R"(["value1", "value2", "value3", "value4", "value5"])";
    HakkaHandle array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &array, 10), HAKKA_JSON_SUCCESS);

    // create a new array object
    HakkaHandle new_array;
    ASSERT_EQ(CreateHakkaArray(&new_array), HAKKA_JSON_SUCCESS);

    // array[:] = new_array is equivalent to array = new_array
    ASSERT_EQ(SetHakkaArraySlice(array, 0, 5, 1, new_array), HAKKA_JSON_SUCCESS);

    // check the size of the array
    uint32_t size = UINT32_MAX;
    ASSERT_EQ(GetHakkaArraySize(array, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 0);

    // Clean up
    HakkaRelease(&array);
    HakkaRelease(&new_array);
}

TEST(HakkaJsonArrayTest, RemoveIndex)
{
    // load a list of json string to array object
    const char json_str[] = R"(["value1", "value2", "value3", "value4", "value5"])";
    HakkaHandle array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &array, 10), HAKKA_JSON_SUCCESS);

    // remove the first element
    ASSERT_EQ(RemoveHakkaArrayIndex(array, 0), HAKKA_JSON_SUCCESS);

    // check the size of the array
    uint32_t size = UINT32_MAX;
    ASSERT_EQ(GetHakkaArraySize(array, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 4);

    // Clean up
    HakkaRelease(&array);
}

TEST(HakkaJsonArrayTest, ClearArray)
{
    // load a list of json string to array object
    const char json_str[] = R"(["value1", "value2", "value3", "value4", "value5"])";
    HakkaHandle array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &array, 10), HAKKA_JSON_SUCCESS);

    // clear the array
    ASSERT_EQ(ClearHakkaArray(array), HAKKA_JSON_SUCCESS);

    // check the size of the array
    uint32_t size = UINT32_MAX;
    ASSERT_EQ(GetHakkaArraySize(array, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 0);

    // Clean up
    HakkaRelease(&array);
}

TEST(HakkaJsonArrayTest, MultiplyArray)
{
    // load a list of json string to array object
    const char json_str[] = R"(["value1", "value2", "value3", "value4", "value5"])";
    HakkaHandle array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &array, 10), HAKKA_JSON_SUCCESS);

    // multiply the array by 3
    ASSERT_EQ(MultiplyHakkaArray(array, 3), HAKKA_JSON_SUCCESS);

    // check the size of the array
    uint32_t size = UINT32_MAX;
    ASSERT_EQ(GetHakkaArraySize(array, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 15);

    // Clean up
    HakkaRelease(&array);
}

TEST(HakkaJsonArrayTest, CountArray)
{
    // load a list of json string to array object
    const char json_str[] = R"(["value1", "value2", "value3", "value4", "value5"])";
    HakkaHandle array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &array, 10), HAKKA_JSON_SUCCESS);

    // count the number of "value2" in the array
    HakkaHandle value;
    ASSERT_EQ(CreateHakkaString(&value, reinterpret_cast<const uint8_t *>("value2"), 6), HAKKA_JSON_SUCCESS);

    uint32_t count = UINT32_MAX;
    ASSERT_EQ(CountHakkaArray(array, value, &count), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(count, 1);

    // Clean up
    HakkaRelease(&array);
    HakkaRelease(&value);
}

TEST(HakkaJsonArrayTest, ExtendArray)
{
    // load a list of json string to array object
    const char json_str[] = R"(["value1", "value2", "value3", "value4", "value5"])";
    HakkaHandle array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &array, 10), HAKKA_JSON_SUCCESS);

    // extend the array with another array
    const char json_str_extend[] = R"(["value6", "value7", "value8", "value9", "value10"])";
    HakkaHandle array_extend;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str_extend), sizeof(json_str_extend) - 1, &array_extend, 10), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(ExtendHakkaArrayArray(array, array_extend), HAKKA_JSON_SUCCESS);

    // check the size of the array
    uint32_t size = UINT32_MAX;
    ASSERT_EQ(GetHakkaArraySize(array, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 10);

    // Clean up
    HakkaRelease(&array);
    HakkaRelease(&array_extend);
}

TEST(HakkaJsonArrayTest, FindFirstArray)
{
    // load a list of json string to array object
    const char json_str[] = R"(["value1", "value2", "value3", "value4", "value5"])";
    HakkaHandle array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &array, 10), HAKKA_JSON_SUCCESS);

    // find the first occurrence of "value3" in the array
    HakkaHandle value;
    ASSERT_EQ(CreateHakkaString(&value, reinterpret_cast<const uint8_t *>("value3"), 6), HAKKA_JSON_SUCCESS);

    uint32_t index = UINT32_MAX;
    ASSERT_EQ(FindFirstHakkaArray(array, value, 0, 5, &index), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(index, 2);

    // Clean up
    HakkaRelease(&array);
    HakkaRelease(&value);
}

TEST(HakkaJsonArrayTest, PushBackArray)
{
    // create an empty array
    HakkaHandle array;
    ASSERT_EQ(CreateHakkaArray(&array), HAKKA_JSON_SUCCESS);

    // push back an integer value
    HakkaHandle int_value;
    ASSERT_EQ(CreateHakkaInt(&int_value, 42), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(PushBackHakkaArray(array, int_value), HAKKA_JSON_SUCCESS);

    // check the size of the array
    uint32_t size = UINT32_MAX;
    ASSERT_EQ(GetHakkaArraySize(array, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 1);

    // Clean up
    HakkaRelease(&array);
    HakkaRelease(&int_value);
}

TEST(HakkaJsonArrayTest, PopArray)
{
    // load a list of json string to array object
    const char json_str[] = R"(["value1", "value2", "value3", "value4", "value5"])";
    HakkaHandle array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &array, 10), HAKKA_JSON_SUCCESS);

    // pop the first element
    HakkaHandle value;
    ASSERT_EQ(PopHakkaArray(array, 0, &value), HAKKA_JSON_SUCCESS);
    ASSERT_NE(value, 0);

    // check the size of the array
    uint32_t size = UINT32_MAX;
    ASSERT_EQ(GetHakkaArraySize(array, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 4);

    // check the value of the popped element
    uint8_t buffer[256] = {0};
    uint32_t buffer_size = sizeof(buffer);
    ASSERT_EQ(GetHakkaString(value, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "value1");

    // Clean up
    HakkaRelease(&array);
    HakkaRelease(&value);
}

TEST(HakkaJsonArrayTest, RemoveValueArray)
{
    // load a list of json string to array object
    const char json_str[] = R"(["value1", "value2", "value3", "value4", "value5"])";
    HakkaHandle array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &array, 10), HAKKA_JSON_SUCCESS);

    // remove the first occurrence of "value3" in the array
    HakkaHandle value;
    ASSERT_EQ(CreateHakkaString(&value, reinterpret_cast<const uint8_t *>("value3"), 6), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(RemoveValueHakkaArray(array, value), HAKKA_JSON_SUCCESS);

    // check the size of the array
    uint32_t size = UINT32_MAX;
    ASSERT_EQ(GetHakkaArraySize(array, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 4);

    // Clean up
    HakkaRelease(&array);
    HakkaRelease(&value);
}

TEST(HakkaJsonArrayTest, ReverseArray)
{
    // load a list of json string to array object
    const char json_str[] = R"(["value1", "value2", "value3", "value4", "value5"])";
    HakkaHandle array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &array, 10), HAKKA_JSON_SUCCESS);

    // reverse the array
    ASSERT_EQ(ReverseHakkaArray(array), HAKKA_JSON_SUCCESS);

    // check the size of the array
    uint32_t size = UINT32_MAX;
    ASSERT_EQ(GetHakkaArraySize(array, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 5);

    // check the value of the first element
    HakkaHandle value;
    ASSERT_EQ(GetHakkaArrayObject(array, 0, &value), HAKKA_JSON_SUCCESS);

    uint8_t buffer[256] = {0};
    uint32_t buffer_size = sizeof(buffer);
    ASSERT_EQ(GetHakkaString(value, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "value5");

    // Clean up
    HakkaRelease(&array);
    HakkaRelease(&value);
}

class HakkaJsonArrayIterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // load a list of json string to array object
        const char json_str[] = R"(["value1", "value2", "value3", "value4", "value5"])";
        ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &array_, 10), HAKKA_JSON_SUCCESS);
        ASSERT_EQ(CreateHakkaArrayIterBegin(array_, &iter), HAKKA_JSON_SUCCESS);
    }

    void TearDown() override
    {
        HakkaArrayIterRelease(&iter);
        HakkaRelease(&array_);
    }

    HakkaHandle array_;
    HakkaObjectIter iter;
};

TEST_F(HakkaJsonArrayIterTest, IterateArray)
{
    // loop through the array, make sure the values are correct
    for (uint32_t i = 0; i < 5; ++i)
    {
        HakkaHandle value = 0;
        ASSERT_EQ(GetHakkaArrayIterDeref(iter, &value), HAKKA_JSON_SUCCESS);
        ASSERT_NE(value, 0);

        uint8_t buffer[256] = {0};
        uint32_t buffer_size = sizeof(buffer);
        ASSERT_EQ(GetHakkaString(value, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
        EXPECT_EQ(std::string(reinterpret_cast<const char *>(buffer)), "value" + std::to_string(i + 1));

        ASSERT_EQ(MoveHakkaArrayIterNext(iter), HAKKA_JSON_SUCCESS);

        HakkaRelease(&value);
    }
}

// extern_c HakkaJsonResultEnum CreateHakkaArrayIterRBegin(HakkaHandle array, HakkaArrayIter *iter);
// reverse Iter array with CreateHakkaArrayIterRBegin
class HakkaJsonArrayIterRBeginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // load a list of json string to array object
        const char json_str[] = R"(["value1", "value2", "value3", "value4", "value5"])";
        ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &array_, 10), HAKKA_JSON_SUCCESS);
        ASSERT_EQ(CreateHakkaArrayIterRBegin(array_, &iter), HAKKA_JSON_SUCCESS);
    }

    void TearDown() override
    {
        HakkaArrayIterRelease(&iter);
        HakkaRelease(&array_);
    }

    HakkaHandle array_;
    HakkaObjectIter iter;
};

TEST_F(HakkaJsonArrayIterRBeginTest, IterateArray)
{
    // loop through the array in reverse order, make sure the values are correct
    for (uint32_t i = 4; i < 5; --i)
    {
        HakkaHandle value = 0;
        ASSERT_EQ(GetHakkaArrayIterDeref(iter, &value), HAKKA_JSON_SUCCESS);
        ASSERT_NE(value, 0);

        uint8_t buffer[256] = {0};
        uint32_t buffer_size = sizeof(buffer);
        ASSERT_EQ(GetHakkaString(value, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
        EXPECT_EQ(std::string(reinterpret_cast<const char *>(buffer)), "value" + std::to_string(i + 1));

        ASSERT_EQ(MoveHakkaArrayIterNext(iter), HAKKA_JSON_SUCCESS);

        HakkaRelease(&value);
    }

    // GetHakkaArrayIterDeref should return HAKKA_JSON_ITERATOR_END
    HakkaHandle value = 0;
    ASSERT_EQ(GetHakkaArrayIterDeref(iter, &value), HAKKA_JSON_ITERATOR_END);
}
