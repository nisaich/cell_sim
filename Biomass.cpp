#include "Biomass.hpp"

#include "Field.hpp"
#include "Food.hpp"

#include <algorithm>
#include <memory>
#include <random>
#include <vector>

int abstract_Biomass::get_age() const { 
    return age_of_cell;
}

void abstract_Biomass::copy_common_state_to(abstract_Biomass& other) const {
    other.age_of_cell = age_of_cell;
    other.biomass = biomass;
    other.max_amount_of_food_consumed = max_amount_of_food_consumed;
    other.using_food_for_step = using_food_for_step;

    other.max_age_of_cell = max_age_of_cell;
    other.level_of_resistance = level_of_resistance;
}

float abstract_Biomass::get_level_of_resistance() const {
    return level_of_resistance;
}


float abstract_Biomass::get_biomass() const {
  return biomass;
}

bool abstract_Biomass::must_he_die(Food& food) const {
    return age_of_cell >= max_age_of_cell || food.get_amount()<=using_food_for_step;
}

void abstract_Biomass::increase_age() {
    ++age_of_cell;
}

void abstract_Biomass::set_nucleus(Cell* current_nucleus) {
    nucleus = current_nucleus;
}

void abstract_Biomass::step_after_death() {
    // Для живых клеток ничего не делаем
}

bool abstract_Biomass::should_be_removed_from_field() const {
    return false;
}

void abstract_Biomass::food_consumption_from_environment(Food& food) {
    float wanted_food = std::min(food.get_amount(), static_cast<float>(max_amount_of_food_consumed));
    if (wanted_food > 0.0f) {
        float eaten = food.take(wanted_food);
        biomass += eaten * biomass * simulation_config::biomass::biomass_growth_per_eaten_unit;
    }
}

void abstract_Biomass::depletion_of_savings(Food& food) {
    float available_food = food.get_amount();
    if (steps_for_nonactivating != simulation_config::biomass::steps_for_nonactivating && steps_for_nonactivating != 0){
      steps_for_nonactivating--;
    }
    else if (available_food < steps_to_live_forward * using_food_for_step) {
      steps_for_nonactivating--;
    }
    else if(steps_for_nonactivating == 0){
      auto nonactive_cell = std::make_shared<nonactive_Biomass>();
      copy_common_state_to(*nonactive_cell);
      nucleus->set_cell(nonactive_cell);
      steps_for_nonactivating= simulation_config::biomass::steps_for_nonactivating;
    }

    food.take(using_food_for_step);
}

bool abstract_Biomass::reproduction(Field& current_field, int x, int y) {
    return false;
}

active_Biomass::active_Biomass(float resistance, int max_age) {
    level_of_resistance = resistance;
    max_age_of_cell = max_age;
}

bool active_Biomass::is_alive() const {
    return true;
}

bool active_Biomass::reproduction(Field& current_field, int x, int y) {
    std::vector<Cell*> free_neighbours = current_field.get_free_neighbours(x, y);

    if (free_neighbours.empty()) {
      return false;
    }

    if (biomass < simulation_config::biomass::reproduction_min_biomass) {
      return false;
    }

    static std::mt19937 generator(std::random_device{}());

    std::uniform_real_distribution<float> chance_distribution(0.00f, 1.00f);

    float reproduction_chance = simulation_config::biomass::reproduction_chance;

    if (chance_distribution(generator) > reproduction_chance) {
      return false;
    }

    std::uniform_int_distribution<std::size_t> distribution(
        0,
        free_neighbours.size() - 1
    );

    Cell* place_for_child = free_neighbours[distribution(generator)];

    float child_biomass = biomass * simulation_config::biomass::child_biomass_ratio;
    biomass = child_biomass;
    auto child = std::make_shared<active_Biomass>(
      level_of_resistance,
      max_age_of_cell
    );
    child->biomass = child_biomass;

    child->max_amount_of_food_consumed = max_amount_of_food_consumed;
    child->using_food_for_step = using_food_for_step;

    place_for_child->set_cell(child);

    return true;
}

nonactive_Biomass::nonactive_Biomass(
    float resistance,
    int max_age,
    float max_food_consumed,
    float food_usage
) {
    level_of_resistance = resistance * resistance_multiplier;
    
    max_age_of_cell = static_cast<int>(max_age * max_life_multiplier);

    max_amount_of_food_consumed = max_food_consumed;
    using_food_for_step = food_usage * food_usage_multiplier;
}

void nonactive_Biomass::depletion_of_savings(Food& food) {
    float available_food = food.get_amount();
    if (available_food < using_food_for_step) {
        biomass = std::max(
            0.0f,
            biomass - using_food_for_step *
                simulation_config::biomass::nonactive_biomass_loss_multiplier
        );
        return;
    }

    food.take(using_food_for_step);
}

bool nonactive_Biomass::reproduction(Field& current_field, int x, int y) {
    const float food_now = simulation_config::biomass::food_usage_for_step;
    Cell& nucleus = current_field.get_nucleus(x, y);
    if (nucleus.get_food().get_amount() < steps_to_live_forward * food_now) {
        return false;
    }
    else if (steps_for_nonactivating == simulation_config::biomass::steps_for_nonactivating) {
      steps_for_nonactivating--;
    }
    else if (steps_for_nonactivating == 0){
      auto active_cell = std::make_shared<active_Biomass>();
      copy_common_state_to(*active_cell);

      nucleus.set_cell(active_cell);
    }

    return false;
}

void dead_Biomass::step_after_death() {
    ++count_of_steps_from_death;
}

bool dead_Biomass::should_be_removed_from_field() const {
    return count_of_steps_from_death >= count_of_steps_to_disappearance;
}

bool dead_Biomass::is_it_still_there() const {
    return count_of_steps_from_death < count_of_steps_to_disappearance;
}

bool dead_Biomass::is_alive() const {
    return false;
}
