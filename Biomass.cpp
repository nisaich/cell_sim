#include "Biomass.hpp"
#include "Field.hpp"
#include "Food.hpp"
#include "Antibiotic.hpp"
#include <algorithm>
#include <cmath>
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

double abstract_Biomass::get_level_of_resistance() const {
    return level_of_resistance;
}

double abstract_Biomass::get_biomass() const {
    return biomass;
}

bool abstract_Biomass::must_he_die(Field& current_field, int x, int y) const {
    if (age_of_cell >= max_age_of_cell || biomass <= 0.001) return true;

    Cell& nucleus_ref = current_field.get_nucleus(x, y);

    // Смерть от голода (если биомасса ниже порога спячки и еды в среде нет)
    if (biomass < simulation_config::monod::starvation_biomass_threshold && nucleus_ref.get_food().get_amount() <= 0.0) {
        return true;
    }

    // Смерть от антибиотика
    double conc = nucleus_ref.get_antibiotic().get_concentration();
    double excess = conc - level_of_resistance;
    if (excess > 0.0) {
        if (excess > simulation_config::antibiotic::death_threshold) {
            nucleus_ref.get_antibiotic().set_concentration(excess);
            return true;
        }
    }

    return false;
}

void abstract_Biomass::increase_age() {
    if (is_active_for_monod() > 0.0) {
        ++age_of_cell;
    }
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
    double a = is_active_for_monod();
    double F = food.get_amount();

    // Плата за активные эффлюксные насосы: чем выше ощить, тем медленнее клетка ест (fitness cost)
    double fitness_penalty = 1.0 - simulation_config::antibiotic::fitness_cost_coef * level_of_resistance;

    double mu = 0.0; // удельная скорость роста (1/с) по уравнению Моно
    if (F > 0.0) {
        mu = simulation_config::monod::U_max * (F / (simulation_config::monod::K_F + F)) * a * fitness_penalty;
    }

    // Классическая кинетика Моно: dB/dt = mu * B
    double desired_biomass_growth = mu * simulation_config::monod::delta_t * biomass;
    double desired_food = (simulation_config::monod::Y_B_F > 0.0)
        ? (desired_biomass_growth / simulation_config::monod::Y_B_F)
        : 0.0;

    double space_limit = std::max(0.0, (simulation_config::biomass::max_biomass - biomass) / simulation_config::monod::Y_B_F);

    double delta_Food = std::min({ F, desired_food, space_limit });
    if (delta_Food < 0.0) delta_Food = 0.0;

    food.take(delta_Food);

    biomass += simulation_config::monod::Y_B_F * delta_Food;

    double m = get_maintenance_rate();
    biomass = std::max(0.0, biomass * (1.0 - m * simulation_config::monod::delta_t));
}

bool abstract_Biomass::reproduction(Field& /*current_field*/, int /*x*/, int /*y*/) {
    return false;
}

// ---------- active_Biomass ----------
active_Biomass::active_Biomass(double resistance, int max_age) {
    level_of_resistance = resistance;
    max_age_of_cell = max_age;
}

bool active_Biomass::is_alive() const {
    return true;
}

void active_Biomass::consume_and_decay(Food& food) {
    abstract_Biomass::consume_and_decay(food);
    steps_active++;

    if (nucleus == nullptr) return;

    double dt = simulation_config::monod::delta_t;
    double conc = nucleus->get_antibiotic().get_concentration();

    // 1. Проверка по количеству оставшейся еды (на сколько шагов обслуживания её хватит)
    double maintenance_food_per_step = (simulation_config::monod::m_act * dt * biomass) / simulation_config::monod::Y_B_F;
    double steps_food_will_last = (maintenance_food_per_step > 0.0) ? (food.get_amount() / maintenance_food_per_step) : 999999.0;
    bool should_sleep_due_to_food = (steps_food_will_last < simulation_config::monod::starvation_steps_threshold);

    // 2. Проверка по уровню антибиотика (близкое к порогу смерти содержание)
    bool should_sleep_due_to_antibiotic = (conc >= simulation_config::antibiotic::sleep_antibiotic_ratio * simulation_config::antibiotic::death_threshold);

    // Если выполняется хотя бы одно из условий, переходим в спящее состояние
    if (should_sleep_due_to_food || should_sleep_due_to_antibiotic) {
        int ticks_needed = static_cast<int>(simulation_config::monod::steps_for_waking_up / dt);
        if (ticks_needed < 1) ticks_needed = 1;
        if (steps_active >= ticks_needed) {
            auto nonactive = std::make_shared<nonactive_Biomass>();
            copy_common_state_to(*nonactive);
            nonactive->apply_dormancy_effects();
            nucleus->set_cell(nonactive);
            return;
        }
    }

    // Физиологическая адаптация резистентности (без деления, без случайности)
    double r_default = simulation_config::biomass::default_resistance;

    if (conc > 0.0) {
        // Режим индукции: насосы включаются пропорционально концентрации (закон Моно)
        double delta_r = simulation_config::antibiotic::k_ind
            * (conc / (simulation_config::antibiotic::K_ind + conc))
            * (1.0 - level_of_resistance)
            * dt;
        level_of_resistance = std::min(1.0, level_of_resistance + delta_r);
    }
    else {
        // Режим релаксации: насосы отключаются, клетка возвращается к базовому состоянию
        double delta_r = simulation_config::antibiotic::k_rec
            * (level_of_resistance - r_default)
            * dt;
        level_of_resistance = std::max(r_default, level_of_resistance - delta_r);
    }
}

