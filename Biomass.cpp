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

bool abstract_Biomass::must_he_die(Food& food, const Antibiotic& antibiotic) const {
    // Старые условия (для истощения порог поднят до 0.001 из-за асимптотического убывания Моно)
    if (age_of_cell >= max_age_of_cell || biomass <= 0.001f) return true;

    // Влияние антибиотика
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
    
    // Формула 1: u_t
    float u = 0.0f;
    if (F > 0.0f) {
        u = simulation_config::monod::U_max * (F / (simulation_config::monod::K_F + F)) * a;
    }
    
    // Формула 2: Delta Food
    float u_dt = u * simulation_config::monod::delta_t;
    float space_limit = std::max(0.0f, (simulation_config::biomass::max_biomass - biomass) / simulation_config::monod::Y_B_F);
    
    float delta_Food = std::min({F, u_dt, space_limit});
    if (delta_Food < 0.0f) delta_Food = 0.0f;
    
    food.take(delta_Food);
    
    // Формула 3: Биомасса после потребления
    biomass += simulation_config::monod::Y_B_F * delta_Food;
    
    // Формула 4: Трата на поддержание жизнедеятельности
    float m = get_maintenance_rate();
    biomass = std::max(0.0f, biomass * (1.0f - m));
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
    // 1. Стандартное поведение (питание по Моно и базовый распад)
    abstract_Biomass::consume_and_decay(food);

    // 2. Проверка на стресс от антибиотика (переход в неактивное состояние)
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
    std::uniform_real_distribution<float> chance_distribution(0.00f, 1.00f);

    float reproduction_chance = simulation_config::biomass::reproduction_chance;

    // Влияние антибиотика
    if (nucleus != nullptr) {
        float conc = nucleus->get_antibiotic().get_concentration();
        float excess = conc - level_of_resistance;
        if (excess > 0.0f) {
            float penalty = std::min(excess * 0.1f, simulation_config::antibiotic::reproduction_penalty);
            reproduction_chance *= (1.0f - penalty);
            if (reproduction_chance < 0.0f) reproduction_chance = 0.0f;
        }
    }

    if (chance_distribution(generator) > reproduction_chance) return false;

    // Выбираем случайного свободного соседа (науч. рук. сказал удалить умный поиск еды)
    std::uniform_int_distribution<std::size_t> distribution(0, free_neighbours.size() - 1);
    Cell* place_for_child = free_neighbours[distribution(generator)];

    float child_biomass = biomass * simulation_config::biomass::child_biomass_ratio;
    biomass = biomass - child_biomass;

    // ----- НОВАЯ ЛОГИКА: мутация резистентности -----
    // Генерируем небольшое случайное изменение (от -0.05 до +0.05)
    std::uniform_real_distribution<float> mutation_dist(-0.05f, 0.05f);
    float mutation = mutation_dist(generator);
    float child_resistance = level_of_resistance + mutation;
    // Ограничиваем, чтобы не выходить за разумные пределы (0..1)
    if (child_resistance < 0.0f) child_resistance = 0.0f;
    if (child_resistance > 1.0f) child_resistance = 1.0f;
    // ------------------------------------------------

    auto child = std::make_shared<active_Biomass>(
        child_resistance,     // теперь с мутацией
        max_age_of_cell
    );
    child->biomass = child_biomass;
    child->max_amount_of_food_consumed = max_amount_of_food_consumed;
    child->using_food_for_step = using_food_for_step;

    place_for_child->set_cell(child);
    return true;
}

// ---------- nonactive_Biomass ----------
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

void nonactive_Biomass::apply_dormancy_effects() {
    level_of_resistance *= resistance_multiplier;
    max_age_of_cell = static_cast<int>(max_age_of_cell * max_life_multiplier);
    using_food_for_step *= food_usage_multiplier;
}

float nonactive_Biomass::baseline_resistance() const {
    if (resistance_multiplier <= 0.0f) return level_of_resistance;
    return level_of_resistance / resistance_multiplier;
}

int nonactive_Biomass::baseline_max_age() const {
    if (max_life_multiplier <= 0.0f) return max_age_of_cell;
    return static_cast<int>(max_age_of_cell / max_life_multiplier);
}

// depletion_of_savings удалён, используется consume_and_decay из abstract_Biomass

bool nonactive_Biomass::reproduction(Field& current_field, int x, int y) {
    const float food_now = simulation_config::biomass::food_usage_for_step;
    Cell& nucleus = current_field.get_nucleus(x, y);
    if (nucleus.get_food().get_amount() < steps_to_live_forward * food_now) {
        return false;
    }
    else if (steps_for_nonactivating > 0) {
        steps_for_nonactivating--;
    }
    else if (steps_for_nonactivating == 0) {
        auto active_cell = std::make_shared<active_Biomass>();
        copy_common_state_to(*active_cell);
        // Возвращаем базовые (немодифицированные дормантностью) параметры.
        active_cell->level_of_resistance = baseline_resistance();
        active_cell->max_age_of_cell = baseline_max_age();
        nucleus.set_cell(active_cell);
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
