#pragma once

class Field;
class Food;

class abstract_Cell {  //основной класс клетки
protected:
    int age_of_cell = 0;

//все данные параметры впоследствии необходимо будет менять(ну както подыскать реальные в природе)//
    int max_food_inside = 30;
    int max_amount_of_food_consumed = 2;
    float using_food_for_step = 1;

public:
    int max_age_of_cell = 90;
    int food_inside = 5;
    float level_of_resistance = 0.0f;

    virtual ~abstract_Cell() = default;

    int get_age() const;
    float get_level_of_resistance() const;
    int get_food_inside() const;

    bool must_he_die() const;
    void increase_age();

    void food_consumption_from_environment(Food& food);
    void depletion_of_savings();

    virtual bool is_alive() const = 0;

    virtual bool reproduction(Field& current_field, int x, int y);

    virtual void step_after_death();
    virtual bool should_be_removed_from_field() const;
};

class active_Cell : public abstract_Cell {
public:
    active_Cell() = default;
    active_Cell(int start_food, float resistance = 0.0f, int max_age = 30);

    bool is_alive() const override;
    bool reproduction(Field& current_field, int x, int y) override;
};

class nonactive_Cell : public abstract_Cell {
private:  
//неактивные клетки потребляют меньшее кол-во питания и имеют повышенный резист к антибиотику
    float resistance_multiplier = 1.0f;
    float food_usage_multiplier = 1.0f;

public:
    bool is_alive() const override;
};

class dead_Cell : public abstract_Cell {
private:
    int count_of_steps_to_disappearance = 5;

public:
    int count_of_steps_from_death = 0;

    void step_after_death() override;
    bool should_be_removed_from_field() const override;


    bool is_it_still_there() const;
    bool is_alive() const override;
};
