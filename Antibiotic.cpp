#include "Antibiotic.hpp"

Antibiotic::Antibiotic(float start_concentration)
    : concentration(start_concentration) {}

float Antibiotic::get_concentration() const {
    return concentration;
}

void Antibiotic::set_concentration(float value) {
    concentration = (value >= 0.0f) ? value : 0.0f;
}

void Antibiotic::add(float value) {
    if (value > 0.0f) {
        concentration += value;
    }
}

void Antibiotic::decrease(float value) {
    if (value <= 0.0f) {
        return;
    }

    concentration -= value;

    if (concentration < 0.0f) {
        concentration = 0.0f;
    }
}