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

// Кодировка состояния:
// черный цвет - пустая 
// зеленый цвет - активная
// желтый цвет - неактивная
// красный цвет - мертвая


// ------------------------------------------------------------
// Режим окрашивания поля
// ------------------------------------------------------------
enum class CellColorMode {
    Age, // Возраст
    Resistance, // Устойчивость к антибиотикам (резистентность)
    Nutrition // Количество пищи
};


// ------------------------------------------------------------
// Форма ячеек поля
// ------------------------------------------------------------
enum class CellShapeMode {
    Square, // Квадрат
    // Hexagon // Шестиугольник
};

class Biomass;

// ----------------------------------------------------------------------------------------------------------
// Визуализация ячейки в зависимости от её состояния, возраста, количества пищи и устойчивости к антибиотикам
// ----------------------------------------------------------------------------------------------------------
class VisualizationCell {
protected:
    // Клетка, которая лежит внутри Cell
    std::shared_ptr<abstract_Biomass> cell;

    float nutrition;
    float antibiotic;
    float resistance;

    // 0 - пустая
    // 1 - активная
    // 2 - неактивная
    // 3 - мертвая
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
                return Color{0, 0, 0, 255};

            case 1:
                return Color{0, 255, 0, 255};

            case 2:
                return Color{255, 255, 0, 255};

            case 3:
                return Color{255, 0, 0, 255};

            default:
                return Color{0, 0, 0, 255};
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

// Окрашивание клетки в зависимости от возраста
class AgeColorCell : public VisualizationCell {
public:
    AgeColorCell(
        const std::shared_ptr<abstract_Biomass>& cell,
        float nutrition,
        float antibiotic,
        float resistance
    )
        : VisualizationCell(cell, nutrition, antibiotic, resistance)
    {
    }

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
        if (max_age == 0) {
            return 1.0f;
        }

        float ageRatio =
            static_cast<float>(age) /
            static_cast<float>(max_age);

        ageRatio = std::clamp(ageRatio, 0.0f, 1.0f);
        ageRatio = std::sqrt(ageRatio);

        float minBrightness = simulation_config::visualization::min_brightness;

        return minBrightness + (1.0f - ageRatio) *
            simulation_config::visualization::brightness_span;
    }
};

// Окрашивание клетки в зависимости от устойчивости к антибиотикам
class ResistanceColorCell : public VisualizationCell {
public:
    ResistanceColorCell(
        const std::shared_ptr<abstract_Biomass>& cell,
        float nutrition,
        float antibiotic,
        float resistance
    )
        : VisualizationCell(cell, nutrition, antibiotic, resistance)
    {
    }

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

// Окрашивание клетки в зависимости от количества пищи
class NutritionColorCell : public VisualizationCell {
public:
    NutritionColorCell(
        const std::shared_ptr<abstract_Biomass>& cell,
        float nutrition,
        float antibiotic,
        float resistance
    )
        : VisualizationCell(cell, nutrition, antibiotic, resistance)
    {
    }

    Color getColor() const override {
        Color baseColor = getBaseColor();

        if (cell == nullptr || !cell->is_alive()) {
            return baseColor;
        }

        float brightness = simulation_config::visualization::min_brightness +
            nutrition * simulation_config::visualization::brightness_span;

        return applyBrightness(baseColor, brightness);
    }
};


// ------------------------------------------------------------------------
// Визуализация ячейки в зависимости от её формы
// ------------------------------------------------------------------------

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
          colorMode(colorMode)
    {
    }

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
            Rectangle{
                x,
                y,
                sizeX,
                sizeY
            },
            color
        );
    }

    Color getCellColor(const Cell& nucleus) const {
        std::shared_ptr<abstract_Biomass> cell = nucleus.get_cell();

        auto environment = nucleus.situation_in_the_environment();

        float foodInEnvironment = environment.first;
        float antibioticInEnvironment = environment.second;

        float nutrition = std::clamp(
            foodInEnvironment / simulation_config::visualization::standard_nutrition_normalizer,
            0.0f,
            1.0f
        );
        float antibiotic = std::clamp(antibioticInEnvironment, 0.0f, 1.0f);

        float resistance = 0.0f;

        if (cell != nullptr) {
            resistance = cell->get_level_of_resistance();
        }

        switch (colorMode) {
            case CellColorMode::Age: {
                AgeColorCell visual(
                    cell,
                    nutrition,
                    antibiotic,
                    resistance
                );

                return visual.getColor();
            }

            case CellColorMode::Resistance: {
                ResistanceColorCell visual(
                    cell,
                    nutrition,
                    antibiotic,
                    resistance
                );

                return visual.getColor();
            }

            case CellColorMode::Nutrition: {
                NutritionColorCell visual(
                    cell,
                    nutrition,
                    antibiotic,
                    resistance
                );

                return visual.getColor();
            }

            default:
                return BLACK;
        }
    }
};


// ------------
// Визуализация
// ------------

// Определяем по какому состаянию будет окрашеваться ячейка
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

    return CellColorMode::Age;
}

// Вычисляем размер ячейки в пикселях 
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

    int width = simulation_field.get_width();
    int height = simulation_field.get_height();

    InitWindow(
        simulation_config::visualization::initial_window_width,
        simulation_config::visualization::initial_window_height,
        "Biomass visualization"
    );

    int monitor = GetCurrentMonitor();

    int screenWidth = GetMonitorWidth(monitor);
    int screenHeight = GetMonitorHeight(monitor);
    const int graphPanelWidth = simulation_config::visualization::graph_panel_width;

    SetWindowSize(screenWidth, screenHeight);
    SetWindowPosition(0, 0);
    ToggleFullscreen();

    SetTargetFPS(simulation_config::visualization::target_fps);

    float cellSize = calculateBiomassSize(
        width,
        height,
        screenWidth - graphPanelWidth - 2 * simulation_config::visualization::outer_margin,
        screenHeight
    );

    float fieldPixelWidth = width * cellSize;
    float fieldPixelHeight = height * cellSize;

    float startX = static_cast<float>(simulation_config::visualization::outer_margin);
    float startY = (screenHeight - fieldPixelHeight) / 2.0f;

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
        if (simulation_field.has_living_cells()) {
            simulation_field.make_one_step();
            ++tick;

            if (statsRecorder.is_open()) {
                statsRecorder.record(simulation_field, tick);
            }
            statsHistory.record(simulation_field, tick);
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        visualizationBiomass.draw(
            width,
            height,
            simulation_field.get_field(),
            startX,
            startY
        );
        statsHistory.draw(
            static_cast<int>(startX + fieldPixelWidth + 20.0f),
            simulation_config::visualization::outer_margin,
            graphPanelWidth,
            screenHeight - 2 * simulation_config::visualization::outer_margin
        );

        EndDrawing();
    }

    CloseWindow();
}
