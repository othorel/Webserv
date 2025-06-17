#ifndef CONFIGVALIDATOR_HPP
# define CONFIGVALIDATOR_HPP

# include <string>
# include <vector>
# include <map>
# include <unistd.h>
# include <iostream>
# include "Location.hpp"

void validateListen(const std::string& ip, const std::string& port);
void validateServerNames(const std::vector<std::string>& names);
void validateRoot(const std::string& root);
void validateErrorPage(const std::string& code, const std::string& path, const std::string& root);
void validateMethods(const std::vector<std::string>& methods);
void validateAutoIndex(const std::string& value);
void validateCgiPass(const std::string& cgi_pass);

#endif