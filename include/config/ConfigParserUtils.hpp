#ifndef CONFIGPARSERUTILS_HPP
# define CONFIGPARSERUTILS_HPP

# include <string>
# include <sstream>
# include <exception>


int toInt(std::string value);
std::string toString(int n);
std::string trim(const std::string& s);
size_t parseSizeWithUnit(const std::string& value);

#endif