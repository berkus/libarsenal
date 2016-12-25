//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <fstream>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/log/trivial.hpp>
#include "arsenal/flurry.h"
#include "arsenal/byte_array_wrap.h"
#include "arsenal/hexdump.h"

using namespace std;
using namespace arsenal;
namespace po = boost::program_options;

int main(int argc, char** argv)
{
    std::string filename;

    po::options_description desc("Log file dumper");
    desc.add_options()
        ("filename,f", po::value<std::string>(&filename)->default_value("dump.bin"),
            "Name of the log dump file")
        ("help,h",
            "Print this help message");
    po::positional_options_description p;
    p.add("filename", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
          options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    byte_array data;
    std::ifstream in(filename, std::ios::in|std::ios::binary);
    flurry::iarchive ia(in);

    while (ia >> data) {
        std::string what, stamp;
        byte_array blob;
        byte_array_iwrap<flurry::iarchive> read(data);
        read.archive() >> what >> stamp >> blob;
        cout << "*** BLOB " << blob.size() << " bytes *** " << stamp << ": " << what << endl;
        debug::hexdump(blob);
    }
}
