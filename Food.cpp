#include "Food.hpp"

#include <algorithm>
#include <cmath>

Food::Food(float start_amount)
    : amount(start_amount) {}

float Food::get_amount() const {
    return amount;
}

void Food::set_amount(float value) {
    amount = (value >= 0.0f) ? value : 0.0f;
}

void Food::add(float value) {
    if (value > 0.0f) {
        amount += value;
    }
}

float Food::take(float wanted_amount) {
    if (wanted_amount <= 0.0f) {
        return 0.0f;
    }

    float taken = std::min(amount, wanted_amount);

    amount -= taken;

    return taken;
}
