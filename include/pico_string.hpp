#ifndef __SCC_PICO_STRING_HPP__
#define __SCC_PICO_STRING_HPP__
#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <functional>
#include <string_view>

namespace scc
{
    template <uint8_t N>
    struct PicoString : public std::array<char, N>
    {
        using std::array<char, N>::array;

        // Constructor to initialize from a C-string
        PicoString(const char *str)
        {
            uint8_t len = static_cast<uint8_t>(std::strlen(str));
            if (len > N)
                len = N;
            std::memcpy(this->data(), str, len);
            if (len < N)
                this->operator[](len) = '\0'; // Ensure null-termination
        }

        // Conversion to std::string
        std::string to_string() const
        {
            return std::string(this->data(), ::strnlen(this->data(), N));
        }
    };

    using PicoString1 = PicoString<1>;
    using PicoString2 = PicoString<2>;
    using PicoString4 = PicoString<4>;
    using PicoString8 = PicoString<8>;
    using PicoString16 = PicoString<16>;
    using PicoString32 = PicoString<32>;
    using PicoString64 = PicoString<64>;
    // fallback to std::string for larger strings
    using PicoStringUnlimited = std::string;

    namespace detail
    {
        static inline constexpr auto UNLIMITED_SIZE = 255;

        // Encode the size in the lower 3 bits of the pointer
        inline uintptr_t encode_size(uint8_t size)
        {
            switch (size)
            {
            case 1:
                return 0x0;
            case 2:
                return 0x1;
            case 4:
                return 0x2;
            case 8:
                return 0x3;
            case 16:
                return 0x4;
            case 32:
                return 0x5;
            case 64:
                return 0x6;
            default:
                return 0x7; // Fallback to std::string
            }
        }

        // Decode the size from the lower 3 bits of the pointer
        inline uint8_t decode_size(uintptr_t compressed)
        {
            switch (compressed & 0x7)
            {
            case 0x0:
                return 1;
            case 0x1:
                return 2;
            case 0x2:
                return 4;
            case 0x3:
                return 8;
            case 0x4:
                return 16;
            case 0x5:
                return 32;
            case 0x6:
                return 64;
            default:
                return UNLIMITED_SIZE; // Fallback to std::string
            }
        }

        inline uint8_t get_encoded_size(uintptr_t compressed)
        {
            return compressed & 0x7;
        }

        // Extract the original pointer by masking out the lower 3 bits
        inline void *extract_pointer(uintptr_t compressed)
        {
            return reinterpret_cast<void *>(compressed & ~uintptr_t(0x7));
        }

        // Dispatch table for handling different sizes of PicoString
        using StringFunction = std::function<std::string(void *)>;
        inline const StringFunction to_string_dispatch[] = {
            [](void *ptr)
            { return static_cast<PicoString1 *>(ptr)->to_string(); },
            [](void *ptr)
            { return static_cast<PicoString2 *>(ptr)->to_string(); },
            [](void *ptr)
            { return static_cast<PicoString4 *>(ptr)->to_string(); },
            [](void *ptr)
            { return static_cast<PicoString8 *>(ptr)->to_string(); },
            [](void *ptr)
            { return static_cast<PicoString16 *>(ptr)->to_string(); },
            [](void *ptr)
            { return static_cast<PicoString32 *>(ptr)->to_string(); },
            [](void *ptr)
            { return static_cast<PicoString64 *>(ptr)->to_string(); },
            [](void *ptr)
            { return std::string(
                  static_cast<PicoStringUnlimited *>(ptr)->data(),
                  static_cast<PicoStringUnlimited *>(ptr)->size()); },
        };

        // dispatch table for string_view
        using StringViewFunction = std::function<std::string_view(void *)>;
        inline const StringViewFunction to_string_view_dispatch[] = {
            [](void *ptr)
            { return std::string_view(static_cast<PicoString1 *>(ptr)->data(), ::strnlen(static_cast<PicoString1 *>(ptr)->data(), 1)); },
            [](void *ptr)
            { return std::string_view(static_cast<PicoString2 *>(ptr)->data(), ::strnlen(static_cast<PicoString2 *>(ptr)->data(), 2)); },
            [](void *ptr)
            { return std::string_view(static_cast<PicoString4 *>(ptr)->data(), ::strnlen(static_cast<PicoString4 *>(ptr)->data(), 4)); },
            [](void *ptr)
            { return std::string_view(static_cast<PicoString8 *>(ptr)->data(), ::strnlen(static_cast<PicoString8 *>(ptr)->data(), 8)); },
            [](void *ptr)
            { return std::string_view(static_cast<PicoString16 *>(ptr)->data(), ::strnlen(static_cast<PicoString16 *>(ptr)->data(), 16)); },
            [](void *ptr)
            { return std::string_view(static_cast<PicoString32 *>(ptr)->data(), ::strnlen(static_cast<PicoString32 *>(ptr)->data(), 32)); },
            [](void *ptr)
            { return std::string_view(static_cast<PicoString64 *>(ptr)->data(), ::strnlen(static_cast<PicoString64 *>(ptr)->data(), 64)); },
            [](void *ptr)
            { return std::string_view(static_cast<PicoStringUnlimited *>(ptr)->data(), static_cast<PicoStringUnlimited *>(ptr)->size()); },
        };

        using DeleteFunction = std::function<void(void *)>;
        inline const DeleteFunction delete_dispatch[] = {
            [](void *ptr)
            { delete static_cast<PicoString1 *>(ptr); },
            [](void *ptr)
            { delete static_cast<PicoString2 *>(ptr); },
            [](void *ptr)
            { delete static_cast<PicoString4 *>(ptr); },
            [](void *ptr)
            { delete static_cast<PicoString8 *>(ptr); },
            [](void *ptr)
            { delete static_cast<PicoString16 *>(ptr); },
            [](void *ptr)
            { delete static_cast<PicoString32 *>(ptr); },
            [](void *ptr)
            { delete static_cast<PicoString64 *>(ptr); },
            [](void *ptr)
            { delete static_cast<PicoStringUnlimited *>(ptr); },
        };

