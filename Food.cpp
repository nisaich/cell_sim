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

int Food::take(int wanted_amount) {
    if (wanted_amount <= 0) {
        return 0;
    }

    int available = static_cast<int>(amount);
    int taken = std::min(available, wanted_amount);

    amount -= static_cast<float>(taken);

    return taken;
}
