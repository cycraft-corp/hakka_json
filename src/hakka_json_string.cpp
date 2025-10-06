#define U_DISABLE_RENAMING 1
#include <hakka_json_string.hpp>
#include <hakka_json_array.hpp>
#include <handles/string_manager.hpp>

#include <unicode/unistr.h>
#include <unicode/ustream.h>
#include <unicode/casemap.h>
#include <unicode/schriter.h>
#include <unicode/uchar.h>

#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <uchar.h>
#include <vector>
#include <functional>
#include <string_view>

using namespace hakka;

namespace
{
    namespace detail
    {
        std::string escape_json_string(std::string_view input)
        {
            std::ostringstream ss;
            ss << '"';
            for (const auto &c : input)
            {
                switch (c)
                {
                case '\"':
                    ss << "\\\"";
                    break;
                case '\\':
                    ss << "\\\\";
                    break;
                case '\b':
                    ss << "\\b";
                    break;
                case '\f':
                    ss << "\\f";
                    break;
                case '\n':
                    ss << "\\n";
                    break;
                case '\r':
                    ss << "\\r";
                    break;
                case '\t':
                    ss << "\\t";
                    break;
                default:
                    if ('\x00' <= c && c <= '\x1f')
                    {
                        ss << "\\u"
                           << std::hex << std::uppercase
                           << std::setw(4) << std::setfill('0') << (int)c;
                    }
                    else
                    {
                        ss << c;
                    }
                }
            }
            ss << '"';
            return ss.str();
        }

        icu::UnicodeString to_unicode_string(std::string_view str)
        {
            return icu::UnicodeString::fromUTF8(str);
        }

        std::string from_unicode_string(const icu::UnicodeString &ustr)
        {
            std::string utf8;
            ustr.toUTF8String(utf8);
            return utf8;
        }

        // try_reserve
        template <typename Reservable>
            requires requires(Reservable &r, size_t s) {
                r.reserve(s);
            }
        HakkaJsonResultEnum try_reserve(Reservable &elements, size_t size)
        {
            if (elements.capacity() < size)
            {
                try
                {
                    elements.reserve(size);
                }
                catch (const std::bad_alloc &)
                {
                    return HAKKA_JSON_NOT_ENOUGH_MEMORY;
                }
                catch (...)
                {
                    return HAKKA_JSON_INTERNAL_ERROR;
                }
            }
            return HAKKA_JSON_SUCCESS;
        }

        using TransformFunc = std::function<HakkaJsonResultEnum(const std::string &, std::string &)>;
        tl::expected<JsonHandleCompact, HakkaJsonResultEnum> transform_string_compact(const std::string &from,
                                                                                       TransformFunc &&func)
        {
            std::string result;
            auto res = func(from, result);
            if (res != HAKKA_JSON_SUCCESS)
                return tl::make_unexpected(res);

            return JsonStringCompact::create(result);
        }

        std::string replace_all(std::string_view str, std::string_view old_sub, std::string_view new_sub)
        {
            std::string result;
            if (old_sub.empty())
            {
                // Special case when old_sub is empty: insert new_sub between every character and at the start and end
                // >>> s = 'hello'
                // >>> s.replace('', 'a')
                // 'ahaealalaoa'

                result.reserve(str.size() + (str.size() + 1) * new_sub.size());
                result.append(new_sub);
                for (char ch : str)
                {
                    result.push_back(ch);
                    result.append(new_sub);
                }
            }
            else
            {
                result.reserve(str.size());
                size_t pos = 0;
                while (pos < str.size())
                {
                    size_t found_pos = str.find(old_sub, pos);
                    if (found_pos == std::string_view::npos)
                    {
                        result.append(str.substr(pos));
                        break;
                    }
                    result.append(str.substr(pos, found_pos - pos));
                    result.append(new_sub);
                    pos = found_pos + old_sub.size();
                }
            }
            return result;
        }

        namespace split_helpers
        {
            // Function to check for line breaks and return the length of the line break sequence
            inline size_t is_line_break(const std::string_view &str, size_t pos)
            {
                if (pos >= str.size())
                    return 0;

                char ch = str[pos];

                // Check for CRLF (\r\n) first - must be before single character check
                if (ch == '\r' && (pos + 1) < str.size() && str[pos + 1] == '\n')
                {
                    return 2;
                }

                static constexpr const char single_line_breaks[] = "\n\v\f\r\x1c\x1d\x1e\x85";
                // Check for single-byte line breaks using a lookup
                if (std::memchr(single_line_breaks, ch, sizeof(single_line_breaks) - 1))
                {
                    return 1;
                }

                // Check for Unicode line separators (UTF-8 encoded \u2028 or \u2029)
                if (static_cast<unsigned char>(ch) == 0xE2 && (pos + 2) < str.size())
                {
                    unsigned char ch1 = static_cast<unsigned char>(str[pos + 1]);
                    unsigned char ch2 = static_cast<unsigned char>(str[pos + 2]);
                    if (ch1 == 0x80 && (ch2 == 0xA8 || ch2 == 0xA9))
                    {
                        return 3; // \u2028 or \u2029
                    }
                }

                return 0;
            }

