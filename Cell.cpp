#include "Cell.hpp"

#include "Field.hpp"
#include "Food.hpp"

#include <algorithm>
#include <memory>
#include <random>
#include <vector>

int abstract_Cell::get_age() const { 
    return age_of_cell;
}

float abstract_Cell::get_level_of_resistance() const {
    return level_of_resistance;
}

int abstract_Cell::get_food_inside() const {
    return food_inside;
}

bool abstract_Cell::must_he_die() const {
    return age_of_cell >= max_age_of_cell || food_inside <= 0;
}

void abstract_Cell::increase_age() {
    ++age_of_cell;
}

void abstract_Cell::step_after_death() {
    // Для живых клеток ничего не делаем
}

bool abstract_Cell::should_be_removed_from_field() const {
    return false;
}

void abstract_Cell::food_consumption_from_environment(Food& food) {
    int free_space_inside = max_food_inside - food_inside;
    int wanted_food = std::min(free_space_inside, max_amount_of_food_consumed);

    int eaten_food = food.take(wanted_food);

    if (eaten_food > 0) {
        food_inside += eaten_food;
    }
}

void abstract_Cell::depletion_of_savings() {
    food_inside -= using_food_for_step;
}

bool abstract_Cell::reproduction(Field& current_field, int x, int y) {
    return false;
}

active_Cell::active_Cell(int start_food, float resistance, int max_age) {
    food_inside = start_food;
    level_of_resistance = resistance;
    max_age_of_cell = max_age;
}

bool active_Cell::is_alive() const {
    return true;
}

bool active_Cell::reproduction(Field& current_field, int x, int y) {
    if (food_inside <= 10) {
        return false;
    }

    std::vector<Nucleus*> free_neighbours = current_field.get_free_neighbours(x, y);

    if (free_neighbours.empty()) {
        return false;
    }

    static std::mt19937 generator(std::random_device{}());

    std::uniform_real_distribution<float> chance_distribution(0.00f, 1.00f);

    float reproduction_chance = 0.02f;

    if (chance_distribution(generator) > reproduction_chance) {
      return false;
    }

    std::uniform_int_distribution<std::size_t> distribution(
        0,
        free_neighbours.size() - 1
    );

    Nucleus* place_for_child = free_neighbours[distribution(generator)];

    int child_food = food_inside / 2;

    auto child = std::make_shared<active_Cell>(
        child_food,
        level_of_resistance,
        max_age_of_cell
    );

    child->max_food_inside = max_food_inside;
    child->max_amount_of_food_consumed = max_amount_of_food_consumed;
    child->using_food_for_step = using_food_for_step;

    food_inside -= child_food;

    place_for_child->set_cell(child);

    return true;
}

bool nonactive_Cell::is_alive() const {
    return true;
}

void dead_Cell::step_after_death() {
    ++count_of_steps_from_death;
}

bool dead_Cell::should_be_removed_from_field() const {
    return count_of_steps_from_death >= count_of_steps_to_disappearance;
}

bool dead_Cell::is_it_still_there() const {
    return count_of_steps_from_death < count_of_steps_to_disappearance;
}

bool dead_Cell::is_alive() const {
    return false;
}

