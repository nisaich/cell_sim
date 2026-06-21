#pragma once

class Food {
private:
    float amount = 0.0f;

public:
    Food() = default;
    explicit Food(float start_amount);

    float get_amount() const;

    void add(float value);
    int take(int wanted_amount);
};
