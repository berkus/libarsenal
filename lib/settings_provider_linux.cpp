// Linux version
// Save config to ~/.config/<orgname>/appname.plist
#include <string>
#include <sstream>

std::string settings_file_name(std::string orgname, std::string orgdomain, std::string appname)
{
    std::ostringstream os;
    os << getenv("HOME") << "/.config/" << orgname << "/" << appname << ".plist";
    return os.str();
}
