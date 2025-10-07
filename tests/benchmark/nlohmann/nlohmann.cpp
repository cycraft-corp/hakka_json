#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using json = nlohmann::json;

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

    std::vector<json> json_array;
    std::string line;
    while (std::getline(input_file, line))
    {
        try
        {
            json obj = json::parse(line);
            json_array.push_back(std::move(obj));
        }
        catch (const json::exception &e)
        {
            std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
            continue;
        }
    }

    // Measure RSS after loading
    long rss_after = get_rss_kb();

    if (rss_before >= 0 && rss_after >= 0)
    {
        long rss_diff = rss_after - rss_before;
        std::cout << "nlohmann_json RSS: " << rss_diff << " KB" << std::endl;
    }
    else
    {
        std::cerr << "RSS measurement not available on this platform" << std::endl;
    }

    return 0;
}