            using StringNextPos = std::size_t;
            using StringStepLen = std::size_t;
            using FindNextResult = std::pair<StringNextPos, StringStepLen>;

            using StringFindNextCallback = std::function<FindNextResult(std::string_view /*target*/, size_t /*current*/)>;
            enum class SplitOrient
            {
                LEFT,
                RIGHT,
            };

            template <SplitOrient Direction>
            std::vector<std::string_view> split(std::string_view source, std::size_t max_split, StringFindNextCallback find_next)
            {
                std::vector<std::string_view> result;
                if (source.empty())
                    return result;

                std::size_t splits_made = 0;

                if constexpr (Direction == SplitOrient::LEFT)
                {
                    std::size_t pos = 0;

                    while (splits_made < max_split)
                    {
                        auto [next, sep_len] = find_next(source, pos);
                        if (next == std::string_view::npos)
                            break;

                        result.emplace_back(source.substr(pos, next - pos));
                        pos = next + sep_len;
                        ++splits_made;
                    }
                    result.emplace_back(source.substr(pos));
                }
                else
                { // Direction == SplitOrient::RIGHT
                    std::size_t pos = source.size();

                    while (splits_made < max_split)
                    {
                        auto [next, sep_len] = find_next(source, pos);
                        if (next == std::string_view::npos)
                            break;

                        result.emplace_back(source.substr(next + sep_len, pos - next - sep_len));
                        pos = next;
                        ++splits_made;
                    }
                    result.emplace_back(source.substr(0, pos));
                    std::reverse(result.begin(), result.end());
                }

                return result;
            }

            // Function to split a string based on a separator from the left or right
            template <SplitOrient Direction>
            std::vector<std::string_view> split_string(
                std::string_view source_str,
                std::string_view separator,
                std::size_t max_split)
            {
                auto find_next = [separator](std::string_view source, std::size_t pos) -> FindNextResult
                {
                    if (Direction == SplitOrient::LEFT)
                    {
                        std::size_t next = source.find(separator, pos);
                        if (next != std::string_view::npos)
                            return {next, separator.size()};
                    }
                    else
                    { // RIGHT
                        if (pos == 0)
                            return {std::string_view::npos, 0};
                        std::size_t next = source.rfind(separator, pos - 1);
                        if (next != std::string_view::npos)
                            return {next, separator.size()};
                    }
                    return {std::string_view::npos, 0};
                };

                return split<Direction>(source_str, max_split, find_next);
            }

            // Function to split a string on whitespace from the left or right
            template <SplitOrient Direction>
            std::vector<std::string_view> split_whitespace(
                std::string_view source_str,
                std::size_t max_split)
            {
                auto find_next = [](std::string_view source, std::size_t pos) -> FindNextResult
                {
                    std::size_t len = source.size();
                    if constexpr (Direction == SplitOrient::LEFT)
                    {
                        // Skip leading whitespace
                        while (pos < len && std::isspace(static_cast<unsigned char>(source[pos])))
                            ++pos;
                        if (pos >= len)
                            return {std::string_view::npos, 0};
                        // Find the end of the word
                        while (pos < len && !std::isspace(static_cast<unsigned char>(source[pos])))
                            ++pos;
                        return {pos, 0};
                    }
                    else
                    { // RIGHT
                        // Skip trailing whitespace
                        if (pos == 0)
                            return {std::string_view::npos, 0};
                        while (pos > 0 && std::isspace(static_cast<unsigned char>(source[pos - 1])))
                            --pos;
                        if (pos == 0)
                            return {std::string_view::npos, 0};
                        // Find the start of the word
                        while (pos > 0 && !std::isspace(static_cast<unsigned char>(source[pos - 1])))
                            --pos;
                        return {pos, 0};
                    }
                };

                return split<Direction>(source_str, max_split, find_next);
            }

            template <SplitOrient Direction>
            std::vector<std::string_view> split_all(std::string_view source)
            {
                std::vector<std::string_view> result;
                auto unicode_str = icu::UnicodeString::fromUTF8(source);
                int32_t len = unicode_str.length();
                if (len == 0)
                    return result;

                for (int32_t pos = 0; pos < len;)
                {
                    char16_t c = unicode_str.charAt(pos);
                    result.emplace_back(source.substr(pos, U16_LENGTH(c)));
                    pos += U16_LENGTH(c);
                }

                if constexpr (Direction == SplitOrient::RIGHT)
                {
                    std::reverse(result.begin(), result.end());
                }

                return result;
            }

        } // namespace split_helpers

