#include "Biomass.hpp"
#include "Field.hpp"
#include "SimulationConfig.hpp"
#include "visualization.hpp"

#include <memory>

int main() {
    const int width = simulation_config::field::width;
    const int height = simulation_config::field::height;

    Field simulation_field(width, height);
    simulation_field.init_environment(simulation_config::field::initial_food);

    // ---- 40 КЛЕТОК В ЦЕНТРЕ НИЖНЕЙ СТРОКИ ----
    const int y = height - 1;
    const int cells_count = 40;
    const int start_x = (width - cells_count) / 2;

    for (int x = start_x; x < start_x + cells_count; ++x) {
        simulation_field.place_cell(
            x,
            y,
            std::make_shared<active_Biomass>()
        );
    }
    // -----------------------------------------

    visualize(simulation_field, simulation_config::visualization::default_color_mode);

    return 0;
}