#include "Food.hpp"

#include <algorithm>
#include <cmath>

Food::Food(double start_amount)
    : amount(start_amount) {}

double Food::get_amount() const {
    return amount;
}

void Food::set_amount(double value) {
    amount = (value >= 0.0) ? value : 0.0;
}

void Food::add(double value) {
    if (value > 0.0) {
        amount += value;
    }
}

double Food::take(double wanted_amount) {
    if (wanted_amount <= 0.0) {
        return 0.0;
    }

    double taken = std::min(amount, wanted_amount);

    amount -= taken;

    return taken;
}
