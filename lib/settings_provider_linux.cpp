// Linux version
// Save config to ~/.config/<orgname>/appname.plist
#include <string>
#include <sstream>
#include <boost/filesystem.hpp>
#include "logging.h"

namespace fs = boost::filesystem;

std::string settings_file_name(std::string orgname, std::string orgdomain, std::string appname)
{
    std::ostringstream os;
    os << getenv("HOME") << "/.config/" << orgname;
    try {
        fs::create_directories(os.str());
    } catch(std::system_error& e) {
        logger::warning() << "Cannot create config file directory " << os.str() << " - " << e.what();
    }
    os << "/" << appname << ".plist";
    return os.str();
}
