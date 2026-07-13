#include "Field.hpp"
#include "Biomass.hpp"
#include <memory>
#include <utility>

// ----- Cell -----
Cell::Cell(
    int x,
    int y,
    double start_food,
    double start_antibiotic
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

std::pair<double, double> Cell::situation_in_the_environment() const {
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
    field(height * width),
    temp_grid(height * width, 0.0),
    food_grid(height * width, 0.0),
    antibiotic_grid(height * width, 0.0) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            field[y * width + x] = Cell(x, y);
        }
    }
}

void Field::init_environment(double initial_food) {
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

void Field::add_antibiotic(double concentration) {
  for (int x = 0; x < width; x++){  //добавляем только по самой верхней строчке, так и должно быть
    get_nucleus(x, 0).get_antibiotic().add(concentration);
  }
}

static void diffuse_grid_adi(std::vector<double>& grid, std::vector<double>& temp, int width, int height, double D, double dt, double decay_rate = 0.0) {
    if (height == 0 || width == 0) return;

    double r = D * dt / 2.0;
    if (r <= 0.0) return;

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
        double d_prime[512];
        double d[512];
#pragma omp for schedule(static)
        for (int j = 0; j < height; ++j) {
            for (int i = 0; i < width; ++i) {
                double val_self = grid[j * width + i];
                double val_up = (j > 0) ? grid[(j - 1) * width + i] : val_self;
                double val_down = (j < height - 1) ? grid[(j + 1) * width + i] : val_self;
                
                d[i] = val_self + r * (val_up - 2.0 * val_self + val_down);
            }

            d_prime[0] = d[0] / (1.0 + r);
            for (int i = 1; i < width - 1; ++i) {
                double denom = (1.0 + 2.0 * r) + r * c_prime_x[i - 1];
                d_prime[i] = (d[i] + r * d_prime[i - 1]) / denom;
            }
            double denom_last = (1.0 + r) + r * c_prime_x[width - 2];
            double d_prime_last = (d[width - 1] + r * d_prime[width - 2]) / denom_last;

            temp[j * width + width - 1] = d_prime_last;
            for (int i = width - 2; i >= 0; --i) {
                temp[j * width + i] = d_prime[i] - c_prime_x[i] * temp[j * width + i + 1];
            }
        }
    }

    // 2. ВТОРОЙ ПОЛУШАГ: Явный по X, неявный по Y
#pragma omp parallel
    {
        double d_prime[512];
        double d[512];
#pragma omp for schedule(static)
        for (int i = 0; i < width; ++i) {
            for (int j = 0; j < height; ++j) {
                double val_self = temp[j * width + i];
                double val_left = (i > 0) ? temp[j * width + i - 1] : val_self;
                double val_right = (i < width - 1) ? temp[j * width + i + 1] : val_self;

                d[j] = val_self + r * (val_left - 2.0 * val_self + val_right);
            }

            d_prime[0] = d[0] / (1.0 + r);
            for (int j = 1; j < height - 1; ++j) {
                double denom = (1.0 + 2.0 * r) + r * c_prime_y[j - 1];
                d_prime[j] = (d[j] + r * d_prime[j - 1]) / denom;
            }
            double denom_last = (1.0 + r) + r * c_prime_y[height - 2];
            double d_prime_last = (d[height - 1] + r * d_prime[height - 2]) / denom_last;

            grid[(height - 1) * width + i] = d_prime_last;
            for (int j = height - 2; j >= 0; --j) {
                grid[j * width + i] = d_prime[j] - c_prime_y[j] * grid[(j + 1) * width + i];
            }
        }
    }

    // Применение коэффициента деградации и ограничение неотрицательности
    if (decay_rate > 0.0) {
        double decay_factor = 1.0 - decay_rate;
#pragma omp parallel for schedule(static)
        for (int i = 0; i < height * width; ++i) {
            grid[i] *= decay_factor;
            if (grid[i] < 0.0) grid[i] = 0.0;
        }
    } else {
#pragma omp parallel for schedule(static)
        for (int i = 0; i < height * width; ++i) {
            if (grid[i] < 0.0) grid[i] = 0.0;
        }
    }
}

