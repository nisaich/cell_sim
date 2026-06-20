#include <iostream>
#include <memory>
#include <thread>
#include <chrono>


#include "visualization.hpp"

#include "Field.hpp"
#include "Cell.hpp"

void visualize_field(const Field& current_field) {
    std::cout << "\033[2J\033[H";

    for (int y = 0; y < current_field.get_height(); ++y) {
        for (int x = 0; x < current_field.get_width(); ++x) {
            std::shared_ptr<abstract_Cell> cell =
                current_field.get_nucleus(x, y).get_cell();

            if (cell == nullptr) {
                //std::cout << '.';
                std::cout << "\033[33m.\033[0m";
            } else if (cell->is_alive()) {
                //std::cout << 'O';
                std::cout << "\033[32m0\033[0m";

            } else {
                //std::cout << 'x';
                std::cout << "\033[31mX\033[0m";

            }
        }

        std::cout << '\n';
    }

    std::cout << std::flush;

    // Задержка между кадрами.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

}