        // split from left with separator
        std::vector<std::string_view> split(
            std::string_view source, std::string_view sep,
            size_t max_split = static_cast<size_t>(-1))
        {
            return split_helpers::split_string<split_helpers::SplitOrient::LEFT>(source, sep, max_split);
        }

        // rsplit from right with separator
        std::vector<std::string_view> rsplit(
            std::string_view source, std::string_view sep,
            size_t max_split = static_cast<size_t>(-1))
        {
            return split_helpers::split_string<split_helpers::SplitOrient::RIGHT>(source, sep, max_split);
        }

        // // split on whitespace from left
        // std::vector<std::string_view> split_whitespace(
        //     std::string_view source, size_t max_split = static_cast<size_t>(-1))
        // {
        //     return split_helpers::split_whitespace<split_helpers::SplitOrient::LEFT>(source, max_split);
        // }

        // // rsplit on whitespace from right
        // std::vector<std::string_view> rsplit_whitespace(
        //     std::string_view source, size_t max_split = static_cast<size_t>(-1))
        // {
        //     return split_helpers::split_whitespace<split_helpers::SplitOrient::RIGHT>(source, max_split);
        // }

        std::vector<std::string_view> splitlines(const std::string_view &source_str,
                                                 bool keepends = false)
        {
            std::vector<std::string_view> result;
            size_t len = source_str.size();

            if (len == 0)
                return result;

            size_t pos = 0;
            size_t start = 0;

            while (pos < len)
            {
                size_t line_break_len = split_helpers::is_line_break(source_str, pos);
                if (line_break_len <= 0)
                {
                    ++pos;
                    continue;
                }

                // Found a line break
                if (keepends)
                    result.emplace_back(
                        source_str.substr(start, pos - start + line_break_len));
                else
                    result.emplace_back(source_str.substr(start, pos - start));
                pos += line_break_len;
                start = pos;
            }

            // Add any remaining text after the last line break
            if (start < len)
                result.emplace_back(source_str.substr(start, len - start));

            return result;
        }

        // split all foreach, sep is empty string
        std::vector<std::string_view> split_all(std::string_view source)
        {
            return split_helpers::split_all<split_helpers::SplitOrient::LEFT>(source);
        }

        // split all from right
        std::vector<std::string_view> rsplit_all(std::string_view source)
        {
            return split_helpers::split_all<split_helpers::SplitOrient::RIGHT>(source);
        }

        using IsXXXCallback = std::function<bool(std::string_view)>;
        tl::expected<bool, HakkaJsonResultEnum> is_xxx(std::string_view sv, IsXXXCallback callback)
        {
            return callback(sv);
        }
    } // namespace detail
}

struct JsonStringCompactIter::Impl
{
    icu::UnicodeString unicode_str;
    JsonStringCompactIter::difference_type pos;

    Impl(std::string_view sv) : unicode_str(icu::UnicodeString::fromUTF8(sv)), pos(0) {}
};

JsonStringCompactIter::~JsonStringCompactIter() = default;

JsonStringCompactIter::JsonStringCompactIter(std::string_view sv, bool end) : pImpl{std::make_unique<Impl>(sv)}
{
    if (end)
    {
        pImpl->pos = pImpl->unicode_str.length();
    }
}

JsonStringCompactIter::JsonStringCompactIter(JsonStringCompactIter &&other) noexcept
{
    pImpl = std::move(other.pImpl);
}

JsonStringCompactIter &JsonStringCompactIter::operator=(JsonStringCompactIter &&other) noexcept
{
    pImpl = std::move(other.pImpl);
    return *this;
}

JsonStringCompactIter::reference JsonStringCompactIter::operator*() const
{
    auto &unicode_str = pImpl->unicode_str;
    auto pos = pImpl->pos;

    // copy the character at the current position
    char32_t c = unicode_str.char32At(pos);
    return c;
}

JsonStringCompactIter &JsonStringCompactIter::operator++()
{
    pImpl->pos = pImpl->unicode_str.moveIndex32(pImpl->pos, 1);
    return *this;
}

JsonStringCompactIter &JsonStringCompactIter::operator--()
{
    pImpl->pos = pImpl->unicode_str.moveIndex32(pImpl->pos, -1);
    return *this;
}

bool JsonStringCompactIter::operator==(const JsonStringCompactIter &other) const
{
    return pImpl->pos == other.pImpl->pos;
}

bool JsonStringCompactIter::operator!=(const JsonStringCompactIter &other) const
{
    return pImpl->pos != other.pImpl->pos;
}

JsonStringCompact::JsonStringCompact(const ValueType &value) : JsonPrimitiveCompact(value)
{
}