void Field::diffuse_all() {
#pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            food_grid[idx] = field[idx].get_food().get_amount();
            antibiotic_grid[idx] = field[idx].get_antibiotic().get_concentration();
        }
    }

    diffuse_grid_adi(food_grid, temp_grid, width, height,
                     simulation_config::field::food_diffusion_coeff,
                     simulation_config::monod::delta_t);

    diffuse_grid_adi(antibiotic_grid, temp_grid, width, height,
                     simulation_config::antibiotic::diffusion_coeff,
                     simulation_config::monod::delta_t,
                     simulation_config::antibiotic::decay_rate);

#pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            field[idx].get_food().set_amount(food_grid[idx]);
            field[idx].get_antibiotic().set_concentration(antibiotic_grid[idx]);
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
    return field[y * width + x];
}

const Cell& Field::get_nucleus(int x, int y) const {
    return field[y * width + x];
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

    nucleus.set_cell(cell);

    if (cell != nullptr) {
        if (cell->is_alive()) {
            active_cells.emplace_back(x, y);
        } else {
            dead_cells.emplace_back(x, y);
        }
    }

    return true;
}

bool Field::has_living_cells() const {
    for (const Cell& nucleus : field) {
        std::shared_ptr<abstract_Biomass> cell = nucleus.get_cell();

        if (cell != nullptr && cell->is_alive()) {
            return true;
        }
    }

    return false;
}

void Field::process_dead_cells_disappearance() {
#pragma omp parallel for schedule(static)
    for (size_t i = 0; i < dead_cells.size(); ++i) {
        const auto& pos = dead_cells[i];
        Cell& nucleus = get_nucleus(pos.first, pos.second);
        std::shared_ptr<abstract_Biomass> cell = nucleus.get_cell();

        if (cell != nullptr && !cell->is_alive()) {
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
        diffuse_all();
    }

    // Добавление пищи (точечное)
    if (number_of_step % simulation_config::field::steps_for_adding_food == 0) {
        add_some_food(simulation_config::field::count_of_adding_food);

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
        }
    }

    std::vector<std::pair<int, int>> cells_for_this_step = active_cells;

    // Проверка смерти
#pragma omp parallel for schedule(static)
    for (size_t i = 0; i < cells_for_this_step.size(); ++i) {
        const auto& position = cells_for_this_step[i];
        Cell& nucleus = get_nucleus(position.first, position.second);
        std::shared_ptr<abstract_Biomass> cell = nucleus.get_cell();

        if (cell == nullptr || !cell->is_alive()) {
            continue;
        }

        if (cell->must_he_die(*this, position.first, position.second)) {
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

    // Фильтруем active_cells (убираем те, что умерли, и перемещаем их в dead_cells)
    std::vector<std::pair<int, int>> next_active_cells;
    next_active_cells.reserve(active_cells.size());
    for (const auto& pos : active_cells) {
        auto cell = get_nucleus(pos.first, pos.second).get_cell();
        if (cell != nullptr) {
            if (cell->is_alive()) {
                next_active_cells.push_back(pos);
            } else {
                dead_cells.push_back(pos);
            }
        }
    }
    active_cells = std::move(next_active_cells);

    // Обрабатываем растворение мертвых клеток
    process_dead_cells_disappearance();

    // Фильтруем dead_cells (убираем те, что полностью исчезли)
    std::vector<std::pair<int, int>> next_dead_cells;
    next_dead_cells.reserve(dead_cells.size());
    for (const auto& pos : dead_cells) {
        auto cell = get_nucleus(pos.first, pos.second).get_cell();
        if (cell != nullptr && !cell->is_alive()) {
            next_dead_cells.push_back(pos);
        }
    }
    dead_cells = std::move(next_dead_cells);
}

const std::vector<Cell>& Field::get_field() const {
    return field;
}
