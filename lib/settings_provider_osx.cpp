// OSX version
// Save config to ~/Library/Preferences/<orgdomain>.<appname>.config
#include <string>
#include <sstream>

std::string settings_file_name(std::string orgname, std::string orgdomain, std::string appname)
{
    std::ostringstream os;
    os << getenv("HOME") << "/Library/Preferences/";
    os << orgdomain << "." << appname << ".config";
    return os.str();
}
