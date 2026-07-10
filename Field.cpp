#include "Field.hpp"
#include "Biomass.hpp"
#include <memory>
#include <utility>

// ----- Cell -----
Cell::Cell(
    int x,
    int y,
    float start_food,
    float start_antibiotic
)
    : cell_coordinates{ x, y },
    food(start_food),
    antibiotic(start_antibiotic) {}

bool Cell::is_this_nucleus_free() const {
    return cell == nullptr;
}

void Cell::set_cell(std::shared_ptr<abstract_Biomass> new_cell) {
    cell = std::move(new_cell);
}

std::shared_ptr<abstract_Biomass> Cell::get_cell() const {
    return cell;
}

void Cell::remove_cell() {
    cell = nullptr;
}

std::pair<float, float> Cell::situation_in_the_environment() const {
    return {
        food.get_amount(),
        antibiotic.get_concentration()
    };
}

std::array<int, 2> Cell::coordinates() const {
    return cell_coordinates;
}

Food& Cell::get_food() {
    return food;
}

const Food& Cell::get_food() const {
    return food;
}

Antibiotic& Cell::get_antibiotic() {
    return antibiotic;
}

const Antibiotic& Cell::get_antibiotic() const {
    return antibiotic;
}

// ----- Field -----
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

void Field::init_environment(float initial_food) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            get_nucleus(x, y).get_food().set_amount(initial_food);
            get_nucleus(x, y).get_antibiotic().set_concentration(0.0f);
        }
    }
}

void Field::add_some_food(int count_of_adding_food) {
    // Точечный источник в центре верхней строки
    int center = width / 2;
    get_nucleus(center, 0).get_food().add(count_of_adding_food);
}

void Field::add_antibiotic(float concentration, int x, int y) {
    if (is_x_inside(x) && is_y_inside(y)) {
        get_nucleus(x, y).get_antibiotic().add(concentration);
    }
}

void Field::diffuse_food() {
    std::vector<std::vector<float>> new_food(height, std::vector<float>(width, 0.0f));

#pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float current = get_nucleus(x, y).get_food().get_amount();
            float sum_neighbors = 0.0f;
            for (Cell* nb : get_neighbours(x, y)) {
                sum_neighbors += nb->get_food().get_amount();
            }
            float num_neighbors = static_cast<float>(get_neighbours(x, y).size());
            float new_val = current + simulation_config::field::food_diffusion_coeff *
                (sum_neighbors - num_neighbors * current);
            if (new_val < 0.0f) new_val = 0.0f;
            new_food[y][x] = new_val;
        }
    }

#pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            get_nucleus(x, y).get_food().set_amount(new_food[y][x]);
        }
    }
}

void Field::diffuse_antibiotic() {
    std::vector<std::vector<float>> new_antibiotic(height, std::vector<float>(width, 0.0f));

#pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float current = get_nucleus(x, y).get_antibiotic().get_concentration();
            float sum_neighbors = 0.0f;
            for (Cell* nb : get_neighbours(x, y)) {
                sum_neighbors += nb->get_antibiotic().get_concentration();
            }
            float num_neighbors = static_cast<float>(get_neighbours(x, y).size());
            float new_val = current + simulation_config::antibiotic::diffusion_coeff *
                (sum_neighbors - num_neighbors * current);
            new_val *= (1.0f - simulation_config::antibiotic::decay_rate);
            if (new_val < 0.0f) new_val = 0.0f;
            new_antibiotic[y][x] = new_val;
        }
    }

#pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            get_nucleus(x, y).get_antibiotic().set_concentration(new_antibiotic[y][x]);
        }
    }
}

