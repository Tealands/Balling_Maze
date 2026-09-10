// Map.h
#pragma once
#include <string>
#include <vector>
#include <map>
#include <utility>

class Map {
public:
	int width = 0;
	int height = 0;
	std::vector<std::string> grid; // each string is a row
	std::map<char, std::vector<std::pair<int,int>>> points; // symbol -> list of (row,col)

	// Load map from text file. Returns true on success.
	bool loadFromFile(const std::string& path);

	// Human-readable debug representation
	std::string toString() const;
	bool isWallAt(int row, int col) const;
};
