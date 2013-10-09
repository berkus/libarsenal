#include "settings_provider.h"
#include "Plist.hpp"

namespace {
static std::string organization_name{"defaultOrg"};
static std::string organization_domain{"defaultDomain"};
static std::string application_name{"defaultApp"};
} // anonymous namespace

void settings_provider::set_organization_name(std::string const& org_name)
{
    organization_name = org_name;
}

void settings_provider::set_organization_domain(std::string const& org_domain)
{
    organization_domain = org_domain;
}

void settings_provider::set_application_name(std::string const& app_name)
{
    application_name = app_name;
}

std::shared_ptr<settings_provider> settings_provider::instance()
{
    return std::make_shared<settings_provider>(private_tag{});
}

settings_provider::settings_provider(settings_provider::private_tag)
{

}

bool settings_provider::enter_section(std::string const& name)
{
    return false;
}

void settings_provider::leave_section()
{}

void settings_provider::set(std::string const& key, std::string const& value)
{}

void settings_provider::set(std::string const& key, byte_array const& value)
{}

void settings_provider::set(std::string const& key, size_t value)
{}

void settings_provider::set(std::string const& key, ssize_t value)
{}

std::string settings_provider::get_string(std::string const& key)
{
    return "";
}

byte_array settings_provider::get_byte_array(std::string const& key)
{
    return byte_array();
}

size_t settings_provider::get_uint(std::string const& key)
{
    return 0;
}

ssize_t settings_provider::get_int(std::string const& key)
{
    return 0;
}
