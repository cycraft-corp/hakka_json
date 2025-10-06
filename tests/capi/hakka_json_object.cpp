#include <gtest/gtest.h>

#include <hakka_json_object.h>
#include <hakka_json_array.h>
#include <hakka_json_primitive.h>

#include <cstring>
#include <string>

TEST(HakkaJsonObjectTest, CreateAndDumpEmptyObject)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    uint8_t buffer[256] = {0};
    uint64_t buffer_size = sizeof(buffer);
    ASSERT_EQ(DumpHakkaObject(object, 10, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "{}");

    // Clean up
    HakkaRelease(&object);
}

TEST(HakkaJsonObjectTest, LoadAndDumpObject)
{
    const char json_str[] = R"({"key1": 42, "key2": "value2"})";
    HakkaHandle object;
    ASSERT_EQ(LoadsHakkaObject(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, &object, 10), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    uint8_t buffer[256] = {0};
    uint64_t buffer_size = sizeof(buffer);
    ASSERT_EQ(DumpHakkaObject(object, 10, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), json_str);

    // Clean up
    HakkaRelease(&object);
}

TEST(HakkaJsonObjectTest, SetAndGetInt)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    const char key_str[] = "age";
    int64_t value_set = 30;

    ASSERT_EQ(SetHakkaObjectInt(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, value_set), HAKKA_JSON_SUCCESS);

    int64_t value_get = 0;
    ASSERT_EQ(GetHakkaObjectInt(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, &value_get), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(value_get, value_set);

    // Clean up
    HakkaRelease(&object);
}

TEST(HakkaJsonObjectTest, SetAndGetFloat)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    const char key_str[] = "pi";
    double value_set = 3.14159;

    ASSERT_EQ(SetHakkaObjectFloat(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, value_set), HAKKA_JSON_SUCCESS);

    double value_get = 0.0;
    ASSERT_EQ(GetHakkaObjectFloat(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, &value_get), HAKKA_JSON_SUCCESS);
    EXPECT_FLOAT_EQ(static_cast<float>(value_get), static_cast<float>(value_set));

    // Clean up
    HakkaRelease(&object);
}

TEST(HakkaJsonObjectTest, SetAndGetString)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    const char key_str[] = "name";
    const char value_set_str[] = "John Doe";

    ASSERT_EQ(SetHakkaObjectString(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, reinterpret_cast<const uint8_t *>(value_set_str), sizeof(value_set_str) - 1), HAKKA_JSON_SUCCESS);

    uint8_t buffer[256] = {0};
    uint32_t buffer_size = sizeof(buffer);
    ASSERT_EQ(GetHakkaObjectString(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), value_set_str);

    // Clean up
    HakkaRelease(&object);
}

TEST(HakkaJsonObjectTest, SetAndGetObject)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    const char key_str[] = "address";
    HakkaHandle value_set;
    ASSERT_EQ(CreateHakkaObject(&value_set), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(SetHakkaObject(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, value_set), HAKKA_JSON_SUCCESS);

    HakkaHandle value_get = 0;
    ASSERT_EQ(GetHakkaObjectObject(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, &value_get), HAKKA_JSON_SUCCESS);
    EXPECT_NE(value_get, 0);

    // Clean up
    HakkaRelease(&object);
    HakkaRelease(&value_set);
    HakkaRelease(&value_get);
}

TEST(HakkaJsonObjectTest, SetAndRemoveKey)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    const char key_str[] = "age";
    int64_t value_set = 30;

    ASSERT_EQ(SetHakkaObjectInt(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, value_set), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(RemoveHakkaObjectKey(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1), HAKKA_JSON_SUCCESS);

    C_BOOL result = 0;
    ASSERT_EQ(GetHakkaObjectNull(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, &result), HAKKA_JSON_KEY_NOT_FOUND);
    EXPECT_FALSE(result);

    // Clean up
    HakkaRelease(&object);
}

TEST(HakkaJsonObjectTest, SetAndGetNull)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    const char key_str[] = "null_value";

    ASSERT_EQ(SetHakkaObjectNull(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1), HAKKA_JSON_SUCCESS);

    C_BOOL is_null = 0;
    ASSERT_EQ(GetHakkaObjectNull(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, &is_null), HAKKA_JSON_SUCCESS);
    EXPECT_TRUE(is_null);

    // Clean up
    HakkaRelease(&object);
}

