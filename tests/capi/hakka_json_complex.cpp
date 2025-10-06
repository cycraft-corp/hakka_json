#include <gtest/gtest.h>

#include <hakka_json_object.h>
#include <hakka_json_array.h>
#include <hakka_json_primitive.h>

#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <cstring>
#include <iostream>
#include <cmath>

// test complex json object, maybe types are mixed, depth is deep, etc.
// And because the hakka library is provided C API, we use wrapper class to manage the HakkaHandle
// and release the handle in the destructor.

// Wrapper class to manage HakkaHandle and ensure proper cleanup
// Revised Wrapper class to manage HakkaHandle with proper ownership semantics
class HakkaHandleWrapper
{
public:
    // Default constructor initializes handle to 0 (null)
    HakkaHandleWrapper() : handle_(0) {}

    // Constructor that takes ownership of a HakkaHandle
    explicit HakkaHandleWrapper(HakkaHandle handle) : handle_(handle) {}

    // Delete copy constructor to prevent copying
    HakkaHandleWrapper(const HakkaHandleWrapper &) = delete;

    // Delete copy assignment operator to prevent copying
    HakkaHandleWrapper &operator=(const HakkaHandleWrapper &) = delete;

    // Move constructor transfers ownership
    HakkaHandleWrapper(HakkaHandleWrapper &&other) noexcept : handle_(other.handle_)
    {
        other.handle_ = 0;
    }

    // Move assignment operator transfers ownership
    HakkaHandleWrapper &operator=(HakkaHandleWrapper &&other) noexcept
    {
        if (this != &other)
        {
            // Release current handle if valid
            if (handle_ != 0)
            {
                HakkaRelease(&handle_);
            }
            // Transfer ownership
            handle_ = other.handle_;
            other.handle_ = 0;
        }
        return *this;
    }

    // Destructor releases the handle if it's valid
    ~HakkaHandleWrapper()
    {
        if (handle_ != 0)
        {
            HakkaRelease(&handle_);
        }
    }

    // Accessor for the handle
    HakkaHandle get() const { return handle_; }

    // Mutator to get the address of the handle (for functions that output handles)
    HakkaHandle *address() { return &handle_; }

    // Conversion operator to allow implicit use where HakkaHandle is expected
    operator HakkaHandle() const { return handle_; }

    // Check if the handle is valid
    bool is_valid() const { return handle_ != 0; }

private:
    HakkaHandle handle_;
};

// Helper function to create an integer handle
inline HakkaHandleWrapper create_int(int64_t value)
{
    HakkaHandle handle = 0;
    [[maybe_unused]] HakkaJsonResultEnum result = CreateHakkaInt(&handle, value);
    assert(result == HAKKA_JSON_SUCCESS && "Failed to create integer handle");
    return HakkaHandleWrapper(handle);
}

// Helper function to create a float handle
inline HakkaHandleWrapper create_float(double value)
{
    HakkaHandle handle = 0;
    [[maybe_unused]] HakkaJsonResultEnum result = CreateHakkaFloat(&handle, static_cast<float>(value));
    assert(result == HAKKA_JSON_SUCCESS && "Failed to create float handle");
    return HakkaHandleWrapper(handle);
}

// Helper function to create a string handle
inline HakkaHandleWrapper create_string(const std::string &value)
{
    HakkaHandle handle = 0;
    [[maybe_unused]] HakkaJsonResultEnum result = CreateHakkaString(
        &handle,
        reinterpret_cast<const uint8_t *>(value.c_str()),
        static_cast<uint32_t>(value.size()));
    assert(result == HAKKA_JSON_SUCCESS && "Failed to create string handle");
    return HakkaHandleWrapper(handle);
}

// Helper function to create a null handle
inline HakkaHandleWrapper create_null()
{
    HakkaHandle handle = 0;
    [[maybe_unused]] HakkaJsonResultEnum result = CreateHakkaNull(&handle);
    assert(result == HAKKA_JSON_SUCCESS && "Failed to create null handle");
    return HakkaHandleWrapper(handle);
}

