#pragma once

class Food {
private:
    double amount = 0.0;

public:
    Food() = default;
    explicit Food(double start_amount);

    double get_amount() const;
    void set_amount(double value);
    void add(double value);
    double take(double wanted_amount);
};