TEST(HakkaJsonObjectTest, GetSize)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    const char key_str[] = "age";
    int64_t value_set = 30;

    ASSERT_EQ(SetHakkaObjectInt(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, value_set), HAKKA_JSON_SUCCESS);

    uint32_t size = 0;
    ASSERT_EQ(GetHakkaObjectSize(object, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 1);

    // Clean up
    HakkaRelease(&object);
}

TEST(HakkaJsonObjectTest, ContainsKey)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    const char key_str[] = "age";
    int64_t value_set = 30;

    ASSERT_EQ(SetHakkaObjectInt(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, value_set), HAKKA_JSON_SUCCESS);

    C_BOOL result = 0;
    ASSERT_EQ(ContainsHakkaObjectKey(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, &result), HAKKA_JSON_SUCCESS);
    EXPECT_TRUE(result);

    // Clean up
    HakkaRelease(&object);
}

TEST(HakkaJsonObjectTest, GetKeys)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    const char key_str1[] = "age";
    int64_t value_set1 = 30;
    ASSERT_EQ(SetHakkaObjectInt(object, reinterpret_cast<const uint8_t *>(key_str1), sizeof(key_str1) - 1, value_set1), HAKKA_JSON_SUCCESS);

    const char key_str2[] = "name";
    const char value_set_str[] = "John Doe";
    ASSERT_EQ(SetHakkaObjectString(object, reinterpret_cast<const uint8_t *>(key_str2), sizeof(key_str2) - 1, reinterpret_cast<const uint8_t *>(value_set_str), sizeof(value_set_str) - 1), HAKKA_JSON_SUCCESS);

    HakkaHandle keys = 0;
    ASSERT_EQ(GetHakkaObjectKeys(object, &keys), HAKKA_JSON_SUCCESS);
    ASSERT_NE(keys, 0);

    uint32_t size = 0;
    ASSERT_EQ(GetHakkaArraySize(keys, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 2);

    // Clean up
    HakkaRelease(&object);
    HakkaRelease(&keys);
}

TEST(HakkaJsonObjectTest, GetValues)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    const char key_str1[] = "age";
    int64_t value_set1 = 30;
    ASSERT_EQ(SetHakkaObjectInt(object, reinterpret_cast<const uint8_t *>(key_str1), sizeof(key_str1) - 1, value_set1), HAKKA_JSON_SUCCESS);

    const char key_str2[] = "name";
    const char value_set_str[] = "John Doe";
    ASSERT_EQ(SetHakkaObjectString(object, reinterpret_cast<const uint8_t *>(key_str2), sizeof(key_str2) - 1, reinterpret_cast<const uint8_t *>(value_set_str), sizeof(value_set_str) - 1), HAKKA_JSON_SUCCESS);

    HakkaHandle values = 0;
    ASSERT_EQ(GetHakkaObjectValues(object, &values), HAKKA_JSON_SUCCESS);
    ASSERT_NE(values, 0);

    uint32_t size = 0;
    ASSERT_EQ(GetHakkaArraySize(values, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 2);

    // Clean up
    HakkaRelease(&object);
    HakkaRelease(&values);
}

TEST(HakkaJsonObjectTest, CreateObjectFromKeys)
{
    const char keys[] = R"(["age", "name"])";
    HakkaHandle keys_array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(keys), sizeof(keys) - 1, &keys_array, 10), HAKKA_JSON_SUCCESS);

    HakkaHandle default_value;
    ASSERT_EQ(CreateHakkaInt(&default_value, 0), HAKKA_JSON_SUCCESS);

    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObjectFromKeys(keys_array, default_value, &object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    // Clean up
    HakkaRelease(&keys_array);
    HakkaRelease(&default_value);
    HakkaRelease(&object);
}

