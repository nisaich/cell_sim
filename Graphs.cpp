#include "Graphs.hpp"
#include "Biomass.hpp"
#include "SimulationConfig.hpp"

#include <raylib.h>

#include <algorithm>
#include <cmath>

namespace {
template<typename GetterA, typename GetterB>
void drawSeries(
    const std::vector<SimulationStats>& history,
    size_t start_idx,
    Rectangle area,
    GetterA getValueA,
    Color colorA,
    const char* labelA,
    GetterB getValueB,
    Color colorB,
    const char* labelB
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

    if (history.size() - start_idx < 2) {
        DrawText(
            "Not enough data",
            area.x + 10,
            area.y + 10,
            simulation_config::graphs::text_font_size,
            GRAY
        );
        return;
    }

    size_t range_size = history.size() - start_idx;

    float minValue = getValueA(history[start_idx]);
    float maxValue = getValueA(history[start_idx]);

    for (size_t i = start_idx; i < history.size(); ++i) {
        float valA = getValueA(history[i]);
        float valB = getValueB(history[i]);
        minValue = std::min(minValue, std::min(valA, valB));
        maxValue = std::max(maxValue, std::max(valA, valB));
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
    DrawText(labelB, left + 110.0f, area.y + 6.0f, 14, colorB);

    DrawLine(left, bottom, right, bottom, GRAY);
    DrawLine(left, top, left, bottom, GRAY);

    auto drawLineForValues = [&](auto getValue, Color color) {
        size_t step = std::max<size_t>(1, range_size / static_cast<size_t>(graphWidth));
        size_t prev_idx = start_idx;
        for (size_t i = start_idx + step; i < history.size(); i += step) {
            if (i + step >= history.size()) {
                i = history.size() - 1; // ensure we draw the last point
            }
            const float prevX = left + graphWidth * static_cast<float>(prev_idx - start_idx) /
                static_cast<float>(range_size - 1);
            const float nextX = left + graphWidth * static_cast<float>(i - start_idx) /
                static_cast<float>(range_size - 1);

            const float prevY = bottom - graphHeight * (getValue(history[prev_idx]) - minValue) /
                (maxValue - minValue);
            const float nextY = bottom - graphHeight * (getValue(history[i]) - minValue) /
                (maxValue - minValue);

            DrawLineEx(
                Vector2{prevX, prevY},
                Vector2{nextX, nextY},
                simulation_config::graphs::chart_line_thickness,
                color
            );
            prev_idx = i;
            
            if (i == history.size() - 1) break;
        }
    };

    drawLineForValues(getValueA, colorA);
    drawLineForValues(getValueB, colorB);

    float nowValueA = getValueA(history.back());
    float nowValueB = getValueB(history.back());

    const std::string maxText = TextFormat("max %.1f", maxValue);
    const std::string minText = TextFormat("min %.1f", minValue);
    const std::string nowText = TextFormat("now %.1f / %.1f", nowValueA, nowValueB);
    DrawText(maxText.c_str(), right - 90.0f, area.y + 6.0f, 14, DARKGRAY);
    DrawText(minText.c_str(), right - 90.0f, area.y + area.height - 18.0f, 14, DARKGRAY);
    DrawText(nowText.c_str(), right - 220.0f, area.y + 6.0f, 14, DARKGRAY);
}

template<typename GetterA>
void drawSeries(
    const std::vector<SimulationStats>& history,
    size_t start_idx,
    Rectangle area,
    GetterA getValueA,
    Color colorA,
    const char* labelA
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

    if (history.size() - start_idx < 2) {
        DrawText(
            "Not enough data",
            area.x + 10,
            area.y + 10,
            simulation_config::graphs::text_font_size,
            GRAY
        );
        return;
    }

    size_t range_size = history.size() - start_idx;

    float minValue = getValueA(history[start_idx]);
    float maxValue = getValueA(history[start_idx]);

    for (size_t i = start_idx; i < history.size(); ++i) {
        float valA = getValueA(history[i]);
        minValue = std::min(minValue, valA);
        maxValue = std::max(maxValue, valA);
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

    DrawLine(left, bottom, right, bottom, GRAY);
    DrawLine(left, top, left, bottom, GRAY);

    auto drawLineForValues = [&](auto getValue, Color color) {
        size_t step = std::max<size_t>(1, range_size / static_cast<size_t>(graphWidth));
        size_t prev_idx = start_idx;
        for (size_t i = start_idx + step; i < history.size(); i += step) {
            if (i + step >= history.size()) {
                i = history.size() - 1; // ensure we draw the last point
            }
            const float prevX = left + graphWidth * static_cast<float>(prev_idx - start_idx) /
                static_cast<float>(range_size - 1);
            const float nextX = left + graphWidth * static_cast<float>(i - start_idx) /
                static_cast<float>(range_size - 1);

            const float prevY = bottom - graphHeight * (getValue(history[prev_idx]) - minValue) /
                (maxValue - minValue);
            const float nextY = bottom - graphHeight * (getValue(history[i]) - minValue) /
                (maxValue - minValue);

            DrawLineEx(
                Vector2{prevX, prevY},
                Vector2{nextX, nextY},
                simulation_config::graphs::chart_line_thickness,
                color
            );
            prev_idx = i;
            
            if (i == history.size() - 1) break;
        }
    };

    drawLineForValues(getValueA, colorA);

    float nowValueA = getValueA(history.back());

    const std::string maxText = TextFormat("max %.1f", maxValue);
    const std::string minText = TextFormat("min %.1f", minValue);
    const std::string nowText = TextFormat("now %.1f", nowValueA);
    DrawText(maxText.c_str(), right - 90.0f, area.y + 6.0f, 14, DARKGRAY);
    DrawText(minText.c_str(), right - 90.0f, area.y + area.height - 18.0f, 14, DARKGRAY);
    DrawText(nowText.c_str(), right - 220.0f, area.y + 6.0f, 14, DARKGRAY);
}
} // namespace

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

    int text_y = y + simulation_config::graphs::panel_padding;
    DrawText(
        "Simulation Graphs",
        x + simulation_config::graphs::panel_padding,
        text_y,
        simulation_config::graphs::title_font_size,
        BLACK
    );
    text_y += 18;

    if (history.empty()) {
        DrawText(
            "No stats yet",
            x + simulation_config::graphs::panel_padding,
            text_y,
            simulation_config::graphs::text_font_size,
            GRAY
        );
        return;
    }

    const SimulationStats& latest = history.back();
    DrawText(
        TextFormat("Tick: %i  Live: %lli  Dead: %lli", latest.tick, latest.liveCells, latest.deadCells),
        x + simulation_config::graphs::panel_padding,
        text_y,
        simulation_config::graphs::text_font_size,
        DARKGRAY
    );
    text_y += 16;

    DrawText(
        TextFormat("Avg food: %.2f  Height: %i", latest.avgFood, latest.maxHeight),
        x + simulation_config::graphs::panel_padding,
        text_y,
        simulation_config::graphs::text_font_size,
        DARKGRAY
    );
    text_y += 16;

    DrawText(
        TextFormat("Antibiotic avg: %.5f  max: %.5f", latest.avgAntibiotic, latest.maxAntibiotic),
        x + simulation_config::graphs::panel_padding,
        text_y,
        simulation_config::graphs::text_font_size,
        DARKBLUE
    );
    text_y += 16;

    DrawText(
        TextFormat("Avg biomass: %.3f", latest.avgBiomass),
        x + simulation_config::graphs::panel_padding,
        text_y,
        simulation_config::graphs::text_font_size,
        DARKGREEN
    );
    text_y += 16;

    const int chartsTop = text_y + 10;
    const int chartsHeight = height - (chartsTop - y) - simulation_config::graphs::panel_padding;
    const int chartHeight = (chartsHeight - 5 * simulation_config::graphs::section_gap) / 6;
    const int chartWidth = width - 2 * simulation_config::graphs::panel_padding;

    size_t start_idx = 0;
    if (history.size() > simulation_config::graphs::max_displayed_points) {
        start_idx = history.size() - simulation_config::graphs::max_displayed_points;
    }

    drawSeries(
        history,
        start_idx,
        Rectangle{
            static_cast<float>(x + simulation_config::graphs::panel_padding),
            static_cast<float>(chartsTop),
            static_cast<float>(chartWidth),
            static_cast<float>(chartHeight)
        },
        [](const SimulationStats& s) { return static_cast<float>(s.liveCells); },
        GREEN,
        "Live",
        [](const SimulationStats& s) { return static_cast<float>(s.deadCells); },
        RED,
        "Dead"
    );
    drawSeries(
        history,
        start_idx,
        Rectangle{
            static_cast<float>(x + simulation_config::graphs::panel_padding),
            static_cast<float>(chartsTop + chartHeight + simulation_config::graphs::section_gap),
            static_cast<float>(chartWidth),
            static_cast<float>(chartHeight)
        },
        [](const SimulationStats& s) { return s.totalFood; },
        BROWN,
        "Total food"
    );
    drawSeries(
        history,
        start_idx,
        Rectangle{
            static_cast<float>(x + simulation_config::graphs::panel_padding),
            static_cast<float>(
                chartsTop + 2 * (chartHeight + simulation_config::graphs::section_gap)
            ),
            static_cast<float>(chartWidth),
            static_cast<float>(chartHeight)
        },
        [](const SimulationStats& s) { return s.avgFood; },
        ORANGE,
        "Avg food"
    );
    drawSeries(
        history,
        start_idx,
        Rectangle{
            static_cast<float>(x + simulation_config::graphs::panel_padding),
            static_cast<float>(
                chartsTop + 3 * (chartHeight + simulation_config::graphs::section_gap)
            ),
            static_cast<float>(chartWidth),
            static_cast<float>(chartHeight)
        },
        [](const SimulationStats& s) { return static_cast<float>(s.maxHeight); },
        BLUE,
        "Max height"
    );
    drawSeries(
        history,
        start_idx,
        Rectangle{
            static_cast<float>(x + simulation_config::graphs::panel_padding),
            static_cast<float>(
                chartsTop + 4 * (chartHeight + simulation_config::graphs::section_gap)
            ),
            static_cast<float>(chartWidth),
            static_cast<float>(chartHeight)
        },
        [](const SimulationStats& s) { return s.avgAntibiotic; },
        DARKBLUE,
        "Avg antibiotic",
        [](const SimulationStats& s) { return s.maxAntibiotic; },
        VIOLET,
        "Max antibiotic"
    );
    drawSeries(
        history,
        start_idx,
        Rectangle{
            static_cast<float>(x + simulation_config::graphs::panel_padding),
            static_cast<float>(
                chartsTop + 5 * (chartHeight + simulation_config::graphs::section_gap)
            ),
            static_cast<float>(chartWidth),
            static_cast<float>(chartHeight)
        },
        [](const SimulationStats& s) { return s.avgBiomass; },
        DARKGREEN,
        "Avg biomass"
    );
}
