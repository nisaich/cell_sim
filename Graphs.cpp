#include "Graphs.hpp"
#include "Biomass.hpp"
#include "SimulationConfig.hpp"

#include <algorithm>
#include <cmath>

CsvStatsRecorder::CsvStatsRecorder(const std::string& path)
    : csv(path) {
    if (csv.is_open()) {
        csv << "tick,liveCells,deadCells,emptyCells,avgFood,totalFood,maxHeight,avgAntibiotic,maxAntibiotic,avgBiomass\n";
    }
}

bool CsvStatsRecorder::is_open() const {
    return csv.is_open();
}

void CsvStatsRecorder::record(const Field& field, int tick) {
    if (!csv.is_open()) {
        return;
    }

    const SimulationStats stats = collectStats(field, tick);

    csv << stats.tick << ","
        << stats.liveCells << ","
        << stats.deadCells << ","
        << stats.emptyCells << ","
        << stats.avgFood << ","
        << stats.totalFood << ","
        << stats.maxHeight << ","
        << stats.avgAntibiotic << ","
        << stats.maxAntibiotic << ","
        << stats.avgBiomass << "\n";
    csv.flush();
}

SimulationStats collectStats(const Field& field, int tick) {
    SimulationStats stats;
    stats.tick = tick;

    const int totalCells = field.get_width() * field.get_height();

    for (int y = 0; y < field.get_height(); ++y) {
        for (int x = 0; x < field.get_width(); ++x) {
            const Cell& nucleus = field.get_nucleus(x, y);
            const auto cell = nucleus.get_cell();

            if (cell == nullptr) {
                ++stats.emptyCells;
            } else if (cell->is_alive()) {
                ++stats.liveCells;
                stats.avgBiomass += cell->get_biomass();

                const int height = field.get_height() - y;
                if (height > stats.maxHeight) {
                    stats.maxHeight = height;
                }
            } else {
                ++stats.deadCells;
            }

            stats.totalFood += nucleus.get_food().get_amount();

            const double antibiotic = nucleus.get_antibiotic().get_concentration();
            stats.avgAntibiotic += antibiotic;
            if (antibiotic > stats.maxAntibiotic) {
                stats.maxAntibiotic = antibiotic;
            }
        }
    }

    if (totalCells > 0) {
        stats.avgFood = stats.totalFood / totalCells;
        stats.avgAntibiotic /= totalCells;
    }
    
    if (stats.liveCells > 0) {
        stats.avgBiomass /= stats.liveCells;
    }

    return stats;
}
