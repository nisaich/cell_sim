#pragma once
#include "SimulationConfig.hpp"
#include <cmath>
#include "Food.hpp"
#include "Antibiotic.hpp"

#include <array>
#include <memory>
#include <utility>
#include <vector>

class abstract_Biomass;

class Cell {
private:
    std::shared_ptr<abstract_Biomass> cell = nullptr;
    std::array<int, 2> cell_coordinates{ 0, 0 };
    Food food;
    Antibiotic antibiotic;

public:
    Cell() = default;
    Cell(
        int x,
        int y,
        double start_food = 0.0,
        double start_antibiotic = 0.0
    );

    bool is_this_nucleus_free() const;

    void set_cell(std::shared_ptr<abstract_Biomass> new_cell);
    std::shared_ptr<abstract_Biomass> get_cell() const;
    void remove_cell();

    std::pair<double, double> situation_in_the_environment() const;
    std::array<int, 2> coordinates() const;

    Food& get_food();
    const Food& get_food() const;

    Antibiotic& get_antibiotic();
    const Antibiotic& get_antibiotic() const;
};

class Field {
private:
    int width;
    int height;
    std::vector<Cell> field;
    std::vector<std::pair<int, int>> active_cells;
    std::vector<std::pair<int, int>> dead_cells;
    std::vector<double> temp_grid;
    std::vector<double> food_grid;
    std::vector<double> antibiotic_grid;
    std::vector<std::pair<int, int>> cells_for_this_step;

    void process_dead_cells_disappearance();

    double concetration_for_next_step = simulation_config::antibiotic::concetration_for_next_step;
    double concentration = simulation_config::antibiotic::concetration_for_next_step;  //начальная концентрация, потом добавляем ещё
    double middle_value_of_antibiotic = simulation_config::antibiotic::middle_value_of_antibiotic;  //посмотрите в файле конфига
public:
    Field(int width, int height);

    bool is_x_inside(int x) const;
    bool is_y_inside(int y) const;

    int get_width() const;
    int get_height() const;

    void diffuse_all();

    void init_environment(
        double initial_food = 0.0
    );
    void add_some_food(
        int count_of_adding_food = simulation_config::field::count_of_adding_food
    );
    void add_antibiotic(          // новый метод
        double concentration
    );

    const std::vector<Cell>& get_field() const;

    const Cell& get_nucleus(int x, int y) const;
    Cell& get_nucleus(int x, int y);

    std::vector<Cell*> get_neighbours(int x, int y);
    std::vector<Cell*> get_free_neighbours(int x, int y);

    // 8-связная (Мур) окрестность — нужна для ветвистого (дендритного) роста:
    // позволяет клетке расти и по диагонали, давая более естественные углы веток,
    // и используется для подсчёта "оголённости" кандидатной точки (см. Biomass.cpp).
    std::vector<Cell*> get_moore_neighbours(int x, int y);
    std::vector<Cell*> get_free_moore_neighbours(int x, int y);

    bool place_cell(int x, int y, std::shared_ptr<abstract_Biomass> cell);

    bool has_living_cells() const;
    void make_one_step(int number_of_step);
};