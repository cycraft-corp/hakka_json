#include <hakka_json_object.hpp>
#include <hakka_json_string.hpp>
#include <hakka_json_int.hpp>
#include <hakka_json_float.hpp>

#include <gtest/gtest.h>

using namespace hakka;

// Helper function to extract compact types from variant
namespace detail
{
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
}

TEST(JsonObject, Create)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);
    ASSERT_EQ(json_object.get_type(), HakkaJsonType::HAKKA_JSON_OBJECT);
    ASSERT_EQ(detail::h2t<JsonObjectCompact>(json_object)->dump(512).value(), "{}");
}

TEST(JsonObject, SetAndGet)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonStringCompact::create("value2");

    // Set key-value pairs
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key2", json_value2), HAKKA_JSON_SUCCESS);

    // Get values by key
    auto result1 = detail::h2t_mut<JsonObjectCompact>(json_object)->get("key1");
    ASSERT_TRUE(result1);
    ASSERT_EQ(result1.value().get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(result1.value())->get().value()), 42);

    auto result2 = detail::h2t_mut<JsonObjectCompact>(json_object)->get("key2");
    ASSERT_TRUE(result2);
    ASSERT_EQ(result2.value().get_type(), HakkaJsonType::HAKKA_JSON_STRING);
    ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(result2.value())->get().value()), "value2");
}

TEST(JsonObject, Remove)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    auto json_value = JsonIntCompact::create(42);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key", json_value), HAKKA_JSON_SUCCESS);

    // Remove key
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->remove("key"), HAKKA_JSON_SUCCESS);

    // Verify removal
    auto result = detail::h2t_mut<JsonObjectCompact>(json_object)->get("key");
    ASSERT_FALSE(result);
    ASSERT_EQ(result.error(), HAKKA_JSON_KEY_NOT_FOUND);
}

TEST(JsonObject, Dump)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonStringCompact::create("value2");

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key2", json_value2), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(detail::h2t<JsonObjectCompact>(json_object)->dump(512).value(), "{\"key1\": 42, \"key2\": \"value2\"}");
}

TEST(JsonObject, DumpSize)
{
    {
        auto json_object = JsonObjectCompact::create();
        ASSERT_TRUE(json_object);

        auto json_value1 = JsonIntCompact::create(42);
        auto json_value2 = JsonStringCompact::create("value2");

        ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
        ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key2", json_value2), HAKKA_JSON_SUCCESS);

        ASSERT_EQ(detail::h2t<JsonObjectCompact>(json_object)->dump_size(), sizeof("{\"key1\": 42, \"key2\": \"value2\"}") - 1);
    }
    // test nested object
    {
        auto json_object = JsonObjectCompact::create();
        ASSERT_TRUE(json_object);

        auto json_value1 = JsonIntCompact::create(42);
        auto json_value2 = JsonStringCompact::create("value2");

        auto nested_object = JsonObjectCompact::create();
        ASSERT_TRUE(nested_object);
        ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(nested_object)->set("nested_key1", json_value1), HAKKA_JSON_SUCCESS);
        ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(nested_object)->set("nested_key2", json_value2), HAKKA_JSON_SUCCESS);

        ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key", nested_object), HAKKA_JSON_SUCCESS);

        ASSERT_EQ(detail::h2t<JsonObjectCompact>(json_object)->dump_size(), sizeof("{\"key\": {\"nested_key1\": 42, \"nested_key2\": \"value2\"}}") - 1);
    }
    // test empty object
    {
        auto json_object = JsonObjectCompact::create();
        ASSERT_TRUE(json_object);

        ASSERT_EQ(detail::h2t<JsonObjectCompact>(json_object)->dump_size(), sizeof("{}") - 1);
    }
}

TEST(JsonObject, ToBytes)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    auto json_value = JsonIntCompact::create(42);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key", json_value), HAKKA_JSON_SUCCESS);

    char buffer[512];
    uint32_t buffer_size = sizeof(buffer);
    auto result = detail::h2t<JsonObjectCompact>(json_object)->to_bytes(buffer, &buffer_size);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);

    ASSERT_STREQ(buffer, "{\"key\": 42}");
    ASSERT_EQ(buffer_size, static_cast<uint32_t>(strlen("{\"key\": 42}")));
}

