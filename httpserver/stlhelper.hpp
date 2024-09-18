#pragma once

#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
inline std::string ReplaceInStr(const std::string& in, const std::string& search_for, const std::string& replace_with) {
	std::string ret = in;

	std::string::size_type pos = ret.find(search_for);

	while (pos != std::string::npos) {
		ret = ret.replace(pos, search_for.size(), replace_with);
		pos = pos - search_for.size() + replace_with.size() + 1;
		pos = ret.find(search_for, pos);
	}

	return ret;
}
inline char toLower_(char c) { return std::tolower(c); }
inline char toUpper_(char c) { return std::toupper(c); }

inline void ToUpper(std::string& s) {
	std::transform(s.begin(), s.end(), s.begin(), toUpper_);
}

inline void ToLower(std::string& s) {
	std::transform(s.begin(), s.end(), s.begin(), toLower_);
}

template <class T>
T To(std::string const& s) {
	T ret;

	std::stringstream stream;
	stream << s;
	stream >> ret;

	return ret;
}

template<class T>
std::string StringFrom(T const& t) {
	std::string ret;

	std::stringstream stream;
	stream << t;
	stream >> ret;

	return ret;
}
