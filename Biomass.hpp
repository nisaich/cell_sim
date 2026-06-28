#pragma once

class Field;
class Food;

class abstract_Biomass {  //основной класс клетки
protected:
    int age_of_cell = 0;

//все данные параметры впоследствии необходимо будет менять(ну както подыскать реальные в природе)//
    int max_amount_of_food_consumed = 2;
    float biomass = 0.01;
    float using_food_for_step = 0 * biomass; //пока что можно поставить 0

    void copy_common_state_to(abstract_Biomass& other) const;

public:
    int max_age_of_cell = 90;
    float level_of_resistance = 0.0f;

    virtual ~abstract_Biomass() = default;

    int get_age() const;
    float get_level_of_resistance() const;
    float get_biomass() const;

    bool must_he_die() const;
    void increase_age();

    void food_consumption_from_environment(Food& food);

    virtual bool is_alive() const = 0;

    virtual bool reproduction(Field& current_field, int x, int y);
    
    virtual void depletion_of_savings();
    virtual void step_after_death();
    virtual bool should_be_removed_from_field() const;
};

class active_Biomass : public abstract_Biomass {
public:
    active_Biomass() = default;
    active_Biomass(float resistance, int max_age);

    bool is_alive() const override;
    bool reproduction(Field& current_field, int x, int y) override;
};

class nonactive_Biomass : public abstract_Biomass {
private:  
//неактивные клетки потребляют меньшее кол-во питания и имеют повышенный резист к антибиотику
    float resistance_multiplier = 2.0f;
    float food_usage_multiplier = 0.2f;
    float max_life_multiplier = 2.0f; 
public:
    nonactive_Biomass()=default;

    nonactive_Biomass(
        float resistance,
        int max_age,
        int max_food_consumed,
        int food_usage
    );

    bool is_alive() const override {
      return true;
    }
    
    void depletion_of_savings() override;

    bool reproduction(Field& current_field, int x, int y) override;
};

class dead_Biomass : public abstract_Biomass {
private:
    int count_of_steps_to_disappearance = 30;

public:
    int count_of_steps_from_death = 0;

    void step_after_death() override;
    bool should_be_removed_from_field() const override;

    bool is_it_still_there() const;
    bool is_alive() const override;
};
