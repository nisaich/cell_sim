#include "visualization.hpp"

#include "Field.hpp"
#include "Cell.hpp"

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
enum class NucleusColorMode {
    Age, // Возраст
    Resistance, // Устойчивость к антибиотикам (резистентность)
    Nutrition // Количество пищи
};


// ------------------------------------------------------------
// Форма ячеек поля
// ------------------------------------------------------------
enum class NucleusShapeMode {
    Square, // Квадрат
    // Hexagon // Шестиугольник
};

class Cell;

// ----------------------------------------------------------------------------------------------------------
// Визуализация ячейки в зависимости от её состояния, возраста, количества пищи и устойчивости к антибиотикам
// ----------------------------------------------------------------------------------------------------------
class VisualizationNucleus {
protected:
    // Клетка, которая лежит внутри Nucleus
    std::shared_ptr<abstract_Cell> cell;

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
    VisualizationNucleus(
        const std::shared_ptr<abstract_Cell>& cell,
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
        else if (std::dynamic_pointer_cast<nonactive_Cell>(cell)) {
            state_nucleus = 2;
        }
        else {
            state_nucleus = 1;
        }

        age = cell->get_age();
        max_age = cell->max_age_of_cell;
    }

    virtual ~VisualizationNucleus() = default;

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
class AgeColorNucleus : public VisualizationNucleus {
public:
    AgeColorNucleus(
        const std::shared_ptr<abstract_Cell>& cell,
        float nutrition,
        float antibiotic,
        float resistance
    )
        : VisualizationNucleus(cell, nutrition, antibiotic, resistance)
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

        float minBrightness = 0.35f;

        return minBrightness + (1.0f - ageRatio) * (1.0f - minBrightness);
    }
};

// Окрашивание клетки в зависимости от устойчивости к антибиотикам
class ResistanceColorNucleus : public VisualizationNucleus {
public:
    ResistanceColorNucleus(
        const std::shared_ptr<abstract_Cell>& cell,
        float nutrition,
        float antibiotic,
        float resistance
    )
        : VisualizationNucleus(cell, nutrition, antibiotic, resistance)
    {
    }

    Color getColor() const override {
        Color baseColor = getBaseColor();

        if (cell == nullptr || !cell->is_alive()) {
            return baseColor;
        }

        float brightness = 0.35f + resistance * 0.65f;

        return applyBrightness(baseColor, brightness);
    }
};

// Окрашивание клетки в зависимости от количества пищи
class NutritionColorNucleus : public VisualizationNucleus {
public:
    NutritionColorNucleus(
        const std::shared_ptr<abstract_Cell>& cell,
        float nutrition,
        float antibiotic,
        float resistance
    )
        : VisualizationNucleus(cell, nutrition, antibiotic, resistance)
    {
    }

    Color getColor() const override {
        Color baseColor = getBaseColor();

        if (cell == nullptr || !cell->is_alive()) {
            return baseColor;
        }

        float brightness = 0.35f + nutrition * 0.65f;

        return applyBrightness(baseColor, brightness);
    }
};


// ------------------------------------------------------------------------
// Визуализация ячейки в зависимости от её формы
// ------------------------------------------------------------------------

class VisualizationCell {
private:
    std::string shape;

    float sizeX;
    float sizeY;

    NucleusColorMode colorMode;

public:
    VisualizationCell(
        const std::string& shape,
        float sizeX,
        float sizeY,
        NucleusColorMode colorMode
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
        const std::vector<std::vector<Nucleus>>& field,
        float startX,
        float startY
    ) const {
        if (shape != "square") {
            return;
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const Nucleus& nucleus = field[y][x];

                float drawX = startX + x * sizeX;
                float drawY = startY + y * sizeY;

                Color color = getNucleusColor(nucleus);

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

    Color getNucleusColor(const Nucleus& nucleus) const {
        std::shared_ptr<abstract_Cell> cell = nucleus.get_cell();

        auto environment = nucleus.situation_in_the_environment();

        float foodInEnvironment = environment.first;
        float antibioticInEnvironment = environment.second;

        float nutrition = std::clamp(foodInEnvironment / 30.0f, 0.0f, 1.0f);
        float antibiotic = std::clamp(antibioticInEnvironment, 0.0f, 1.0f);

        float resistance = 0.0f;

        if (cell != nullptr) {
            resistance = cell->get_level_of_resistance();
        }

        switch (colorMode) {
            case NucleusColorMode::Age: {
                AgeColorNucleus visual(
                    cell,
                    nutrition,
                    antibiotic,
                    resistance
                );

                return visual.getColor();
            }

            case NucleusColorMode::Resistance: {
                ResistanceColorNucleus visual(
                    cell,
                    nutrition,
                    antibiotic,
                    resistance
                );

                return visual.getColor();
            }

            case NucleusColorMode::Nutrition: {
                NutritionColorNucleus visual(
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
static NucleusColorMode getColorModeFromText(const std::string& colorMode) {
    if (colorMode == "age") {
        return NucleusColorMode::Age;
    }
    else if (colorMode == "resistance") {
        return NucleusColorMode::Resistance;
    }
    else if (colorMode == "nutrition") {
        return NucleusColorMode::Nutrition;
    }

    return NucleusColorMode::Age;
}

// Вычисляем размер ячейки в пикселях 
static float calculateCellSize(
    int width,
    int height,
    int screenWidth,
    int screenHeight
) {
    float cellSizeByWidth =
        static_cast<float>(screenWidth) / static_cast<float>(width);

    float cellSizeByHeight =
        static_cast<float>(screenHeight) / static_cast<float>(height);

    return std::min(cellSizeByWidth, cellSizeByHeight);
}


void visualize(
    Field& simulation_field,
    const std::string& colorMode) {

    int width = simulation_field.get_width();
    int height = simulation_field.get_height();

    InitWindow(800, 600, "Cell visualization");

    int monitor = GetCurrentMonitor();

    int screenWidth = GetMonitorWidth(monitor);
    int screenHeight = GetMonitorHeight(monitor);

    SetWindowSize(screenWidth, screenHeight);
    SetWindowPosition(0, 0);
    ToggleFullscreen();

    SetTargetFPS(0);

    float cellSize = calculateCellSize(
        width,
        height,
        screenWidth,
        screenHeight
    );

    float fieldPixelWidth = width * cellSize;
    float fieldPixelHeight = height * cellSize;

    float startX = (screenWidth - fieldPixelWidth) / 2.0f;
    float startY = (screenHeight - fieldPixelHeight) / 2.0f;

    NucleusColorMode mode = getColorModeFromText(colorMode);

    VisualizationCell visualizationCell(
        "square",
        cellSize,
        cellSize,
        mode
    );

    while (!WindowShouldClose()) {
        if (simulation_field.has_living_cells()) {
            simulation_field.make_one_step();
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        visualizationCell.draw(
            width,
            height,
            simulation_field.get_field(),
            startX,
            startY
        );

        EndDrawing();
    }

    CloseWindow();
}


