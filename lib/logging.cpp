//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "arsenal/logging.h"

namespace logger {

boost::mutex file_dump::m;
boost::mutex logging::m;
std::ostream& logging::log_stream_{std::clog};
nul_ostream logging::nul_stream_;
int logging::log_level{255}; // log all the things

}
