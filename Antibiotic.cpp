#include "Antibiotic.hpp"

Antibiotic::Antibiotic(double start_concentration)
    : concentration(start_concentration) {}

double Antibiotic::get_concentration() const {
    return concentration;
}

void Antibiotic::set_concentration(double value) {
    concentration = (value >= 0.0) ? value : 0.0;
}

void Antibiotic::add(double value) {
    if (value > 0.0) {
        concentration += value;
    }
}

void Antibiotic::decrease(double value) {
    if (value <= 0.0) {
        return;
    }

    concentration -= value;

    if (concentration < 0.0) {
        concentration = 0.0;
    }
}