TEST(HakkaJsonObjectTest, PopKey)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    const char key_str[] = "age";
    int64_t value_set = 30;
    ASSERT_EQ(SetHakkaObjectInt(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, value_set), HAKKA_JSON_SUCCESS);

    HakkaHandle value = 0;
    ASSERT_EQ(PopHakkaObject(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, &value), HAKKA_JSON_SUCCESS);
    ASSERT_NE(value, 0);

    C_BOOL is_null = 0;
    ASSERT_EQ(GetHakkaObjectNull(object, reinterpret_cast<const uint8_t *>(key_str), sizeof(key_str) - 1, &is_null), HAKKA_JSON_KEY_NOT_FOUND);
    EXPECT_FALSE(is_null);

    // Clean up
    HakkaRelease(&object);
    HakkaRelease(&value);
}

TEST(HakkaJsonObjectTest, PopItem)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    const char key_str1[] = "age";
    int64_t value_set1 = 30;
    ASSERT_EQ(SetHakkaObjectInt(object, reinterpret_cast<const uint8_t *>(key_str1), sizeof(key_str1) - 1, value_set1), HAKKA_JSON_SUCCESS);

    const char key_str2[] = "name";
    const char value_set_str[] = "John Doe";
    ASSERT_EQ(SetHakkaObjectString(object, reinterpret_cast<const uint8_t *>(key_str2), sizeof(key_str2) - 1, reinterpret_cast<const uint8_t *>(value_set_str), sizeof(value_set_str) - 1), HAKKA_JSON_SUCCESS);

    HakkaHandle key = 0;
    HakkaHandle value = 0;
    ASSERT_EQ(PopItemHakkaObject(object, &key, &value), HAKKA_JSON_SUCCESS); // default pop last
    ASSERT_NE(key, 0);
    ASSERT_NE(value, 0);

    int64_t value_check = 0;
    ASSERT_EQ(GetHakkaObjectInt(object, reinterpret_cast<const uint8_t *>(key_str2), sizeof(key_str2) - 1, &value_check), HAKKA_JSON_KEY_NOT_FOUND);
    ASSERT_EQ(value_check, 0);

    // Clean up
    HakkaRelease(&object);
    HakkaRelease(&key);
    HakkaRelease(&value);
}

TEST(HakkaJsonObjectTest, ClearObject)
{
    HakkaHandle object;
    ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object, 0);

    const char key_str1[] = "age";
    int64_t value_set1 = 30;
    ASSERT_EQ(SetHakkaObjectInt(object, reinterpret_cast<const uint8_t *>(key_str1), sizeof(key_str1) - 1, value_set1), HAKKA_JSON_SUCCESS);

    const char key_str2[] = "name";
    const char value_set_str[] = "John Doe";
    ASSERT_EQ(SetHakkaObjectString(object, reinterpret_cast<const uint8_t *>(key_str2), sizeof(key_str2) - 1, reinterpret_cast<const uint8_t *>(value_set_str), sizeof(value_set_str) - 1), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(ClearHakkaObject(object), HAKKA_JSON_SUCCESS);

    C_BOOL is_null = 0;
    ASSERT_EQ(GetHakkaObjectNull(object, reinterpret_cast<const uint8_t *>(key_str1), sizeof(key_str1) - 1, &is_null), HAKKA_JSON_KEY_NOT_FOUND);
    EXPECT_FALSE(is_null);

    // Clean up
    HakkaRelease(&object);
}

