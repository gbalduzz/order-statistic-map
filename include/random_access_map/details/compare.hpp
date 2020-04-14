// Copyright (C) 2020 Giovanni Balduzzi
// All rights reserved.
//
// See LICENSE for terms of usage.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// Utility to perform three way comparison waiting for c++20.

#pragma once

#include <string>

namespace ramlib {
namespace details {

template <class T>
int compare(const T& a, const T& b) noexcept {
  if (a < b)
    return -1;
  else if (a == b)
    return 0;
  else
    return 1;
}

template <>
int compare(const std::string& a, const std::string& b) noexcept {
  return a.compare(b);
}

}  // namespace details
}  // namespace ramlib
