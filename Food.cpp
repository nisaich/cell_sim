#include "Food.hpp"

#include <algorithm>
#include <cmath>

Food::Food(float start_amount)
    : amount(start_amount) {}

float Food::get_amount() const {
    return amount;
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

    if (std::isinf(amount)) {
        return wanted_amount;
    }

    int available = static_cast<int>(amount);
    int taken = std::min(available, wanted_amount);

    amount -= static_cast<float>(taken);

    return taken;
}