TEST(HakkaJsonObjectTest, UpdateObject)
{
    HakkaHandle object1;
    ASSERT_EQ(CreateHakkaObject(&object1), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object1, 0);

    const char key_str1[] = "age";
    int64_t value_set1 = 30;
    ASSERT_EQ(SetHakkaObjectInt(object1, reinterpret_cast<const uint8_t *>(key_str1), sizeof(key_str1) - 1, value_set1), HAKKA_JSON_SUCCESS);

    const char key_str2[] = "name";
    const char value_set_str[] = "John Doe";
    ASSERT_EQ(SetHakkaObjectString(object1, reinterpret_cast<const uint8_t *>(key_str2), sizeof(key_str2) - 1, reinterpret_cast<const uint8_t *>(value_set_str), sizeof(value_set_str) - 1), HAKKA_JSON_SUCCESS);

    HakkaHandle object2;
    ASSERT_EQ(CreateHakkaObject(&object2), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object2, 0);

    const char key_str3[] = "city";
    const char value_set_str2[] = "New York";
    ASSERT_EQ(SetHakkaObjectString(object2, reinterpret_cast<const uint8_t *>(key_str3), sizeof(key_str3) - 1, reinterpret_cast<const uint8_t *>(value_set_str2), sizeof(value_set_str2) - 1), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(UpdateHakkaObject(object1, object2), HAKKA_JSON_SUCCESS);

    uint32_t size = 0;
    ASSERT_EQ(GetHakkaObjectSize(object1, &size), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(size, 3);

    // Clean up
    HakkaRelease(&object1);
    HakkaRelease(&object2);
}

class HakkaJsonObjectIterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_EQ(CreateHakkaObject(&object), HAKKA_JSON_SUCCESS);
        ASSERT_NE(object, 0);

        const char key_str1[] = "age";
        int64_t value_set1 = 30;
        ASSERT_EQ(SetHakkaObjectInt(object, reinterpret_cast<const uint8_t *>(key_str1), sizeof(key_str1) - 1, value_set1), HAKKA_JSON_SUCCESS);

        const char key_str2[] = "name";
        const char value_set_str[] = "John Doe";
        ASSERT_EQ(SetHakkaObjectString(object, reinterpret_cast<const uint8_t *>(key_str2), sizeof(key_str2) - 1, reinterpret_cast<const uint8_t *>(value_set_str), sizeof(value_set_str) - 1), HAKKA_JSON_SUCCESS);

        const char key_str3[] = "city";
        const char value_set_str2[] = "New York";
        ASSERT_EQ(SetHakkaObjectString(object, reinterpret_cast<const uint8_t *>(key_str3), sizeof(key_str3) - 1, reinterpret_cast<const uint8_t *>(value_set_str2), sizeof(value_set_str2) - 1), HAKKA_JSON_SUCCESS);

        ASSERT_EQ(CreateHakkaObjectIterBegin(object, &iter), HAKKA_JSON_SUCCESS);
    }

    void TearDown() override
    {
        HakkaObjectIterRelease(&iter);
        HakkaRelease(&object);
    }

    HakkaHandle object;
    HakkaObjectIter iter;
};

TEST_F(HakkaJsonObjectIterTest, IterateObject)
{
    {
        HakkaHandle key = 0;
        HakkaHandle value = 0;

        ASSERT_EQ(GetHakkaObjectIterDeref(iter, &key, &value), HAKKA_JSON_SUCCESS);
        ASSERT_NE(key, 0);
        ASSERT_NE(value, 0);

        int64_t value_check = 0;
        ASSERT_EQ(GetHakkaInt(value, &value_check), HAKKA_JSON_SUCCESS);
        EXPECT_EQ(value_check, 30);

        // Clean up
        HakkaRelease(&key);
        HakkaRelease(&value);
    }

    ASSERT_EQ(MoveHakkaObjectIterNext(iter), HAKKA_JSON_SUCCESS);

    {
        HakkaHandle key = 0;
        HakkaHandle value = 0;
        ASSERT_EQ(GetHakkaObjectIterDeref(iter, &key, &value), HAKKA_JSON_SUCCESS);
        ASSERT_NE(key, 0);
        ASSERT_NE(value, 0);

        uint8_t buffer[256] = {0};
        uint32_t buffer_size = sizeof(buffer);
        ASSERT_EQ(GetHakkaString(value, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
        EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "John Doe");

        // Clean up
        HakkaRelease(&key);
        HakkaRelease(&value);
    }

    ASSERT_EQ(MoveHakkaObjectIterNext(iter), HAKKA_JSON_SUCCESS);

    {
        HakkaHandle key = 0;
        HakkaHandle value = 0;

        ASSERT_EQ(GetHakkaObjectIterDeref(iter, &key, &value), HAKKA_JSON_SUCCESS);
        ASSERT_NE(key, 0);
        ASSERT_NE(value, 0);

        uint8_t buffer[256] = {0};
        uint32_t buffer_size = sizeof(buffer);
        ASSERT_EQ(GetHakkaString(value, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
        EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "New York");

        // Clean up
        HakkaRelease(&key);
        HakkaRelease(&value);
    }

    ASSERT_EQ(MoveHakkaObjectIterNext(iter), HAKKA_JSON_ITERATOR_END);
}