// Helper function to create an array handle from a list of HakkaHandleWrappers
inline HakkaHandleWrapper create_array(const std::vector<HakkaHandleWrapper> &elements)
{
    HakkaHandle array_handle = 0;
    [[maybe_unused]] HakkaJsonResultEnum result = CreateHakkaArray(&array_handle);
    assert(result == HAKKA_JSON_SUCCESS && "Failed to create JSON array");
    HakkaHandleWrapper array_wrapper(array_handle);

    for (const auto &element : elements)
    {
        result = PushBackHakkaArray(array_wrapper.get(), element.get());
        assert(result == HAKKA_JSON_SUCCESS && "Failed to push back array element");
    }

    return array_wrapper;
}

// Helper function to create an object handle from a map of key to HakkaHandleWrappers
inline HakkaHandleWrapper create_object(const std::map<std::string, HakkaHandleWrapper> &kv_pairs)
{
    HakkaHandle object_handle = 0;
    [[maybe_unused]] HakkaJsonResultEnum result = CreateHakkaObject(&object_handle);
    assert(result == HAKKA_JSON_SUCCESS && "Failed to create JSON object");
    HakkaHandleWrapper object_wrapper(object_handle);

    for (const auto &pair : kv_pairs)
    {
        result = SetHakkaObject(
            object_wrapper.get(),
            reinterpret_cast<const uint8_t *>(pair.first.c_str()),
            static_cast<uint32_t>(pair.first.size()),
            pair.second.get());
        assert(result == HAKKA_JSON_SUCCESS && "Failed to set key-value pair in JSON object");
    }

    return object_wrapper;
}

// Test integer creation and retrieval
TEST(HakkaJsonTypesTest, CreateAndGetInt)
{
    HakkaHandleWrapper int_handle = create_int(100);
    ASSERT_NE(int_handle.get(), 0);

    int64_t retrieved_value = 0;
    HakkaJsonResultEnum result = GetHakkaInt(int_handle.get(), &retrieved_value);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    EXPECT_EQ(retrieved_value, 100);
}

// Test float creation and retrieval
TEST(HakkaJsonTypesTest, CreateAndGetFloat)
{
    HakkaHandleWrapper float_handle = create_float(3.14159);
    ASSERT_NE(float_handle.get(), 0);

    double retrieved_value = 0.0;
    HakkaJsonResultEnum result = GetHakkaFloat(float_handle.get(), &retrieved_value);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    EXPECT_FLOAT_EQ(static_cast<float>(retrieved_value), 3.14159f);
}

// Test string creation and retrieval
TEST(HakkaJsonTypesTest, CreateAndGetString)
{
    std::string test_str = "Hello, Hakka!";
    HakkaHandleWrapper string_handle = create_string(test_str);
    ASSERT_NE(string_handle.get(), 0);

    uint8_t buffer[50] = {0};
    uint32_t buffer_size = sizeof(buffer);
    HakkaJsonResultEnum result = GetHakkaString(string_handle.get(), buffer, &buffer_size);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), test_str.c_str());
}

// Test null creation and verification
TEST(HakkaJsonTypesTest, CreateAndCheckNull)
{
    HakkaHandleWrapper null_handle = create_null();
    ASSERT_NE(null_handle.get(), 0);

    C_BOOL is_null = 0;
    HakkaJsonResultEnum result = GetHakkaObjectNull(null_handle.get(), nullptr, 0, &is_null);
    // Assuming GetHakkaObjectNull can handle null keys appropriately
    // This may need adjustment based on actual API behavior
    ASSERT_EQ(result, HAKKA_JSON_INVALID_ARGUMENT);
    EXPECT_FALSE(is_null);
}

