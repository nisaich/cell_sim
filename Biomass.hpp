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
    double biomass = simulation_config::biomass::initial_biomass;
    Cell* nucleus = nullptr;
    void copy_common_state_to(abstract_Biomass& other) const;

public:
    int max_age_of_cell = simulation_config::biomass::default_max_age;
    double level_of_resistance = simulation_config::biomass::default_resistance;

    virtual ~abstract_Biomass() = default;

    int get_age() const;
    double get_level_of_resistance() const;
    double get_biomass() const;

    // Теперь must_he_die принимает антибиотик
    bool must_he_die(Field& current_field, int x, int y) const;
    void increase_age();
    void set_nucleus(Cell* current_nucleus);

    virtual bool is_alive() const = 0;

    virtual bool reproduction(Field& current_field, int x, int y);

    virtual void consume_and_decay(Food& food);
    virtual double is_active_for_monod() const = 0;
    virtual double get_maintenance_rate() const = 0;

    virtual void step_after_death();
    virtual bool should_be_removed_from_field() const;
};

class active_Biomass : public abstract_Biomass {
private:
    int steps_active = 0;
public:
    active_Biomass() = default;
    active_Biomass(double resistance, int max_age);

    bool is_alive() const override;
    bool reproduction(Field& current_field, int x, int y) override;
    void consume_and_decay(Food& food) override;
    double is_active_for_monod() const override { return 1.0; }
    double get_maintenance_rate() const override { return simulation_config::monod::m_act; }
};

class nonactive_Biomass : public abstract_Biomass {
private:
    double resistance_multiplier = simulation_config::biomass::nonactive_resistance_multiplier;
    double max_life_multiplier = simulation_config::biomass::nonactive_max_life_multiplier;
    int steps_until_wakeup = 0;
public:
    nonactive_Biomass() = default;
    nonactive_Biomass(
        double resistance,
        int max_age
    );

    bool is_alive() const override { return true; }
    bool reproduction(Field& current_field, int x, int y) override;
    
    double is_active_for_monod() const override { return 0.0; }
    double get_maintenance_rate() const override { return simulation_config::monod::m_inactiv; }

    // Применяет коэффициенты дормантности (устойчивость/возраст/расход пищи)
    // к уже скопированному общему состоянию клетки. Вызывается сразу после
    // copy_common_state_to при переходе active -> nonactive.
    void apply_dormancy_effects();

    // Обратные величины для восстановления "обычных" параметров клетки
    // при переходе nonactive -> active.
    double baseline_resistance() const;
    int baseline_max_age() const;
};

class dead_Biomass : public abstract_Biomass {
private:
    int count_of_steps_to_disappearance = simulation_config::biomass::dead_steps_to_disappearance;
public:
    int count_of_steps_from_death = 0;
    
    double is_active_for_monod() const override { return 0.0; }
    double get_maintenance_rate() const override { return 0.0; }
    
    void step_after_death() override;
    bool should_be_removed_from_field() const override;
    bool is_it_still_there() const;
    bool is_alive() const override;
};
