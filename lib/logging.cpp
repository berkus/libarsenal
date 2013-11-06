//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "logging.h"

namespace logger {
boost::mutex file_dump::m;
boost::mutex logging::m;
std::ostream& logging::log_stream_{std::clog};
nul_ostream logging::nul_stream_;
int logging::log_level{255}; // log everything
}
