#pragma once

class Food {
private:
    float amount = 0.0f;

public:
    Food() = default;
    explicit Food(float start_amount);

    float get_amount() const;
    void set_amount(float value);
    void add(float value);
    float take(float wanted_amount);
};
