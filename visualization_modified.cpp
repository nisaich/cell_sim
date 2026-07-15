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
// #include <csignal>   // отключаем, т.к. не используем sigint_handler
// #include <cstdlib>
// #include <unistd.h>   // не используется в Windows
// #include <sys/types.h>
// #include <sys/wait.h> // уже закомментирован

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

    // sigint_handler удалён – не используется
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

// ---------- НОВЫЙ КЛАСС ДЛЯ АНТИБИОТИКА ----------
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
// --------------------------------------------------

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
            antibioticInEnvironment / simulation_config::antibiotic::visualization_normalizer,
            0.0,
            1.0
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

// =========================================================================
// ========== УПРОЩЁННАЯ ВЕРСИЯ ДЛЯ WINDOWS (без форков) =================
// =========================================================================
void visualize(
    Field& simulation_field,
    int argc,
    char* argv[]) {

    // ========== ЗАКОММЕНТИРОВАН ВЕСЬ БЛОК С fork/execvp/kill/waitpid ==========
    /*
    std::signal(SIGINT, sigint_handler);

    std::string viewer_mode = "";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--viewer" && i + 1 < argc) {
            viewer_mode = argv[i + 1];
            ++i;
        }
    }

    if (!viewer_mode.empty()) {
        runViewer(viewer_mode);
        return;
    }

    unlink("/dev/shm/simulation_state.dat");
    exportSharedState(simulation_field, 0, true);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        char* child_argv[] = { argv[0], (char*)"--viewer", (char*)"antibiotic", nullptr };
        execvp(child_argv[0], child_argv);
        std::exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        char* child_argv[] = { argv[0], (char*)"--viewer", (char*)"age", nullptr };
        execvp(child_argv[0], child_argv);
        std::exit(1);
    }
    */
    // ========== КОНЕЦ ЗАКОММЕНТИРОВАННОГО БЛОКА ==========

    // ========== ВМЕСТО ЭТОГО – ПРОСТОЙ ЦИКЛ С ОДНИМ ОКНОМ ==========
    int width = simulation_field.get_width();
    int height = simulation_field.get_height();

    float defaultCellSize = 4.0f;
    int windowW = static_cast<int>(width * defaultCellSize + 16);
    int windowH = static_cast<int>(height * defaultCellSize + 48);

    std::string title = "Food Concentration (Nutrition)";
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(windowW, windowH, title.c_str());

    SetTargetFPS(simulation_config::visualization::target_fps);

    VisualizationBiomass visualizer("square");

    const std::string statsPath = "simulation_stats.csv";
    CsvStatsRecorder statsRecorder(statsPath);
    int tick = 0;

    if (statsRecorder.is_open()) {
        statsRecorder.record(simulation_field, tick);
    }

    start_simulation_time = std::chrono::high_resolution_clock::now();
    total_ticks_counter = 0;

    while (!WindowShouldClose()) {
        bool was_running = simulation_field.has_living_cells();

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

        if (was_running) {
            if (statsRecorder.is_open()) {
                statsRecorder.record(simulation_field, tick);
            }
        }

        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Получаем вектор ячеек напрямую из поля
        std::vector<Cell> cells;
        cells.reserve(width * height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                cells.push_back(simulation_field.get_nucleus(x, y));
            }
        }

        float cell = std::min((float)(screenWidth - 16) / width, (float)(screenHeight - 32 - 16) / height);
        float startX = 8.0f + (screenWidth - 16 - width * cell) / 2.0f;
        float startY = 32.0f + 8.0f + (screenHeight - 32 - 16 - height * cell) / 2.0f;

        // Отрисовка – режим Nutrition (можно поменять на Antibiotic)
        visualizer.draw(width, height, cells, startX, startY, cell, CellColorMode::Nutrition);

        std::string infoText = title + " | Tick: " + std::to_string(tick);
        if (!simulation_field.has_living_cells()) {
            infoText += " (FINISHED)";
        }
        DrawText(infoText.c_str(), 10, 8, 14, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();

    // Закомментирован вызов kill/waitpid
    // kill(pid1, SIGINT);
    // kill(pid2, SIGINT);
    // waitpid(pid1, nullptr, 0);
    // waitpid(pid2, nullptr, 0);
    // unlink("/dev/shm/simulation_state.dat");

    print_average_tick_time();
}