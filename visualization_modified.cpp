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
    std::vector<double> tick_times;

    void print_average_tick_time() {
        if (tick_times.empty()) {
            std::cout << "\nNo simulation ticks calculated yet.\n" << std::endl;
            return;
        }
        double sum = std::accumulate(tick_times.begin(), tick_times.end(), 0.0);
        double avg = sum / tick_times.size();
        std::cout << "\n--- Simulation Performance Statistics ---" << std::endl;
        std::cout << "Total ticks: " << tick_times.size() << std::endl;
        std::cout << "Average time per tick: " << (avg * 1000.0) << " ms" << std::endl;
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
    Antibiotic   // новый режим
};

enum class CellShapeMode {
    Square,
};

class VisualizationCell {
protected:
    std::shared_ptr<abstract_Biomass> cell;
    float nutrition;
    float antibiotic;
    float resistance;
    int state_nucleus;
    int age;
    int max_age;

public:
    VisualizationCell(
        const std::shared_ptr<abstract_Biomass>& cell,
        float nutrition,
        float antibiotic,
        float resistance
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

    float getNutrition() const {
        return nutrition;
    }

    float getAntibiotic() const {
        return antibiotic;
    }

    float getResistance() const {
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

    Color applyBrightness(Color color, float brightness) const {
        brightness = std::clamp(brightness, 0.0f, 1.0f);
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
        float nutrition,
        float antibiotic,
        float resistance
    )
        : VisualizationCell(cell, nutrition, antibiotic, resistance) {}

    Color getColor() const override {
        Color baseColor = getBaseColor();
        if (cell == nullptr || !cell->is_alive()) {
            return baseColor;
        }
        float brightness = getBrightnessByAge();
        return applyBrightness(baseColor, brightness);
    }

private:
    float getBrightnessByAge() const {
        if (max_age == 0) return 1.0f;
        float ageRatio = static_cast<float>(age) / static_cast<float>(max_age);
        ageRatio = std::clamp(ageRatio, 0.0f, 1.0f);
        ageRatio = std::sqrt(ageRatio);
        return simulation_config::visualization::min_brightness +
            (1.0f - ageRatio) * simulation_config::visualization::brightness_span;
    }
};

class ResistanceColorCell : public VisualizationCell {
public:
    ResistanceColorCell(
        const std::shared_ptr<abstract_Biomass>& cell,
        float nutrition,
        float antibiotic,
        float resistance
    )
        : VisualizationCell(cell, nutrition, antibiotic, resistance) {}

    Color getColor() const override {
        Color baseColor = getBaseColor();
        if (cell == nullptr || !cell->is_alive()) {
            return baseColor;
        }
        float brightness = simulation_config::visualization::min_brightness +
            resistance * simulation_config::visualization::brightness_span;
        return applyBrightness(baseColor, brightness);
    }
};

class NutritionColorCell : public VisualizationCell {
public:
    NutritionColorCell(
        const std::shared_ptr<abstract_Biomass>& cell,
        float nutrition,
        float antibiotic,
        float resistance
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

        float biomass_ratio = std::clamp(
            static_cast<float>(cell->get_biomass() / simulation_config::biomass::reproduction_min_biomass),
            0.0f,
            1.0f
        );
        float brightness = simulation_config::visualization::min_brightness +
            biomass_ratio * simulation_config::visualization::brightness_span;
        return applyBrightness(getBaseColor(), brightness);
    }
};

// ---------- НОВЫЙ КЛАСС ДЛЯ АНТИБИОТИКА ----------
class AntibioticColorCell : public VisualizationCell {
public:
    AntibioticColorCell(
        const std::shared_ptr<abstract_Biomass>& cell,
        float nutrition,
        float antibiotic,
        float resistance
    )
        : VisualizationCell(cell, nutrition, antibiotic, resistance) {}

    Color getColor() const override {
        // Если клетка живая или мёртвая, показываем её цвет, но с наложением яркости от антибиотика
        if (cell != nullptr) {
            Color base = getBaseColor();
            // Яркость зависит от концентрации антибиотика (нормируем)
            float conc = std::clamp(
                static_cast<float>(antibiotic / simulation_config::antibiotic::visualization_normalizer),
                0.0f,
                1.0f
            );
            // Для живых клеток делаем цвет тусклее при высоком антибиотике (стресс)
            float brightness = 1.0f - conc * 0.7f;  // от 1.0 до 0.3
            brightness = std::clamp(brightness, 0.3f, 1.0f);
            return applyBrightness(base, brightness);
        }
        else {
            // Пустая клетка – показываем концентрацию антибиотика в синих тонах
            float conc = std::clamp(
                static_cast<float>(antibiotic / simulation_config::antibiotic::visualization_normalizer),
                0.0f,
                1.0f
            );
            // От чёрного (0) до ярко-синего (1)
            unsigned char intensity = static_cast<unsigned char>(conc * 255);
            return Color{ 0, 0, intensity, 255 };
        }
    }
};
// --------------------------------------------------

class VisualizationBiomass {
private:
    std::string shape;
    float sizeX;
    float sizeY;
    CellColorMode colorMode;

public:
    VisualizationBiomass(
        const std::string& shape,
        float sizeX,
        float sizeY,
        CellColorMode colorMode
    )
        : shape(shape),
        sizeX(sizeX),
        sizeY(sizeY),
        colorMode(colorMode) {}

    void draw(
        int width,
        int height,
        const std::vector<std::vector<Cell>>& field,
        float startX,
        float startY
    ) const {
        if (shape != "square") {
            return;
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const Cell& nucleus = field[y][x];

                float drawX = startX + x * sizeX;
                float drawY = startY + y * sizeY;

                Color color = getCellColor(nucleus);

                drawSquare(drawX, drawY, color);
            }
        }
    }

protected:
    void drawSquare(float x, float y, Color color) const {
        DrawRectangleRec(
            Rectangle{ x, y, sizeX, sizeY },
            color
        );
    }

    Color getCellColor(const Cell& nucleus) const {
        std::shared_ptr<abstract_Biomass> cell = nucleus.get_cell();

        auto environment = nucleus.situation_in_the_environment();

        double foodInEnvironment = environment.first;
        float antibioticInEnvironment = environment.second;

        float nutrition = std::clamp(
            static_cast<float>(foodInEnvironment / simulation_config::visualization::modified_nutrition_normalizer),
            0.0f,
            1.0f
        );
        float antibiotic = std::clamp(
            antibioticInEnvironment,
            0.0f,
            static_cast<float>(simulation_config::antibiotic::visualization_normalizer)
        );

        float resistance = 0.0f;
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

static CellColorMode getColorModeFromText(const std::string& colorMode) {
    if (colorMode == "age") {
        return CellColorMode::Age;
    }
    else if (colorMode == "resistance") {
        return CellColorMode::Resistance;
    }
    else if (colorMode == "nutrition") {
        return CellColorMode::Nutrition;
    }
    else if (colorMode == "antibiotic") {
        return CellColorMode::Antibiotic;
    }
    return CellColorMode::Age;
}

static void drawAntibioticLegend(int x, int y) {
    const int barWidth = simulation_config::visualization::legend_width;
    const int barHeight = simulation_config::visualization::legend_height;
    const int fontSize = simulation_config::visualization::legend_font_size;

    // Полупрозрачная подложка, чтобы легенда читалась над полем любого цвета
    DrawRectangle(x - 6, y - 6, barWidth + 90, barHeight + 20, Color{ 255, 255, 255, 180 });

    // Градиент "чёрный (0) -> синий (max)", как у пустых клеток в этом режиме
    for (int i = 0; i < barHeight; ++i) {
        float t = 1.0f - static_cast<float>(i) / static_cast<float>(barHeight - 1);
        unsigned char intensity = static_cast<unsigned char>(t * 255);
        DrawRectangle(x, y + i, barWidth, 1, Color{ 0, 0, intensity, 255 });
    }
    DrawRectangleLines(x, y, barWidth, barHeight, GRAY);

    DrawText(
        TextFormat("%.0f", simulation_config::antibiotic::visualization_normalizer),
        x + barWidth + 6, y - 2, fontSize, DARKGRAY
    );
    DrawText("0", x + barWidth + 6, y + barHeight - fontSize + 2, fontSize, DARKGRAY);
    DrawText("Antibiotic", x, y + barHeight + 4, fontSize, DARKGRAY);
}

static float calculateBiomassSize(
    int width,
    int height,
    int availableWidth,
    int screenHeight
) {
    float cellSizeByWidth =
        static_cast<float>(availableWidth) / static_cast<float>(width);
    float cellSizeByHeight =
        static_cast<float>(screenHeight) / static_cast<float>(height);
    return std::min(cellSizeByWidth, cellSizeByHeight);
}

void visualize(
    Field& simulation_field,
    const std::string& colorMode) {
    std::signal(SIGINT, sigint_handler);

    int width = simulation_field.get_width();
    int height = simulation_field.get_height();

    InitWindow(
        simulation_config::visualization::initial_window_width,
        simulation_config::visualization::initial_window_height,
        "Biomass visualization"
    );

    int monitor = GetCurrentMonitor();
    const int graphPanelWidth = simulation_config::visualization::graph_panel_width;
    const int contentGap = simulation_config::visualization::modified_content_gap;

    int maxScreenWidth = GetMonitorWidth(monitor) -
        simulation_config::visualization::modified_window_screen_margin;
    int maxScreenHeight = GetMonitorHeight(monitor) -
        simulation_config::visualization::modified_window_screen_margin;

    float cellSize = calculateBiomassSize(
        width,
        height,
        maxScreenWidth - graphPanelWidth - contentGap,
        maxScreenHeight
    );

    int windowWidth = static_cast<int>(width * cellSize) + graphPanelWidth + contentGap;
    int windowHeight = static_cast<int>(height * cellSize);

    SetWindowSize(windowWidth, windowHeight);

    int windowPosX = (GetMonitorWidth(monitor) - windowWidth) / 2;
    int windowPosY = (GetMonitorHeight(monitor) - windowHeight) / 2;
    SetWindowPosition(windowPosX, windowPosY);

    SetTargetFPS(simulation_config::visualization::target_fps);

    float startX = 0.0f;
    float startY = 0.0f;

    CellColorMode mode = getColorModeFromText(colorMode);

    VisualizationBiomass visualizationBiomass(
        "square",
        cellSize,
        cellSize,
        mode
    );
    const std::string statsPath = "simulation_stats.csv";
    CsvStatsRecorder statsRecorder(statsPath);
    StatsHistory statsHistory;
    int tick = 0;

    if (statsRecorder.is_open()) {
        statsRecorder.record(simulation_field, tick);
    }
    statsHistory.record(simulation_field, tick);

    while (!WindowShouldClose()) {
        for (int i = 0; i < simulation_config::visualization::steps_per_frame; ++i) {
            if (simulation_field.has_living_cells()) {
                auto start_time = std::chrono::high_resolution_clock::now();
                simulation_field.make_one_step(tick);
                auto end_time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> tick_duration = end_time - start_time;
                tick_times.push_back(tick_duration.count());

                ++tick;
            } else {
                break;
            }
        }

        if (statsRecorder.is_open()) {
            statsRecorder.record(simulation_field, tick);
        }
        statsHistory.record(simulation_field, tick);

        BeginDrawing();

        ClearBackground(RAYWHITE);

        visualizationBiomass.draw(
            width,
            height,
            simulation_field.get_field(),
            startX,
            startY
        );

        if (mode == CellColorMode::Antibiotic) {
            drawAntibioticLegend(
                simulation_config::visualization::legend_x,
                simulation_config::visualization::legend_y
            );
        }

        statsHistory.draw(
            static_cast<int>(width * cellSize + contentGap),
            0,
            graphPanelWidth,
            windowHeight
        );

        EndDrawing();
    }

    CloseWindow();
    print_average_tick_time();
}