TEST(JsonObject, InvalidToBytes)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    auto json_value = JsonIntCompact::create(42);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key", json_value), HAKKA_JSON_SUCCESS);

    char buffer[8]; // Insufficient buffer size
    uint32_t buffer_size = sizeof(buffer);
    auto result = detail::h2t<JsonObjectCompact>(json_object)->to_bytes(buffer, &buffer_size);
    ASSERT_EQ(result, HAKKA_JSON_NOT_ENOUGH_MEMORY);
}

TEST(JsonObject, KeysAndValues)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonStringCompact::create("value2");

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key2", json_value2), HAKKA_JSON_SUCCESS);

    const auto &keys = detail::h2t_mut<JsonObjectCompact>(json_object)->keys();
    const auto &values = detail::h2t_mut<JsonObjectCompact>(json_object)->values();

    ASSERT_EQ(keys.length(), 2);
    ASSERT_EQ(values.length(), 2);

    ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(keys.at(0).value())->get().value()), "key1");
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(values.at(0).value())->get().value()), 42);

    ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(keys.at(1).value())->get().value()), "key2");
    ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(values.at(1).value())->get().value()), "value2");
}

TEST(JsonObject, Contains)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    auto json_value = JsonIntCompact::create(42);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key", json_value), HAKKA_JSON_SUCCESS);

    ASSERT_TRUE(detail::h2t_mut<JsonObjectCompact>(json_object)->contains("key"));
    ASSERT_FALSE(detail::h2t_mut<JsonObjectCompact>(json_object)->contains("missing_key"));
}

TEST(JsonObject, Pop)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    auto json_value = JsonIntCompact::create(42);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key", json_value), HAKKA_JSON_SUCCESS);

    auto result = detail::h2t_mut<JsonObjectCompact>(json_object)->pop("key");
    ASSERT_TRUE(result);
    ASSERT_EQ(result.value().get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(result.value())->get().value()), 42);

    ASSERT_FALSE(detail::h2t_mut<JsonObjectCompact>(json_object)->contains("key"));
}

TEST(JsonObject, PopItem)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonStringCompact::create("value2");

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key2", json_value2), HAKKA_JSON_SUCCESS);

    auto pop_result = detail::h2t_mut<JsonObjectCompact>(json_object)->popitem();
    ASSERT_TRUE(pop_result);
    ASSERT_EQ(std::get<std::string>(pop_result.value().first), std::string("key2"));
    ASSERT_EQ(pop_result.value().second.get_type(), HakkaJsonType::HAKKA_JSON_STRING);
    ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(pop_result.value().second)->get().value()), "value2");

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->length(), 1);
    ASSERT_FALSE(detail::h2t_mut<JsonObjectCompact>(json_object)->contains("key2"));
    ASSERT_TRUE(detail::h2t_mut<JsonObjectCompact>(json_object)->contains("key1"));
}

TEST(JsonObject, Clear)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonStringCompact::create("value2");

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key2", json_value2), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->clear(), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->length(), 0);
    ASSERT_EQ(detail::h2t<JsonObjectCompact>(json_object)->dump(512).value(), "{}");
}

TEST(JsonObject, Update)
{
    auto json_object1 = JsonObjectCompact::create();
    auto json_object2 = JsonObjectCompact::create();

    ASSERT_TRUE(json_object1);
    ASSERT_TRUE(json_object2);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonStringCompact::create("value2");

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("key2", json_value2), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->update(*detail::h2t_mut<JsonObjectCompact>(json_object2)), HAKKA_JSON_SUCCESS);

    ASSERT_TRUE(detail::h2t_mut<JsonObjectCompact>(json_object1)->contains("key1"));
    ASSERT_TRUE(detail::h2t_mut<JsonObjectCompact>(json_object1)->contains("key2"));
}