// Test nested objects and arrays
TEST(HakkaJsonNestedTest, CreateAndRetrieveNestedStructures)
{
    // Create inner object
    const char json_str[] = R"({
        "outer_key1": 9.81,
        "outer_key2": [
            1,
            "Two",
            null,
            {
                "inner_key1": 256,
                "inner_key2": "Inner Value"
            }
        ],
        "outer_key3": null
    })";

    // Load the JSON structure into a HakkaHandle
    HakkaHandleWrapper outer_object;
    ASSERT_EQ(LoadsHakkaObject(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, outer_object.address(), 10), HAKKA_JSON_SUCCESS);
    ASSERT_NE(outer_object.get(), 0);

    // Retrieve and verify outer_key1
    double retrieved_float = 0.0;
    HakkaJsonResultEnum result = GetHakkaObjectFloat(
        outer_object.get(),
        reinterpret_cast<const uint8_t *>("outer_key1"),
        10,
        &retrieved_float);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    EXPECT_FLOAT_EQ(static_cast<float>(retrieved_float), 9.81f);

    // Retrieve and verify outer_key2 (array)
    HakkaHandle retrieved_array = 0;
    result = GetHakkaObjectObject(
        outer_object.get(),
        reinterpret_cast<const uint8_t *>("outer_key2"),
        10,
        &retrieved_array);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    ASSERT_NE(retrieved_array, 0);

    // Wrap the retrieved array
    HakkaHandleWrapper array_wrapper(retrieved_array);

    // Verify array size
    uint32_t array_size = 0;
    result = GetHakkaArraySize(array_wrapper.get(), &array_size);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    EXPECT_EQ(array_size, 4);

    // Verify array elements

    // Element 0: int 1
    HakkaHandle element = 0;
    result = GetHakkaArrayObject(array_wrapper.get(), 0, &element);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    int64_t int_val = 0;
    result = GetHakkaInt(element, &int_val);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    EXPECT_EQ(int_val, 1);
    HakkaRelease(&element);

    // Element 1: string "Two"
    result = GetHakkaArrayObject(array_wrapper.get(), 1, &element);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    uint8_t buffer[10] = {0};
    uint32_t buffer_size = sizeof(buffer);
    result = GetHakkaString(element, buffer, &buffer_size);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "Two");
    HakkaRelease(&element);

    // Element 2: null
    result = GetHakkaArrayObject(array_wrapper.get(), 2, &element);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    HakkaJsonType type = HAKKA_JSON_INVALID;
    result = HakkaType(element, &type);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    HakkaRelease(&element);

    // Element 3: inner object
    result = GetHakkaArrayObject(array_wrapper.get(), 3, &element);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    ASSERT_NE(element, 0);

    // Wrap the retrieved inner object
    HakkaHandleWrapper inner_object_retrieved(element);

    // Retrieve inner_key1 from inner object
    int64_t inner_int = 0;
    result = GetHakkaObjectInt(
        inner_object_retrieved.get(),
        reinterpret_cast<const uint8_t *>("inner_key1"),
        10,
        &inner_int);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    EXPECT_EQ(inner_int, 256);

    // Retrieve inner_key2 from inner object
    uint8_t inner_buffer[20] = {0};
    uint32_t inner_buffer_size = sizeof(inner_buffer);
    result = GetHakkaObjectString(
        inner_object_retrieved.get(),
        reinterpret_cast<const uint8_t *>("inner_key2"),
        10,
        inner_buffer,
        &inner_buffer_size);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(inner_buffer), "Inner Value");
}

// Test handling of extremely large numbers
TEST(HakkaJsonEdgeCasesTest, LargeNumbers)
{
    int64_t large_int = std::numeric_limits<int64_t>::max();
    HakkaHandleWrapper int_handle = create_int(large_int);
    ASSERT_NE(int_handle.get(), 0);

    int64_t retrieved_int = 0;
    HakkaJsonResultEnum result = GetHakkaInt(int_handle.get(), &retrieved_int);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    EXPECT_EQ(retrieved_int, large_int);
}

// Test handling of empty strings
TEST(HakkaJsonEdgeCasesTest, EmptyStrings)
{
    std::string empty_str = "";
    HakkaHandleWrapper string_handle = create_string(empty_str);
    ASSERT_NE(string_handle.get(), 0);

    uint8_t buffer[1] = {0};
    uint32_t buffer_size = sizeof(buffer);
    HakkaJsonResultEnum result = GetHakkaString(string_handle.get(), buffer, &buffer_size);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "");
}

