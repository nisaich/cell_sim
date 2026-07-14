#pragma once

namespace simulation_config {

    // ============================================================
    // СЦЕНАРИЙ: Тяжёлая осада — Выживание ядра (Heavy Siege)
    // ============================================================
    // Жёсткое давление: ~5×MIC на границе. Внешние 80% купола гибнут.
    // Только самые глубоко залегающие клетки выживают:
    //  - они в зоне с conc < 0.5 из-за медленной диффузии
    //  - у них максимально высокая индуцированная резистентность
    //  - они начинают формировать "ядро выживания" (persistent core)
    //
    // Физика смерти:
    //   death_threshold = 1.5 (более чувствительные дикие штаммы)
    //   outer: conc~8.0, resistance~0.0 → гибель немедленно
    //   1-й эшелон: conc~3.0, resistance~0.3 (успели немного) → гибель
    //   ядро: conc~0.3, resistance→1.0 → ВЫЖИВАЕТ
    //
    // Итог: видна живая "жемчужина" посередине мёртвой колонии,
    // вокруг которой начинают прорастать устойчивые побеги.
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

        inline constexpr int    steps_for_adding_food  = 500;
        inline constexpr double count_of_adding_food   = 5000.0;
    }

    namespace biomass {
        inline constexpr double dispersion_chance  = 0.0005;
        inline constexpr int    dispersion_radius  = 8;

        inline constexpr int    max_count_reps     = 10000;
        inline constexpr double initial_biomass    = 0.5;
        inline constexpr double max_biomass        = 1.0;
        inline constexpr double child_biomass_ratio      = 0.5;
        inline constexpr double reproduction_min_biomass = 0.7;
        inline constexpr int    default_max_age    = 2880;

        // Немного выше стартовая резистентность — "адаптированный дикий штамм"
        inline constexpr double default_resistance = 0.01;

        inline constexpr double nonactive_resistance_multiplier = 3.0; // спящие гораздо устойчивее
        inline constexpr double nonactive_max_life_multiplier   = 3.0; // и живут дольше
        inline constexpr int    dead_steps_to_disappearance     = 80;
    }

    namespace antibiotic {
        // Более низкий MIC — дикий штамм чувствительнее
        inline constexpr double death_threshold = 1.5;
        // Засыпаем уже при 60% от MIC
        inline constexpr double sleep_antibiotic_ratio = 0.60;

        inline constexpr double reproduction_penalty   = 0.4; // сильный штраф при размножении под антибиотиком
        inline constexpr double stress_transition_chance = 0.05;

        // Очень медленная диффузия: EPS матрикс плотной биоплёнки
        inline constexpr double diffusion_coeff = 0.004;
        inline constexpr double decay_rate = 0.0001;

        // Агрессивная дозировка: ~5×MIC
        // outer (conc~7.5): 7.5 - 0.01 > 1.5 → смерть
        // ядро (conc~0.1 из-за diffusion=0.004): 0.1 - 0.01 < 1.5 → выживание
        inline constexpr double concetration_for_next_step     = 3.0;
        inline constexpr double middle_value_of_antibiotic     = 7.5;
        inline constexpr double visualization_normalizer       = 10.0;

        // ОЧЕНЬ быстрая индукция: ядро за 2-3 тика достигает resistance~0.8
        inline constexpr double k_ind = 0.50;
        inline constexpr double K_ind = 0.50; // индукция начинается при очень низком conc
        inline constexpr double k_rec = 0.001; // почти не теряет резистентность
        inline constexpr double fitness_cost_coef = 0.02; // минимальный штраф за резистентность
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
