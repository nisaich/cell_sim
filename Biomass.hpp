#pragma once

#include "SimulationConfig.hpp"

class Field;
class Food;
class Cell;
class Antibiotic;

class abstract_Biomass {
    friend class Field;
protected:
    int age_of_cell = 0;
    int max_count_reps = simulation_config::biomass::max_count_reps;
    float max_amount_of_food_consumed = simulation_config::biomass::default_max_food_consumed;
    int steps_for_nonactivating = simulation_config::biomass::steps_for_nonactivating;
    float biomass = simulation_config::biomass::initial_biomass;
    float using_food_for_step = simulation_config::biomass::food_usage_per_step;
    int steps_to_live_forward = simulation_config::biomass::steps_to_live_forward;
    Cell* nucleus = nullptr;
    void copy_common_state_to(abstract_Biomass& other) const;

public:
    int max_age_of_cell = simulation_config::biomass::default_max_age;
    float level_of_resistance = simulation_config::biomass::default_resistance;

    virtual ~abstract_Biomass() = default;

    int get_age() const;
    float get_level_of_resistance() const;
    float get_biomass() const;

    // Теперь must_he_die принимает антибиотик
    bool must_he_die(Food& food, const Antibiotic& antibiotic) const;
    void increase_age();
    void set_nucleus(Cell* current_nucleus);

    virtual bool is_alive() const = 0;

    virtual bool reproduction(Field& current_field, int x, int y);

    virtual void consume_and_decay(Food& food);
    virtual float is_active_for_monod() const = 0;
    virtual float get_maintenance_rate() const = 0;

    virtual void step_after_death();
    virtual bool should_be_removed_from_field() const;
};

class active_Biomass : public abstract_Biomass {
public:
    active_Biomass() = default;
    active_Biomass(float resistance, int max_age);

    bool is_alive() const override;
    bool reproduction(Field& current_field, int x, int y) override;
    void consume_and_decay(Food& food) override;
    float is_active_for_monod() const override { return 1.0f; }
    float get_maintenance_rate() const override { return simulation_config::monod::m_act; }
};

class nonactive_Biomass : public abstract_Biomass {
private:
    float resistance_multiplier = simulation_config::biomass::nonactive_resistance_multiplier;
    float food_usage_multiplier = simulation_config::biomass::nonactive_food_usage_multiplier;
    float max_life_multiplier = simulation_config::biomass::nonactive_max_life_multiplier;
public:
    nonactive_Biomass() = default;
    nonactive_Biomass(
        float resistance,
        int max_age,
        float max_food_consumed,
        float food_usage
    );

    bool is_alive() const override { return true; }
    bool reproduction(Field& current_field, int x, int y) override;
    
    float is_active_for_monod() const override { return 0.0f; }
    float get_maintenance_rate() const override { return simulation_config::monod::m_inactiv; }

    // Применяет коэффициенты дормантности (устойчивость/возраст/расход пищи)
    // к уже скопированному общему состоянию клетки. Вызывается сразу после
    // copy_common_state_to при переходе active -> nonactive.
    void apply_dormancy_effects();

    // Обратные величины для восстановления "обычных" параметров клетки
    // при переходе nonactive -> active.
    float baseline_resistance() const;
    int baseline_max_age() const;
};

class dead_Biomass : public abstract_Biomass {
private:
    int count_of_steps_to_disappearance = simulation_config::biomass::dead_steps_to_disappearance;
public:
    int count_of_steps_from_death = 0;
    
    float is_active_for_monod() const override { return 0.0f; }
    float get_maintenance_rate() const override { return 0.0f; }
    
    void step_after_death() override;
    bool should_be_removed_from_field() const override;
    bool is_it_still_there() const;
    bool is_alive() const override;
};