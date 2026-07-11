#include "Field.hpp"
#include "Biomass.hpp"
#include <memory>
#include <utility>

// ----- Cell -----
Cell::Cell(
    int x,
    int y,
    double start_food,
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

std::pair<double, float> Cell::situation_in_the_environment() const {
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
        }
    }
}

void Field::add_some_food(int count_of_adding_food) {
    // Точечный источник в центре верхней строки
    int center = width / 2;
    get_nucleus(center, 0).get_food().add(count_of_adding_food);
}

void Field::add_antibiotic(float concentration) {
  for (int x = 0; x < width; x++){  //добавляем только по самой верхней строчке, так и должно быть
    get_nucleus(x, 0).get_antibiotic().add(concentration);
  }
}

static void diffuse_grid_adi(std::vector<std::vector<double>>& grid, double D, double dt, double decay_rate = 0.0) {
    int height = grid.size();
    if (height == 0) return;
    int width = grid[0].size();
    if (width == 0) return;

    double r = D * dt / 2.0;
    if (r <= 0.0) return;

    // Промежуточная сетка для первого полушага
    std::vector<std::vector<double>> temp(height, std::vector<double>(width, 0.0));

    // Предвычисление c_prime по ширине (X)
    std::vector<double> c_prime_x(width);
    c_prime_x[0] = -r / (1.0 + r);
    for (int i = 1; i < width - 1; ++i) {
        double denom = (1.0 + 2.0 * r) + r * c_prime_x[i - 1];
        c_prime_x[i] = -r / denom;
    }

    // Предвычисление c_prime по высоте (Y)
    std::vector<double> c_prime_y(height);
    c_prime_y[0] = -r / (1.0 + r);
    for (int j = 1; j < height - 1; ++j) {
        double denom = (1.0 + 2.0 * r) + r * c_prime_y[j - 1];
        c_prime_y[j] = -r / denom;
    }

    // 1. ПЕРВЫЙ ПОЛУШАГ: Неявный по X, явный по Y
#pragma omp parallel
    {
        std::vector<double> d_prime(width);
#pragma omp for schedule(static)
        for (int j = 0; j < height; ++j) {
            std::vector<double> d(width);
            for (int i = 0; i < width; ++i) {
                double val_self = grid[j][i];
                double val_up = (j > 0) ? grid[j - 1][i] : val_self;
                double val_down = (j < height - 1) ? grid[j + 1][i] : val_self;
                
                d[i] = val_self + r * (val_up - 2.0 * val_self + val_down);
            }

            d_prime[0] = d[0] / (1.0 + r);
            for (int i = 1; i < width - 1; ++i) {
                double denom = (1.0 + 2.0 * r) + r * c_prime_x[i - 1];
                d_prime[i] = (d[i] + r * d_prime[i - 1]) / denom;
            }
            double denom_last = (1.0 + r) + r * c_prime_x[width - 2];
            double d_prime_last = (d[width - 1] + r * d_prime[width - 2]) / denom_last;

            temp[j][width - 1] = d_prime_last;
            for (int i = width - 2; i >= 0; --i) {
                temp[j][i] = d_prime[i] - c_prime_x[i] * temp[j][i + 1];
            }
        }
    }

    // 2. ВТОРОЙ ПОЛУШАГ: Явный по X, неявный по Y
#pragma omp parallel
    {
        std::vector<double> d_prime(height);
#pragma omp for schedule(static)
        for (int i = 0; i < width; ++i) {
            std::vector<double> d(height);
            for (int j = 0; j < height; ++j) {
                double val_self = temp[j][i];
                double val_left = (i > 0) ? temp[j][i - 1] : val_self;
                double val_right = (i < width - 1) ? temp[j][i + 1] : val_self;

                d[j] = val_self + r * (val_left - 2.0 * val_self + val_right);
            }

            d_prime[0] = d[0] / (1.0 + r);
            for (int j = 1; j < height - 1; ++j) {
                double denom = (1.0 + 2.0 * r) + r * c_prime_y[j - 1];
                d_prime[j] = (d[j] + r * d_prime[j - 1]) / denom;
            }
            double denom_last = (1.0 + r) + r * c_prime_y[height - 2];
            double d_prime_last = (d[height - 1] + r * d_prime[height - 2]) / denom_last;

            grid[height - 1][i] = d_prime_last;
            for (int j = height - 2; j >= 0; --j) {
                grid[j][i] = d_prime[j] - c_prime_y[j] * grid[j + 1][i];
            }
        }
    }

    // Применение коэффициента деградации и ограничение неотрицательности
    if (decay_rate > 0.0) {
        double decay_factor = 1.0 - decay_rate;
#pragma omp parallel for collapse(2) schedule(static)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                grid[y][x] *= decay_factor;
                if (grid[y][x] < 0.0) grid[y][x] = 0.0;
            }
        }
    } else {
#pragma omp parallel for collapse(2) schedule(static)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (grid[y][x] < 0.0) grid[y][x] = 0.0;
            }
        }
    }
}

void Field::diffuse_food() {
    std::vector<std::vector<double>> grid(height, std::vector<double>(width, 0.0));
#pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            grid[y][x] = get_nucleus(x, y).get_food().get_amount();
        }
    }

    diffuse_grid_adi(grid, 
                     simulation_config::field::food_diffusion_coeff, 
                     simulation_config::monod::delta_t);

#pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            get_nucleus(x, y).get_food().set_amount(grid[y][x]);
        }
    }
}

void Field::diffuse_antibiotic() {
    std::vector<std::vector<double>> grid(height, std::vector<double>(width, 0.0));
#pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            grid[y][x] = get_nucleus(x, y).get_antibiotic().get_concentration();
        }
    }

    diffuse_grid_adi(grid, 
                     simulation_config::antibiotic::diffusion_coeff, 
                     simulation_config::monod::delta_t,
                     simulation_config::antibiotic::decay_rate);

#pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            get_nucleus(x, y).get_antibiotic().set_concentration(static_cast<float>(grid[y][x]));
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
    double sum_antibiotic = 0.0;
#pragma omp parallel for reduction(+:sum_antibiotic) collapse(2) schedule(static)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            sum_antibiotic += get_nucleus(x, y).get_antibiotic().get_concentration();
        }
    }
    double avg_antibiotic = sum_antibiotic / (width * height);

    if (avg_antibiotic < middle_value_of_antibiotic) {
        add_antibiotic(concentration);
        concentration += concetration_for_next_step;    
        middle_value_of_antibiotic += concetration_for_next_step;
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
