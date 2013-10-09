//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <memory>
#include <string>
#include "byte_array.h"

/**
 * Settings provider is an interface to platform-specific key-value settings storage.
 * For example, on Linux it could be .rc file, on OSX it could be plists
 * and on Windows it could be registry or .ini file.
 * Selection of actual settings file is left to the settings provider implementation,
 * you can indirectly influence it by setting application name, organization name and domain
 * before instantiating settings accessor.
 *
 * Settings are implicitly grouped into trees.
 * Key name may contain dots to indicate sub-tree elements, for example
 * "globals.user.name" would refer to a key "name" inside group "user" inside group "globals".
 * Groups are called "sections" and you can limit the scope of the key search via
 * enter_section(name) and leave_section() calls.
 */
class settings_provider
{
    bool enter_section(std::string const& name);
    void leave_section();

    struct private_tag{};

public:
    settings_provider(private_tag);

    // Call these functions before instantiating settings_provider.
    static void set_organization_name(std::string const& org_name);
    static void set_organization_domain(std::string const& org_domain);
    static void set_application_name(std::string const& app_name);

    static std::shared_ptr<settings_provider> instance();

    // A RAII wrapper for section scope limiting.
    class scoped_section
    {
        settings_provider& provider_;
    public:
        scoped_section(settings_provider& provider, std::string const& section)
            : provider_(provider)
        {
            provider_.enter_section(section);
        }
        ~scoped_section() {
            provider_.leave_section();
        }
    };

    template <typename T>
    void set(std::string const& key, T const& value);

    /**
     * Persist modified settings to disk.
     */
    void sync();

    boost::any get(std::string const& key);
    byte_array get_byte_array(std::string const& key);
};