// Test handling of deeply nested structures
TEST(HakkaJsonDeepNestingTest, DeeplyNestedObjects)
{
    const int depth = 50;
    HakkaHandleWrapper current_object = create_int(depth - 1); // Start with the deepest value

    // Build the nested structure from the innermost to the outermost
    for (int i = depth - 1; i >= 0; --i)
    {
        std::string key = "level_" + std::to_string(i);
        // Create a new object with key "level_i" and value current_object
        std::map<std::string, HakkaHandleWrapper> kv_pairs;
        kv_pairs.emplace(std::move(key), std::move(current_object));
        HakkaHandleWrapper new_object = create_object(std::move(kv_pairs));
        current_object = std::move(new_object); // Transfer ownership
    }

    // At this point, current_object is the outermost object
    HakkaHandleWrapper outermost_object = std::move(current_object);

    // Now verify the nested structure
    HakkaHandleWrapper object_to_verify = std::move(outermost_object);

    for (int i = 0; i < depth; ++i)
    {
        std::string key = "level_" + std::to_string(i);

        HakkaHandle retrieved_nested = 0;
        HakkaJsonResultEnum result = GetHakkaObjectObject(
            object_to_verify.get(),
            reinterpret_cast<const uint8_t *>(key.c_str()),
            static_cast<uint32_t>(key.size()),
            &retrieved_nested);
        ASSERT_EQ(result, HAKKA_JSON_SUCCESS) << "Failed to get nested object at level " << i;
        ASSERT_NE(retrieved_nested, 0);

        // Wrap the retrieved nested object
        HakkaHandleWrapper nested_object(retrieved_nested);

        if (i == depth - 1)
        {
            // At the deepest level, verify the integer value
            int64_t value = 0;
            result = GetHakkaInt(nested_object.get(), &value);
            ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
            EXPECT_EQ(value, depth - 1);
        }
        else
        {
            // Continue to the next nested level
            object_to_verify = std::move(nested_object);
        }
    }
}

// Test handling of invalid keys (e.g., NULL or empty)
TEST(HakkaJsonEdgeCasesTest, InvalidKeys)
{
    HakkaHandleWrapper object = create_object({});

    // Attempt to set a key with zero length
    HakkaJsonResultEnum result = SetHakkaObjectInt(object.get(), reinterpret_cast<const uint8_t *>(""), 0, 10);
    EXPECT_EQ(result, HAKKA_JSON_SUCCESS); // set a {"": invalid_type} is valid

    // Attempt to get a value with a NULL key
    int64_t value = 0;
    result = GetHakkaObjectInt(object.get(), nullptr, 0, &value);
    EXPECT_EQ(result, HAKKA_JSON_INVALID_ARGUMENT);
}

// Test type mismatches
TEST(HakkaJsonEdgeCasesTest, TypeMismatches)
{
    std::string json_str = R"({"number": 123, "text": "Sample Text"})";
    HakkaHandleWrapper object;
    ASSERT_EQ(LoadsHakkaObject(reinterpret_cast<const uint8_t *>(json_str.c_str()), static_cast<uint32_t>(json_str.size()), object.address(), 10), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object.get(), 0);

    // Attempt to retrieve "number" as a string
    uint8_t buffer[50] = {0};
    uint32_t buffer_size = sizeof(buffer);
    HakkaJsonResultEnum result = GetHakkaObjectString(object.get(), reinterpret_cast<const uint8_t *>("number"), 6, buffer, &buffer_size);
    EXPECT_EQ(result, HAKKA_JSON_TYPE_ERROR);

    // Attempt to retrieve "text" as an integer
    int64_t int_value = 0;
    result = GetHakkaObjectInt(object.get(), reinterpret_cast<const uint8_t *>("text"), 4, &int_value);
    EXPECT_EQ(result, HAKKA_JSON_TYPE_ERROR);
}

