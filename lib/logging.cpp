//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "arsenal/logging.h"
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

namespace logger {

boost::mutex file_dump::m;
boost::mutex logging::m;
boost::iostreams::filtering_ostream logging::log_stream_;
nul_ostream logging::nul_stream_;
int logging::log_level{255}; // log all the things
boost::filesystem::path logging::log_file_path;
bool logging::compress_log;
bool logging::initialized_{false};

void shutdown()
{
    logging::log_stream_.flush();
    logging::log_stream_.pop(); // Pop the sink to close whole filtering_ostream
}

void initialize()
{
    using namespace std;
    using namespace boost::iostreams;
    using namespace boost::iostreams::gzip;
    using namespace boost::filesystem;

    if (logging::log_file_path.empty()) {
        logging::log_stream_.push(std::clog);
    }
    else {
        if (exists(logging::log_file_path)) {
            if (file_size(logging::log_file_path) > 10*1024*1024) { // @todo Configurable size
                remove(logging::log_file_path); // @todo Rotate log file
            }
        }

        if (logging::compress_log) {
            logging::log_stream_.push(gzip_compressor(best_compression));
        }
        file_sink fs(logging::log_file_path.native(), ios_base::app | ios_base::binary);
        logging::log_stream_.push(fs);
    }

    logging::log_stream_.set_auto_close(true);
    atexit(shutdown);

    logging::initialized_ = true;
}

}
