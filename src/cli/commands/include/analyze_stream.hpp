// analyze_stream.hpp
#pragma once

#include <istream>

#include "vanitas/profile.hpp"

namespace vanitas::cli {
int analyze_stream(std::istream &in, const vanitas::Profile &prof);
}
