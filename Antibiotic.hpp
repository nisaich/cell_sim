#pragma once

class Antibiotic {
private:
    double concentration = 0.0;

public:
    Antibiotic() = default;
    explicit Antibiotic(double start_concentration);

    double get_concentration() const;
    void set_concentration(double value);   // новый метод
    void add(double value);
    void decrease(double value);
};
