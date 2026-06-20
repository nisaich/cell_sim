#include "Cell.hpp"
#include "Field.hpp"
#include "visualization.hpp"

#include <memory>

int main() {
  //задаём начальные параметры:
    //размер поля:
    const int width = 100;
    const int height = 50;

    Field simulation_field(width, height);

    //начальные клетки
    int y=0;
    for (int x = width/2; x < width/2+1; ++x) {
        simulation_field.place_cell(
            x,
            height-1,
            std::make_shared<active_Cell>()
        );
        y++;
    }

    //непосредственная симуляция
    while (simulation_field.has_living_cells()) {
        simulation_field.make_one_step();

        visualize_field(simulation_field);
    }

    return 0;
}
