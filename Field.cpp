#include "Field.hpp"

#include "Cell.hpp"

#include <memory>
#include <utility>

Nucleus::Nucleus(
    int x,
    int y,
    float start_food,
    float start_antibiotic
)
    : cell_coordinates{x, y},
      food(start_food),
      antibiotic(start_antibiotic) {}

bool Nucleus::is_this_nucleus_free() const {
    return cell == nullptr;
}

void Nucleus::set_cell(std::shared_ptr<abstract_Cell> new_cell) {
    cell = std::move(new_cell);
}

std::shared_ptr<abstract_Cell> Nucleus::get_cell() const {
    return cell;
}

void Nucleus::remove_cell() {
    cell = nullptr;
}

std::pair<float, float> Nucleus::situation_in_the_environment() const {
    return {
        food.get_amount(),
        antibiotic.get_concentration()
    };
}

std::array<int, 2> Nucleus::coordinates() const {
    return cell_coordinates;
}

Food& Nucleus::get_food() {
    return food;
}

const Food& Nucleus::get_food() const {
    return food;
}

Antibiotic& Nucleus::get_antibiotic() {
    return antibiotic;
}

const Antibiotic& Nucleus::get_antibiotic() const {
    return antibiotic;
}

Field::Field(int width, int height)
    : width(width),
      height(height),
      field(height) {
    for (int y = 0; y < height; ++y) {
        field[y].reserve(width);

        for (int x = 0; x < width; ++x) {
            field[y].emplace_back(x, y);
        }
    }
}

int Field::normalize_x(int x) const {
    return (x % width + width) % width;
}

bool Field::is_y_inside(int y) const {
    return y >= 0 && y < height;
}

int Field::get_width() const {
    return width;
}

int Field::get_height() const {
    return height;
}

Nucleus& Field::get_nucleus(int x, int y) {
    return field[y][normalize_x(x)];
}

const Nucleus& Field::get_nucleus(int x, int y) const {
    return field[y][normalize_x(x)];
}

std::vector<Nucleus*> Field::get_neighbours(int x, int y) {
    std::vector<Nucleus*> neighbours;

    const std::array<std::pair<int, int>, 4> directions = {
        std::pair<int, int>{0, -1},
        std::pair<int, int>{0, 1},
        std::pair<int, int>{-1, 0},
        std::pair<int, int>{1, 0}
    };

    for (const auto& direction : directions) {
        int neighbour_x = normalize_x(x + direction.first);
        int neighbour_y = y + direction.second;

        if (is_y_inside(neighbour_y)) {
            neighbours.push_back(&get_nucleus(neighbour_x, neighbour_y));
        }
    }

    return neighbours;
}

std::vector<Nucleus*> Field::get_free_neighbours(int x, int y) {
    std::vector<Nucleus*> free_neighbours;

    for (Nucleus* neighbour : get_neighbours(x, y)) {
        if (neighbour->is_this_nucleus_free()) {
            free_neighbours.push_back(neighbour);
        }
    }

    return free_neighbours;
}

bool Field::place_cell(int x, int y, std::shared_ptr<abstract_Cell> cell) {
    if (!is_y_inside(y)) {
        return false;
    }

    Nucleus& nucleus = get_nucleus(x, y);

    if (!nucleus.is_this_nucleus_free()) {
        return false;
    }

    nucleus.set_cell(std::move(cell));

    return true;
}

bool Field::has_living_cells() const {
    for (const auto& row : field) {
        for (const Nucleus& nucleus : row) {
            std::shared_ptr<abstract_Cell> cell = nucleus.get_cell();

            if (cell != nullptr && cell->is_alive()) {
                return true;
            }
        }
    }

    return false;
}

void Field::process_dead_cells_disappearance() {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Nucleus& nucleus = get_nucleus(x, y);
            std::shared_ptr<abstract_Cell> cell = nucleus.get_cell();

            if (cell == nullptr) {
                continue;
            }

            if (cell->is_alive()) {
                continue;
            }

            cell->step_after_death();

            if (cell->should_be_removed_from_field()) {
                nucleus.remove_cell();
            }
        }
    }
}

void Field::make_one_step() {
    std::vector<std::pair<int, int>> cells_for_this_step;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            std::shared_ptr<abstract_Cell> cell = field[y][x].get_cell();

            if (cell != nullptr && cell->is_alive()) {
                cells_for_this_step.emplace_back(x, y);
            }
        }
    }

    for (const auto& position : cells_for_this_step) {
        Nucleus& nucleus = get_nucleus(position.first, position.second);
        std::shared_ptr<abstract_Cell> cell = nucleus.get_cell();

        if (cell == nullptr || !cell->is_alive()) {
            continue;
        }

        if (cell->must_he_die()) {
            nucleus.set_cell(std::make_shared<dead_Cell>());
        } else {
            cell->increase_age();
        }
    }

    for (const auto& position : cells_for_this_step) {
        Nucleus& nucleus = get_nucleus(position.first, position.second);
        std::shared_ptr<abstract_Cell> cell = nucleus.get_cell();

        if (cell != nullptr && cell->is_alive()) {
            cell->food_consumption_from_environment(nucleus.get_food());
        }
    }

    for (const auto& position : cells_for_this_step) {
        std::shared_ptr<abstract_Cell> cell =
            get_nucleus(position.first, position.second).get_cell();

        if (cell != nullptr && cell->is_alive()) {
            cell->depletion_of_savings();
        }
    }

    for (const auto& position : cells_for_this_step) {
        std::shared_ptr<abstract_Cell> cell =
            get_nucleus(position.first, position.second).get_cell();

        if (cell != nullptr && cell->is_alive()) {
            cell->reproduction(*this, position.first, position.second);
        }
    }

  process_dead_cells_disappearance();

}


const std::vector<std::vector<Nucleus>>& Field::get_field() const {
    return field;
}
