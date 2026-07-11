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
    float conc = nucleus_ref.get_antibiotic().get_concentration();
    float excess = conc - level_of_resistance;
    if (excess > 0.0f) {
        if (excess > simulation_config::antibiotic::death_threshold) {
            nucleus_ref.get_antibiotic().set_concentration(excess);
            return true;
        }
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
    double a = is_active_for_monod();
    double F = food.get_amount();

    // Плата за активные эффлюксные насосы: чем выше ощить, тем медленнее клетка ест (fitness cost)
    double fitness_penalty = 1.0 - simulation_config::antibiotic::fitness_cost_coef * level_of_resistance;

    double u = 0.0;
    if (F > 0.0) {
        u = simulation_config::monod::U_max * (F / (simulation_config::monod::K_F + F)) * a * fitness_penalty;
    }

    double u_dt = u * simulation_config::monod::delta_t;
    double space_limit = std::max(0.0, (simulation_config::biomass::max_biomass - biomass) / simulation_config::monod::Y_B_F);

    double delta_Food = std::min({ F, u_dt, space_limit });
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
active_Biomass::active_Biomass(float resistance, int max_age) {
    level_of_resistance = resistance;
    max_age_of_cell = max_age;
}

bool active_Biomass::is_alive() const {
    return true;
}

void active_Biomass::consume_and_decay(Food& food) {
    abstract_Biomass::consume_and_decay(food);
    steps_active++;

    // Переход в дормантное состояние при длительном голодании
    if (nucleus != nullptr && biomass < simulation_config::monod::starvation_biomass_threshold) {
        int ticks_needed = static_cast<int>(simulation_config::monod::steps_for_waking_up / simulation_config::monod::delta_t);
        if (ticks_needed < 1) ticks_needed = 1;
        if (steps_active >= ticks_needed) {
            auto nonactive = std::make_shared<nonactive_Biomass>();
            copy_common_state_to(*nonactive);
            nonactive->apply_dormancy_effects();
            nucleus->set_cell(nonactive);
            return;
        }
    }

    if (nucleus == nullptr) return;

    // Физиологическая адаптация резистентности (без деления, без случайности)
    // Индукция (по закону Моно): есть антибиотик → поднимаем щиты
    // Релаксация: нет антибиотика → возвращаемся к "дикому" уровню
    float conc = nucleus->get_antibiotic().get_concentration();
    float dt = static_cast<float>(simulation_config::monod::delta_t);
    float r_default = simulation_config::biomass::default_resistance;

    if (conc > 0.0f) {
        // Режим индукции: насосы включаются пропорционально концентрации (закон Моно)
        float delta_r = simulation_config::antibiotic::k_ind
                        * (conc / (simulation_config::antibiotic::K_ind + conc))
                        * (1.0f - level_of_resistance)
                        * dt;
        level_of_resistance = std::min(1.0f, level_of_resistance + delta_r);
    } else {
        // Режим релаксации: насосы отключаются, клетка возвращается к базовому состоянию
        float delta_r = simulation_config::antibiotic::k_rec
                        * (level_of_resistance - r_default)
                        * dt;
        level_of_resistance = std::max(r_default, level_of_resistance - delta_r);
    }

    // Переход в спящее состояние из-за стресса от антибиотика
    float excess = conc - level_of_resistance;
    if (excess > 0.0f) {
        static std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float base_chance = std::min(excess * 0.01f, static_cast<float>(simulation_config::antibiotic::stress_transition_chance));
        // Адаптация вероятности к размеру шага dt
        float adjusted_chance = 1.0f - std::pow(1.0f - base_chance, dt);
        if (dist(gen) < adjusted_chance) {
            auto nonactive = std::make_shared<nonactive_Biomass>();
            copy_common_state_to(*nonactive);
            nonactive->apply_dormancy_effects();
            nucleus->set_cell(nonactive);
            return;
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
            float penalty = std::min(excess * 0.1f, static_cast<float>(simulation_config::antibiotic::reproduction_penalty));
            current_chance *= (1.0f - penalty);
        }
    }

    if (current_chance < 1.0f) {
        std::uniform_real_distribution<float> chance_distribution(0.00f, 1.00f);
        if (chance_distribution(generator) > current_chance) return false;
    }

    // Выбираем случайного свободного соседа
    std::uniform_int_distribution<std::size_t> distribution(0, free_neighbours.size() - 1);
    Cell* place_for_child = free_neighbours[distribution(generator)];

    float child_biomass = biomass * simulation_config::biomass::child_biomass_ratio;
    biomass = biomass - child_biomass;

    auto child = std::make_shared<active_Biomass>(level_of_resistance, max_age_of_cell);
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
