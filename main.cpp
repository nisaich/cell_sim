#include "Biomass.hpp"
#include "Field.hpp"
#include "SimulationConfig.hpp"
#include "visualization.hpp"

#include <algorithm>
#include <memory>
#include <vector>

int main() {
    const int width = simulation_config::field::width;
    const int height = simulation_config::field::height;

    Field simulation_field(width, height);
    simulation_field.init_environment(simulation_config::field::initial_food);

    // ---- ОДИНОЧНЫЕ ОЧАГИ ВДОЛЬ НИЖНЕЙ ГРАНИЦЫ ----
    const int y = height - 1;
    const int groups = 6;                // меньше очагов, чтобы не сливались
    const int spacing = width / (groups + 1);

    for (int g = 1; g <= groups; ++g) {
        int x = g * spacing;
        x = std::clamp(x, 0, width - 1);
        simulation_field.place_cell(x, y, std::make_shared<active_Biomass>());
        // Можно добавить по второй клетке рядом, но оставляем одиночные для максимального ветвления
    }
    // -------------------------------------------------

    visualize(simulation_field);
    return 0;
}