// Map.cpp
#include "Map.h"
#include <fstream>
#include <sstream>
#include <algorithm>

bool Map::loadFromFile(const std::string& path)
{
	std::ifstream ifs(path);
	if (!ifs.is_open()) return false;

	grid.clear();
	points.clear();
	std::string line;
	int row = 0;
	int maxw = 0;
	while (std::getline(ifs, line)) {
		// strip possible CR
		if (!line.empty() && line.back() == '\r') line.pop_back();
		grid.push_back(line);
		if ((int)line.size() > maxw) maxw = (int)line.size();
		++row;
	}

	height = (int)grid.size();
	width = maxw;

	for (int r = 0; r < (int)grid.size(); ++r) {
		const std::string &rowStr = grid[r];
		for (int c = 0; c < (int)rowStr.size(); ++c) {
			char ch = rowStr[c];
			if (ch != '.' && ch != ' ' ) {
				// record special symbols including walls and markers
				points[ch].push_back({r, c});
			}
		}
	}

	return true;
}

std::string Map::toString() const
{
	std::ostringstream oss;
	oss << "Map: " << width << "x" << height << "\n";
	for (const auto &row : grid) oss << row << "\n";
	oss << "Markers:\n";
	for (const auto &p : points) {
		oss << p.first << ": ";
		for (const auto &xy : p.second) oss << "(" << xy.first << "," << xy.second << ") ";
		oss << "\n";
	}
	return oss.str();
}

bool Map::isWallAt(int row, int col) const
{
	if (row < 0 || col < 0 || row >= height || col >= width) return true;
	if (col >= (int)grid[row].size()) return true;
	return grid[row][col] == '#';
}
