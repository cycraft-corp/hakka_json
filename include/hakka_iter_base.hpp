#ifndef __HAKKA_ITER_BASE_HPP__
#define __HAKKA_ITER_BASE_HPP__
#pragma once

namespace hakka {

// traits
template <typename T>
struct HakkaIterTraits
{
    using value_type = typename T::value_type;
    using reference = typename T::reference;
    using pointer = typename T::pointer;
    using difference_type = typename T::difference_type;
    using iterator_category = typename T::iterator_category;
};

// iterator base (bidirectional)
template <typename Derived>
class HakkaIterBase
{
public:
    using value_type = typename HakkaIterTraits<Derived>::value_type;
    using reference = typename HakkaIterTraits<Derived>::reference;
    using pointer = typename HakkaIterTraits<Derived>::pointer;
    using difference_type = typename HakkaIterTraits<Derived>::difference_type;
    using iterator_category = typename HakkaIterTraits<Derived>::iterator_category;

    HakkaIterBase() = default;
    ~HakkaIterBase() = default;

    reference operator*();
    pointer operator->();
    HakkaIterBase &operator++();
    HakkaIterBase &operator--();
    bool operator==(const HakkaIterBase &other) const;
    bool operator!=(const HakkaIterBase &other) const;
};

} // namespace hakka

#endif // __HAKKA_ITER_BASE_HPP__