#pragma once

namespace simulation_config {

    // ============================================================
    // СЦЕНАРИЙ: Осада биоплёнки (Biofilm Under Siege)
    // ============================================================
    // Умеренное давление антибиотика: ~2×MIC на границе.
    // Внешний слой купола погибает, внутренние клетки успевают
    // накопить резистентность через TtgABC помпы и выживают.
    // Это НАИБОЛЕЕ реалистичный сценарий.
    //
    // Физика:
    //   - antibiotic diffusion_coeff = 0.008 → медленное проникновение
    //   - death: conc - resistance > 2.0
    //   - outer cells: conc~4.0, resistance~0.0 → умирают мгновенно
    //   - inner cells: conc~0.5, resistance→1.0 (k_ind=0.3) → выживают
    // ============================================================

    namespace monod {
        inline constexpr double delta_t = 60.0;

        inline constexpr double U_max = 0.48 / 3600.0;
        inline constexpr double m_act = 0.048 / 3600.0;
        inline constexpr double m_inactiv = 0.000048 / 3600.0;

        inline constexpr double K_F = 1500.0;
        inline constexpr double Y_B_F = 0.45;

        inline constexpr double starvation_biomass_threshold = 0.2;
        inline constexpr double starvation_steps_threshold   = 3.0; // засыпаем если еды меньше чем на 3 шага
        inline constexpr double greed_coefficient = 1.3;
        inline constexpr int    steps_for_waking_up = 60;
    }

    namespace field {
        inline constexpr int width  = 100;
        inline constexpr int height = 200;

        // Хорошее питание: клетки растут до насыщения (ок. ~0.7 биомассы), купол образуется
        inline constexpr double initial_food         = 280.0;
        inline constexpr double food_diffusion_coeff = 0.20; // нормальная диффузия еды

        inline constexpr int    steps_for_adding_food  = 500;
        inline constexpr double count_of_adding_food   = 150.0; // поддерживаем питание
    }

    namespace biomass {
        inline constexpr double dispersion_chance  = 0.001;
        inline constexpr int    dispersion_radius  = 10;

        inline constexpr int    max_count_reps     = 10000;
        inline constexpr double initial_biomass    = 0.5;
        inline constexpr double max_biomass        = 1.0;
        inline constexpr double child_biomass_ratio      = 0.5;
        inline constexpr double reproduction_min_biomass = 0.7;
        inline constexpr int    default_max_age    = 2880; // 2 суток
        inline constexpr double default_resistance = 0.000015;

        inline constexpr double nonactive_resistance_multiplier = 2.0;
        inline constexpr double nonactive_max_life_multiplier   = 2.0;
        inline constexpr int    dead_steps_to_disappearance     = 100;
    }

    namespace antibiotic {
        // MIC = 2.0 мкг/мл: клетка гибнет если (conc - resistance) > 2.0
        inline constexpr double death_threshold = 2.0;
        // Засыпаем при 80% от MIC — у внутренних клеток шанс на индукцию помп
        inline constexpr double sleep_antibiotic_ratio = 0.80;

        inline constexpr double reproduction_penalty   = 0.5;
        inline constexpr double stress_transition_chance = 0.05;

        // МЕДЛЕННАЯ диффузия антибиотика — EPS матрикс биоплёнки замедляет проникновение
        inline constexpr double diffusion_coeff = 0.008;
        inline constexpr double decay_rate = 0.0002; // медленный распад

        // Умеренное давление: ~2×MIC на верхней границе
        // Внешние клетки (conc~4): 4 - 0 > 2 → гибель
        // Внутренние клетки (conc~0.5): 0.5 - resistance(→1.0) < 2 → выживание
        inline constexpr double concetration_for_next_step     = 2.0; // доза за добавление
        inline constexpr double middle_value_of_antibiotic     = 4.0; // поддерживаем ~4 мкг/мл в верхней строке
        inline constexpr double visualization_normalizer       = 6.0;

        // Быстрая индукция TtgABC: внутренние клетки при conc~0.5 успевают поднять resistance
        // За ~5 тиков (5 минут) resistance доходит до 0.5, за ~20 тиков — до 0.9
        inline constexpr double k_ind = 0.30;
        inline constexpr double K_ind = 0.80; // полуиндукция уже при 0.8 мкг/мл
        inline constexpr double k_rec = 0.002; // медленная релаксация — клетки сохраняют устойчивость
        inline constexpr double fitness_cost_coef = 0.05; // минимальный штраф — устойчивые клетки размножаются почти нормально
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

        inline constexpr int legend_x      = 10;
        inline constexpr int legend_y      = 10;
        inline constexpr int legend_width  = 18;
        inline constexpr int legend_height = 18;

        inline constexpr int legend_font_size = 15;
    }

    namespace graphs {
        inline constexpr double chart_roundness        = 0.05;
        inline constexpr int    chart_round_segments   = 4;
        inline constexpr double chart_outline_thickness = 1.0;
        inline constexpr double chart_line_thickness   = 1.5;
        inline constexpr double chart_span_epsilon     = 1e-6;
        inline constexpr double chart_min_span         = 1.0;
        inline constexpr int    text_font_size         = 12;
        inline constexpr int    title_font_size        = 14;
        inline constexpr int    max_displayed_points   = 300;

        inline constexpr int panel_padding  = 12;
        inline constexpr int header_bottom  = 160;
        inline constexpr int section_gap    = 8;
    }
}
