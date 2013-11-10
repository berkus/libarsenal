// OSX version
// Save config to ~/Library/Preferences/<orgdomain>.<appname>.plist
#include <string>
#include <sstream>

std::string settings_file_name(std::string orgname, std::string orgdomain, std::string appname)
{
    std::ostringstream os;
    os << getenv("HOME") << "/Library/Preferences/";
    os << orgdomain << "." << appname << ".plist";
    return os.str();
}