        using CopyFunction = std::function<void *(void *)>;
        inline const CopyFunction copy_dispatch[] = {
            [](void *ptr)
            { return new PicoString1(*static_cast<PicoString1 *>(ptr)); },
            [](void *ptr)
            { return new PicoString2(*static_cast<PicoString2 *>(ptr)); },
            [](void *ptr)
            { return new PicoString4(*static_cast<PicoString4 *>(ptr)); },
            [](void *ptr)
            { return new PicoString8(*static_cast<PicoString8 *>(ptr)); },
            [](void *ptr)
            { return new PicoString16(*static_cast<PicoString16 *>(ptr)); },
            [](void *ptr)
            { return new PicoString32(*static_cast<PicoString32 *>(ptr)); },
            [](void *ptr)
            { return new PicoString64(*static_cast<PicoString64 *>(ptr)); },
            [](void *ptr)
            { return new PicoStringUnlimited(*static_cast<PicoStringUnlimited *>(ptr)); },
        };
    }

    class PicoStringProxy
    {
    public:
        // Constructor that takes a copy of the underlying PicoString<N> object
        template <uint8_t N>
        PicoStringProxy(const PicoString<N> &pico_str)
        {
            pico_str_ = compress_pointer(new PicoString<N>(pico_str), N);
        }

        // fallback to std::string
        PicoStringProxy(const PicoStringUnlimited &pico_str)
        {
            pico_str_ = compress_pointer(new PicoStringUnlimited(pico_str), detail::UNLIMITED_SIZE);
        }

        // Destructor
        ~PicoStringProxy()
        {
            destroy_pico_string();
        }

        // Copy constructor
        PicoStringProxy(const PicoStringProxy &other)
        {
            pico_str_ = copy_pico_string(other.pico_str_);
        }

        // Move constructor
        PicoStringProxy(PicoStringProxy &&other) noexcept : pico_str_(other.pico_str_)
        {
            other.pico_str_ = nullptr;
        }

        // Copy assignment
        PicoStringProxy &operator=(const PicoStringProxy &other)
        {
            if (this != &other)
            {
                destroy_pico_string();
                pico_str_ = copy_pico_string(other.pico_str_);
            }
            return *this;
        }

        // Move assignment
        PicoStringProxy &operator=(PicoStringProxy &&other) noexcept
        {
            if (this != &other)
            {
                destroy_pico_string();
                pico_str_ = other.pico_str_;
                other.pico_str_ = nullptr;
            }
            return *this;
        }

        // Conversion to std::string
        std::string to_string() const
        {
            if (!pico_str_)
                return "";

            uintptr_t compressed = reinterpret_cast<uintptr_t>(pico_str_);
            uint8_t code = detail::get_encoded_size(compressed);

            void *ptr = detail::extract_pointer(compressed);
            return detail::to_string_dispatch[code](ptr);
        }

        // Conversion to std::string_view
        std::string_view to_string_view() const
        {
            if (!pico_str_)
                return "";

            uintptr_t compressed = reinterpret_cast<uintptr_t>(pico_str_);
            uint8_t code = detail::get_encoded_size(compressed);

            void *ptr = detail::extract_pointer(compressed);
            return std::string_view(detail::to_string_view_dispatch[code](ptr));
        }

        // operator==
        bool operator==(const PicoStringProxy &other) const
        {
            return to_string() == other.to_string();
        }

        // operator!=
        bool operator!=(const PicoStringProxy &other) const
        {
            return !(*this == other);
        }

    private:
        // Compress the pointer by encoding the size in the lower bits
        static void *compress_pointer(void *ptr, uint8_t size)
        {
            uintptr_t compressed = reinterpret_cast<uintptr_t>(ptr) | detail::encode_size(size);
            return reinterpret_cast<void *>(compressed);
        }

        // Destroy the managed PicoString object
        void destroy_pico_string()
        {
            if (pico_str_)
            {
                uintptr_t compressed = reinterpret_cast<uintptr_t>(pico_str_);
                uint8_t code = detail::get_encoded_size(compressed);
                void *ptr = detail::extract_pointer(compressed);
                detail::delete_dispatch[code](ptr);
            }
        }

        // Copy the underlying PicoString object
        void *copy_pico_string(void *src)
        {
            auto code = detail::get_encoded_size(reinterpret_cast<uintptr_t>(src));
            auto ptr = detail::extract_pointer(reinterpret_cast<uintptr_t>(src));
            return compress_pointer(detail::copy_dispatch[code](ptr), detail::decode_size(reinterpret_cast<uintptr_t>(src)));
        }

        void *pico_str_; // Compressed pointer storing both the pointer and the size
    };
}

namespace std
{
    // Specialization of std::hash for PicoString<N>
    template <uint8_t N>
    struct hash<scc::PicoString<N>>
    {
        std::size_t operator()(const scc::PicoString<N> &pico_str) const noexcept
        {
            return std::hash<std::string_view>{}(std::string_view(pico_str.data(), ::strnlen(pico_str.data(), N)));
        }
    };

    // Specialization of std::hash for PicoStringProxy
    template <>
    struct hash<scc::PicoStringProxy>
    {
        std::size_t operator()(const scc::PicoStringProxy &proxy) const noexcept
        {
            return std::hash<std::string_view>{}(proxy.to_string_view());
        }
    };
}

#endif // __SCC_PICO_STRING_HPP__