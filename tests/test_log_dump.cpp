//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "flurry.h"
#include "logging.h"
#include "byte_array_wrap.h"

using namespace std;

int main()
{
    byte_array data;
    std::ifstream in("dump.bin", std::ios::in|std::ios::binary);
    flurry::iarchive ia(in);

    while (!in.eof() and !in.fail()) {
        ia >> data;
        cout << "Found blob with " << data.size() << " bytes." << endl;
    }
}
