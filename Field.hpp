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

    std::array<int, 2> cell_coordinates{0, 0};

    Food food;
    Antibiotic antibiotic;

public:
    Cell() = default;

    Cell(
        int x,
        int y,
        float start_food = 0,
        float start_antibiotic = 0.0f
    );

    bool is_this_nucleus_free() const;

    void set_cell(std::shared_ptr<abstract_Biomass> new_cell);
    std::shared_ptr<abstract_Biomass> get_cell() const;
    void remove_cell();

    std::pair<float, float> situation_in_the_environment() const;
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

    std::vector<std::vector<Cell>> field;

    void process_dead_cells_disappearance();

public:
    Field(int width, int height);

    bool is_x_inside(int x) const;
    bool is_y_inside(int y) const;

    int get_width() const;
    int get_height() const;
    
    void diffuse_food ();
    void init_environment(
        float initial_food = simulation_config::field::default_initial_food
    );

    const std::vector<std::vector<Cell>>& get_field() const;

    const Cell& get_nucleus(int x, int y) const;
    Cell& get_nucleus(int x, int y);

    std::vector<Cell*> get_neighbours(int x, int y);
    std::vector<Cell*> get_free_neighbours(int x, int y);

    bool place_cell(int x, int y, std::shared_ptr<abstract_Biomass> cell);

    bool has_living_cells() const;
    void make_one_step();
};