TEST(JsonObject, FromKeys)
{
    std::vector<KeyType> keys = {std::string("key1"), std::string("key2"), std::string("key3")};
    auto default_value = JsonIntCompact::create(0);

    auto fromkeys_result = JsonObjectCompact::fromkeys(keys, default_value);
    ASSERT_TRUE(fromkeys_result);
    auto json_object = detail::h2t_mut<JsonObjectCompact>(fromkeys_result.value());
    ASSERT_TRUE(json_object);

    ASSERT_EQ(json_object->length(), 3);
    for (const auto &key : keys)
    {
        ASSERT_TRUE(json_object->contains(key));
        auto value = json_object->get(key);
        ASSERT_TRUE(value);
        ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(value.value())->get().value()), 0);
    }
}

TEST(JsonObject, SetDefault)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    // Key does not exist, setdefault should add it with the default value
    auto default_value = JsonIntCompact::create(100);
    auto result = detail::h2t_mut<JsonObjectCompact>(json_object)->setdefault("key", default_value);
    ASSERT_TRUE(result);
    ASSERT_EQ(result.value().get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(result.value())->get().value()), 100);
    ASSERT_TRUE(detail::h2t_mut<JsonObjectCompact>(json_object)->contains("key"));

    // Key exists, setdefault should return the existing value
    auto existing_value = detail::h2t_mut<JsonObjectCompact>(json_object)->setdefault("key", JsonIntCompact::create(200));
    ASSERT_TRUE(existing_value);
    ASSERT_EQ(existing_value.value().get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(existing_value.value())->get().value()), 100);
}

TEST(JsonObject, Compare)
{
    auto json_object1 = JsonObjectCompact::create();
    auto json_object2 = JsonObjectCompact::create();
    auto json_object3 = JsonObjectCompact::create();

    ASSERT_TRUE(json_object1);
    ASSERT_TRUE(json_object2);
    ASSERT_TRUE(json_object3);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonStringCompact::create("value2");
    auto json_value3 = JsonIntCompact::create(100);

    // Populate json_object1 and json_object2 identically
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key2", json_value2), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("key2", json_value2), HAKKA_JSON_SUCCESS);

    // Populate json_object3 differently
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object3)->set("key1", json_value3), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object3)->set("key2", json_value2), HAKKA_JSON_SUCCESS);

    // Compare json_object1 and json_object2 (should be equal)
    auto compare_result = detail::h2t<JsonObjectCompact>(json_object1)->compare(json_object2);
    ASSERT_TRUE(compare_result);
    ASSERT_EQ(*compare_result, 0);

    // Compare json_object1 and json_object3 (json_object1 < json_object3)
    compare_result = detail::h2t<JsonObjectCompact>(json_object1)->compare(json_object3);
    ASSERT_TRUE(compare_result);
    ASSERT_LT(*compare_result, 0);

    // Compare json_object3 and json_object1 (json_object3 > json_object1)
    compare_result = detail::h2t<JsonObjectCompact>(json_object3)->compare(json_object1);
    ASSERT_TRUE(compare_result);
    ASSERT_GT(*compare_result, 0);
}

TEST(JsonObject, Insert)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    auto json_value1 = JsonIntCompact::create(1);
    auto json_value2 = JsonIntCompact::create(2);
    auto json_value3 = JsonIntCompact::create(3);

    // Initially, object is empty
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->length(), 0);

    // Insert key-value pair "key1":1 at index 0
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->insert(std::string("key1"), json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->length(), 1);
    ASSERT_TRUE(detail::h2t_mut<JsonObjectCompact>(json_object)->contains("key1"));
    auto result = detail::h2t_mut<JsonObjectCompact>(json_object)->get("key1");
    ASSERT_TRUE(result);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(result.value())->get().value()), 1);

    // Insert key-value pair "key2":2 at index 1
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->insert(std::string("key2"), json_value2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->length(), 2);
    ASSERT_TRUE(detail::h2t_mut<JsonObjectCompact>(json_object)->contains("key2"));
    result = detail::h2t_mut<JsonObjectCompact>(json_object)->get("key2");
    ASSERT_TRUE(result);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(result.value())->get().value()), 2);

    // Insert key-value pair "key3":3 at index 1 (middle)
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->insert(std::string("key3"), json_value3), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->length(), 3);
    ASSERT_TRUE(detail::h2t_mut<JsonObjectCompact>(json_object)->contains("key3"));
    result = detail::h2t_mut<JsonObjectCompact>(json_object)->get("key3");
    ASSERT_TRUE(result);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(result.value())->get().value()), 3);
}

