#pragma once

#include "Field.hpp"

#include <fstream>
#include <string>
#include <vector>

struct SimulationStats {
    int tick = 0;
    long long liveCells = 0;
    long long deadCells = 0;
    long long emptyCells = 0;
    float avgFood = 0.0f;
    float totalFood = 0.0f;
    int maxHeight = 0;
    float avgAntibiotic = 0.0f;
    float maxAntibiotic = 0.0f;
};

SimulationStats collectStats(const Field& field, int tick);

class CsvStatsRecorder {
private:
    std::ofstream csv;

public:
    explicit CsvStatsRecorder(const std::string& path);

    bool is_open() const;
    void record(const Field& field, int tick);
};

class StatsHistory {
private:
    std::vector<SimulationStats> history;

public:
    void record(const Field& field, int tick);
    void draw(int x, int y, int width, int height) const;
};
