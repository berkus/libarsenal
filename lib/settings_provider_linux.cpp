// Linux version
// Save config to ~/.config/<orgname>/appname.config
#include <string>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

namespace fs = std::filesystem;

namespace arsenal::detail
{

std::string settings_file_name(std::string orgname, std::string orgdomain, std::string appname)
{
    fs::path p = getenv("HOME") / ".config" / orgname;
    try {
        fs::create_directories(p);
    } catch(std::filesystem_error const& e) {
        BOOST_LOG_TRIVIAL(warning) << "Cannot create config file directory " << os.str() << " - " << e.what();
    }
    p /= appname + ".config";
    return p.native();
}

} // arsenal::detail namespace
