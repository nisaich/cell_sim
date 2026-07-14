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
    double avgFood = 0.0;
    double totalFood = 0.0;
    int maxHeight = 0;
    double avgAntibiotic = 0.0;
    double maxAntibiotic = 0.0;
    double avgBiomass = 0.0;
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
