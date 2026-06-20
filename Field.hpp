#pragma once
#include <cmath>
#include "Food.hpp"
#include "Antibiotic.hpp"

#include <array>
#include <memory>
#include <utility>
#include <vector>

class abstract_Cell;

class Nucleus {
private:
    std::shared_ptr<abstract_Cell> cell = nullptr;

    std::array<int, 2> cell_coordinates{0, 0};

    Food food;
    Antibiotic antibiotic;

public:
    Nucleus() = default;

    Nucleus(
        int x,
        int y,
        float start_food = 1.0f/0.0f,
        float start_antibiotic = 0.0f
    );

    bool is_this_nucleus_free() const;

    void set_cell(std::shared_ptr<abstract_Cell> new_cell);
    std::shared_ptr<abstract_Cell> get_cell() const;
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

    std::vector<std::vector<Nucleus>> field;

    void process_dead_cells_disappearance();

public:
    Field(int width, int height);

    int normalize_x(int x) const;
    bool is_y_inside(int y) const;

    int get_width() const;
    int get_height() const;

    const Nucleus& get_nucleus(int x, int y) const;
    Nucleus& get_nucleus(int x, int y);

    std::vector<Nucleus*> get_neighbours(int x, int y);
    std::vector<Nucleus*> get_free_neighbours(int x, int y);

    bool place_cell(int x, int y, std::shared_ptr<abstract_Cell> cell);

    bool has_living_cells() const;
    void make_one_step();
};
