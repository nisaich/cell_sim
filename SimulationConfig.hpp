#pragma once

namespace simulation_config {

    namespace monod {
        inline constexpr double delta_t = 60.0;

        inline constexpr double U_max    = 0.48  / 3600.0;
        inline constexpr double m_act    = 0.048 / 3600.0;
        inline constexpr double m_inactiv= 0.000048 / 3600.0;
        inline constexpr double K_F      = 1500.0;
        inline constexpr double Y_B_F    = 0.45;

        inline constexpr double starvation_biomass_threshold = 0.2;
        inline constexpr double starvation_steps_threshold   = 5.0;
        inline constexpr double greed_coefficient = 1.3;
        inline constexpr int    steps_for_waking_up = 120; // медленнее просыпаются -> меньше риск проснуться прямо под антибиотиком
    }

    namespace field {
        inline constexpr int width  = 100;
        inline constexpr int height = 200;

        inline constexpr double initial_food         = 600.0;

        // Снижено с 0.20: при высокой диффузии еда мгновенно "перетекает" обратно
        // в съеденные клетки, градиент питания у фронта колонии смазывается,
        // и колония растёт сплошным пятном (Eden-модель).
        // При низком D локальное истощение еды опережает восполнение —
        // возникает диффузионно-лимитированный режим роста (эффект экранирования
        // внутренних/вогнутых участков), который и даёт ветвящиеся дендриты.
        inline constexpr double food_diffusion_coeff = 0.02;

        inline constexpr int    steps_for_adding_food  = 10000000;
        inline constexpr double count_of_adding_food   = 0.0;
    }

    namespace biomass {
        inline constexpr double dispersion_chance  = 0.001;
        inline constexpr int    dispersion_radius  = 10;

        inline constexpr double initial_biomass    = 0.5;
        inline constexpr double max_biomass        = 1.0;
        inline constexpr double child_biomass_ratio      = 0.5;

        inline constexpr double reproduction_min_biomass = 0.7;
        inline constexpr double reproduction_chance      = 0.01;

        // --- ветвление / tip-biased growth ---
        // Множитель вероятности размножения зависит от "открытости" клетки:
        // exposure = (кол-во свободных соседей) / 4.
        // Клетка на кончике ветки (открыта со всех сторон, exposure=1) размножается
        // с полным шансом; клетка, зажатая в углублении контура (exposure -> 0),
        // получает множитель ~branching_min_factor. Это усиливает естественный
        // эффект экранирования и превращает фронт роста в ветвящиеся дендриты
        // вместо равномерного расползания пятна.
        // 1.0 = ветвление выключено (как было раньше), меньше значение = более
        // тонкие и выраженные ветки.
        inline constexpr double branching_min_factor = 0.12;

        inline constexpr int    default_max_age    = 2880;

        inline constexpr double default_resistance = 0.0005; // небольшой стартовый запас устойчивости

        inline constexpr double nonactive_resistance_multiplier = 400.0;  // сильно, но не абсолютно неубиваемо
        inline constexpr double nonactive_max_life_multiplier   = 400.0;
        inline constexpr int    dead_steps_to_disappearance     = 100;
    }

    namespace antibiotic {
        inline constexpr double death_threshold = 3.0; // больше запас прочности перед гибелью

        inline constexpr double sleep_antibiotic_ratio = 0.55; // уход в спячку заранее, до критической концентрации

        inline constexpr double reproduction_penalty     = 0.3;
        inline constexpr double stress_transition_chance = 0.05;

        inline constexpr double diffusion_coeff = 0.0009; // антибиотик подходит медленнее — колония успевает адаптироваться
        inline constexpr double decay_rate      = 0.00004; // фоновое давление не растёт бесконечно, выходит на плато

        inline constexpr double concetration_for_next_step = 0.0;  // более плавный прирост давления
        inline constexpr double middle_value_of_antibiotic = 0.0;
        inline constexpr double visualization_normalizer   = 10.0;

        inline constexpr double k_ind = 0.25;   // быстрее индукция резистентности
        inline constexpr double K_ind = 0.60;   // чувствительность к антибиотику выше (насосы включаются раньше)
        inline constexpr double k_rec = 0.0003; // медленнее теряется приобретённая устойчивость
        inline constexpr double fitness_cost_coef = 0.015; // меньше штраф за резистентность к росту
    }

    namespace visualization {
        inline constexpr int initial_window_width  = 1200;
        inline constexpr int initial_window_height = 900;

        inline constexpr int target_fps      = 0;
        inline constexpr int steps_per_frame = 100;

        inline constexpr int    graph_panel_width            = 400;
        inline constexpr int    modified_content_gap         = 20;
        inline constexpr int    modified_window_screen_margin = 100;
        inline constexpr int    number_of_step_to_diffuse    = 1;

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