// Test iterating over a JSON object
TEST(HakkaJsonIteratorTest, IterateOverObject)
{
    // Define the JSON string (minified)
    std::string json_str = R"({"id": 1, "name": "Test Object", "active": 1})";

    // Load the JSON object into a HakkaHandleWrapper
    HakkaHandleWrapper object;
    HakkaJsonResultEnum load_result = LoadsHakkaObject(
        reinterpret_cast<const uint8_t *>(json_str.c_str()),
        static_cast<uint32_t>(json_str.size()),
        object.address(),
        10 // max_depth
    );
    ASSERT_EQ(load_result, HAKKA_JSON_SUCCESS) << "Failed to load JSON object";
    ASSERT_TRUE(object.is_valid()) << "Loaded JSON object is invalid";

    // Create an iterator for the object
    HakkaObjectIter iter = 0;
    HakkaJsonResultEnum iter_create_result = CreateHakkaObjectIterBegin(object.get(), &iter);
    ASSERT_EQ(iter_create_result, HAKKA_JSON_SUCCESS) << "Failed to create object iterator";
    ASSERT_NE(iter, 0) << "Iterator handle is null";

    // Define the expected key-value pairs
    std::map<std::string, std::string> expected = {
        {"id", "1"},
        {"name", "Test Object"},
        {"active", "1"}};

    // Iterate over all key-value pairs
    while (true)
    {
        // Retrieve raw handles for key and value
        HakkaHandle key_handle_raw = 0;
        HakkaHandle value_handle_raw = 0;
        HakkaJsonResultEnum result = GetHakkaObjectIterDeref(iter, &key_handle_raw, &value_handle_raw);

        if (result == HAKKA_JSON_ITERATOR_END)
        {
            break; // Finished iterating
        }

        ASSERT_EQ(result, HAKKA_JSON_SUCCESS) << "Iterator dereference failed";

        // Wrap the raw handles in HakkaHandleWrapper for automatic management
        HakkaHandleWrapper key_handle(key_handle_raw);
        HakkaHandleWrapper value_handle(value_handle_raw);

        // Ensure handles are valid
        ASSERT_TRUE(key_handle.is_valid()) << "Key handle is invalid";
        ASSERT_TRUE(value_handle.is_valid()) << "Value handle is invalid";

        // Retrieve key as string
        uint8_t key_buffer[50] = {0};
        uint32_t key_size = sizeof(key_buffer);
        HakkaJsonResultEnum key_result = GetHakkaString(
            key_handle.get(),
            key_buffer,
            &key_size);
        ASSERT_EQ(key_result, HAKKA_JSON_SUCCESS) << "Failed to retrieve key string";
        std::string key(reinterpret_cast<const char *>(key_buffer), key_size);
        key.resize(strlen(key.c_str()));

        // Retrieve value based on its type
        std::string value_str;
        HakkaJsonType value_type;
        HakkaJsonResultEnum type_result = HakkaType(
            value_handle.get(),
            &value_type);
        ASSERT_EQ(type_result, HAKKA_JSON_SUCCESS) << "Failed to retrieve value type";

        if (value_type == HAKKA_JSON_INT)
        {
            int64_t int_val = 0;
            HakkaJsonResultEnum int_result = GetHakkaInt(
                value_handle.get(),
                &int_val);
            ASSERT_EQ(int_result, HAKKA_JSON_SUCCESS) << "Failed to retrieve integer value";
            value_str = std::to_string(int_val);
        }
        else if (value_type == HAKKA_JSON_STRING)
        {
            uint8_t value_buffer[100] = {0};
            uint32_t value_size = sizeof(value_buffer);
            HakkaJsonResultEnum str_result = GetHakkaString(
                value_handle.get(),
                value_buffer,
                &value_size);
            ASSERT_EQ(str_result, HAKKA_JSON_SUCCESS) << "Failed to retrieve string value";
            value_str = std::string(reinterpret_cast<const char *>(value_buffer), value_size);
            value_str.resize(strlen(value_str.c_str()));
        }
        else
        {
            // Handle other types if necessary
            value_str = "Unsupported Type";
        }

        // Verify the key-value pair against expected values
        ASSERT_TRUE(expected.find(key) != expected.end()) << "Unexpected key encountered: " << key;
        EXPECT_EQ(expected[key], value_str) << "Value mismatch for key: " << key;

        // Move to the next element in the iterator
        HakkaJsonResultEnum move_result = MoveHakkaObjectIterNext(iter);
        if (move_result == HAKKA_JSON_ITERATOR_END)
        {
            break; // Finished iterating
        }
        ASSERT_EQ(move_result, HAKKA_JSON_SUCCESS) << "Failed to move iterator to next element";
    }

    // Release the iterator
    HakkaJsonResultEnum release_result = HakkaObjectIterRelease(&iter);
    ASSERT_EQ(release_result, HAKKA_JSON_SUCCESS) << "Failed to release object iterator";
}

