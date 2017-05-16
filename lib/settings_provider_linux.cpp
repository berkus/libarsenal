// Linux version
// Save config to ~/.config/<orgname>/appname.config
#include <string>
#include <sstream>
#include <boost/filesystem.hpp>
#include "arsenal/logging.h"

namespace fs = boost::filesystem;

namespace arsenal::detail
{

std::string settings_file_name(std::string orgname, std::string orgdomain, std::string appname)
{
    std::ostringstream os;
    os << getenv("HOME") << "/.config/" << orgname;
    try {
        fs::create_directories(os.str());
    } catch(std::system_error& e) {
        logger::warning() << "Cannot create config file directory " << os.str() << " - " << e.what();
    }
    os << "/" << appname << ".config";
    return os.str();
}

} // arsenal::detail namespace