TEST(JsonObject, UpdateWithExistingKeys)
{
    auto json_object1 = JsonObjectCompact::create();
    auto json_object2 = JsonObjectCompact::create();

    ASSERT_TRUE(json_object1);
    ASSERT_TRUE(json_object2);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonStringCompact::create("value2");
    auto json_value3 = JsonIntCompact::create(100);

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key2", json_value2), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("key2", json_value3), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("key3", json_value2), HAKKA_JSON_SUCCESS);

    // Update json_object1 with json_object2
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->update(*detail::h2t_mut<JsonObjectCompact>(json_object2)), HAKKA_JSON_SUCCESS);

    // key1 should remain unchanged
    auto result1 = detail::h2t_mut<JsonObjectCompact>(json_object1)->get("key1");
    ASSERT_TRUE(result1);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(result1.value())->get().value()), 42);

    // key2 should be updated to 100
    auto result2 = detail::h2t_mut<JsonObjectCompact>(json_object1)->get("key2");
    ASSERT_TRUE(result2);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(result2.value())->get().value()), 100);

    // key3 should be added
    auto result3 = detail::h2t_mut<JsonObjectCompact>(json_object1)->get("key3");
    ASSERT_TRUE(result3);
    ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(result3.value())->get().value()), "value2");
}

TEST(JsonObject, CompareDifferentSizes)
{
    auto json_object1 = JsonObjectCompact::create();
    auto json_object2 = JsonObjectCompact::create();

    ASSERT_TRUE(json_object1);
    ASSERT_TRUE(json_object2);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonStringCompact::create("value2");

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key2", json_value2), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("key1", json_value1), HAKKA_JSON_SUCCESS);

    // Compare json_object1 (2 keys) with json_object2 (1 key)
    auto compare_result = detail::h2t<JsonObjectCompact>(json_object1)->compare(json_object2);
    ASSERT_TRUE(compare_result);
    ASSERT_GT(*compare_result, 0);

    // Compare json_object2 (1 key) with json_object1 (2 keys)
    compare_result = detail::h2t<JsonObjectCompact>(json_object2)->compare(json_object1);
    ASSERT_TRUE(compare_result);
    ASSERT_LT(*compare_result, 0);
}

TEST(JsonObject, CompareDifferentKeyOrder)
{
    auto json_object1 = JsonObjectCompact::create();
    auto json_object2 = JsonObjectCompact::create();

    ASSERT_TRUE(json_object1);
    ASSERT_TRUE(json_object2);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonStringCompact::create("value2");

    // Set keys in different orders
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key2", json_value2), HAKKA_JSON_SUCCESS);

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("key2", json_value2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("key1", json_value1), HAKKA_JSON_SUCCESS);

    // Depending on implementation, compare might consider order
    // Based on current compare implementation, it compares based on the order of keys
    auto compare_result = detail::h2t<JsonObjectCompact>(json_object1)->compare(json_object2);
    ASSERT_TRUE(compare_result);
    ASSERT_EQ(*compare_result, 0);
}

// test some deep nesting complex objects
// create a basic object, (which has a key with a value of another object), and so on
TEST(JsonObject, DeepNesting)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    constexpr int depth = 10;
    JsonObjectCompact *current = detail::h2t_mut<JsonObjectCompact>(json_object);

    // Create nested objects
    for (int i = 0; i < depth; ++i)
    {
        auto nested_object = JsonObjectCompact::create();
        ASSERT_EQ(current->set("nested", nested_object), HAKKA_JSON_SUCCESS);
        auto nested_handle = current->get("nested").value();
        current = detail::h2t_mut<JsonObjectCompact>(nested_handle);
        ASSERT_TRUE(current);
    }

    // Set a value at the deepest level
    auto final_value = JsonIntCompact::create(999);
    ASSERT_EQ(current->set("value", final_value), HAKKA_JSON_SUCCESS);

    // Traverse back and verify each level
    current = detail::h2t_mut<JsonObjectCompact>(json_object);
    for (int i = 0; i < depth; ++i)
    {
        auto nested_handle = current->get("nested");
        ASSERT_TRUE(nested_handle);
        current = detail::h2t_mut<JsonObjectCompact>(nested_handle.value());
        ASSERT_TRUE(current);
    }

    // Verify the final value
    auto retrieved_value = current->get("value");
    ASSERT_TRUE(retrieved_value);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(retrieved_value.value())->get().value()), 999);

    // dump it and print to stdout
    auto dump_result = detail::h2t<JsonObjectCompact>(json_object)->dump(512);
    ASSERT_TRUE(dump_result);
    std::cout << dump_result.value() << std::endl;
}

