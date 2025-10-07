#include <hakka_json_object.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

using namespace hakka;

// Get RSS (Resident Set Size) in kilobytes from /proc/self/status
long get_rss_kb()
{
#ifdef __linux__
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line))
    {
        if (line.substr(0, 6) == "VmRSS:")
        {
            // Extract the number from "VmRSS:    12345 kB"
            size_t start = line.find_first_of("0123456789");
            size_t end = line.find(" kB");
            if (start != std::string::npos && end != std::string::npos)
            {
                return std::stol(line.substr(start, end - start));
            }
        }
    }
#endif
    return -1; // Not available on non-Linux systems
}

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

    // Measure RSS before loading
    long rss_before = get_rss_kb();

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

    // Measure RSS after loading
    long rss_after = get_rss_kb();

    if (rss_before >= 0 && rss_after >= 0)
    {
        long rss_diff = rss_after - rss_before;
        std::cout << "hakka_json RSS: " << rss_diff << " KB" << std::endl;
    }
    else
    {
        std::cerr << "RSS measurement not available on this platform" << std::endl;
    }

    return 0;
}
