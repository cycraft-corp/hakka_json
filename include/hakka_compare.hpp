#ifndef __HAKKA_COMPARE_HPP__
#define __HAKKA_COMPARE_HPP__
#pragma once

#include <hakka_json_base.hpp>
#include <uniform_compact_pointer.hpp>

#include <tl/expected.hpp>

namespace hakka {

// some helper function to comare two HakkaJsonBase objects
tl::expected<int, HakkaJsonResultEnum> compare(const UniformCompactPointerView &a, const UniformCompactPointerView &b, uint32_t max_depth = 0);

} // namespace hakka

#endif // __HAKKA_COMPARE_HPP__