// test the load function
TEST(JsonObject, Load)
{
    std::string json_str = R"({
            "key1": 42,
            "key2": "value2",
            "key3": {
            "nestedKey1": [1, 2, 3],
            "nestedKey2": {
                "deepKey": "deepValue"
            }
            },
            "key4": true,
            "key5": null
        })";

    std::string dump_outed;
    {
        auto load_result = JsonObjectCompact::loads(json_str, 512);
        ASSERT_TRUE(load_result);

        //
        auto json_object = detail::h2t_mut<JsonObjectCompact>(load_result.value());
        ASSERT_TRUE(json_object);

        auto result1 = json_object->get("key1");
        ASSERT_TRUE(result1);
        auto tmp = detail::h2t<JsonIntCompact>(result1.value())->get();

        ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(result1.value())->get().value()), 42);

        auto result2 = json_object->get("key2");
        ASSERT_TRUE(result2);
        ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(result2.value())->get().value()), "value2");

        auto result3 = json_object->get("key3");
        ASSERT_TRUE(result3);
        auto nested_object = detail::h2t_mut<JsonObjectCompact>(result3.value());
        ASSERT_TRUE(nested_object);

        auto nested_result1 = nested_object->get("nestedKey1");
        ASSERT_TRUE(nested_result1);
        // Add assertions for nestedKey1 array elements

        auto nested_result2 = nested_object->get("nestedKey2");
        ASSERT_TRUE(nested_result2);
        auto deep_object = detail::h2t_mut<JsonObjectCompact>(nested_result2.value());
        ASSERT_TRUE(deep_object);
        auto deep_result = deep_object->get("deepKey");
        ASSERT_TRUE(deep_result);
        ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(deep_result.value())->get().value()), "deepValue");

        auto result4 = json_object->get("key4");
        ASSERT_TRUE(result4);
        auto view4 = result4.value().get_view();
        ASSERT_EQ(std::get<bool>(reinterpret_cast<const JsonFloatCompact*>(std::get<const JsonBoolCompact*>(view4))->get().value()), true);

        auto result5 = json_object->get("key5");
        ASSERT_TRUE(result5);
        auto view5 = result5.value().get_view();
        ASSERT_EQ(std::get<std::nullptr_t>(reinterpret_cast<const JsonFloatCompact*>(std::get<const JsonNullCompact*>(view5))->get().value()), nullptr);

        dump_outed = detail::h2t<JsonObjectCompact>(load_result.value())->dump(512).value();
    }

    { // dump outed string can be loaded again
        auto load_result = JsonObjectCompact::loads(dump_outed, 512);
        ASSERT_TRUE(load_result);
        auto json_object = detail::h2t_mut<JsonObjectCompact>(load_result.value());
        ASSERT_TRUE(json_object);
        auto dump_outed_again = detail::h2t<JsonObjectCompact>(load_result.value())->dump(512).value();
        ASSERT_EQ(dump_outed, dump_outed_again);
    }
}

// test iterator
TEST(JsonObject, Iterator)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonStringCompact::create("value2");
    auto json_value3 = JsonIntCompact::create(100);

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key2", json_value2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key3", json_value3), HAKKA_JSON_SUCCESS);

    auto it = detail::h2t_mut<JsonObjectCompact>(json_object)->begin();
    auto end = detail::h2t_mut<JsonObjectCompact>(json_object)->end();

    ASSERT_TRUE(it != end);
    ASSERT_EQ(std::get<std::string>(it->first), "key1");
    ASSERT_EQ(it->second.get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(it->second)->get().value()), 42);

    ++it;
    ASSERT_TRUE(it != end);
    ASSERT_EQ(std::get<std::string>(it->first), "key2");
    ASSERT_EQ(it->second.get_type(), HakkaJsonType::HAKKA_JSON_STRING);
    ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(it->second)->get().value()), "value2");

    ++it;
    ASSERT_TRUE(it != end);
    ASSERT_EQ(std::get<std::string>(it->first), "key3");
    ASSERT_EQ(it->second.get_type(), HakkaJsonType::HAKKA_JSON_INT);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(it->second)->get().value()), 100);

    ++it;
    ASSERT_TRUE(it == end);
}

