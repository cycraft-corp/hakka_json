#ifndef __HAKKA_JSON_STRUCTURED_KEY_HPP__
#define __HAKKA_JSON_STRUCTURED_KEY_HPP__
#pragma once

#include <variant>
#include <string>

using KeyType = std::variant<std::monostate, std::string, int64_t>;

#endif // __HAKKA_JSON_STRUCTURED_KEY_HPP__