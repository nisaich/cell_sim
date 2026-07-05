#include "Graphs.hpp"
#include "Biomass.hpp"
#include "SimulationConfig.hpp"

#include <raylib.h>

#include <algorithm>
#include <cmath>

namespace {
void drawSeries(
    const std::vector<SimulationStats>& history,
    Rectangle area,
    const std::vector<float>& valuesA,
    Color colorA,
    const char* labelA,
    const std::vector<float>* valuesB = nullptr,
    Color colorB = BLACK,
    const char* labelB = nullptr
) {
    DrawRectangleRounded(
        area,
        simulation_config::graphs::chart_roundness,
        simulation_config::graphs::chart_round_segments,
        Color{245, 245, 245, 255}
    );
    DrawRectangleLinesEx(
        area,
        simulation_config::graphs::chart_outline_thickness,
        LIGHTGRAY
    );

    if (history.size() < 2 || valuesA.size() != history.size()) {
        DrawText(
            "Not enough data",
            area.x + 10,
            area.y + 10,
            simulation_config::graphs::text_font_size,
            GRAY
        );
        return;
    }

    float minValue = valuesA.front();
    float maxValue = valuesA.front();

    for (float value : valuesA) {
        minValue = std::min(minValue, value);
        maxValue = std::max(maxValue, value);
    }

    if (valuesB != nullptr) {
        for (float value : *valuesB) {
            minValue = std::min(minValue, value);
            maxValue = std::max(maxValue, value);
        }
    }

    if (std::fabs(maxValue - minValue) < simulation_config::graphs::chart_span_epsilon) {
        maxValue = minValue + simulation_config::graphs::chart_min_span;
    }

    const float left = area.x + 10.0f;
    const float right = area.x + area.width - 10.0f;
    const float top = area.y + 28.0f;
    const float bottom = area.y + area.height - 16.0f;
    const float graphWidth = right - left;
    const float graphHeight = bottom - top;

    DrawText(labelA, left, area.y + 6.0f, 14, colorA);
    if (labelB != nullptr) {
        DrawText(labelB, left + 110.0f, area.y + 6.0f, 14, colorB);
    }

    DrawLine(left, bottom, right, bottom, GRAY);
    DrawLine(left, top, left, bottom, GRAY);

    auto drawLineForValues = [&](const std::vector<float>& values, Color color) {
        for (size_t i = 1; i < values.size(); ++i) {
            const float prevX = left + graphWidth * static_cast<float>(i - 1) /
                static_cast<float>(values.size() - 1);
            const float nextX = left + graphWidth * static_cast<float>(i) /
                static_cast<float>(values.size() - 1);

            const float prevY = bottom - graphHeight * (values[i - 1] - minValue) /
                (maxValue - minValue);
            const float nextY = bottom - graphHeight * (values[i] - minValue) /
                (maxValue - minValue);

            DrawLineEx(
                Vector2{prevX, prevY},
                Vector2{nextX, nextY},
                simulation_config::graphs::chart_line_thickness,
                color
            );
        }
    };

    drawLineForValues(valuesA, colorA);
    if (valuesB != nullptr && valuesB->size() == history.size()) {
        drawLineForValues(*valuesB, colorB);
    }

    const std::string maxText = TextFormat("max %.1f", maxValue);
    const std::string minText = TextFormat("min %.1f", minValue);
    DrawText(maxText.c_str(), right - 90.0f, area.y + 6.0f, 14, DARKGRAY);
    DrawText(minText.c_str(), right - 90.0f, area.y + area.height - 18.0f, 14, DARKGRAY);
}
}  // namespace

CsvStatsRecorder::CsvStatsRecorder(const std::string& path)
    : csv(path) {
    if (csv.is_open()) {
        csv << "tick,liveCells,deadCells,emptyCells,avgFood,totalFood,maxHeight,avgAntibiotic,maxAntibiotic\n";
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
        << stats.maxAntibiotic << "\n";
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

                const int height = field.get_height() - y;
                if (height > stats.maxHeight) {
                    stats.maxHeight = height;
                }
            } else {
                ++stats.deadCells;
            }

            stats.totalFood += nucleus.get_food().get_amount();

            const float antibiotic = nucleus.get_antibiotic().get_concentration();
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

    return stats;
}

void StatsHistory::record(const Field& field, int tick) {
    history.push_back(collectStats(field, tick));
}

