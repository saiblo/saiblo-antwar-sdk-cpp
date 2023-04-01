#pragma once

#include "optional-impl.hpp"

template <typename T>
using optional = nonstd::optional<T>;

using nonstd::make_optional;

using nullopt_t = nonstd::nullopt_t;
static constexpr nullopt_t nullopt = nonstd::nullopt;

using nonstd::bad_optional_access;
