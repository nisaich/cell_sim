#pragma once

namespace simulation_config {


    //у нас микроб:pseudomonas putida, чтоб никто не забывал
    namespace monod {
        // Один тик симуляции равен ровно 1.0 секунде!
        inline constexpr double delta_t = 60.0;

        // --- ПАРАМЕТРЫ РОСТА ДЛЯ PSEUDOMONAS AERUGINOSA (комнатная температура ~22°C) ---
        // Переводим часовые константы биодеградации и роста в секундные (делением на 3600)
        inline constexpr double U_max = 0.48 / 3600.0;         // ~1.33e-4 c^-1 (максимальная скорость поглощения)
        inline constexpr double m_act = 0.048 / 3600.0;        // ~1.33e-5 c^-1 (расход активной клетки)
        inline constexpr double m_inactiv = 0.000048 / 3600.0; // ~1.33e-8 c^-1 (расход спящей клетки, в 1000 раз меньше)

        inline constexpr double K_F = 1500.0;                  // Полунасыщение глюкозы для Pseudomonas (мкг/л)
        inline constexpr double Y_B_F = 0.45;                  // Коэффициент выхода биомассы (Yield)

        inline constexpr double starvation_biomass_threshold = 0.2; // Порог биомассы для засыпания
        inline constexpr double greed_coefficient = 1.3;       // Насколько потенциальный доход должен превышать расходы для пробуждения
        inline constexpr int steps_for_waking_up = 3600;       // Время реактивации в секундах (1 час = 3600 секунд)

    }

    namespace field {
        // 1 ячейка = 1 мкм. 1000x1000 ячеек = 1 мм x 1 мм.
        // Это физически достоверно для Pseudomonas aeruginosa (размер клетки ~1-2 мкм)
        inline constexpr int width = 100;
        inline constexpr int height = 100;

        // Начальная концентрация глюкозы в среде.
        // Для активного роста в закрытой колбе/чашке Петри используем 3.0 г/л = 3 000 000 мкг/л.
        // При такой концентрации бактерии смогут делиться многократно.
        inline constexpr double initial_food = 300.0;
        inline constexpr double default_initial_food = 0.0;
        inline constexpr double D = 1.0 / 600.0; //используется в food_diffusion_coeff
        inline constexpr double food_diffusion_coeff = 0.20;//field::D * monod::delta_t;   // Быстрая диффузия для адекватного перераспределения
        inline constexpr int steps_for_adding_food = 10000;
        inline constexpr double count_of_adding_food = 100.0;
    }



    namespace biomass {
        inline constexpr double dispersion_chance = 0.01;
        inline constexpr int dispersion_radius = 5;

        inline constexpr int max_count_reps = 10000;
        inline constexpr double initial_biomass = 0.5;
        inline constexpr double max_biomass = 1.0;
        inline constexpr double child_biomass_ratio = 0.5;     // Деление строго пополам
        inline constexpr double reproduction_min_biomass = 0.7; // Минимальная биомасса для деления
        inline constexpr int default_max_age = 1000000;
        inline constexpr double default_resistance = 0.000015;

        // Параметры спящего состояния (дормантность)
        inline constexpr double nonactive_resistance_multiplier = 2.0; // Спящие клетки устойчивее
        inline constexpr double nonactive_max_life_multiplier = 2.0;   // Спящие клетки живут дольше
        inline constexpr int dead_steps_to_disappearance = 100;        // Через сколько шагов исчезает мёртвая клетка
    }

    namespace antibiotic {
        inline constexpr double death_threshold = 0.5;

        inline constexpr double reproduction_penalty = 0.5;
        inline constexpr double stress_transition_chance = 0.05; // Вероятность перехода в спящее состояние из-за стресса

        // Диффузия антибиотика
        inline constexpr double diffusion_coeff = 0.01;
        inline constexpr double decay_rate = 0.001;             // Деградация антибиотика за шаг

        // Добавление антибиотика
        inline constexpr double concetration_for_next_step = 0.01;  //сколько мы добавим на следующем ходу
        inline constexpr double visualization_normalizer =  0.02; // Нормировка для отображения концентрации
        inline constexpr double middle_value_of_antibiotic = 0.00003;  //среднее значение антибиотика после которого произойдёт добавление нового антибиотика

        // Параметры физиологической (адаптивной) резистентности
        // (механизмы эффлюксных насосов MexAB-OprM и регуляции поринов OprD у Pseudomonas)
        inline constexpr double k_ind = 0.005;         // Макс. скорость индукции (полная адаптация ~3-5 мин стресса)
        inline constexpr double K_ind = 0.5;           // Константа индукции: концентрация A, при которой запускается 50% мощности
        inline constexpr double k_rec = 0.001;         // Скорость релаксации (откат в 5x медленнее подъёма)
        inline constexpr double fitness_cost_coef = 0.35; // Штраф за активные насосы: при r=1 клетка ест на 35% медленнее
    }

    namespace visualization {
        inline constexpr int initial_window_width = 1200;
        inline constexpr int initial_window_height = 900;

        inline constexpr int target_fps = 0;
        inline constexpr int steps_per_frame = 500;              // Количество шагов симуляции за один кадр отрисовки

        inline constexpr int graph_panel_width = 400;
        inline constexpr int modified_content_gap = 20;
        inline constexpr int modified_window_screen_margin = 100;
        inline constexpr int number_of_step_to_diffuse = 1;    // Диффузия каждый шаг

        inline constexpr double min_brightness = 0.35;
        inline constexpr double brightness_span = 0.65;

        inline constexpr double modified_nutrition_normalizer = 500; //field::initial_food;

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

        inline constexpr int legend_x = 10;
        inline constexpr int legend_y = 10;
        inline constexpr int legend_width = 18;
        inline constexpr int legend_height = 18;

        inline constexpr int legend_font_size = 15;

    }

    namespace graphs {
        inline constexpr double chart_roundness = 0.05;
        inline constexpr int chart_round_segments = 4;
        inline constexpr double chart_outline_thickness = 1.0;
        inline constexpr double chart_line_thickness = 1.5;
        inline constexpr double chart_span_epsilon = 1e-6;
        inline constexpr double chart_min_span = 1.0;
        inline constexpr int text_font_size = 12;
        inline constexpr int title_font_size = 14;

        inline constexpr int panel_padding = 12;
        inline constexpr int header_bottom = 160; // высота заголовка панели
        inline constexpr int section_gap = 8;     // отступ между графиками
    }
}
