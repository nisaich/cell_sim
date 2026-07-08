#pragma once

namespace simulation_config {

    namespace field {
        inline constexpr int width = 100;
        inline constexpr int height = 100;
        inline constexpr float initial_food = 100.0f;
        inline constexpr float default_initial_food = 0.0f;
        inline constexpr float food_diffusion_coeff = 0.25f;
        inline constexpr int steps_for_adding_food = 1000;
        inline constexpr int count_of_adding_food = 0;
    }

    namespace colony {
        inline constexpr int initial_cells_start_x = field::width / 2;
        inline constexpr int initial_cells_count = 1;
        inline constexpr int initial_cells_y_from_bottom = 1;
    }

    namespace biomass {
        inline constexpr float dispersion_chance = 0.001f;
        inline constexpr int dispersion_radius = 10;
        inline constexpr int steps_for_activating = 13500;  //количество шагов на выход из стационарного состояния
        inline constexpr int max_count_reps = 10000;
        inline constexpr float initial_biomass = 0.5f;
        inline constexpr float max_biomass = 1.0f;
        inline constexpr int default_max_age = 10000000;
        inline constexpr float default_resistance = 0.0f;
        inline constexpr float reproduction_min_biomass = 0.9f;
        inline constexpr float child_biomass_ratio = 0.5f;
        inline constexpr float nonactive_resistance_multiplier = 2.0f;
        inline constexpr float nonactive_max_life_multiplier = 100.0f;
        inline constexpr int dead_steps_to_disappearance = 10000;
    }

    namespace antibiotic {
        inline constexpr float diffusion_coeff = 0.22f;
        inline constexpr float adding_concentration = 0.0f;
        inline constexpr int adding_interval = 20;
        inline constexpr float visualization_normalizer = 5.0f;
        inline constexpr float death_threshold = 0.1f;
        inline constexpr float death_probability_factor = 0.05f;
        inline constexpr float reproduction_penalty = 0.5f;
        inline constexpr float stress_transition_chance = 0.02f;
        inline constexpr float decay_rate = 0.00015f;
    }

    namespace monod {
        // Values converted from hour^-1 to second^-1 (divided by 3600)
        inline constexpr float U_max = 5.14e-5f;      // 0.185 h^-1
        inline constexpr float K_F = 33.3f;
        inline constexpr float delta_t = 0.1f;
        inline constexpr float Y_B_F = 0.52f;
        inline constexpr float m_act = 2.056e-5f;     // 0.074 h^-1
        inline constexpr float m_inactiv = 2.056e-7f; // 0.00074 h^-1
        inline constexpr float starvation_biomass_threshold = 0.2f;
        inline constexpr float greed_coefficient = 1.5f;
        inline constexpr int steps_for_waking_up = 50;
    }

    namespace graphs {
        inline constexpr int panel_padding = 12;
        inline constexpr int section_gap = 10;
        inline constexpr int title_font_size = 20;
        inline constexpr int text_font_size = 16;
        inline constexpr float chart_roundness = 0.08f;
        inline constexpr int chart_round_segments = 8;
        inline constexpr float chart_outline_thickness = 1.0f;
        inline constexpr float chart_line_thickness = 2.0f;
        inline constexpr float chart_min_span = 1.0f;
        inline constexpr float chart_span_epsilon = 0.001f;
        inline constexpr int header_bottom = 104;
    }

    namespace visualization {
        inline constexpr int number_of_step_to_diffuse = 1;
        inline constexpr const char* default_color_mode = "nutrition";
        inline constexpr int initial_window_width = 800;
        inline constexpr int initial_window_height = 600;
        inline constexpr int graph_panel_width = 420;
        inline constexpr int outer_margin = 20;
        inline constexpr int modified_window_screen_margin = 100;
        inline constexpr int modified_content_gap = 20;
        inline constexpr int target_fps = 0;
        inline constexpr float min_brightness = 0.35f;
        inline constexpr float brightness_span = 0.65f;
        inline constexpr float standard_nutrition_normalizer = field::initial_food * 0.25;
        inline constexpr float modified_nutrition_normalizer = 2.0f;

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
        inline constexpr int legend_height = 140;
        inline constexpr int legend_font_size = 14;
    }

} // namespace simulation_config