// Test removing from beginning and verify no holes
TEST(JsonObject, RemoveFromBeginningNoHoles)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    // Create dict {"key1": 42, "key2": 43, "key3": 44}
    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonIntCompact::create(43);
    auto json_value3 = JsonIntCompact::create(44);

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key2", json_value2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key3", json_value3), HAKKA_JSON_SUCCESS);

    // Verify initial state - key1 is at index 0
    const auto &keys = detail::h2t_mut<JsonObjectCompact>(json_object)->keys();
    const auto &values = detail::h2t_mut<JsonObjectCompact>(json_object)->values();
    ASSERT_EQ(keys.length(), 3);
    ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(keys.at(0).value())->get().value()), "key1");
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(values.at(0).value())->get().value()), 42);

    // Remove key1 (at index 0)
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->remove("key1"), HAKKA_JSON_SUCCESS);

    // Verify no holes - elements should shift
    ASSERT_EQ(keys.length(), 2);
    ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(keys.at(0).value())->get().value()), "key2");
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(values.at(0).value())->get().value()), 43);
    ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(keys.at(1).value())->get().value()), "key3");
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(values.at(1).value())->get().value()), 44);

    // Verify remaining keys work correctly
    auto result2 = detail::h2t_mut<JsonObjectCompact>(json_object)->get("key2");
    ASSERT_TRUE(result2);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(result2.value())->get().value()), 43);

    auto result3 = detail::h2t_mut<JsonObjectCompact>(json_object)->get("key3");
    ASSERT_TRUE(result3);
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(result3.value())->get().value()), 44);

    // Verify key1 is gone
    auto result1 = detail::h2t_mut<JsonObjectCompact>(json_object)->get("key1");
    ASSERT_FALSE(result1);
    ASSERT_EQ(result1.error(), HAKKA_JSON_KEY_NOT_FOUND);
}

// Test removing from middle and verify no holes
TEST(JsonObject, RemoveFromMiddleNoHoles)
{
    auto json_object = JsonObjectCompact::create();
    ASSERT_TRUE(json_object);

    auto json_value1 = JsonIntCompact::create(42);
    auto json_value2 = JsonIntCompact::create(43);
    auto json_value3 = JsonIntCompact::create(44);

    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key1", json_value1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key2", json_value2), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->set("key3", json_value3), HAKKA_JSON_SUCCESS);

    // Remove key2 (at index 1 - middle)
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object)->remove("key2"), HAKKA_JSON_SUCCESS);

    // Verify no holes - key3 should shift to index 1
    const auto &keys = detail::h2t_mut<JsonObjectCompact>(json_object)->keys();
    const auto &values = detail::h2t_mut<JsonObjectCompact>(json_object)->values();
    ASSERT_EQ(keys.length(), 2);
    ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(keys.at(0).value())->get().value()), "key1");
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(values.at(0).value())->get().value()), 42);
    ASSERT_EQ(std::get<std::string>(detail::h2t<JsonStringCompact>(keys.at(1).value())->get().value()), "key3");
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(values.at(1).value())->get().value()), 44);
}