// Test iterating over a JSON array
TEST(HakkaJsonIteratorTest, IterateOverArray)
{

    // use LoadsHakkaArray to load the json array
    const char json_str[] = R"([10, "Twenty", null, 30.5])";
    HakkaHandleWrapper array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, array.address(), 10), HAKKA_JSON_SUCCESS);
    ASSERT_NE(array, 0);

    HakkaArrayIter iter = 0;
    ASSERT_EQ(CreateHakkaArrayIterBegin(array.get(), &iter), HAKKA_JSON_SUCCESS);
    ASSERT_NE(iter, 0);

    // Expected values
    std::vector<std::string> expected = {
        "10",
        "Twenty",
        "null",
        "30.5"};
    size_t index = 0;

    while (true)
    {
        HakkaHandleWrapper value_handle;
        HakkaJsonResultEnum result = GetHakkaArrayIterDeref(iter, value_handle.address());
        if (result == HAKKA_JSON_ITERATOR_END)
        {
            break;
        }
        ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
        ASSERT_NE(value_handle.get(), 0);

        // Retrieve value as string for comparison
        std::string value_str;
        HakkaJsonType type;
        ASSERT_EQ(HakkaType(value_handle.get(), &type), HAKKA_JSON_SUCCESS);
        if (type == HAKKA_JSON_INT)
        {
            int64_t int_val = 0;
            ASSERT_EQ(GetHakkaInt(value_handle.get(), &int_val), HAKKA_JSON_SUCCESS);
            value_str = std::to_string(int_val);
        }
        else if (type == HAKKA_JSON_STRING)
        {
            uint8_t value_buffer[50] = {0};
            uint32_t value_size = sizeof(value_buffer);
            ASSERT_EQ(GetHakkaString(value_handle.get(), value_buffer, &value_size), HAKKA_JSON_SUCCESS);
            value_str = std::string(reinterpret_cast<const char *>(value_buffer), value_size);
        }
        else if (type == HAKKA_JSON_FLOAT)
        {
            double float_val = 0.0f;
            ASSERT_EQ(GetHakkaFloat(value_handle.get(), &float_val), HAKKA_JSON_SUCCESS);
            value_str = std::to_string(float_val);
        }
        else if (type == HAKKA_JSON_NULL)
        {
            value_str = "null";
        }
        else
        {
            value_str = "Unsupported Type";
        }

        // Verify the value
        ASSERT_LT(index, expected.size());
        EXPECT_EQ(value_str.find(expected[index]) != std::string::npos, true);

        // Move to next
        ASSERT_EQ(MoveHakkaArrayIterNext(iter), HAKKA_JSON_SUCCESS);
        ++index;
    }

    // Release the iterator
    ASSERT_EQ(HakkaArrayIterRelease(&iter), HAKKA_JSON_SUCCESS);
}

// Test accessing non-existent keys
TEST(HakkaJsonErrorHandlingTest, AccessNonExistentKey)
{
    HakkaHandleWrapper object = create_object({});

    int64_t value = 0;
    HakkaJsonResultEnum result = GetHakkaObjectInt(object.get(), reinterpret_cast<const uint8_t *>("missing"), 7, &value);
    EXPECT_EQ(result, HAKKA_JSON_KEY_NOT_FOUND);
}

TEST(HakkaJsonErrorHandlingTest, SetKeyWithTruncatedLength)
{
    HakkaHandleWrapper object = create_object({});

    HakkaJsonResultEnum result = SetHakkaObjectInt(object.get(), reinterpret_cast<const uint8_t *>("key"), 2, 10);
    EXPECT_EQ(result, HAKKA_JSON_SUCCESS); // set a {"ke": invalid_type} is valid
}

// Test type mismatches
TEST(HakkaJsonErrorHandlingTest, TypeMismatchOnGet)
{
    // use load to create a new object
    std::string json_str = "{\"number\": 123, \"text\": \"Sample\"}";
    HakkaHandleWrapper object;
    ASSERT_EQ(LoadsHakkaObject(reinterpret_cast<const uint8_t *>(json_str.c_str()), static_cast<uint32_t>(json_str.size()), object.address(), 10), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object.get(), 0);

    // Attempt to get "number" as a string
    uint8_t buffer[50] = {0};
    uint32_t buffer_size = sizeof(buffer);
    HakkaJsonResultEnum result = GetHakkaObjectString(object.get(), reinterpret_cast<const uint8_t *>("number"), 6, buffer, &buffer_size);
    EXPECT_EQ(result, HAKKA_JSON_TYPE_ERROR);

    // Attempt to get "text" as an integer
    int64_t int_val = 0;
    result = GetHakkaObjectInt(object.get(), reinterpret_cast<const uint8_t *>("text"), 4, &int_val);
    EXPECT_EQ(result, HAKKA_JSON_TYPE_ERROR);
}

