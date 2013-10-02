#pragma once

/**
 * Settings provider is an interface to platform-specific key-value settings storage.
 * For example, on Linux it could be .rc file, on OSX it could be plists
 * and on Windows it could be registry or .ini file.
 *
 * Settings are implicitly grouped into trees.
 * Key name may contain dots to indicate sub-tree elements, for example
 * "globals.user.name" would refer to a key "name" inside group "user" inside group "globals".
 * Groups are called "sections" and you can limit the scope of the key search via
 * enter_section(name) and leave_section() calls.
 */
class settings_provider
{
public:
    static shared_ptr<settings_provider> instance();

    bool enter_section(std::string const& name);
    void leave_section();

    void set(std::string const& key, std::string const& value);
    void set(std::string const& key, byte_array const& value);
    void set(std::string const& key, size_t value); // @todo various int overloads...
    void set(std::string const& key, ssize_t value);

    std::string get_string(std::string const& key);
    byte_array get_byte_array(std::string const& key);
    size_t get_uint(std::string const& key);
    ssize_t get_int(std::string const& key);
};
