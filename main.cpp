#include "Biomass.hpp"
#include "Field.hpp"
#include "SimulationConfig.hpp"
#include "visualization.hpp"

#include <memory>
#include <vector>

int main() {
    const int width = simulation_config::field::width;
    const int height = simulation_config::field::height;

    Field simulation_field(width, height);
    simulation_field.init_environment(simulation_config::field::initial_food);

    // ---- ТРИ УЧАСТКА ПО 5 КЛЕТОК НА НИЖНЕЙ ГРАНИЦЕ ----
    const int y = height - 1;  // нижняя строка
    const int cells_per_group = 5;
    const int groups = 3;
    const int total_width = cells_per_group * groups; // 15
    const int spacing = (width - total_width) / (groups + 1); // отступ между группами

    for (int g = 0; g < groups; ++g) {
        int start_x = spacing + g * (cells_per_group + spacing);
        if (start_x + cells_per_group > width) break;
        for (int x = start_x; x < start_x + cells_per_group; ++x) {
            simulation_field.place_cell(
                x,
                y,
                std::make_shared<active_Biomass>()
            );
        }
    }
    // -----------------------------------------------------

    visualize(simulation_field);

    return 0;
}