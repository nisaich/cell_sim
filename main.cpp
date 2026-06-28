#include "Biomass.hpp"
#include "Field.hpp"
#include "visualization.hpp"

#include <memory>

int main() {
  //задаём начальные параметры:
    //размер поля:
    const int width = 80;
    const int height = 60;

    Field simulation_field(width, height);

    //начальные клетки
    int y=0;
    for (int x = width/2; x < width/2+1; ++x) {
        simulation_field.place_cell(
            x,
            height-1,
            std::make_shared<active_Biomass>()
        );
        y++;
    }

    //непосредственная симуляция
    visualize(simulation_field, "age");

    return 0;
}
