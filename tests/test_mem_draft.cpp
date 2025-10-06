#include <hakka_json_object.hpp>

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

using namespace hakka;

template <typename T>
const T *get_compact_ptr(const JsonHandleCompact &handle)
{
    try {
        auto view = handle.get_view();
        return std::get<const T*>(view);
    } catch (...) {
        return nullptr;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::ifstream input_file(argv[1]);
    if (!input_file.is_open())
    {
        std::cerr << "Failed to open the file: " << argv[1] << std::endl;
        return 1;
    }

    auto array_handle = JsonArrayCompact::create();
    const JsonArrayCompact *array = get_compact_ptr<JsonArrayCompact>(array_handle);
    if (!array)
    {
        std::cerr << "Failed to create JsonArrayCompact" << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(input_file, line))
    {
        auto sub_object = JsonObjectCompact::loads(line, 512);
        if (!sub_object)
        {
            std::cerr << "Failed to load JSON object: " << line << std::endl;
            continue;
        }

        // Get mutable access to the array for modification
        auto array_mut_ptr = array_handle.get_mut_ptr();
        auto *array_mut = std::get<JsonArrayCompact*>(array_mut_ptr);
        if (array_mut)
        {
            array_mut->push_back(std::move(sub_object.value()));
        }
    }

    std::cout << "Loaded JSON objects." << std::endl;

    // // dump the array to to stdout
    // auto dump_result = array->dump(512);
    // if (!dump_result)
    // {
    //     std::cerr << "Failed to dump JSON array." << std::endl;
    //     return 1;
    // }
    // std::cout << dump_result.value() << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(5));

    return 0;
}
