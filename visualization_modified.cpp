#include "visualization.hpp"

#include "Field.hpp"
#include "Biomass.hpp"
#include "Graphs.hpp"
#include "SimulationConfig.hpp"

#include <raylib.h>

#include <algorithm>
#include <memory>
#include <string>
#include <cctype>
#include <chrono>
#include <vector>
#include <numeric>
#include <iostream>
#include <csignal>
#include <cstdlib>

namespace {
    std::chrono::high_resolution_clock::time_point start_simulation_time;
    long long total_ticks_counter = 0;

    void print_average_tick_time() {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> total_duration = end_time - start_simulation_time;
        double elapsed_seconds = total_duration.count();

        std::cout << "\n--- Simulation Performance Statistics ---" << std::endl;
        std::cout << "Total elapsed time: " << elapsed_seconds << " seconds" << std::endl;
        std::cout << "Total simulation ticks: " << total_ticks_counter << std::endl;
        if (total_ticks_counter > 0 && elapsed_seconds > 0.0) {
            double avg_ms = (elapsed_seconds / total_ticks_counter) * 1000.0;
            double ticks_per_sec = total_ticks_counter / elapsed_seconds;
            std::cout << "Average time per tick (end-to-end): " << avg_ms << " ms" << std::endl;
            std::cout << "Effective tick rate: " << ticks_per_sec << " ticks/second" << std::endl;
        }
        else {
            std::cout << "No simulation ticks calculated yet." << std::endl;
        }
    }

    void sigint_handler(int signal) {
        print_average_tick_time();
        std::exit(signal);
    }
}

enum class CellColorMode {
    Age,
    Resistance,
    Nutrition,
    Antibiotic
};

enum class CellShapeMode {
    Square,
};

class VisualizationCell {
protected:
    std::shared_ptr<abstract_Biomass> cell;
    double nutrition;
    double antibiotic;
    double resistance;
    int state_nucleus;
    int age;
    int max_age;

public:
    VisualizationCell(
        const std::shared_ptr<abstract_Biomass>& cell,
        double nutrition,
        double antibiotic,
        double resistance
    )
        : cell(cell),
        nutrition(nutrition),
        antibiotic(antibiotic),
        resistance(resistance),
        state_nucleus(0),
        age(0),
        max_age(0)
    {
        if (cell == nullptr) {
            state_nucleus = 0;
            return;
        }

        if (!cell->is_alive()) {
            state_nucleus = 3;
        }
        else if (std::dynamic_pointer_cast<nonactive_Biomass>(cell)) {
            state_nucleus = 2;
        }
        else {
            state_nucleus = 1;
        }

        age = cell->get_age();
        max_age = cell->max_age_of_cell;
    }

    virtual ~VisualizationCell() = default;

    virtual Color getColor() const = 0;

    double getNutrition() const {
        return nutrition;
    }

    double getAntibiotic() const {
        return antibiotic;
    }

    double getResistance() const {
        return resistance;
    }

protected:
    Color getBaseColor() const {
        switch (state_nucleus) {
        case 0:
            return Color{
                simulation_config::visualization::empty_cell_r,
                simulation_config::visualization::empty_cell_g,
                simulation_config::visualization::empty_cell_b,
                255
            };

        case 1:
            return Color{
                simulation_config::visualization::active_cell_r,
                simulation_config::visualization::active_cell_g,
                simulation_config::visualization::active_cell_b,
                255
            };

        case 2:
            return Color{
                simulation_config::visualization::nonactive_cell_r,
                simulation_config::visualization::nonactive_cell_g,
                simulation_config::visualization::nonactive_cell_b,
                255
            };

        case 3:
            return Color{
                simulation_config::visualization::dead_cell_r,
                simulation_config::visualization::dead_cell_g,
                simulation_config::visualization::dead_cell_b,
                255
            };

        default:
            return Color{
                simulation_config::visualization::empty_cell_r,
                simulation_config::visualization::empty_cell_g,
                simulation_config::visualization::empty_cell_b,
                255
            };
        }
    }

    Color applyBrightness(Color color, double brightness) const {
        brightness = std::clamp(brightness, 0.0, 1.0);
        return Color{
            static_cast<unsigned char>(color.r * brightness),
            static_cast<unsigned char>(color.g * brightness),
            static_cast<unsigned char>(color.b * brightness),
            color.a
        };
    }
};

class AgeColorCell : public VisualizationCell {
public:
    AgeColorCell(
        const std::shared_ptr<abstract_Biomass>& cell,
        double nutrition,
        double antibiotic,
        double resistance
    )
        : VisualizationCell(cell, nutrition, antibiotic, resistance) {}

