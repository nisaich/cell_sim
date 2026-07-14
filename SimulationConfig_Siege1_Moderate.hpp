#pragma once

namespace simulation_config {

    // ============================================================
    // СЦЕНАРИЙ: Купол под давлением антибиотика
    // ============================================================
    // МАТЕМАТИЧЕСКОЕ ОБОСНОВАНИЕ:
    //   F_breakeven (рост > 0) = K_F * m_act / (Y_BF * U_max - m_act) = 429 мкг/л
    //   → initial_food ОБЯЗАН быть > 429, иначе клетки не растут ВООБЩЕ
    //   → все предыдущие конфиги (280 мкг/л) были физически неработоспособны!
    //
    // КАК РАБОТАЕТ ЗАЩИТА (диффузионный щит):
    //   Антибиотик добавляется в верхнюю строку и диффундирует вниз.
    //   При D_ab = 0.003, глубина 200 ячеек → на дно нужно > 6 млн тиков.
    //   → нижние клетки практически никогда не видят антибиотик
    //   → они свободно растут и делятся
    //   → верхний слой купола → гибнет от антибиотика
    //   → средний слой → засыпает при 70% MIC, копит resistance
    //   → нижнее ядро → растёт, накапливает resistance, даёт потомство
    //
    // ЧТО ВИДНО НА ЭКРАНЕ:
    //   [зелёный купол] → постепенно внешняя корка [серая] (мёртвые)
    //   → [оранжевый] средний слой (спящие с накопленной resistance)
    //   → [зелёный] нарастающее живое ядро снизу
    // ============================================================

    namespace monod {
        inline constexpr double delta_t = 60.0;

        // НЕ МЕНЯТЬ — оригинальные Monod-параметры Pseudomonas putida
        inline constexpr double U_max    = 0.48  / 3600.0;
        inline constexpr double m_act    = 0.048 / 3600.0;
        inline constexpr double m_inactiv= 0.000048 / 3600.0;
        inline constexpr double K_F      = 1500.0;
        inline constexpr double Y_B_F    = 0.45;

        // Засыпаем если еды меньше чем на 5 шагов обслуживания
        inline constexpr double starvation_biomass_threshold = 0.2;
        inline constexpr double starvation_steps_threshold   = 5.0;
        inline constexpr double greed_coefficient = 1.3;
        inline constexpr int    steps_for_waking_up = 60;
    }

    namespace field {
        inline constexpr int width  = 100;
        inline constexpr int height = 200;

        // КРИТИЧНО: initial_food >> 428.6 (F_breakeven)
        // При F=2000: net_growth = +0.00126/тик, от 0.5 до 0.7 за ~160 тиков
        inline constexpr double initial_food         = 2000.0;

        // Нормальная диффузия еды: равномерно по всему полю
        inline constexpr double food_diffusion_coeff = 0.20;

        // Поддерживаем питание: большая доза редко
        inline constexpr int    steps_for_adding_food  = 500;
        inline constexpr double count_of_adding_food   = 5000.0;
    }

    namespace biomass {
        inline constexpr double dispersion_chance  = 0.001;
        inline constexpr int    dispersion_radius  = 10;

        inline constexpr double initial_biomass    = 0.5;
        inline constexpr double max_biomass        = 1.0;
        inline constexpr double child_biomass_ratio      = 0.5;

        // При F=2000 клетки достигают 0.7 за ~160 тиков — достижимо
        inline constexpr double reproduction_min_biomass = 0.7;
        inline constexpr double reproduction_chance      = 0.5;

        // 2 дня — 2880 тиков (60 сек/тик)
        inline constexpr int    default_max_age    = 2880;

        // Базовая резистентность нулевая — помпы не активированы
        inline constexpr double default_resistance = 0.0001;

        // Спящие клетки значительно устойчивее и живут дольше
        inline constexpr double nonactive_resistance_multiplier = 2.0;
        inline constexpr double nonactive_max_life_multiplier   = 2.0;
        inline constexpr int    dead_steps_to_disappearance     = 100;
    }

    namespace antibiotic {
        // MIC = 2.0: клетка гибнет если (conc - resistance) > 2.0
        inline constexpr double death_threshold = 2.0;

        // Засыпаем при достижении 70% MIC — средний слой уходит в спячку
        inline constexpr double sleep_antibiotic_ratio = 0.70;

        inline constexpr double reproduction_penalty     = 0.5;
        inline constexpr double stress_transition_chance = 0.05;

        // ОЧЕНЬ МЕДЛЕННАЯ диффузия — имитирует EPS-матрикс биоплёнки
        // Антибиотик физически не может проникнуть к нижним клеткам быстро
        inline constexpr double diffusion_coeff = 0.003;
        inline constexpr double decay_rate      = 0.0002;

        // 4×MIC у верхней строки: внешние клетки (conc≈8, resistance≈0) → гибнут
        // Средние клетки (conc≈1-2) → засыпают, копят resistance
        // Нижние клетки (conc≈0) → живут и размножаются свободно
        inline constexpr double concetration_for_next_step = 4.0;
        inline constexpr double middle_value_of_antibiotic = 8.0;
        inline constexpr double visualization_normalizer   = 10.0;

        // Умеренная индукция: за ~5 тиков при conc=1.0 resistance вырастает на +0.1
        // Накапливается постепенно — реалистично
        inline constexpr double k_ind = 0.15;
        inline constexpr double K_ind = 0.80;
        // Медленная релаксация: устойчивость сохраняется надолго
        inline constexpr double k_rec = 0.001;
        // Минимальный штраф за насосы — устойчивые клетки почти так же быстро делятся
        inline constexpr double fitness_cost_coef = 0.03;
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
