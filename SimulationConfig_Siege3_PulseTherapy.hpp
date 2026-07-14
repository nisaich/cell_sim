#pragma once

namespace simulation_config {

    // ============================================================
    // СЦЕНАРИЙ: Импульсная терапия (Pulse Antibiotic Therapy)
    // ============================================================
    // Самый интересный для наблюдения: антибиотик подаётся крупными
    // редкими импульсами (высокий concetration_for_next_step, высокий steps_for_adding).
    // Между волнами колония успевает частично восстановиться.
    //
    // Между пульсами:
    //   - выжившие клетки из ядра делятся, поднимая resistance потомства
    //   - купол начинает отрастать заново
    //   - концентрация антибиотика спадает (decay)
    //
    // При следующей волне:
    //   - внешние клетки нового роста гибнут
    //   - устойчивое ядро снова выживает
    //   - с каждой волной ядро немного вырастает → ЭВОЛЮЦИЯ УСТОЙЧИВОСТИ
    //
    // Видно: ритмичное "мигание" колонии + постепенное нарастание зелёного ядра
    // ============================================================

    namespace monod {
        inline constexpr double delta_t = 60.0;

        inline constexpr double U_max = 0.48 / 3600.0;
        inline constexpr double m_act = 0.048 / 3600.0;
        inline constexpr double m_inactiv = 0.000048 / 3600.0;

        inline constexpr double K_F = 1500.0;
        inline constexpr double Y_B_F = 0.45;

        inline constexpr double starvation_biomass_threshold = 0.2;
        inline constexpr double starvation_steps_threshold   = 3.0;
        inline constexpr double greed_coefficient = 1.3;
        inline constexpr int    steps_for_waking_up = 60;
    }

    namespace field {
        inline constexpr int width  = 100;
        inline constexpr int height = 200;

        // КРИТИЧНО: initial_food >> 428.6 (F_breakeven при Monod-параметрах Pseudomonas)
        inline constexpr double initial_food         = 2000.0;
        inline constexpr double food_diffusion_coeff = 0.20;

        // Частое добавление еды — купол успевает восстанавливаться между волнами
        inline constexpr int    steps_for_adding_food  = 200;
        inline constexpr double count_of_adding_food   = 5000.0;
    }

    namespace biomass {
        inline constexpr double dispersion_chance  = 0.001;
        inline constexpr int    dispersion_radius  = 10;

        inline constexpr double initial_biomass    = 0.5;
        inline constexpr double max_biomass        = 1.0;
        inline constexpr double child_biomass_ratio      = 0.5;
        inline constexpr double reproduction_min_biomass = 0.7;
        inline constexpr double reproduction_chance      = 0.5;
        inline constexpr int    default_max_age    = 2880;
        inline constexpr double default_resistance = 0.000015;

        inline constexpr double nonactive_resistance_multiplier = 2.5;
        inline constexpr double nonactive_max_life_multiplier   = 2.5;
        inline constexpr int    dead_steps_to_disappearance     = 60; // быстрее очищается — лучше видно рост
    }

    namespace antibiotic {
        inline constexpr double death_threshold = 2.0;
        inline constexpr double sleep_antibiotic_ratio = 0.70;

        inline constexpr double reproduction_penalty   = 0.35;
        inline constexpr double stress_transition_chance = 0.05;

        // Умеренная диффузия: волна проходит быстрее, но и падает быстрее
        inline constexpr double diffusion_coeff = 0.012;
        // БЫСТРЫЙ распад: между пульсами антибиотик рассасывается за ~500 шагов
        inline constexpr double decay_rate = 0.004;

        // ИМПУЛЬСНАЯ ДОЗИРОВКА:
        // Добавляем каждые 300 шагов большую дозу (5.0 единиц)
        // Затем она диффундирует и распадается: к следующему пульсу остаток ~0.1
        // За 300 шагов (5 часов) выжившие клетки успевают поделиться 2-3 раза
        inline constexpr double concetration_for_next_step     = 5.0; // крупный импульс
        inline constexpr double middle_value_of_antibiotic     = 4.0; // пополняем только если упало ниже 4
        inline constexpr double visualization_normalizer       = 6.0;

        // Умеренная индукция: резистентность строится постепенно за несколько волн
        inline constexpr double k_ind = 0.20;
        inline constexpr double K_ind = 0.80;
        // ВАЖНО: медленная релаксация — клетки "помнят" предыдущие волны
        inline constexpr double k_rec = 0.0005;
        inline constexpr double fitness_cost_coef = 0.04;
    }

    namespace visualization {
        inline constexpr int initial_window_width  = 1200;
        inline constexpr int initial_window_height = 900;

        inline constexpr int target_fps      = 0;
        inline constexpr int steps_per_frame = 500;

        inline constexpr int    graph_panel_width            = 400;
        inline constexpr int    modified_content_gap         = 20;
        inline constexpr int    modified_window_screen_margin = 100;
        inline constexpr int    number_of_step_to_diffuse    = 5;

        inline constexpr double min_brightness   = 0.35;
        inline constexpr double brightness_span  = 0.65;

        inline constexpr double modified_nutrition_normalizer = field::initial_food;

        inline constexpr unsigned char empty_cell_blue_r = 0;
        inline constexpr unsigned char empty_cell_blue_g = 100;
        inline constexpr unsigned char empty_cell_blue_b = 255;

        inline constexpr unsigned char empty_cell_r = 0;
        inline constexpr unsigned char empty_cell_g = 0;
        inline constexpr unsigned char empty_cell_b = 0;

        inline constexpr unsigned char active_cell_r = 0;
        inline constexpr unsigned char active_cell_g = 255;
        inline constexpr unsigned char active_cell_b = 0;

        inline constexpr unsigned char nonactive_cell_r = 255;
        inline constexpr unsigned char nonactive_cell_g = 165;
        inline constexpr unsigned char nonactive_cell_b = 0;

        inline constexpr unsigned char dead_cell_r = 128;
        inline constexpr unsigned char dead_cell_g = 128;
        inline constexpr unsigned char dead_cell_b = 128;

    }
}