bool active_Biomass::reproduction(Field& current_field, int x, int y) {
    std::vector<Cell*> free_neighbours = current_field.get_free_moore_neighbours(x, y);
    if (max_count_reps == 0) return false;
    if (free_neighbours.empty()) return false;
    if (biomass < simulation_config::biomass::reproduction_min_biomass) return false;

    static std::mt19937 generator(std::random_device{}());

    double current_chance = 1.0;
    if (nucleus != nullptr) {
        double conc = nucleus->get_antibiotic().get_concentration();
        double excess = conc - level_of_resistance;
        if (excess > 0.0) {
            double penalty = std::min(excess * 0.1, static_cast<double>(simulation_config::antibiotic::reproduction_penalty));
            current_chance *= (1.0 - penalty);
        }
    }

    if (current_chance < 1.0) {
        std::uniform_real_distribution<double> chance_distribution(0.0, 1.0);
        if (chance_distribution(generator) > current_chance) return false;
    }

    // ---- ВЕТВИСТЫЙ (ДЕНДРИТНЫЙ) ВЫБОР МЕСТА ДЛЯ ДОЧЕРНЕЙ КЛЕТКИ ----
    const double sharpness = simulation_config::biomass::branching_sharpness;
    const double food_weight = simulation_config::biomass::branching_food_weight;
    const double min_weight = simulation_config::biomass::branching_min_weight;

    double max_food_found = 0.0;
    for (Cell* nb : free_neighbours) {
        max_food_found = std::max(max_food_found, nb->get_food().get_amount());
    }

    std::vector<double> weights;
    weights.reserve(free_neighbours.size());

    for (Cell* nb : free_neighbours) {
        std::array<int, 2> coords = nb->coordinates();

        int occupied = 0;
        for (Cell* around : current_field.get_moore_neighbours(coords[0], coords[1])) {
            if (!around->is_this_nucleus_free()) ++occupied;
        }

        double exposure_weight = 1.0 / std::pow(1.0 + static_cast<double>(occupied), sharpness);

        double food_ratio = (max_food_found > 1e-9) ? (nb->get_food().get_amount() / max_food_found) : 1.0;
        double food_bonus = 1.0 + food_weight * food_ratio;

        weights.push_back(std::max(min_weight, exposure_weight * food_bonus));
    }

    // ---------- ДОБАВЛЯЕМ СЛУЧАЙНЫЙ ШУМ ДЛЯ ЕСТЕСТВЕННОСТИ ----------
    std::uniform_real_distribution<double> noise(0.9, 1.1);
    for (auto& w : weights) w *= noise(generator);
    // --------------------------------------------------------------

    std::discrete_distribution<std::size_t> place_distribution(weights.begin(), weights.end());
    Cell* place_for_child = free_neighbours[place_distribution(generator)];

    double child_biomass = biomass * simulation_config::biomass::child_biomass_ratio;
    biomass = biomass - child_biomass;
    age_of_cell = 0; // Сброс возраста родителя при делении

    auto child = std::make_shared<active_Biomass>(level_of_resistance, max_age_of_cell);
    child->biomass = child_biomass;

    // ---- ЛОГИКА ПРЫЖКА (DISPERSION) ----
    std::uniform_real_distribution<double> dispersion_dist(0.0, 1.0);
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
                    for (Cell* nb : current_field.get_neighbours(i, j)) {
                        if (!nb->is_this_nucleus_free()) {
                            supported = true;
                            break;
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

    current_field.place_cell(place_for_child->coordinates()[0], place_for_child->coordinates()[1], child);
    return true;
}

// ---------- nonactive_Biomass ----------
nonactive_Biomass::nonactive_Biomass(double resistance, int max_age) {
    level_of_resistance = resistance * resistance_multiplier;
    max_age_of_cell = static_cast<int>(max_age * max_life_multiplier);
}

void nonactive_Biomass::apply_dormancy_effects() {
    level_of_resistance *= resistance_multiplier;
    max_age_of_cell = static_cast<int>(max_age_of_cell * max_life_multiplier);
}

double nonactive_Biomass::baseline_resistance() const {
    if (resistance_multiplier <= 0.0) return level_of_resistance;
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
        double F = nucleus.get_food().get_amount();
        double potential_income = simulation_config::monod::Y_B_F *
            (simulation_config::monod::U_max * F / (simulation_config::monod::K_F + F) * simulation_config::monod::delta_t);
        double threshold = simulation_config::monod::m_act * simulation_config::monod::delta_t * biomass * simulation_config::monod::greed_coefficient;

        if (potential_income > threshold) {
            int ticks_needed = static_cast<int>(simulation_config::monod::steps_for_waking_up / simulation_config::monod::delta_t);
            if (ticks_needed < 1) ticks_needed = 1;
            steps_until_wakeup = ticks_needed;
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