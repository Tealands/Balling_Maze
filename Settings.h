// Settings.h
#pragma once
#include <string>
#include <vector>

struct Settings {
	int playCount = 0;
	int wins = 0;
	std::vector<std::string> lastClears; // ISO timestamps or map names

	bool load(const std::string& path);
	bool save(const std::string& path) const;
	void recordWin(const std::string& mapName);
};
