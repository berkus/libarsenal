//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <fstream>
#include <iomanip>
#include <mutex>
#include <thread>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/filesystem/path.hpp>
#include "flurry.h"
#include "byte_array_wrap.h"

namespace logger {

/**
 * Binary dump a container to the log binary file.
 * Usually only for byte_arrays.
 */
class file_dump
{
    static boost::mutex m;
public:
    template <typename T>
    file_dump(T const& data, std::string const& comment, std::string const& filename = "dump.bin")
    {
        m.lock();
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
        std::ofstream out(filename, std::ios::out|std::ios::app|std::ios::binary);
        flurry::oarchive oa(out);
        // Each log entry is wrapped into a byte array starting with comment and timestamp.
        byte_array blob;
        {
            byte_array_owrap<flurry::oarchive> write(blob);
            write.archive() << comment << boost::posix_time::to_iso_extended_string(now) << data;
        }
        oa << blob;
    }

    ~file_dump() { m.unlock(); }
};

} // logger namespace