void StatsHistory::draw(int x, int y, int width, int height) const {
    Rectangle panel{
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(width),
        static_cast<float>(height)
    };

    DrawRectangleRec(panel, Color{252, 252, 252, 255});
    DrawRectangleLinesEx(
        panel,
        simulation_config::graphs::chart_outline_thickness,
        LIGHTGRAY
    );

    DrawText(
        "Simulation Graphs",
        x + simulation_config::graphs::panel_padding,
        y + simulation_config::graphs::panel_padding,
        simulation_config::graphs::title_font_size,
        BLACK
    );

    if (history.empty()) {
        DrawText(
            "No stats yet",
            x + simulation_config::graphs::panel_padding,
            y + 44,
            simulation_config::graphs::text_font_size,
            GRAY
        );
        return;
    }

    const SimulationStats& latest = history.back();
    DrawText(
        TextFormat("Tick: %i  Live: %lli  Dead: %lli", latest.tick, latest.liveCells, latest.deadCells),
        x + simulation_config::graphs::panel_padding,
        y + 42,
        simulation_config::graphs::text_font_size,
        DARKGRAY
    );
    DrawText(
        TextFormat("Avg food: %.2f  Height: %i", latest.avgFood, latest.maxHeight),
        x + simulation_config::graphs::panel_padding,
        y + 62,
        simulation_config::graphs::text_font_size,
        DARKGRAY
    );
    DrawText(
        TextFormat("Antibiotic avg: %.2f  max: %.2f", latest.avgAntibiotic, latest.maxAntibiotic),
        x + simulation_config::graphs::panel_padding,
        y + 78,
        simulation_config::graphs::text_font_size,
        DARKBLUE
    );

    std::vector<float> liveCells;
    std::vector<float> deadCells;
    std::vector<float> totalFood;
    std::vector<float> avgFood;
    std::vector<float> maxHeight;
    std::vector<float> avgAntibiotic;
    std::vector<float> maxAntibiotic;

    liveCells.reserve(history.size());
    deadCells.reserve(history.size());
    totalFood.reserve(history.size());
    avgFood.reserve(history.size());
    maxHeight.reserve(history.size());
    avgAntibiotic.reserve(history.size());
    maxAntibiotic.reserve(history.size());

    for (const SimulationStats& stats : history) {
        liveCells.push_back(static_cast<float>(stats.liveCells));
        deadCells.push_back(static_cast<float>(stats.deadCells));
        totalFood.push_back(stats.totalFood);
        avgFood.push_back(stats.avgFood);
        maxHeight.push_back(static_cast<float>(stats.maxHeight));
        avgAntibiotic.push_back(stats.avgAntibiotic);
        maxAntibiotic.push_back(stats.maxAntibiotic);
    }

    const int chartsTop = y + simulation_config::graphs::header_bottom;
    const int chartsHeight = height - simulation_config::graphs::header_bottom -
        simulation_config::graphs::panel_padding;
    const int chartHeight = (chartsHeight - 4 * simulation_config::graphs::section_gap) / 5;
    const int chartWidth = width - 2 * simulation_config::graphs::panel_padding;

    drawSeries(
        history,
        Rectangle{
            static_cast<float>(x + simulation_config::graphs::panel_padding),
            static_cast<float>(chartsTop),
            static_cast<float>(chartWidth),
            static_cast<float>(chartHeight)
        },
        liveCells,
        GREEN,
        "Live",
        &deadCells,
        RED,
        "Dead"
    );
    drawSeries(
        history,
        Rectangle{
            static_cast<float>(x + simulation_config::graphs::panel_padding),
            static_cast<float>(chartsTop + chartHeight + simulation_config::graphs::section_gap),
            static_cast<float>(chartWidth),
            static_cast<float>(chartHeight)
        },
        totalFood,
        BROWN,
        "Total food"
    );
    drawSeries(
        history,
        Rectangle{
            static_cast<float>(x + simulation_config::graphs::panel_padding),
            static_cast<float>(
                chartsTop + 2 * (chartHeight + simulation_config::graphs::section_gap)
            ),
            static_cast<float>(chartWidth),
            static_cast<float>(chartHeight)
        },
        avgFood,
        ORANGE,
        "Avg food"
    );
    drawSeries(
        history,
        Rectangle{
            static_cast<float>(x + simulation_config::graphs::panel_padding),
            static_cast<float>(
                chartsTop + 3 * (chartHeight + simulation_config::graphs::section_gap)
            ),
            static_cast<float>(chartWidth),
            static_cast<float>(chartHeight)
        },
        maxHeight,
        BLUE,
        "Max height"
    );
    drawSeries(
        history,
        Rectangle{
            static_cast<float>(x + simulation_config::graphs::panel_padding),
            static_cast<float>(
                chartsTop + 4 * (chartHeight + simulation_config::graphs::section_gap)
            ),
            static_cast<float>(chartWidth),
            static_cast<float>(chartHeight)
        },
        avgAntibiotic,
        DARKBLUE,
        "Avg antibiotic",
        &maxAntibiotic,
        VIOLET,
        "Max antibiotic"
    );
}
