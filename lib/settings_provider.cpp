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
std::string settings_file_name(std::string orgname, std::string orgdomain, std::string appname);

namespace {
std::string organization_name{"Atta"};
std::string organization_domain{"net.atta-metta"};
std::string application_name{"MettaNode"};
map<string, boost::any> data;
} // anonymous namespace

void settings_provider::set_organization_name(std::string const& org_name) {
    organization_name = org_name;
}

void settings_provider::set_organization_domain(std::string const& org_domain) {
    organization_domain = org_domain;
}

void settings_provider::set_application_name(std::string const& app_name) {
    application_name = app_name;
}

std::shared_ptr<settings_provider> settings_provider::instance()
{
    static std::shared_ptr<settings_provider>
        inst{std::make_shared<settings_provider>(private_tag{})};
    return inst;
}

settings_provider::settings_provider(settings_provider::private_tag)
{
    std::ifstream stream(
        settings_file_name(organization_name, organization_domain, application_name),
        std::ios::binary);

    if (stream)
    {
        flurry::iarchive in(stream);
        in >> data; // @todo Atomicity of reads.
    }
}

bool settings_provider::enter_section(std::string const& name) {
    return false;
}

void settings_provider::leave_section()
{}

template <typename T>
void settings_provider::set(std::string const& key, T const& value) {
    data[key] = value;
}

template void settings_provider::set<int8_t>(std::string const& key, int8_t const& value);
template void settings_provider::set<int16_t>(std::string const& key, int16_t const& value);
template void settings_provider::set<int32_t>(std::string const& key, int32_t const& value);
template void settings_provider::set<int64_t>(std::string const& key, int64_t const& value);
template void settings_provider::set<uint8_t>(std::string const& key, uint8_t const& value);
template void settings_provider::set<uint16_t>(std::string const& key, uint16_t const& value);
template void settings_provider::set<uint32_t>(std::string const& key, uint32_t const& value);
template void settings_provider::set<uint64_t>(std::string const& key, uint64_t const& value);
template void settings_provider::set<vector<char>>(std::string const& key, vector<char> const& value);
template void settings_provider::set<byte_array>(std::string const& key, byte_array const& value);

void settings_provider::sync()
{
    std::ofstream stream( // @todo Atomicity of writes via temp file and move.
        settings_file_name(organization_name, organization_domain, application_name),
        std::ios::binary|std::ios::trunc);

    if (stream)
    {
        flurry::oarchive out(stream);
        out << data;
    }
}

boost::any settings_provider::get(std::string const& key) {
    return data[key];
}

byte_array settings_provider::get_byte_array(std::string const& key)
{
    auto& v = data[key];
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