bool Field::is_x_inside(int x) const {
    return x >= 0 && x < width;
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

Cell& Field::get_nucleus(int x, int y) {
    return field[y][x];
}

const Cell& Field::get_nucleus(int x, int y) const {
    return field[y][x];
}

std::vector<Cell*> Field::get_neighbours(int x, int y) {
    std::vector<Cell*> neighbours;

    const std::array<std::pair<int, int>, 4> directions = {
        std::pair<int, int>{0, -1},
        std::pair<int, int>{0, 1},
        std::pair<int, int>{-1, 0},
        std::pair<int, int>{1, 0}
    };

    for (const auto& direction : directions) {
        int neighbour_x = x + direction.first;
        int neighbour_y = y + direction.second;

        if (is_x_inside(neighbour_x) && is_y_inside(neighbour_y)) {
            neighbours.push_back(&get_nucleus(neighbour_x, neighbour_y));
        }
    }

    return neighbours;
}

std::vector<Cell*> Field::get_free_neighbours(int x, int y) {
    std::vector<Cell*> free_neighbours;

    for (Cell* neighbour : get_neighbours(x, y)) {
        if (neighbour->is_this_nucleus_free()) {
            free_neighbours.push_back(neighbour);
        }
    }

    return free_neighbours;
}

bool Field::place_cell(int x, int y, std::shared_ptr<abstract_Biomass> cell) {
    if (!is_x_inside(x) || !is_y_inside(y)) {
        return false;
    }

    Cell& nucleus = get_nucleus(x, y);

    if (!nucleus.is_this_nucleus_free()) {
        return false;
    }

    nucleus.set_cell(std::move(cell));

    return true;
}

bool Field::has_living_cells() const {
    for (const auto& row : field) {
        for (const Cell& nucleus : row) {
            std::shared_ptr<abstract_Biomass> cell = nucleus.get_cell();

            if (cell != nullptr && cell->is_alive()) {
                return true;
            }
        }
    }

    return false;
}

void Field::process_dead_cells_disappearance() {
#pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Cell& nucleus = get_nucleus(x, y);
            std::shared_ptr<abstract_Biomass> cell = nucleus.get_cell();

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

void Field::make_one_step(int number_of_step) {
    // Диффузия
    if (number_of_step % simulation_config::visualization::number_of_step_to_diffuse == 0) {
        diffuse_food();
        diffuse_antibiotic();
    }

    // Добавление пищи (точечное)
    if (number_of_step % simulation_config::field::steps_for_adding_food == 0) {
        add_some_food(simulation_config::field::count_of_adding_food);
    }

    // Добавление антибиотика
    if (number_of_step % simulation_config::antibiotic::adding_interval == 0) {
        add_antibiotic(
            simulation_config::antibiotic::adding_concentration,
            width / 2,
            0
        );
    }

    std::vector<std::pair<int, int>> cells_for_this_step;

#pragma omp parallel
    {
        std::vector<std::pair<int, int>> local_cells;
#pragma omp for collapse(2) schedule(static)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                std::shared_ptr<abstract_Biomass> cell = field[y][x].get_cell();

                if (cell != nullptr && cell->is_alive()) {
                    local_cells.emplace_back(x, y);
                }
            }
        }
#pragma omp critical
        {
            cells_for_this_step.insert(cells_for_this_step.end(), local_cells.begin(), local_cells.end());
        }
    }

    // Проверка смерти
#pragma omp parallel for schedule(static)
    for (size_t i = 0; i < cells_for_this_step.size(); ++i) {
        const auto& position = cells_for_this_step[i];
        Cell& nucleus = get_nucleus(position.first, position.second);
        std::shared_ptr<abstract_Biomass> cell = nucleus.get_cell();

        if (cell == nullptr || !cell->is_alive()) {
            continue;
        }

        if (cell->must_he_die(nucleus.get_food(), nucleus.get_antibiotic())) {
            nucleus.set_cell(std::make_shared<dead_Biomass>());
        }
        else {
            cell->increase_age();
        }
    }

#pragma omp parallel for schedule(static)
    for (size_t i = 0; i < cells_for_this_step.size(); ++i) {
        const auto& position = cells_for_this_step[i];
        Cell& nucleus = get_nucleus(position.first, position.second);
        std::shared_ptr<abstract_Biomass> cell = nucleus.get_cell();

        if (cell != nullptr && cell->is_alive()) {
            cell->set_nucleus(&nucleus);
            cell->consume_and_decay(nucleus.get_food());
        }
    }

    for (const auto& position : cells_for_this_step) {
        std::shared_ptr<abstract_Biomass> cell =
            get_nucleus(position.first, position.second).get_cell();

        if (cell != nullptr && cell->is_alive()) {
            cell->reproduction(*this, position.first, position.second);
        }
    }

    process_dead_cells_disappearance();
}

const std::vector<std::vector<Cell>>& Field::get_field() const {
    return field;
}