// Test comparison after removal from beginning
TEST(JsonObject, CompareAfterRemovalFromBeginning)
{
    // Create dict1: {"key1": 42, "key2": 43, "key3": 44}
    auto json_object1 = JsonObjectCompact::create();
    ASSERT_TRUE(json_object1);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key1", JsonIntCompact::create(42)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key2", JsonIntCompact::create(43)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key3", JsonIntCompact::create(44)), HAKKA_JSON_SUCCESS);

    // Remove key1 from dict1 -> {"key2": 43, "key3": 44}
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->remove("key1"), HAKKA_JSON_SUCCESS);

    // Create dict2 directly as {"key2": 43, "key3": 44}
    auto json_object2 = JsonObjectCompact::create();
    ASSERT_TRUE(json_object2);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("key2", JsonIntCompact::create(43)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("key3", JsonIntCompact::create(44)), HAKKA_JSON_SUCCESS);

    // Compare: should be equal because no holes, elements shifted
    auto compare_result = detail::h2t<JsonObjectCompact>(json_object1)->compare(json_object2);
    ASSERT_TRUE(compare_result);
    ASSERT_EQ(*compare_result, 0) << "Dicts should be equal after removal (no holes)";

    // Verify dump output is identical
    auto dump1 = detail::h2t<JsonObjectCompact>(json_object1)->dump(512).value();
    auto dump2 = detail::h2t<JsonObjectCompact>(json_object2)->dump(512).value();
    ASSERT_EQ(dump1, dump2);
}

// Test comparison after removal from middle
TEST(JsonObject, CompareAfterRemovalFromMiddle)
{
    // Create dict1: {"key1": 42, "key2": 43, "key3": 44}
    auto json_object1 = JsonObjectCompact::create();
    ASSERT_TRUE(json_object1);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key1", JsonIntCompact::create(42)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key2", JsonIntCompact::create(43)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("key3", JsonIntCompact::create(44)), HAKKA_JSON_SUCCESS);

    // Remove key2 from dict1 -> {"key1": 42, "key3": 44}
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->remove("key2"), HAKKA_JSON_SUCCESS);

    // Create dict2 directly as {"key1": 42, "key3": 44}
    auto json_object2 = JsonObjectCompact::create();
    ASSERT_TRUE(json_object2);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("key1", JsonIntCompact::create(42)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("key3", JsonIntCompact::create(44)), HAKKA_JSON_SUCCESS);

    // Compare: should be equal
    auto compare_result = detail::h2t<JsonObjectCompact>(json_object1)->compare(json_object2);
    ASSERT_TRUE(compare_result);
    ASSERT_EQ(*compare_result, 0);
}

// Test multiple removals and comparison
TEST(JsonObject, MultipleRemovalsAndComparison)
{
    // Create dict1: {"a": 1, "b": 2, "c": 3, "d": 4, "e": 5}
    auto json_object1 = JsonObjectCompact::create();
    ASSERT_TRUE(json_object1);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("a", JsonIntCompact::create(1)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("b", JsonIntCompact::create(2)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("c", JsonIntCompact::create(3)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("d", JsonIntCompact::create(4)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->set("e", JsonIntCompact::create(5)), HAKKA_JSON_SUCCESS);

    // Remove a, c, e (alternating pattern)
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->remove("a"), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->remove("c"), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->remove("e"), HAKKA_JSON_SUCCESS);

    // Create dict2 directly as {"b": 2, "d": 4}
    auto json_object2 = JsonObjectCompact::create();
    ASSERT_TRUE(json_object2);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("b", JsonIntCompact::create(2)), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object2)->set("d", JsonIntCompact::create(4)), HAKKA_JSON_SUCCESS);

    // Verify length
    ASSERT_EQ(detail::h2t_mut<JsonObjectCompact>(json_object1)->length(), 2);

    // Compare
    auto compare_result = detail::h2t<JsonObjectCompact>(json_object1)->compare(json_object2);
    ASSERT_TRUE(compare_result);
    ASSERT_EQ(*compare_result, 0);

    // Verify iteration works correctly after multiple removals
    auto it = detail::h2t_mut<JsonObjectCompact>(json_object1)->begin();
    auto end = detail::h2t_mut<JsonObjectCompact>(json_object1)->end();

    ASSERT_TRUE(it != end);
    ASSERT_EQ(std::get<std::string>(it->first), "b");
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(it->second)->get().value()), 2);

    ++it;
    ASSERT_TRUE(it != end);
    ASSERT_EQ(std::get<std::string>(it->first), "d");
    ASSERT_EQ(std::get<int64_t>(detail::h2t<JsonIntCompact>(it->second)->get().value()), 4);

    ++it;
    ASSERT_TRUE(it == end);
}