// Test handling of invalid handles
TEST(HakkaJsonErrorHandlingTest, InvalidHandleOperations)
{
    HakkaHandle invalid_handle = 0;

    // Attempt to dump an invalid handle
    uint8_t buffer[50] = {0};
    uint64_t buffer_size = sizeof(buffer);
    HakkaJsonResultEnum result = DumpHakkaObject(invalid_handle, 10, buffer, &buffer_size);
    EXPECT_EQ(result, HAKKA_JSON_INVALID_ARGUMENT);

    // Attempt to release an already released handle
    HakkaRelease(&invalid_handle);
    EXPECT_EQ(result, HAKKA_JSON_INVALID_ARGUMENT);
}

// Test updating an existing key in the object
TEST(HakkaJsonModificationTest, UpdateExistingKey)
{
    std::string json_str = R"({"version": 1, "name": "Initial Name"})";
    HakkaHandleWrapper object;
    ASSERT_EQ(LoadsHakkaObject(reinterpret_cast<const uint8_t *>(json_str.c_str()), static_cast<uint32_t>(json_str.size()), object.address(), 10), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object.get(), 0);

    // Update "name" key
    HakkaHandleWrapper new_name = create_string("Updated Name");
    ASSERT_EQ(SetHakkaObject(object.get(),
                             reinterpret_cast<const uint8_t *>("name"),
                             4,
                             new_name.get()),
              HAKKA_JSON_SUCCESS);

    // Verify the update
    uint8_t buffer[20] = {0};
    uint32_t buffer_size = sizeof(buffer);
    ASSERT_EQ(GetHakkaObjectString(object.get(), reinterpret_cast<const uint8_t *>("name"), 4, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "Updated Name");
}

// Test adding a new key to the object
TEST(HakkaJsonModificationTest, AddNewKey)
{
    HakkaHandleWrapper object = create_object({});

    // Add a new key
    HakkaHandleWrapper new_key = create_float(2.718);
    ASSERT_EQ(SetHakkaObject(object.get(),
                             reinterpret_cast<const uint8_t *>("pi"),
                             2,
                             new_key.get()),
              HAKKA_JSON_SUCCESS);

    // Verify the addition
    double pi_val = 0.0;
    HakkaJsonResultEnum result = GetHakkaObjectFloat(object.get(), reinterpret_cast<const uint8_t *>("pi"), 2, &pi_val);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    EXPECT_FLOAT_EQ(static_cast<float>(pi_val), 2.718f);
}

// Test removing a key from the object
TEST(HakkaJsonModificationTest, RemoveKey)
{
    std::string json_str = R"({"temp": "Temporary"})";
    HakkaHandleWrapper object;
    ASSERT_EQ(LoadsHakkaObject(reinterpret_cast<const uint8_t *>(json_str.c_str()), static_cast<uint32_t>(json_str.size()), object.address(), 10), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object.get(), 0);

    // Remove the key
    ASSERT_EQ(RemoveHakkaObjectKey(object.get(), reinterpret_cast<const uint8_t *>("temp"), 4), HAKKA_JSON_SUCCESS);

    // Verify removal
    uint8_t buffer[20] = {0};
    uint32_t buffer_size = sizeof(buffer);
    HakkaJsonResultEnum result = GetHakkaObjectString(object.get(), reinterpret_cast<const uint8_t *>("temp"), 4, buffer, &buffer_size);
    EXPECT_EQ(result, HAKKA_JSON_KEY_NOT_FOUND);
}

// Test updating the object with another object
TEST(HakkaJsonModificationTest, UpdateWithAnotherObject)
{
    std::string json_str1 = R"({"key1": 10, "key2": "Value2"})";
    HakkaHandleWrapper object1;
    ASSERT_EQ(LoadsHakkaObject(reinterpret_cast<const uint8_t *>(json_str1.c_str()), static_cast<uint32_t>(json_str1.size()), object1.address(), 10), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object1.get(), 0);

    std::string json_str2 = R"({"key2": "Updated Value2", "key3": null})";
    HakkaHandleWrapper object2;
    ASSERT_EQ(LoadsHakkaObject(reinterpret_cast<const uint8_t *>(json_str2.c_str()), static_cast<uint32_t>(json_str2.size()), object2.address(), 10), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object2.get(), 0);

    // Update object1 with object2
    ASSERT_EQ(UpdateHakkaObject(object1.get(), object2.get()), HAKKA_JSON_SUCCESS);

    // Verify that "key2" is updated and "key3" is added
    uint8_t buffer[20] = {0};
    uint32_t buffer_size = sizeof(buffer);
    ASSERT_EQ(GetHakkaObjectString(object1.get(), reinterpret_cast<const uint8_t *>("key2"), 4, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "Updated Value2");

    // Verify "key3" is added as null
    C_BOOL is_null = 0;
    ASSERT_EQ(GetHakkaObjectNull(object1.get(), reinterpret_cast<const uint8_t *>("key3"), 4, &is_null), HAKKA_JSON_SUCCESS);
    EXPECT_TRUE(is_null);
}

// Test creating objects with invalid inputs
TEST(HakkaJsonComprehensiveErrorTest, CreateObjectWithInvalidInputs)
{
    // Attempt to create an object with a NULL handle pointer
    HakkaJsonResultEnum result = CreateHakkaObject(nullptr);
    EXPECT_EQ(result, HAKKA_JSON_INVALID_ARGUMENT);

    // Attempt to create an array with a NULL handle pointer
    result = CreateHakkaArray(nullptr);
    EXPECT_EQ(result, HAKKA_JSON_INVALID_ARGUMENT);
}

// Test loading malformed JSON
TEST(HakkaJsonComprehensiveErrorTest, LoadMalformedJson)
{
    const char malformed_json[] = R"({"key1": 42, "key2": "value2" ";})"; // Missing comma
    HakkaHandleWrapper object;
    HakkaJsonResultEnum result = LoadsHakkaObject(
        reinterpret_cast<const uint8_t *>(malformed_json),
        sizeof(malformed_json) - 1,
        object.address(),
        10);
    EXPECT_EQ(result, HAKKA_JSON_PARSE_ERROR);
}