// string_view version of constructor
JsonStringCompact::JsonStringCompact(std::string_view value) : JsonPrimitiveCompact([](std::string_view value) -> ValueType {
    if (value.size() == 0)
        return scc::PicoString1(""); // special case for empty string

    // Create a null-terminated string for PicoString constructors
    // PicoString constructors use strlen which requires null termination
    std::string temp_str(value);

    if (value.size() == 1)
        return scc::PicoString1(temp_str.c_str());
    else if (value.size() <= 2)
        return scc::PicoString2(temp_str.c_str());
    else if (value.size() <= 4)
        return scc::PicoString4(temp_str.c_str());
    else if (value.size() <= 8)
        return scc::PicoString8(temp_str.c_str());
    else if (value.size() <= 16)
        return scc::PicoString16(temp_str.c_str());
    else if (value.size() <= 32)
        return scc::PicoString32(temp_str.c_str());
    else if (value.size() <= 64)
        return scc::PicoString64(temp_str.c_str());
    return scc::PicoStringUnlimited(value);
}(value))
{
}

// Factory methods - placeholder implementations for StringManagerCompact
JsonHandleCompact JsonStringCompact::create(const ValueType &value)
{
    return JsonHandleCompact(StringManagerCompact::get_instance().create(value.to_string_view()));
}

JsonHandleCompact JsonStringCompact::create(std::string_view value)
{
    return JsonHandleCompact(StringManagerCompact::get_instance().create(value));
}

std::unique_ptr<JsonStringCompact> JsonStringCompact::create_unique(const ValueType &value)
{
    return std::unique_ptr<JsonStringCompact>(new (std::nothrow) JsonStringCompact(value));
}

std::unique_ptr<JsonStringCompact> JsonStringCompact::create_unique(std::string_view value)
{
    return std::unique_ptr<JsonStringCompact>(new (std::nothrow) JsonStringCompact(value));
}

uint64_t JsonStringCompact::inc_ref_impl() const
{
    return ref_count.fetch_add(1, std::memory_order_relaxed) + 1;
}

uint64_t JsonStringCompact::dec_ref_impl() const
{
    return ref_count.fetch_sub(1, std::memory_order_relaxed) - 1;
}

