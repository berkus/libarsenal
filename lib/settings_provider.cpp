//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/any.hpp>
#include <map>
#include <string>
#include <fstream>
#include "arsenal/flurry.h"
#include "arsenal/settings_provider.h"

using namespace std;

// Generate file name for settings file, this is platform-dependent.
// @todo Make more complex structure allowing to share some data at org level and some per-app.
string settings_file_name(string orgname, string orgdomain, string appname);

namespace {
string organization_name_{"Atta"};
string organization_domain_{"net.atta-metta"};
string application_name_{"Uvvy"};
map<string, boost::any> data_;
} // anonymous namespace

void
settings_provider::set_organization_name(string const& org_name)
{
    organization_name_ = org_name;
}

void
settings_provider::set_organization_domain(string const& org_domain)
{
    organization_domain_ = org_domain;
}

void
settings_provider::set_application_name(string const& app_name)
{
    application_name_ = app_name;
}

shared_ptr<settings_provider>
settings_provider::instance()
{
    static shared_ptr<settings_provider> inst{make_shared<settings_provider>(private_tag{})};
    return inst;
}

settings_provider::settings_provider(settings_provider::private_tag)
{
    ifstream stream(settings_file_name(organization_name_, organization_domain_, application_name_),
                    ios::binary);

    if (stream) {
        flurry::iarchive in(stream);
        in >> data_; // @todo Atomicity of reads.
    }
}

bool
settings_provider::enter_section(string const& name)
{
    return false;
}

void
settings_provider::leave_section()
{
}

template <typename T>
void
settings_provider::set(string const& key, T const& value)
{
    data_[key] = value;
}

template void settings_provider::set<int8_t>(string const& key, int8_t const& value);
template void settings_provider::set<int16_t>(string const& key, int16_t const& value);
template void settings_provider::set<int32_t>(string const& key, int32_t const& value);
template void settings_provider::set<int64_t>(string const& key, int64_t const& value);
template void settings_provider::set<uint8_t>(string const& key, uint8_t const& value);
template void settings_provider::set<uint16_t>(string const& key, uint16_t const& value);
template void settings_provider::set<uint32_t>(string const& key, uint32_t const& value);
template void settings_provider::set<uint64_t>(string const& key, uint64_t const& value);
template void settings_provider::set<vector<char>>(string const& key, vector<char> const& value);
template void settings_provider::set<byte_array>(string const& key, byte_array const& value);
template void settings_provider::set<string>(string const& key, string const& value);

void
settings_provider::sync()
{
    ofstream stream( // @todo Atomicity of writes via temp file and move.
        settings_file_name(organization_name_, organization_domain_, application_name_),
        ios::binary | ios::trunc);

    if (stream) {
        flurry::oarchive out(stream);
        out << data_;
    }
}

boost::any
settings_provider::get(string const& key)
{
    return data_[key];
}

byte_array
settings_provider::get_byte_array(string const& key)
{
    auto& v = data_[key];
    if (v.empty()) {
        return byte_array();
    }
    if (v.type() == typeid(string)) {
        return boost::any_cast<string>(v);
    }
    if (v.type() == typeid(vector<char>)) {
        return boost::any_cast<vector<char>>(v);
    }
    return std::move(boost::any_cast<byte_array>(v));
}