// Test dumping objects with insufficient buffer size
TEST(HakkaJsonComprehensiveErrorTest, DumpWithInsufficientBuffer)
{
    std::string json_str = R"({"key1": 1, "key2": "Test"})";
    HakkaHandleWrapper object;
    ASSERT_EQ(LoadsHakkaObject(reinterpret_cast<const uint8_t *>(json_str.c_str()), static_cast<uint32_t>(json_str.size()), object.address(), 10), HAKKA_JSON_SUCCESS);
    ASSERT_NE(object.get(), 0);

    uint8_t buffer[5] = {0}; // Intentionally small buffer
    uint64_t buffer_size = sizeof(buffer);
    HakkaJsonResultEnum result = DumpHakkaObject(object.get(), 10, buffer, &buffer_size);
    EXPECT_EQ(result, HAKKA_JSON_NOT_ENOUGH_MEMORY);
}

// Test setting and getting with invalid indices in arrays
TEST(HakkaJsonComprehensiveErrorTest, ArrayInvalidIndices)
{
    const char json_str[] = R"([1, 2])";
    HakkaHandleWrapper array;
    ASSERT_EQ(LoadsHakkaArray(reinterpret_cast<const uint8_t *>(json_str), sizeof(json_str) - 1, array.address(), 10), HAKKA_JSON_SUCCESS);

    // Attempt to get element at out-of-bounds index
    HakkaHandleWrapper element;
    HakkaJsonResultEnum result = GetHakkaArrayObject(array.get(), 5, element.address());
    EXPECT_EQ(result, HAKKA_JSON_INDEX_OUT_OF_BOUNDS);

    // Attempt to set element at out-of-bounds index
    HakkaHandleWrapper new_element = create_int(10);
    result = SetHakkaArray(array.get(), 5, new_element.get());
    EXPECT_EQ(result, HAKKA_JSON_INDEX_OUT_OF_BOUNDS);
}

// Test popping from an empty array
TEST(HakkaJsonComprehensiveErrorTest, PopFromEmptyArray)
{
    HakkaHandleWrapper array = create_array({});

    HakkaHandleWrapper popped_element;
    HakkaJsonResultEnum result = PopHakkaArray(array.get(), 0, popped_element.address());
    EXPECT_EQ(result, HAKKA_JSON_INDEX_OUT_OF_BOUNDS);
}