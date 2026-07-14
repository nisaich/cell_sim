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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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
        } else {
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
    Antibiotic   // новый режим
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
        // Если клетка живая или мёртвая, показываем её цвет, но с наложением яркости от антибиотика
        if (cell != nullptr) {
            Color base = getBaseColor();
            // Яркость зависит от концентрации антибиотика (нормируем)
            double conc = std::clamp(
                antibiotic / simulation_config::antibiotic::visualization_normalizer,
                0.0,
                1.0
            );
            // Для живых клеток делаем цвет тусклее при высоком антибиотике (стресс)
            double brightness = 1.0 - conc * 0.7;  // от 1.0 до 0.3
            brightness = std::clamp(brightness, 0.3, 1.0);
            return applyBrightness(base, brightness);
        }
        else {
            // Пустая клетка – показываем концентрацию антибиотика в синих тонах
            double conc = std::clamp(
                antibiotic / simulation_config::antibiotic::visualization_normalizer,
                0.0,
                1.0
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



struct SharedCellState {
    double food;
    double antibiotic;
    int cell_type; // 0=empty, 1=active, 2=nonactive, 3=dead
    double biomass;
    int age;
    int max_age;
    double resistance;
};

struct SharedHeader {
    int width;
    int height;
    int tick;
    bool is_running;
};

static void exportSharedState(const Field& field, int tick, bool is_running) {
    std::string tmp_path = "/dev/shm/simulation_state.tmp";
    std::string dat_path = "/dev/shm/simulation_state.dat";

    std::ofstream out(tmp_path, std::ios::binary);
    if (!out.is_open()) return;

    SharedHeader header { field.get_width(), field.get_height(), tick, is_running };
    out.write(reinterpret_cast<const char*>(&header), sizeof(header));

    int totalCells = field.get_width() * field.get_height();
    std::vector<SharedCellState> states(totalCells);

    for (int y = 0; y < field.get_height(); ++y) {
        for (int x = 0; x < field.get_width(); ++x) {
            const Cell& nucleus = field.get_nucleus(x, y);
            const auto cell = nucleus.get_cell();
            auto env = nucleus.situation_in_the_environment();

            SharedCellState& state = states[y * field.get_width() + x];
            state.food = env.first;
            state.antibiotic = env.second;
            
            if (cell == nullptr) {
                state.cell_type = 0;
                state.biomass = 0.0;
                state.age = 0;
                state.max_age = 0;
                state.resistance = 0.0;
            } else {
                if (!cell->is_alive()) {
                    state.cell_type = 3;
                } else if (std::dynamic_pointer_cast<nonactive_Biomass>(cell)) {
                    state.cell_type = 2;
                } else {
                    state.cell_type = 1;
                }
                state.biomass = cell->get_biomass();
                state.age = cell->get_age();
                state.max_age = cell->max_age_of_cell;
                state.resistance = cell->get_level_of_resistance();
            }
        }
    }

    out.write(reinterpret_cast<const char*>(states.data()), states.size() * sizeof(SharedCellState));
    out.close();

    // Атомарное переименование файла, чтобы исключить чтение неполных данных
    std::rename(tmp_path.c_str(), dat_path.c_str());
}

class VisualizationBiomassShared {
private:
    std::string shape;

public:
    explicit VisualizationBiomassShared(const std::string& shape)
        : shape(shape) {}

    void draw(
        int width,
        int height,
        const std::vector<SharedCellState>& field_states,
        float startX,
        float startY,
        float cellSize,
        CellColorMode colorMode
    ) const {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const SharedCellState& state = field_states[y * width + x];

                float drawX = startX + x * cellSize;
                float drawY = startY + y * cellSize;

                Color color = getCellColor(state, colorMode);

                DrawRectangleRec(
                    Rectangle{ drawX, drawY, cellSize, cellSize },
                    color
                );
            }
        }
    }

private:
    Color getCellColor(const SharedCellState& state, CellColorMode colorMode) const {
        double nutrition = std::clamp(
            state.food / simulation_config::visualization::modified_nutrition_normalizer,
            0.0,
            1.0
        );
        double antibiotic = std::clamp(
            state.antibiotic,
            0.0,
            simulation_config::antibiotic::visualization_normalizer
        );

        // Calculate base color based on state_nucleus
        Color baseColor;
        switch (state.cell_type) {
        case 0:
            baseColor = Color{
                simulation_config::visualization::empty_cell_r,
                simulation_config::visualization::empty_cell_g,
                simulation_config::visualization::empty_cell_b,
                255
            };
            break;
        case 1:
            baseColor = Color{
                simulation_config::visualization::active_cell_r,
                simulation_config::visualization::active_cell_g,
                simulation_config::visualization::active_cell_b,
                255
            };
            break;
        case 2:
            baseColor = Color{
                simulation_config::visualization::nonactive_cell_r,
                simulation_config::visualization::nonactive_cell_g,
                simulation_config::visualization::nonactive_cell_b,
                255
            };
            break;
        case 3:
            baseColor = Color{
                simulation_config::visualization::dead_cell_r,
                simulation_config::visualization::dead_cell_g,
                simulation_config::visualization::dead_cell_b,
                255
            };
            break;
        default:
            baseColor = Color{ 0, 0, 0, 255 };
        }

        auto applyBrightness = [](Color color, double brightness) {
            brightness = std::clamp(brightness, 0.0, 1.0);
            return Color{
                static_cast<unsigned char>(color.r * brightness),
                static_cast<unsigned char>(color.g * brightness),
                static_cast<unsigned char>(color.b * brightness),
                color.a
            };
        };

        if (state.cell_type == 0 && colorMode == CellColorMode::Nutrition) {
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

        if (state.cell_type == 0 && colorMode == CellColorMode::Antibiotic) {
            unsigned char intensity = static_cast<unsigned char>(antibiotic * 255);
            return Color{ 0, 0, intensity, 255 };
        }

        if (state.cell_type > 0) {
            switch (colorMode) {
            case CellColorMode::Age: {
                if (state.max_age == 0) return baseColor;
                double ageRatio = static_cast<double>(state.age) / static_cast<double>(state.max_age);
                ageRatio = std::clamp(ageRatio, 0.0, 1.0);
                ageRatio = std::sqrt(ageRatio);
                double brightness = simulation_config::visualization::min_brightness +
                    (1.0 - ageRatio) * simulation_config::visualization::brightness_span;
                return applyBrightness(baseColor, brightness);
            }
            case CellColorMode::Resistance: {
                double brightness = simulation_config::visualization::min_brightness +
                    state.resistance * simulation_config::visualization::brightness_span;
                return applyBrightness(baseColor, brightness);
            }
            case CellColorMode::Nutrition: {
                double biomass_ratio = std::clamp(
                    state.biomass / simulation_config::biomass::reproduction_min_biomass,
                    0.0,
                    1.0
                );
                double brightness = simulation_config::visualization::min_brightness +
                    biomass_ratio * simulation_config::visualization::brightness_span;
                return applyBrightness(baseColor, brightness);
            }
            case CellColorMode::Antibiotic: {
                double brightness = 1.0 - antibiotic * 0.7;
                brightness = std::clamp(brightness, 0.3, 1.0);
                return applyBrightness(baseColor, brightness);
            }
            default:
                return baseColor;
            }
        }

        return baseColor;
    }
};

static void runViewer(const std::string& mode) {
    std::string title = "Simulation View";
    CellColorMode colorMode = CellColorMode::Nutrition;
    if (mode == "food") {
        title = "Food Concentration (Nutrition)";
        colorMode = CellColorMode::Nutrition;
    } else if (mode == "antibiotic") {
        title = "Antibiotic Concentration";
        colorMode = CellColorMode::Antibiotic;
    } else if (mode == "age") {
        title = "Cells (Age & State)";
        colorMode = CellColorMode::Age;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(400, 900, title.c_str());

    // Размещаем окна в зависимости от режима при открытии
    int monitor = GetCurrentMonitor();
    int screenW = GetMonitorWidth(monitor);
    int screenH = GetMonitorHeight(monitor);
    int startY = (screenH - 900) / 2;
    if (mode == "food") {
        SetWindowPosition((screenW - 1200) / 2, startY);
    } else if (mode == "antibiotic") {
        SetWindowPosition((screenW - 1200) / 2 + 410, startY);
    } else if (mode == "age") {
        SetWindowPosition((screenW - 1200) / 2 + 820, startY);
    }

    SetTargetFPS(60);

    VisualizationBiomassShared visualizer("square");

    SharedHeader last_header{0, 0, 0, false};
    std::vector<SharedCellState> last_states;
    bool has_cache = false;

    while (!WindowShouldClose()) {
        SharedHeader header{0, 0, 0, false};
        std::vector<SharedCellState> states;
        bool loaded = false;

        std::ifstream in("/dev/shm/simulation_state.dat", std::ios::binary);
        if (in.is_open()) {
            in.read(reinterpret_cast<char*>(&header), sizeof(header));
            if (in) {
                states.resize(header.width * header.height);
                in.read(reinterpret_cast<char*>(states.data()), states.size() * sizeof(SharedCellState));
                if (in) {
                    loaded = true;
                }
            }
            in.close();
        }

        if (loaded) {
            last_header = header;
            last_states = states;
            has_cache = true;
        }

        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (has_cache) {
            float cell = std::min((float)(screenWidth - 16) / last_header.width, (float)(screenHeight - 32 - 16) / last_header.height);
            float startX = 8.0f + (screenWidth - 16 - last_header.width * cell) / 2.0f;
            float startY = 32.0f + 8.0f + (screenHeight - 32 - 16 - last_header.height * cell) / 2.0f;

            visualizer.draw(last_header.width, last_header.height, last_states, startX, startY, cell, colorMode);
            
            std::string infoText = title + " | Tick: " + std::to_string(last_header.tick);
            if (!last_header.is_running) {
                infoText += " (FINISHED)";
            }
            DrawText(infoText.c_str(), 10, 8, 14, DARKGRAY);
        } else {
            DrawText("Waiting for simulation data...", screenWidth / 2 - 100, screenHeight / 2, 14, DARKGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
}

void visualize(
    Field& simulation_field,
    int argc,
    char* argv[]) {
    std::signal(SIGINT, sigint_handler);

    std::string viewer_mode = "";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--viewer" && i + 1 < argc) {
            viewer_mode = argv[i + 1];
            ++i;
        }
    }

    // Если запущен как вспомогательный вьюер, просто запускаем его окно
    if (!viewer_mode.empty()) {
        runViewer(viewer_mode);
        return;
    }

    // Иначе это главный координатор (запускает симуляцию + вьюер еды)
    unlink("/dev/shm/simulation_state.dat");

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

    // Запускаем окно для еды (Food) в главном процессе
    std::string title = "Food Concentration (Nutrition)";
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(400, 900, title.c_str());

    int monitor = GetCurrentMonitor();
    int screenW = GetMonitorWidth(monitor);
    int screenH = GetMonitorHeight(monitor);
    SetWindowPosition((screenW - 1200) / 2, (screenH - 900) / 2);

    SetTargetFPS(simulation_config::visualization::target_fps);

    VisualizationBiomassShared visualizer("square");

    const std::string statsPath = "simulation_stats.csv";
    CsvStatsRecorder statsRecorder(statsPath);
    StatsHistory statsHistory;
    int tick = 0;

    if (statsRecorder.is_open()) {
        statsRecorder.record(simulation_field, tick);
    }
    statsHistory.record(simulation_field, tick);

    start_simulation_time = std::chrono::high_resolution_clock::now();
    total_ticks_counter = 0;

    SharedHeader last_header{0, 0, 0, false};
    std::vector<SharedCellState> last_states;
    bool has_cache = false;

    while (!WindowShouldClose()) {
        bool was_running = simulation_field.has_living_cells();

        for (int i = 0; i < simulation_config::visualization::steps_per_frame; ++i) {
            if (simulation_field.has_living_cells()) {
                simulation_field.make_one_step(tick);
                ++tick;
                ++total_ticks_counter;
            } else {
                break;
            }
        }

        if (was_running) {
            if (statsRecorder.is_open()) {
                statsRecorder.record(simulation_field, tick);
            }
            statsHistory.record(simulation_field, tick);
        }

        // Экспортируем состояние в разделяемую память
        exportSharedState(simulation_field, tick, was_running);

        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Читаем собственное состояние для Food из памяти
        SharedHeader header{0, 0, 0, false};
        std::vector<SharedCellState> states;
        bool loaded = false;

        std::ifstream in("/dev/shm/simulation_state.dat", std::ios::binary);
        if (in.is_open()) {
            in.read(reinterpret_cast<char*>(&header), sizeof(header));
            if (in) {
                states.resize(header.width * header.height);
                in.read(reinterpret_cast<char*>(states.data()), states.size() * sizeof(SharedCellState));
                if (in) {
                    loaded = true;
                }
            }
            in.close();
        }

        if (loaded) {
            last_header = header;
            last_states = states;
            has_cache = true;
        }

        if (has_cache) {
            float cell = std::min((float)(screenWidth - 16) / last_header.width, (float)(screenHeight - 32 - 16) / last_header.height);
            float startX = 8.0f + (screenWidth - 16 - last_header.width * cell) / 2.0f;
            float startY = 32.0f + 8.0f + (screenHeight - 32 - 16 - last_header.height * cell) / 2.0f;

            visualizer.draw(last_header.width, last_header.height, last_states, startX, startY, cell, CellColorMode::Nutrition);
            
            std::string infoText = title + " | Tick: " + std::to_string(last_header.tick);
            if (!last_header.is_running) {
                infoText += " (FINISHED)";
            }
            DrawText(infoText.c_str(), 10, 8, 14, DARKGRAY);
        } else {
            DrawText("Running simulation steps...", screenWidth / 2 - 100, screenHeight / 2, 14, DARKGRAY);
        }

        EndDrawing();
    }

    CloseWindow();

    // Закрываем дочерние процессы-вьюеры при закрытии главного окна
    kill(pid1, SIGINT);
    kill(pid2, SIGINT);
    
    // Ждем их завершения
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);

    unlink("/dev/shm/simulation_state.dat");
    print_average_tick_time();
}
