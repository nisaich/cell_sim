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

    const int y = height - simulation_config::colony::initial_cells_y_from_bottom;
    const int end_x = std::min(
        width,
        simulation_config::colony::initial_cells_start_x +
            simulation_config::colony::initial_cells_count
    );

    for (int x = simulation_config::colony::initial_cells_start_x; x < end_x; ++x) {
      simulation_field.place_cell(
        x,
        y,
        std::make_shared<active_Biomass>()
      );
    }

    visualize(simulation_field, "antibiotic");

    return 0;
}
