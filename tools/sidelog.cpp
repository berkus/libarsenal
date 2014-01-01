//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "byte_array.h"
#include "hexdump.h"
#include "flurry.h"
#include "byte_array_wrap.h"

using namespace std;
using namespace boost::posix_time;
using namespace boost::date_time;

// left log line | left binary dump | right binary dump | right log line
// sorted by timestamp

// @todo Thread switch indication
// @todo Time adjust factor?

class file_input
{
protected:
    ifstream if_;
    struct record {
        ptime timestamp;
        string title_or_text; // bin title or log text line
        byte_array data; // only in bin
    } rec_; // Last read record
    bool eof_{false};
    bool drained_{false};
    string name_;

public:
    file_input(string filename, string name)
        : if_(filename, std::ios::in|std::ios::binary)
        , name_(name)
    {}

    bool accept(ptime& timestamp) {
        if (rec_.timestamp < timestamp) {
            timestamp = rec_.timestamp;
            return true;
        }
        return false;
    }

    bool eof() const { return eof_; }
    bool done() const { return eof_ and drained_; }
    string name() const { return name_; }
    ptime time() const { return rec_.timestamp; }

    virtual void display() { drained_ = true; }
    virtual void advance() { drained_ = false; }
};

class log_input : public file_input
{
public:
    log_input(string filename, string name)
        : file_input(filename, name)
    {
        advance(); // Load first rec
    }

    void display() override
    {
        file_input::display();
        cout << name_ << " " << rec_.title_or_text;
    }

    void advance() override
    {
        if (eof_) {
            return;
        }
        string text;
        if (getline(if_, text)) {
            string stamp = std::string(text.begin() + 8, text.begin() + 34);
            rec_.timestamp = parse_delimited_time<ptime>(stamp, 'T');
            rec_.title_or_text = text;
            file_input::advance();
        }
        else
        {
            eof_ = true;
        }
    }
};

class bin_input : public file_input
{
    flurry::iarchive ia_;

public:
    bin_input(string filename, string name)
        : file_input(filename, name)
        , ia_(if_)
    {
        advance(); // Load first rec
    }

    void display() override
    {
        file_input::display();
        cout << name_ << " *** BLOB " << rec_.data.size() << " bytes *** " << rec_.timestamp << ": "
            << rec_.title_or_text << endl;
        hexdump(rec_.data);
    }

    void advance() override
    {
        if (eof_) {
            return;
        }
        byte_array data;
        if (ia_ >> data) {
            std::string what, stamp;
            byte_array blob;
            byte_array_iwrap<flurry::iarchive> read(data);
            read.archive() >> what >> stamp >> blob;
            rec_.timestamp = parse_delimited_time<ptime>(stamp, 'T');
            rec_.title_or_text = what;
            rec_.data = blob;
            file_input::advance();
        }
        else
        {
            eof_ = true;
        }
    }
};

int main(int argc, char* argv[])
{
    assert(argc > 4);
    log_input left_log(argv[1], "local_txt");
    bin_input left_bin(argv[2], "local_bin");
    bin_input right_bin(argv[3], "remote_bin");
    log_input right_log(argv[4], "remote_txt");

    file_input* inputs[4] = { &left_log, &left_bin, &right_bin, &right_log };
    bool end{false};

    while (!end)
    {
        ptime timestamp{pos_infin};
        file_input* input{nullptr};
        for (int i = 0; i < 4; ++i)
        {
            if (!inputs[i]->done() and inputs[i]->accept(timestamp)) {
                input = inputs[i];
            }
        }
        assert(input);
        input->display();
        input->advance();

        end = inputs[0]->done() and inputs[1]->done() and inputs[2]->done() and inputs[3]->done();
    }
}
