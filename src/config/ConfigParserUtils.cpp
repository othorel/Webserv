#include "../../include/config/ConfigParserUtils.hpp"
#include "../../include/config/ParseException.hpp"

int toInt(std::string value) {
	int result;
	char c;
	std::istringstream iss(value);
	
	if (!(iss >> result))		
		return (-1);
	if (iss >> c)
		return (-1);
	return (result);
}

std::string toString(int n) {
	std::ostringstream oss;
	oss << n;
	return (oss.str());
}

std::string trim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t\r\n");
	size_t end = s.find_last_not_of(" \t\r\n");
	if (start == std::string::npos)
		return ("");
	return (s.substr(start, end - start + 1));
}

size_t parseSizeWithUnit(const std::string& value) {
	if (value.empty())
		throw ParseException("Client max body size value is empty");
	char unit = value[value.size() - 1];
	size_t mul = 1;
	std::string numberPart = value;

	if (!isdigit(unit)) {
		numberPart = value.substr(0, value.size() - 1);
		if (unit == 'k' || unit == 'K')
			mul = 1024;
		else if (unit == 'm' || unit == 'M')
			mul = 1024 * 1024;
		else
			throw ParseException("Invalid size unit for client max body size: " + value);
	}
	int number;
	std::istringstream iss(numberPart);
	iss >> number;
	if (iss.fail())
		throw ParseException("Invalid number for clien max body size" + value);
	if (number <= 0)
		throw ParseException("Client max body size must be greater than zero");
	return (static_cast<size_t>(number) * mul);
}
