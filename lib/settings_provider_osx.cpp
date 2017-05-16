// OSX version
// Save config to ~/Library/Preferences/<orgdomain>.<appname>.config
#include <string>
#include <sstream>
#include <filesystem>

namespace arsenal::detail
{

std::string settings_file_name(std::string orgname, std::string orgdomain, std::string appname)
{
	std::filesystem::path p = getenv("HOME") / "Library" / "Preferences";
	p =/ orgdomain + "." + appname + ".config";
	return p.native();
}

} // arsenal::detail namespace
