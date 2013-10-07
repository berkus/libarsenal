#include "settings_provider.h"

bool settings_provider::enter_section(std::string const& name)
{
    return false;
}

void settings_provider::leave_section()
{}

void settings_provider::set_organization_name(std::string const& org_name)
{}

void settings_provider::set_organization_domain(std::string const& org_domain)
{}

void settings_provider::set_application_name(std::string const& app_name)
{}

std::shared_ptr<settings_provider> settings_provider::instance()
{
    return std::make_shared<settings_provider>();
}

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
