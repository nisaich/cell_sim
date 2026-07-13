#pragma once

// =============================================================================
// ЕДИНИЦЫ ИЗМЕРЕНИЯ (SYSTEM OF UNITS)
// =============================================================================
// Пространство: 1 ячейка = 0.1 мкм (100 нм). 10 000 × 10 000 ячеек = 1 мм × 1 мм
// Время:        1 тик = 60 секунд
// Еда:          1 food unit = 1 мг/л глюкозы
// Антибиотик:   1 antibiotic unit = 1 мкг/мл = 1 мг/л левофлоксацина
// Биомасса:     нормированная, 0.0 – 1.0 (1.0 = полностью выросшая клетка)
// =============================================================================

namespace simulation_config {

    // ============================================================
    // СЦЕНАРИЙ: Древовидная (ветвистая) структура биоплёнки (Dendritic Pattern)
    // ============================================================
    // Причина ветвления в физике роста микроколоний — Diffusion-Limited Growth (DLG).
    // Если питательных веществ мало (голодный режим) и диффузия медленная,
    // клетки на поверхности съедают всю пищу до того, как она проникнет вглубь.
    // В итоге растут только выдающиеся наружу "пальцы" или ветви, стремящиеся к пище.
    //
    // Настройки для этого режима:
    // 1. Крайне низкое начальное питание: initial_food = 25.0
    // 2. Медленная диффузия глюкозы: food_diffusion_coeff = 0.02 (в 10 раз меньше нормы)
    // 3. Редкое добавление малой порции еды сверху.
    // 4. Порог засыпания поднят, чтобы внутренние клетки быстро впадали в спячку.
    // ============================================================

    namespace monod {
        inline constexpr double delta_t = 60.0;

        // Повышенная скорость потребления для создания жесткого дефицита
        inline constexpr double U_max = 0.800 / 3600.0;  // очень жадное поглощение

        inline constexpr double m_act    = 0.024 / 3600.0;
        inline constexpr double m_inactiv = 0.024 / 360000.0;

        inline constexpr double K_F = 40.0;  // Константа Моно чуть ниже, чтобы на краях ели быстро
        inline constexpr double Y_B_F = 0.45;

        // Внутренние клетки быстро засыпают при голоде, останавливая рост в ширину
        inline constexpr double starvation_biomass_threshold = 0.35;
        inline constexpr double greed_coefficient = 1.5;
        inline constexpr int steps_for_waking_up = 60;
    }

    namespace field {
        inline constexpr int width  = 100;
        inline constexpr int height = 200;

        // Низкая начальная концентрация
        inline constexpr double initial_food         = 25.0; // мг/л

        // Заниженная диффузия для формирования резких градиентов
        inline constexpr double food_diffusion_coeff = 0.02; // DLA-режим

        inline constexpr int    steps_for_adding_food  = 800;
        inline constexpr double count_of_adding_food   = 8.0; // маленькие порции
    }

    namespace biomass {
        inline constexpr double dispersion_chance  = 0.0005; // низкий разлёт, чтобы ветки не склеивались
        inline constexpr int    dispersion_radius  = 8;

        inline constexpr int    max_count_reps     = 10000;
        inline constexpr double initial_biomass    = 0.5;
        inline constexpr double max_biomass        = 1.0;
        inline constexpr double child_biomass_ratio      = 0.5;
        inline constexpr double reproduction_min_biomass = 0.75; // высокий порог деления
        inline constexpr int    default_max_age    = 1000000;
        inline constexpr double default_resistance = 0.000015;

        inline constexpr double nonactive_resistance_multiplier = 2.0;
        inline constexpr double nonactive_max_life_multiplier   = 3.0;
        inline constexpr int    dead_steps_to_disappearance     = 120;
    }

    namespace antibiotic {
        // В этом сценарии антибиотик выключен, чтобы дать биопленке вырасти в ветки
        inline constexpr double death_threshold = 2.0;

        inline constexpr double reproduction_penalty   = 0.5;
        inline constexpr double stress_transition_chance = 0.05;

        inline constexpr double diffusion_coeff = 0.01;
        inline constexpr double decay_rate = 0.0001;

        inline constexpr double concetration_for_next_step     = 0.0;
        inline constexpr double middle_value_of_antibiotic     = 0.0;
        inline constexpr double visualization_normalizer       = 1.0;

        inline constexpr double k_ind = 0.10;
        inline constexpr double K_ind = 1.0;
        inline constexpr double k_rec = 0.01;
        inline constexpr double fitness_cost_coef = 0.20;
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