    Color getColor() const override {
        Color baseColor = getBaseColor();
        if (cell == nullptr || !cell->is_alive()) {
            return baseColor;
        }
        double brightness = getBrightnessByAge();
        return applyBrightness(baseColor, brightness);
    }

private:
    double getBrightnessByAge() const {
        if (max_age == 0) return 1.0;
        double ageRatio = static_cast<double>(age) / static_cast<double>(max_age);
        ageRatio = std::clamp(ageRatio, 0.0, 1.0);
        ageRatio = std::sqrt(ageRatio);
        return simulation_config::visualization::min_brightness +
            (1.0 - ageRatio) * simulation_config::visualization::brightness_span;
    }
};

class ResistanceColorCell : public VisualizationCell {
public:
    ResistanceColorCell(
        const std::shared_ptr<abstract_Biomass>& cell,
        double nutrition,
        double antibiotic,
        double resistance
    )
        : VisualizationCell(cell, nutrition, antibiotic, resistance) {}

    Color getColor() const override {
        Color baseColor = getBaseColor();
        if (cell == nullptr || !cell->is_alive()) {
            return baseColor;
        }
        double brightness = simulation_config::visualization::min_brightness +
            resistance * simulation_config::visualization::brightness_span;
        return applyBrightness(baseColor, brightness);
    }
};

class NutritionColorCell : public VisualizationCell {
public:
    NutritionColorCell(
        const std::shared_ptr<abstract_Biomass>& cell,
        double nutrition,
        double antibiotic,
        double resistance
    )
        : VisualizationCell(cell, nutrition, antibiotic, resistance) {}

    Color getColor() const override {
        if (cell == nullptr) {
            return applyBrightness(
                Color{
                    simulation_config::visualization::empty_cell_blue_r,
                    simulation_config::visualization::empty_cell_blue_g,
                    simulation_config::visualization::empty_cell_blue_b,
                    255
                },
                nutrition
            );
        }

        if (!cell->is_alive()) {
            return getBaseColor();
        }

        double biomass_ratio = std::clamp(
            cell->get_biomass() / simulation_config::biomass::reproduction_min_biomass,
            0.0,
            1.0
        );
        double brightness = simulation_config::visualization::min_brightness +
            biomass_ratio * simulation_config::visualization::brightness_span;
        return applyBrightness(getBaseColor(), brightness);
    }
};

class AntibioticColorCell : public VisualizationCell {
public:
    AntibioticColorCell(
        const std::shared_ptr<abstract_Biomass>& cell,
        double nutrition,
        double antibiotic,
        double resistance
    )
        : VisualizationCell(cell, nutrition, antibiotic, resistance) {}

    Color getColor() const override {
        if (cell != nullptr) {
            Color base = getBaseColor();
            double conc = std::clamp(
                antibiotic / simulation_config::antibiotic::visualization_normalizer,
                0.0,
                1.0
            );
            double brightness = 1.0 - conc * 0.7;
            brightness = std::clamp(brightness, 0.3, 1.0);
            return applyBrightness(base, brightness);
        }
        else {
            double conc = std::clamp(
                antibiotic / simulation_config::antibiotic::visualization_normalizer,
                0.0,
                1.0
            );
            unsigned char intensity = static_cast<unsigned char>(conc * 255);
            return Color{ 0, 0, intensity, 255 };
        }
    }
};

class VisualizationBiomass {
private:
    std::string shape;

public:
    explicit VisualizationBiomass(const std::string& shape)
        : shape(shape) {}

    void draw(
        int width,
        int height,
        const std::vector<Cell>& field,
        float startX,
        float startY,
        float cellSize,
        CellColorMode colorMode
    ) const {
        if (shape != "square") {
            return;
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const Cell& nucleus = field[y * width + x];

                float drawX = startX + x * cellSize;
                float drawY = startY + y * cellSize;

                Color color = getCellColor(nucleus, colorMode);

                DrawRectangleRec(
                    Rectangle{ drawX, drawY, cellSize, cellSize },
                    color
                );
            }
        }
    }

protected:
    Color getCellColor(const Cell& nucleus, CellColorMode colorMode) const {
        std::shared_ptr<abstract_Biomass> cell = nucleus.get_cell();

        auto environment = nucleus.situation_in_the_environment();

        double foodInEnvironment = environment.first;
        double antibioticInEnvironment = environment.second;

        double nutrition = std::clamp(
            foodInEnvironment / simulation_config::visualization::modified_nutrition_normalizer,
            0.0,
            1.0
        );
        double antibiotic = std::clamp(
            antibioticInEnvironment,
            0.0,
            simulation_config::antibiotic::visualization_normalizer
        );

        double resistance = 0.0;
        if (cell != nullptr) {
            resistance = cell->get_level_of_resistance();
        }

        switch (colorMode) {
        case CellColorMode::Age: {
            AgeColorCell visual(cell, nutrition, antibiotic, resistance);
            return visual.getColor();
        }
        case CellColorMode::Resistance: {
            ResistanceColorCell visual(cell, nutrition, antibiotic, resistance);
            return visual.getColor();
        }
        case CellColorMode::Nutrition: {
            NutritionColorCell visual(cell, nutrition, antibiotic, resistance);
            return visual.getColor();
        }
        case CellColorMode::Antibiotic: {
            AntibioticColorCell visual(cell, nutrition, antibiotic, resistance);
            return visual.getColor();
        }
        default:
            return BLACK;
        }
    }
};

