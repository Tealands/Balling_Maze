// Settings.cpp
#include "Settings.h"
#include <fstream>
#include <sstream>
#include <ctime>

static std::string now_iso()
{
	std::time_t t = std::time(nullptr);
	char buf[64];
	std::tm tm;
#ifdef _WIN32
	localtime_s(&tm, &t);
#else
	localtime_r(&t, &tm);
#endif
	std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
	return std::string(buf);
}

bool Settings::load(const std::string& path)
{
	std::ifstream ifs(path);
	if (!ifs.is_open()) return false;
	// naive parse: look for numbers and strings
	std::string s((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	size_t pos = s.find("\"playCount\"");
	if (pos != std::string::npos) {
		size_t colon = s.find(':', pos);
		if (colon != std::string::npos) playCount = std::stoi(s.substr(colon+1));
	}
	pos = s.find("\"wins\"");
	if (pos != std::string::npos) {
		size_t colon = s.find(':', pos);
		if (colon != std::string::npos) wins = std::stoi(s.substr(colon+1));
	}
	// not robust but sufficient for minimal data
	return true;
}

bool Settings::save(const std::string& path) const
{
	std::ofstream ofs(path);
	if (!ofs.is_open()) return false;
	ofs << "{\n";
	ofs << "  \"playCount\": " << playCount << ",\n";
	ofs << "  \"wins\": " << wins << ",\n";
	ofs << "  \"lastClears\": [\n";
	for (size_t i=0;i<lastClears.size();++i) {
		ofs << "    \"" << lastClears[i] << "\"";
		if (i+1<lastClears.size()) ofs << ",";
		ofs << "\n";
	}
	ofs << "  ]\n";
	ofs << "}\n";
	return true;
}

void Settings::recordWin(const std::string& mapName)
{
	++wins;
	++playCount;
	lastClears.push_back(now_iso() + " " + mapName);
	if (lastClears.size() > 10) lastClears.erase(lastClears.begin());
}
