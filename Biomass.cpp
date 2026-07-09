#include "Biomass.hpp"
#include "Field.hpp"
#include "Food.hpp"
#include "Antibiotic.hpp"
#include <algorithm>
#include <memory>
#include <random>
#include <vector>

// ---------- abstract_Biomass ----------
int abstract_Biomass::get_age() const {
    return age_of_cell;
}

void abstract_Biomass::copy_common_state_to(abstract_Biomass& other) const {
    other.age_of_cell = age_of_cell;
    other.biomass = biomass;
    other.max_age_of_cell = max_age_of_cell;
    other.level_of_resistance = level_of_resistance;
    other.nucleus = nucleus;
}

float abstract_Biomass::get_level_of_resistance() const {
    return level_of_resistance;
}

float abstract_Biomass::get_biomass() const {
    return biomass;
}

bool abstract_Biomass::must_he_die(Food& food, const Antibiotic& antibiotic) const {
    if (age_of_cell >= max_age_of_cell || biomass <= 0.001f) return true;

    float conc = antibiotic.get_concentration();
    float excess = conc - level_of_resistance;
    if (excess > simulation_config::antibiotic::death_threshold) {
        static std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float prob = (excess - simulation_config::antibiotic::death_threshold) *
            simulation_config::antibiotic::death_probability_factor;
        prob = std::min(prob, 1.0f);
        if (dist(gen) < prob) return true;
    }
    return false;
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

void abstract_Biomass::consume_and_decay(Food& food) {
    float a = is_active_for_monod();
    float F = food.get_amount();

    float u = 0.0f;
    if (F > 0.0f) {
        u = simulation_config::monod::U_max * (F / (simulation_config::monod::K_F + F)) * a;
    }

    float u_dt = u * simulation_config::monod::delta_t;
    float space_limit = std::max(0.0f, (simulation_config::biomass::max_biomass - biomass) / simulation_config::monod::Y_B_F);

    float delta_Food = std::min({ F, u_dt, space_limit });
    if (delta_Food < 0.0f) delta_Food = 0.0f;

    food.take(delta_Food);

    biomass += simulation_config::monod::Y_B_F * delta_Food;

    float m = get_maintenance_rate();
    biomass = std::max(0.0f, biomass * (1.0f - m * simulation_config::monod::delta_t));
}

bool abstract_Biomass::reproduction(Field& current_field, int x, int y) {
    return false;
}

// ---------- active_Biomass ----------
active_Biomass::active_Biomass(float resistance, int max_age) {
    level_of_resistance = resistance;
    max_age_of_cell = max_age;
}

bool active_Biomass::is_alive() const {
    return true;
}

void active_Biomass::consume_and_decay(Food& food) {
    abstract_Biomass::consume_and_decay(food);

    if (nucleus != nullptr && biomass < simulation_config::monod::starvation_biomass_threshold) {
        auto nonactive = std::make_shared<nonactive_Biomass>();
        copy_common_state_to(*nonactive);
        nonactive->apply_dormancy_effects();
        nucleus->set_cell(nonactive);
        return;
    }

    if (nucleus != nullptr) {
        float conc = nucleus->get_antibiotic().get_concentration();
        float excess = conc - level_of_resistance;
        if (excess > 0.0f) {
            static std::mt19937 gen(std::random_device{}());
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            float chance = std::min(excess * 0.01f, simulation_config::antibiotic::stress_transition_chance);
            if (dist(gen) < chance) {
                auto nonactive = std::make_shared<nonactive_Biomass>();
                copy_common_state_to(*nonactive);
                nonactive->apply_dormancy_effects();
                nucleus->set_cell(nonactive);
                return;
            }
        }
    }
}

bool active_Biomass::reproduction(Field& current_field, int x, int y) {
    std::vector<Cell*> free_neighbours = current_field.get_free_neighbours(x, y);
    if (max_count_reps == 0) return false;
    if (free_neighbours.empty()) return false;
    if (biomass < simulation_config::biomass::reproduction_min_biomass) return false;

    static std::mt19937 generator(std::random_device{}());

    float current_chance = 1.0f;
    if (nucleus != nullptr) {
        float conc = nucleus->get_antibiotic().get_concentration();
        float excess = conc - level_of_resistance;
        if (excess > 0.0f) {
            float penalty = std::min(excess * 0.1f, simulation_config::antibiotic::reproduction_penalty);
            current_chance *= (1.0f - penalty);
        }
    }

    if (current_chance < 1.0f) {
        std::uniform_real_distribution<float> chance_distribution(0.00f, 1.00f);
        if (chance_distribution(generator) > current_chance) return false;
    }

    // ---- ВЫБОР СОСЕДА ПО ПИЩЕ (БЕЗ УКЛОНА) ----
    float max_food = -1.0f;
    for (Cell* nb : free_neighbours) {
        max_food = std::max(max_food, nb->get_food().get_amount());
    }

    float threshold = max_food * simulation_config::biomass::lateral_growth_tolerance;
    std::vector<Cell*> candidates;
    for (Cell* nb : free_neighbours) {
        if (nb->get_food().get_amount() >= threshold) {
            candidates.push_back(nb);
        }
    }

    if (candidates.empty()) {
        candidates = free_neighbours;
    }

    std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
    Cell* place_for_child = candidates[dist(generator)];
    // -------------------------------------------------

    float child_biomass = biomass * simulation_config::biomass::child_biomass_ratio;
    biomass = biomass - child_biomass;

    std::uniform_real_distribution<float> mutation_dist(-0.05f, 0.05f);
    float mutation = mutation_dist(generator);
    float child_resistance = level_of_resistance + mutation;
    if (child_resistance < 0.0f) child_resistance = 0.0f;
    if (child_resistance > 1.0f) child_resistance = 1.0f;

    auto child = std::make_shared<active_Biomass>(child_resistance, max_age_of_cell);
    child->biomass = child_biomass;

    // ---- ЛОГИКА ПРЫЖКА (DISPERSION) ----
    std::uniform_real_distribution<float> dispersion_dist(0.0f, 1.0f);
    if (dispersion_dist(generator) < simulation_config::biomass::dispersion_chance) {
        int R = simulation_config::biomass::dispersion_radius;
        std::vector<Cell*> potential_targets;

        int start_x = std::max(0, x - R);
        int end_x = std::min(simulation_config::field::width - 1, x + R);
        int start_y = std::max(0, y - R);
        int end_y = std::min(simulation_config::field::height - 1, y + R);

        for (int i = start_x; i <= end_x; ++i) {
            for (int j = start_y; j <= end_y; ++j) {
                if (i == x && j == y) continue;
                Cell& target = current_field.get_nucleus(i, j);
                if (target.is_this_nucleus_free()) {
                    bool supported = false;
                    if (j == 0) {
                        supported = true;
                    }
                    else {
                        Cell& below = current_field.get_nucleus(i, j - 1);
                        if (!below.is_this_nucleus_free()) {
                            supported = true;
                        }
                    }
                    if (supported) {
                        potential_targets.push_back(&target);
                    }
                }
            }
        }

        if (!potential_targets.empty()) {
            std::uniform_int_distribution<std::size_t> target_dist(0, potential_targets.size() - 1);
            place_for_child = potential_targets[target_dist(generator)];
        }
    }
    // ----------------------------------

    place_for_child->set_cell(child);
    return true;
}
// ---------- nonactive_Biomass ----------
nonactive_Biomass::nonactive_Biomass(float resistance, int max_age) {
    level_of_resistance = resistance * resistance_multiplier;
    max_age_of_cell = static_cast<int>(max_age * max_life_multiplier);
}

void nonactive_Biomass::apply_dormancy_effects() {
    level_of_resistance *= resistance_multiplier;
    max_age_of_cell = static_cast<int>(max_age_of_cell * max_life_multiplier);
}

float nonactive_Biomass::baseline_resistance() const {
    if (resistance_multiplier <= 0.0f) return level_of_resistance;
    return level_of_resistance / resistance_multiplier;
}

int nonactive_Biomass::baseline_max_age() const {
    if (max_life_multiplier <= 0.0f) return max_age_of_cell;
    return static_cast<int>(max_age_of_cell / max_life_multiplier);
}

bool nonactive_Biomass::reproduction(Field& current_field, int x, int y) {
    Cell& nucleus = current_field.get_nucleus(x, y);

    if (steps_until_wakeup > 0) {
        steps_until_wakeup--;
        if (steps_until_wakeup == 0) {
            auto active_cell = std::make_shared<active_Biomass>();
            copy_common_state_to(*active_cell);
            active_cell->level_of_resistance = baseline_resistance();
            active_cell->max_age_of_cell = baseline_max_age();
            nucleus.set_cell(active_cell);
        }
    }
    else {
        float F = nucleus.get_food().get_amount();
        float potential_income = simulation_config::monod::Y_B_F *
            (simulation_config::monod::U_max * F / (simulation_config::monod::K_F + F) * simulation_config::monod::delta_t);
        float threshold = simulation_config::monod::m_act * simulation_config::monod::delta_t * biomass * simulation_config::monod::greed_coefficient;

        if (potential_income > threshold) {
            steps_until_wakeup = simulation_config::monod::steps_for_waking_up;
        }
    }

    return false;
}

// ---------- dead_Biomass ----------
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