static void drawAntibioticLegend(int x, int y) {
    const int barWidth = simulation_config::visualization::legend_width;
    const int barHeight = simulation_config::visualization::legend_height;
    const int fontSize = simulation_config::visualization::legend_font_size;

    DrawRectangle(x - 6, y - 6, barWidth + 90, barHeight + 20, Color{ 255, 255, 255, 180 });

    for (int i = 0; i < barHeight; ++i) {
        float t = 1.0f - static_cast<float>(i) / static_cast<float>(barHeight - 1);
        unsigned char intensity = static_cast<unsigned char>(t * 255);
        DrawRectangle(x, y + i, barWidth, 1, Color{ 0, 0, intensity, 255 });
    }
    DrawRectangleLines(x, y, barWidth, barHeight, GRAY);

    DrawText(
        TextFormat("%.2f", simulation_config::antibiotic::visualization_normalizer),
        x + barWidth + 6, y - 2, fontSize, DARKGRAY
    );
    DrawText("0", x + barWidth + 6, y + barHeight - fontSize + 2, fontSize, DARKGRAY);
    DrawText("Antibiotic", x, y + barHeight + 4, fontSize, DARKGRAY);
}

static void drawPanelContainer(const char* title, int x, int y, int w, int h) {
    DrawRectangle(x, y, w, h, RAYWHITE);
    DrawRectangleLinesEx(Rectangle{ (float)x, (float)y, (float)w, (float)h }, 1.0f, LIGHTGRAY);
    DrawRectangle(x, y, w, 24, Color{ 240, 240, 240, 255 });
    DrawRectangleLinesEx(Rectangle{ (float)x, (float)y, (float)w, 24.0f }, 1.0f, LIGHTGRAY);
    DrawText(title, x + 8, y + 5, 14, DARKGRAY);
}

void visualize(Field& simulation_field) {
    std::signal(SIGINT, sigint_handler);

    int width = simulation_field.get_width();
    int height = simulation_field.get_height();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1200, 900, "Antibiotic Visualization");

    int monitor = GetCurrentMonitor();
    int windowPosX = (GetMonitorWidth(monitor) - 1200) / 2;
    int windowPosY = (GetMonitorHeight(monitor) - 900) / 2;
    SetWindowPosition(windowPosX, windowPosY);

    SetTargetFPS(simulation_config::visualization::target_fps);

    VisualizationBiomass visualizationBiomass("square");

    int tick = 0;

    // Запись CSV (раскомментировано)
    CsvStatsRecorder statsRecorder("simulation_stats.csv");
    if (statsRecorder.is_open()) {
        statsRecorder.record(simulation_field, tick);
    }

    start_simulation_time = std::chrono::high_resolution_clock::now();
    total_ticks_counter = 0;

    while (!WindowShouldClose()) {
        for (int i = 0; i < simulation_config::visualization::steps_per_frame; ++i) {
            if (simulation_field.has_living_cells()) {
                simulation_field.make_one_step(tick);
                ++tick;
                ++total_ticks_counter;
            }
            else {
                break;
            }
        }

        if (statsRecorder.is_open()) {
            statsRecorder.record(simulation_field, tick);
        }

        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        int panelX = 10;
        int panelY = 10;
        int panelW = screenWidth - 20;
        int panelH = screenHeight - 20;

        BeginDrawing();
        ClearBackground(Color{ 220, 220, 220, 255 });

        drawPanelContainer("Antibiotic Concentration", panelX, panelY, panelW, panelH);

        float cellSize = std::min(
            (float)(panelW - 16) / width,
            (float)(panelH - 24 - 16) / height
        );
        float startX = panelX + 8.0f + (panelW - 16 - width * cellSize) / 2.0f;
        float startY = panelY + 24.0f + 8.0f + (panelH - 24 - 16 - height * cellSize) / 2.0f;

        visualizationBiomass.draw(
            width, height,
            simulation_field.get_field(),
            startX, startY,
            cellSize,
            CellColorMode::Antibiotic
        );

        drawAntibioticLegend(panelX + 12, panelY + 36);

        EndDrawing();
    }

    CloseWindow();
    print_average_tick_time();
}