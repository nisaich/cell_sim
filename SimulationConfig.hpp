#pragma once

namespace simulation_config {

    // ============================================================
    // СЦЕНАРИЙ: Ветвление + выживаемость (сбалансированный)
    // ============================================================
    // - Достаточно пищи для роста (начальный запас 500)
    // - Умеренно медленная диффузия (0.05) создаёт градиент
    // - Частая подпитка сверху (каждые 100 тиков по 30 ед.)
    // - Мягкие пороги голодания и спячки
    // ============================================================

    namespace monod {
        inline constexpr double delta_t = 60.0;

        inline constexpr double U_max = 0.48 / 3600.0;
        inline constexpr double m_act = 0.048 / 3600.0;
        inline constexpr double m_inactiv = 0.000048 / 3600.0;
        inline constexpr double K_F = 1500.0;
        inline constexpr double Y_B_F = 0.45;

        // Пороги: клетка засыпает при биомассе < 0.12 или если еды осталось < 30 шагов
        inline constexpr double starvation_biomass_threshold = 0.12;
        inline constexpr double starvation_steps_threshold = 30.0;
        inline constexpr double greed_coefficient = 1.3;
        inline constexpr int    steps_for_waking_up = 60;
    }

    namespace field {
        inline constexpr int width = 100;
        inline constexpr int height = 200;

        // Начальная еда – достаточно для активного роста
        inline constexpr double initial_food = 500.0;

        // Диффузия – умеренная, чтобы градиент был, но еда доходила до низа
        inline constexpr double food_diffusion_coeff = 0.05;

        // Подпитка: каждые 100 тиков добавляем 30 ед. в верхнюю точку
        inline constexpr int    steps_for_adding_food = 100;
        inline constexpr double count_of_adding_food = 30.0;
    }

    namespace biomass {
        inline constexpr double dispersion_chance = 0.001;
        inline constexpr int    dispersion_radius = 10;

        inline constexpr int    max_count_reps = 10000;
        inline constexpr double initial_biomass = 0.5;
        inline constexpr double max_biomass = 1.0;
        inline constexpr double child_biomass_ratio = 0.5;

        // Снижаем порог размножения, чтобы клетки быстрее начинали делиться
        inline constexpr double reproduction_min_biomass = 0.6;

        inline constexpr int    default_max_age = 1000000;
        inline constexpr double default_resistance = 1.0;   // для супербактерий

        inline constexpr double nonactive_resistance_multiplier = 2.0;
        inline constexpr double nonactive_max_life_multiplier = 2.0;
        inline constexpr int    dead_steps_to_disappearance = 100;

        // Параметры ветвления – оставляем те, что хорошо работали
        inline constexpr double branching_sharpness = 1.8;
        inline constexpr double branching_food_weight = 2.0;
        inline constexpr double branching_min_weight = 0.05;
    }

    namespace antibiotic {
        // Оставляем как было (супербактерии с высокой устойчивостью)
        inline constexpr double death_threshold = 2.0;
        inline constexpr double sleep_antibiotic_ratio = 0.8;

        inline constexpr double reproduction_penalty = 0.90;
        inline constexpr double stress_transition_chance = 0.01;

        inline constexpr double diffusion_coeff = 0.01;
        inline constexpr double decay_rate = 0.0001;

        inline constexpr double concetration_for_next_step = 3.0;
        inline constexpr double middle_value_of_antibiotic = 8.0;
        inline constexpr double visualization_normalizer = 10.0;

        inline constexpr double k_ind = 0.50;
        inline constexpr double K_ind = 1.0;
        inline constexpr double k_rec = 0.001;
        inline constexpr double fitness_cost_coef = 0.02;
    }

    namespace visualization {
        inline constexpr int initial_window_width = 1200;
        inline constexpr int initial_window_height = 900;

        inline constexpr int target_fps = 0;
        inline constexpr int steps_per_frame = 3000;   // комфортная скорость

        inline constexpr int    graph_panel_width = 400;
        inline constexpr int    modified_content_gap = 20;
        inline constexpr int    modified_window_screen_margin = 100;
        inline constexpr int    number_of_step_to_diffuse = 5;

        inline constexpr double min_brightness = 0.35;
        inline constexpr double brightness_span = 0.65;

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

        inline constexpr int legend_x = 10;
        inline constexpr int legend_y = 10;
        inline constexpr int legend_width = 18;
        inline constexpr int legend_height = 18;

        inline constexpr int legend_font_size = 15;
    }

    namespace graphs {
        inline constexpr double chart_roundness = 0.05;
        inline constexpr int    chart_round_segments = 4;
        inline constexpr double chart_outline_thickness = 1.0;
        inline constexpr double chart_line_thickness = 1.5;
        inline constexpr double chart_span_epsilon = 1e-6;
        inline constexpr double chart_min_span = 1.0;
        inline constexpr int    text_font_size = 12;
        inline constexpr int    title_font_size = 14;
        inline constexpr int    max_displayed_points = 300;

        inline constexpr int panel_padding = 12;
        inline constexpr int header_bottom = 160;
        inline constexpr int section_gap = 8;
    }
}