tl::expected<std::string, HakkaJsonResultEnum> JsonStringCompact::dump_impl([[maybe_unused]] uint32_t /*max_depth*/) const
{
    try {
        return detail::escape_json_string(value_.to_string_view());
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

HakkaJsonResultEnum JsonStringCompact::to_bytes_impl(char *buffer, uint32_t *buffer_size) const
{
    try {
        std::string serialized = dump(1).value();
        uint32_t required_size = static_cast<uint32_t>(serialized.size()) + 1; // +1 for null terminator

        if (*buffer_size < required_size) {
            *buffer_size = required_size;
            return HAKKA_JSON_NOT_ENOUGH_MEMORY;
        }

        std::memcpy(buffer, serialized.c_str(), required_size);
        buffer[serialized.size()] = '\0';
        *buffer_size = static_cast<uint32_t>(serialized.size());
        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

HakkaJsonType JsonStringCompact::type_impl() const
{
    return HakkaJsonType::HAKKA_JSON_STRING;
}

tl::expected<int, HakkaJsonResultEnum> JsonStringCompact::compare_impl(const JsonHandleCompact &other) const
{
    if (other.get_type() != HakkaJsonType::HAKKA_JSON_STRING) {
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }

    try {
        // Get the other string's view through UniformCompactPointerView
        auto other_view = other.get_view();
        // Extract JsonStringCompact* from the variant
        const auto *other_str = std::get<const JsonStringCompact*>(other_view);
        if (!other_str) {
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
        }

        const auto &my_value = value_.to_string_view();
        const auto &other_value = other_str->value_.to_string_view();

        if (my_value < other_value) return -1;
        if (my_value > other_value) return 1;
        return 0;
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

uint64_t JsonStringCompact::hash_impl() const
{
    return std::hash<std::string>{}(value_.to_string());
}

uint64_t JsonStringCompact::dump_size_impl() const
{
    return dump_impl(0).value_or("").size();
}

tl::expected<PrimitiveType, HakkaJsonResultEnum> JsonStringCompact::get_impl() const
{
    return value_.to_string();
}

tl::expected<int64_t, HakkaJsonResultEnum> JsonStringCompact::length() const
{
    // The reason we need to convert to icu::UnicodeString is storing utf32 is too expensive for memory footprint
    const auto &unicode_str = icu::UnicodeString::fromUTF8(value_.to_string_view());
    return unicode_str.length();
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::capitalize() const
{
    // python magic - Only its first character capitalized and the rest lowercased.
    // See: https://docs.python.org/3/library/stdtypes.html#str.capitalize
    auto unicode_str = detail::to_unicode_string(value_.to_string_view());
    if (unicode_str.isEmpty())
        return JsonStringCompact::create("");

    char16_t first_char = unicode_str.charAt(0);
    char16_t upper_first_char = u_toupper(first_char);
    unicode_str.replace(0, U16_LENGTH(first_char), upper_first_char);

    for (int32_t i = U16_LENGTH(first_char); i < unicode_str.length();)
    {
        char16_t c = unicode_str.charAt(i);
        char16_t lower_c = u_tolower(c);
        unicode_str.replace(i, U16_LENGTH(c), lower_c);
        i += U16_LENGTH(c);
    }

    std::string result = detail::from_unicode_string(unicode_str);
    return JsonStringCompact::create(result);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::casefold() const
{
    return lower();
}

tl::expected<int64_t, HakkaJsonResultEnum> JsonStringCompact::count(const std::string &substring) const
{
    return count(std::string_view(substring));
}

tl::expected<int64_t, HakkaJsonResultEnum> JsonStringCompact::count(std::string_view substring) const
{
    int64_t count = 0;
    std::string_view my_sv = value_.to_string_view();

    while (size_t pos = my_sv.find(substring) != std::string::npos)
    {
        ++count;
        my_sv.remove_prefix(pos + substring.size());
    }

    return count;
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::endswith(const std::string &suffix) const
{
    return endswith(std::string_view(suffix));
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::endswith(std::string_view suffix) const
{
    std::string_view my_sv = value_.to_string_view();
    if (my_sv.size() < suffix.size())
        return false;
    return my_sv.substr(my_sv.size() - suffix.size()) == suffix;
}

tl::expected<int64_t, HakkaJsonResultEnum> JsonStringCompact::find(const std::string &substring) const
{
    return find(std::string_view(substring));
}

tl::expected<int64_t, HakkaJsonResultEnum> JsonStringCompact::find(std::string_view substring) const
{
    std::string_view my_sv = value_.to_string_view();
    size_t pos = my_sv.find(substring);
    if (pos == std::string::npos)
        return -1;
    return pos;
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::concatenate(const std::string &other) const
{
    return concatenate(std::string_view(other));
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::concatenate(std::string_view other) const
{
    std::string result = value_.to_string() + std::string(other);
    return JsonStringCompact::create(result);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::multiply(int64_t times) const
{
    if (times <= 0)
        return JsonStringCompact::create("");

    std::string result;
    const auto &reserve_result = detail::try_reserve(result, value_.to_string_view().size() * times);
    if (reserve_result != HAKKA_JSON_SUCCESS)
        return tl::make_unexpected(reserve_result);

    for (int64_t i = 0; i < times; ++i)
        result += value_.to_string();

    return JsonStringCompact::create(result);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::slice(int64_t start, int64_t stop, int64_t step) const
{
    if (step == 0)
        return JsonStringCompact::create("");

    const auto &unicode_str = icu::UnicodeString::fromUTF8(value_.to_string_view());
    if (start < 0)
        start += unicode_str.length();
    if (stop < 0)
        stop += unicode_str.length();

    if (start < 0 || start >= static_cast<int64_t>(unicode_str.length()) ||
        stop < -1 || stop > static_cast<int64_t>(unicode_str.length()))
        return tl::make_unexpected(HAKKA_JSON_INDEX_OUT_OF_BOUNDS);

    icu::UnicodeString result;

    if (step > 0) // loop forward
    {
        int64_t start_index = unicode_str.moveIndex32(start, 0);
        int64_t stop_index = unicode_str.moveIndex32(stop, 0);
        for (int64_t i = start_index; i < stop_index;)
        {
            UChar32 c = unicode_str.char32At(i);
            result.append(c);
            i = unicode_str.moveIndex32(i, step);
        }
    }
    else // loop backward
    {
        int64_t start_index = unicode_str.moveIndex32(start, 0);
        int64_t stop_index = unicode_str.moveIndex32(stop, 0);
        for (int64_t i = start_index; i > stop_index;)
        {
            UChar32 c = unicode_str.char32At(i);
            result.append(c);
            i = unicode_str.moveIndex32(i, step);
        }

        if (stop == -1)
        {
            UChar32 c = unicode_str.char32At(stop_index);
            result.append(c);
        }
    }
    return JsonStringCompact::create(detail::from_unicode_string(result));
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::lower() const
{
    auto to_lower = [](const std::string &from, std::string &to) -> HakkaJsonResultEnum
    {
        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(from);
        if (unicode_str.isEmpty())
        {
            to.clear();
            return HAKKA_JSON_SUCCESS;
        }

        unicode_str.toLower();
        auto reserve_res = detail::try_reserve(to, unicode_str.length());
        if (reserve_res != HAKKA_JSON_SUCCESS)
            return reserve_res;

        to.resize(0);
        unicode_str.toUTF8String(to);
        return HAKKA_JSON_SUCCESS;
    };

    return detail::transform_string_compact(value_.to_string(), to_lower);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::removeprefix(const std::string &prefix) const
{
    return removeprefix(std::string_view(prefix));
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::removeprefix(std::string_view prefix) const
{
    if (value_.to_string_view().size() < prefix.size())
        return JsonStringCompact::create(value_.to_string_view());

    if (value_.to_string_view().substr(0, prefix.size()) != prefix)
        return JsonStringCompact::create(value_.to_string_view());

    auto result = value_.to_string_view().substr(prefix.size());
    // Create a proper std::string to ensure null termination
    std::string result_str(result);
    return JsonStringCompact::create(result_str);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::removesuffix(const std::string &suffix) const
{
    return removesuffix(std::string_view(suffix));
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::removesuffix(std::string_view suffix) const
{
    if (value_.to_string_view().size() < suffix.size())
        return JsonStringCompact::create(value_.to_string_view());

    if (value_.to_string_view().substr(value_.to_string_view().size() - suffix.size()) != suffix)
        return JsonStringCompact::create(value_.to_string_view());

    auto result = value_.to_string_view().substr(0, value_.to_string_view().size() - suffix.size());
    // Create a proper std::string to ensure null termination
    std::string result_str(result);
    return JsonStringCompact::create(result_str);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::replace(const std::string &old_substr, const std::string &new_substr) const
{
    return replace(std::string_view(old_substr), std::string_view(new_substr));
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::replace(std::string_view old_substr, std::string_view new_substr) const
{
    return JsonStringCompact::create(detail::replace_all(value_.to_string_view(), old_substr, new_substr));
}

tl::expected<int64_t, HakkaJsonResultEnum> JsonStringCompact::rfind(const std::string &substring) const
{
    return rfind(std::string_view(substring));
}

tl::expected<int64_t, HakkaJsonResultEnum> JsonStringCompact::rfind(std::string_view substring) const
{
    std::string_view my_sv = value_.to_string_view();
    size_t pos = my_sv.rfind(substring);
    if (pos == std::string::npos)
        return -1;
    return pos;
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::rsplit(const std::string &separator, int64_t maxsplit) const
{
    return rsplit(std::string_view(separator), maxsplit);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::rsplit(std::string_view separator, int64_t maxsplit) const
{
    // TODO: implement the JsonArrayCompact
    auto arr_handle = JsonArrayCompact::create();
    auto arr_ptr = std::get<JsonArrayCompact*>(arr_handle.get_mut_ptr());
    if (!arr_ptr)
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

    std::vector<std::string_view> split_result;
    if (separator.empty())
        split_result = detail::rsplit_all(value_.to_string_view());
    else
        split_result = detail::rsplit(value_.to_string_view(), separator, maxsplit);

    for (const auto &part : split_result)
        arr_ptr->push_back(JsonStringCompact::create(std::string(part)));

    return arr_handle;
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::split(const std::string &separator, int64_t maxsplit) const
{
    return split(std::string_view(separator), maxsplit);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::split(std::string_view separator, int64_t maxsplit) const
{
    // TODO: implement the JsonArrayCompact
    auto arr_handle = JsonArrayCompact::create();
    auto arr_ptr = std::get<JsonArrayCompact*>(arr_handle.get_mut_ptr());
    if (!arr_ptr)
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

    std::vector<std::string_view> split_result;
    if (separator.empty())
        split_result = detail::split_all(value_.to_string_view());
    else
        split_result = detail::split(value_.to_string_view(), separator, maxsplit);

    for (const auto &part : split_result)
        arr_ptr->push_back(JsonStringCompact::create(std::string(part)));

    return arr_handle;
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::splitlines(bool keepends) const
{
    // TODO: implement the JsonArrayCompact
    auto arr_handle = JsonArrayCompact::create();
    auto arr_ptr = std::get<JsonArrayCompact*>(arr_handle.get_mut_ptr());
    if (!arr_ptr)
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

    auto split_result = detail::splitlines(value_.to_string_view(), keepends);
    for (const auto &part : split_result)
    {
        arr_ptr->push_back(JsonStringCompact::create(std::string(part)));
    }

    return arr_handle;
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::startswith(const std::string &prefix) const
{
    return startswith(std::string_view(prefix));
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::startswith(std::string_view prefix) const
{
    std::string_view my_sv = value_.to_string_view();
    if (my_sv.size() < prefix.size())
        return false;
    return my_sv.substr(0, prefix.size()) == prefix;
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::upper() const
{
    auto to_upper = [](const std::string &from, std::string &to) -> HakkaJsonResultEnum
    {
        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(from);
        if (unicode_str.isEmpty())
        {
            to.clear();
            return HAKKA_JSON_SUCCESS;
        }

        unicode_str.toUpper();
        auto reserve_res = detail::try_reserve(to, unicode_str.length());
        if (reserve_res != HAKKA_JSON_SUCCESS)
            return reserve_res;

        to.resize(0);
        unicode_str.toUTF8String(to);
        return HAKKA_JSON_SUCCESS;
    };

    return detail::transform_string_compact(value_.to_string(), to_upper);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::swapcase() const
{
    auto swap_case = [](const std::string &from, std::string &to) -> HakkaJsonResultEnum
    {
        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(from);
        if (unicode_str.isEmpty())
        {
            to.clear();
            return HAKKA_JSON_SUCCESS;
        }

        for (int32_t i = 0; i < unicode_str.length();)
        {
            char16_t c = unicode_str.charAt(i);
            char16_t swapped_c = u_toupper(c);
            if (swapped_c == c)
                swapped_c = u_tolower(c);
            unicode_str.replace(i, U16_LENGTH(c), swapped_c);
            i += U16_LENGTH(c);
        }

        auto reserve_res = detail::try_reserve(to, unicode_str.length());
        if (reserve_res != HAKKA_JSON_SUCCESS)
            return reserve_res;

        to.resize(0);
        unicode_str.toUTF8String(to);
        return HAKKA_JSON_SUCCESS;
    };

    return detail::transform_string_compact(value_.to_string(), swap_case);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::title() const
{
    auto title_case = [](const std::string &from, std::string &to) -> HakkaJsonResultEnum
    {
        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(from);
        if (unicode_str.isEmpty())
        {
            to.clear();
            return HAKKA_JSON_SUCCESS;
        }

        bool capitalize_next = true;
        for (int32_t i = 0; i < unicode_str.length();)
        {
            char16_t c = unicode_str.charAt(i);
            char16_t title_c = c;
            if (capitalize_next)
            {
                title_c = u_toupper(c);
                capitalize_next = false;
            }
            else
            {
                title_c = u_tolower(c);
            }

            if (u_isspace(c))
                capitalize_next = true;

            unicode_str.replace(i, U16_LENGTH(c), title_c);
            i += U16_LENGTH(c);
        }

        auto reserve_res = detail::try_reserve(to, unicode_str.length());
        if (reserve_res != HAKKA_JSON_SUCCESS)
            return reserve_res;

        to.resize(0);
        unicode_str.toUTF8String(to);
        return HAKKA_JSON_SUCCESS;
    };

    return detail::transform_string_compact(value_.to_string(), title_case);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonStringCompact::zfill(int64_t width) const
{
    if (width <= static_cast<int64_t>(value_.to_string_view().size()))
        return JsonStringCompact::create(value_.to_string_view());

    std::string result;
    const auto reserve_result = detail::try_reserve(result, width);
    if (reserve_result != HAKKA_JSON_SUCCESS)
        return tl::make_unexpected(reserve_result);

    result.append(width - value_.to_string_view().size(), '0');
    result.append(value_.to_string());
    return JsonStringCompact::create(result);
}

tl::expected<uint64_t, HakkaJsonResultEnum> JsonStringCompact::utf8_length() const
{
    return value_.to_string_view().size();
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::isalnum() const
{
    auto unicode_is_alnum = [](std::string_view sv)
    {
        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(sv);
        if (unicode_str.isEmpty())
            return false;

        for (int32_t i = 0; i < unicode_str.length();)
        {
            char16_t c = unicode_str.charAt(i);
            if (!u_isalnum(c))
                return false;
            i += U16_LENGTH(c);
        }
        return true;
    };

    return detail::is_xxx(value_.to_string_view(), unicode_is_alnum);
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::isalpha() const
{
    auto unicode_is_alpha = [](std::string_view sv)
    {
        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(sv);
        if (unicode_str.isEmpty())
            return false;

        for (int32_t i = 0; i < unicode_str.length();)
        {
            char16_t c = unicode_str.charAt(i);
            if (!u_isalpha(c))
                return false;
            i += U16_LENGTH(c);
        }
        return true;
    };

    return detail::is_xxx(value_.to_string_view(), unicode_is_alpha);
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::isascii() const
{
    auto unicode_is_ascii = [](std::string_view sv)
    {
        return std::all_of(sv.begin(), sv.end(), [](char c)
                           { return static_cast<unsigned char>(c) <= 0x7F; });
    };

    return detail::is_xxx(value_.to_string_view(), unicode_is_ascii);
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::isdecimal() const
{
    auto unicode_is_decimal = [](std::string_view sv)
    {
        if (sv.empty())
            return false;

        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(sv);
        for (int32_t i = 0; i < unicode_str.length();)
        {
            char16_t c = unicode_str.charAt(i);
            if (!u_isdigit(c))
                return false;
            i += U16_LENGTH(c);
        }
        return true;
    };

    return detail::is_xxx(value_.to_string_view(), unicode_is_decimal);
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::isdigit() const
{
    auto unicode_is_digit = [](std::string_view sv)
    {
        if (sv.empty())
            return false;

        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(sv);
        for (int32_t i = 0; i < unicode_str.length();)
        {
            char16_t c = unicode_str.charAt(i);
            if (!u_isdigit(c))
                return false;
            i += U16_LENGTH(c);
        }
        return true;
    };

    return detail::is_xxx(value_.to_string_view(), unicode_is_digit);
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::isidentifier() const
{
    auto unicode_is_identifier = [](std::string_view sv)
    {
        if (sv.empty())
            return false;

        if (!std::isalpha(static_cast<unsigned char>(sv[0])) && sv[0] != '_')
            return false;

        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(sv);
        for (int32_t i = 1; i < unicode_str.length();)
        {
            char16_t c = unicode_str.charAt(i);
            if (!u_isalnum(c) && c != '_')
                return false;
            i += U16_LENGTH(c);
        }

        return true;
    };

    return detail::is_xxx(value_.to_string_view(), unicode_is_identifier);
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::islower() const
{
    auto unicode_is_lower = [](std::string_view sv)
    {
        if (sv.empty())
            return false;

        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(sv);
        for (int32_t i = 0; i < unicode_str.length();)
        {
            char16_t c = unicode_str.charAt(i);
            if (u_isupper(c))
                return false;
            i += U16_LENGTH(c);
        }
        return true;
    };

    return detail::is_xxx(value_.to_string_view(), unicode_is_lower);
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::isnumeric() const
{
    auto unicode_is_numeric = [](std::string_view sv)
    {
        if (sv.empty())
            return false;

        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(sv);
        for (int32_t i = 0; i < unicode_str.length();)
        {
            char16_t c = unicode_str.charAt(i);
            if (!u_isdigit(c) && !u_isdigit(c))
                return false;
            i += U16_LENGTH(c);
        }
        return true;
    };

    return detail::is_xxx(value_.to_string_view(), unicode_is_numeric);
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::isprintable() const
{
    auto unicode_is_printable = [](std::string_view sv) -> bool
    {
        if (sv.empty())
            return true;

        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(sv);
        for (int32_t i = 0; i < unicode_str.length();)
        {
            // Return True if all characters in the string are printable or the string is empty, False otherwise.
            // Nonprintable characters are those characters defined in the
            // Unicode character database as “Other” or “Separator”, excepting the ASCII space (0x20)
            // which is considered printable.
            // (Note that printable characters in this context are those which should not be escaped
            // when repr() is invoked on a string. It has no bearing on the handling of strings written
            // to sys.stdout or sys.stderr.)

            char16_t c = unicode_str.charAt(i);
            UCharCategory category = static_cast<UCharCategory>(u_charType(c));

            // Note: "こんにちは世界" is printable, but "こんにちは 世界" is in U_OTHER_LETTER
            if (category == U_SPACE_SEPARATOR || category == U_LINE_SEPARATOR || category == U_PARAGRAPH_SEPARATOR ||
                /* category == U_OTHER_LETTER || category == U_OTHER_NUMBER || category == U_OTHER_PUNCTUATION || */ category == U_OTHER_SYMBOL)
                return false;

            i += U16_LENGTH(c);
        }
        return true;
    };

    return detail::is_xxx(value_.to_string_view(), unicode_is_printable);
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::isspace() const
{
    auto unicode_is_space = [](std::string_view sv)
    {
        if (sv.empty())
            return false;

        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(sv);
        for (int32_t i = 0; i < unicode_str.length();)
        {
            char16_t c = unicode_str.charAt(i);
            if (!u_isspace(c))
                return false;
            i += U16_LENGTH(c);
        }
        return true;
    };

    return detail::is_xxx(value_.to_string_view(), unicode_is_space);
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::istitle() const
{
    auto unicode_is_title = [](std::string_view sv)
    {
        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(sv);
        bool has_cased_character = false;
        bool previous_is_cased = false;
        for (int32_t i = 0; i < unicode_str.length();)
        {
            UChar32 c = unicode_str.char32At(i);
            if (u_isupper(c))
            {
                if (previous_is_cased)
                    return false;
                previous_is_cased = true;
                has_cased_character = true;
            }
            else if (u_islower(c))
            {
                if (!previous_is_cased)
                    return false;
                previous_is_cased = true;
                has_cased_character = true;
            }
            else
            {
                previous_is_cased = false;
            }
            i = unicode_str.moveIndex32(i, 1);
        }
        return has_cased_character;
    };

    return detail::is_xxx(value_.to_string_view(), unicode_is_title);
}

tl::expected<bool, HakkaJsonResultEnum> JsonStringCompact::isupper() const
{
    auto unicode_is_upper = [](std::string_view sv)
    {
        if (sv.empty())
            return false;

        icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(sv);
        for (int32_t i = 0; i < unicode_str.length();)
        {
            char16_t c = unicode_str.charAt(i);
            if (u_islower(c))
                return false;
            i += U16_LENGTH(c);
        }
        return true;
    };

    return detail::is_xxx(value_.to_string_view(), unicode_is_upper);
}

JsonStringCompactIter JsonStringCompact::begin() const
{
    return JsonStringCompactIter(value_.to_string_view(), false);
}

JsonStringCompactIter JsonStringCompact::end() const
{
    return JsonStringCompactIter(value_.to_string_view(), true);
}


