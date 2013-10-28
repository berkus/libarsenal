//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <thread>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "flurry.h"

namespace logger { // logger::debug()

/**
 * Binary dump a container to the log binary file.
 * Usually only for byte_arrays.
 * @todo Add timestamps for replaying.
 */
class file_dump
{
    static std::mutex m;
public:
    template <typename T>
    file_dump(T const& data, std::string const& filename = "dump.bin") {
        m.lock();
        std::ofstream out(filename, std::ios::out|std::ios::app|std::ios::binary);
        flurry::oarchive oa(out);
        oa << data;
    }
    ~file_dump() { m.unlock(); }
};

// nulstream based on answer here:
// http://stackoverflow.com/questions/7818371/printing-to-nowhere-with-ostream
class nul_streambuf : public std::streambuf
{
    /**
     * The buffer will avoid some unnecessary virtual function calls.
     * On some platforms, this makes a significant difference.
     */
    char dummy_buffer[64];
protected:
    virtual int overflow(int c)
    {
        setp(dummy_buffer, dummy_buffer + sizeof(dummy_buffer));
        return (c == EOF) ? 0 : c;
    }
};

class nul_ostream : public nul_streambuf, public std::ostream
{
public:
    nul_ostream() : std::ostream(this) {}
};

/**
 * Base class for logging output.
 *
 * @todo: to control different output of different debug levels, consider replacing the
 * streambuf on std::clog depending on logging levels, e.g.
 * class debug would set clog wrbuf to either some output buf or null buf.
 */
class logging
{
    static std::mutex m;
    static int log_level;
    int actual_level; // if log_level >= actual_level, then log.
protected:
    static std::ostream& log_stream_;
    static nul_ostream nul_stream_;

    logging(int level) : actual_level(level) {
        m.lock();
	}
    ~logging() { stream() << std::endl; m.unlock(); }

    inline std::ostream& stream() const {
        if (log_level >= actual_level) return log_stream_;
        return nul_stream_;
    }

public:
    // inline static void redirect_output_to(std::ostream& stream) { out_stream = stream; }

    /**
     * Set logging verbosity level.
     * @param level Verbosity level, the higher - the more verbose.
     * 0 - only fatal messages
     * 1 - fatal and warnings
     * 2 - fatal, warnings and info
     * 3 - fatal, warnings, info and debug (spammy)
     * Default is 255 in the debug version, so all logging is permitted.
     * Release version defaults to 2.
     */
    inline static void set_verbosity(int level) { log_level = level; }

    template <typename T>
    std::ostream& operator << (const T& v) { stream() << v; return stream(); }
};

// Helpers
enum class verbosity : int
{
    fatals = 0,
    warnings = 1,
    info = 2,
    debug = 3
};

inline void set_verbosity(verbosity level) { logging::set_verbosity(to_underlying(level)); }


class debug : public logging
{
public:
    debug() : logging(3) {
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
        stream() << "[DEBUG] " << boost::posix_time::to_iso_extended_string(now) << " T#" << std::this_thread::get_id() << ' ';
    }
};

class info : public logging
{
public:
    info() : logging(2) {
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
        stream() << "[INFO ] " << boost::posix_time::to_iso_extended_string(now) << " T#" << std::this_thread::get_id() << ' ';
    }
};

class warning : public logging
{
public:
    warning() : logging(1) {
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
        stream() << "[WARN ] " << boost::posix_time::to_iso_extended_string(now) << " T#" << std::this_thread::get_id() << ' ';
    }
};

class fatal : public logging
{
public:
    fatal() : logging(0) {
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
        stream() << "[FATAL] " << boost::posix_time::to_iso_extended_string(now) << " T#" << std::this_thread::get_id() << ' ';
    }
    ~fatal() { stream() << std::endl; std::abort(); } // Can't call base class dtor after abort()
};

} // namespace logger

/**
 * Helper to output a hexadecimal value with formatting to an iostream.
 * Usage: io << hex(value, 8, true, false)
 */
struct hex_output
{
    int ch;
    int width;
    bool fill;
    bool base;

    hex_output(int c, int w, bool f, bool b) : ch(c), width(w), fill(f), base(b) {}
};

inline std::ostream& operator<<(std::ostream& o, const hex_output& hs)
{
    return (o << std::setw(hs.width) << std::setfill(hs.fill ? '0' : ' ') << std::hex << (hs.base ? std::showbase : std::noshowbase) << hs.ch);
}

inline hex_output hex(int c, int w = 2, bool f = true, bool b = false)
{
    return hex_output(c,w,f,b);
}

