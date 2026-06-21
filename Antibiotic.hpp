#pragma once

class Antibiotic {
private:
    float concentration = 0.0f;

public:
    Antibiotic() = default;
    explicit Antibiotic(float start_concentration);

    float get_concentration() const;

    void add(float value);
    void decrease(float value);
};
