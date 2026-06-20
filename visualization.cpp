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
          std::cout << "\033[36m.\033[0m";
        } 
        else if (std::dynamic_pointer_cast<active_Cell>(cell) != nullptr) {
          std::cout << "\033[32mA\033[0m";
        } 
        else if (std::dynamic_pointer_cast<nonactive_Cell>(cell) != nullptr) {
          std::cout << "\033[33mN\033[0m";
        }
        else if (std::dynamic_pointer_cast<dead_Cell>(cell) != nullptr) {
          std::cout << "\033[31mD\033[0m";
        }
      }

        std::cout << '\n';
    }

    std::cout << std::flush;

    // Задержка между кадрами.
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

}
