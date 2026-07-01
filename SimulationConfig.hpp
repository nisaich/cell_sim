#pragma once

namespace simulation_config {

namespace field {
inline constexpr int width = 200;
inline constexpr int height = 200;
inline constexpr float initial_food = 0.5f;
inline constexpr float default_initial_food = 0.5f;
inline constexpr float food_diffusion_coeff = 0.25f;  // Не больше 0.25 (математическое ограничение стабильности)
inline constexpr float biomass_diffusion_coeff = 0.05f; // Коэффициент диффузии биомассы
}

namespace colony {
inline constexpr int initial_cells_start_x = field::width/2;
inline constexpr int initial_cells_count = 1;
inline constexpr int initial_cells_y_from_bottom = 1;
}

namespace biomass {
inline constexpr int max_count_reps = 100;
inline constexpr float initial_biomass = 0.5f; // Разделенная клетка
inline constexpr float max_biomass = 1.0f; // Взрослая клетка (около 1 пг)
inline constexpr float default_max_food_consumed = 0.015f; // Скорость поедания (деление за ~400 тиков)
inline constexpr float food_usage_per_step = 0.0015f; // Траты на поддержание (около 10% от макс. еды)
inline constexpr float food_usage_for_step = food_usage_per_step;
inline constexpr int default_max_age = 10000; // Старение за 10000 тиков
inline constexpr float default_resistance = 0.0f;
inline constexpr float biomass_growth_per_eaten_unit = 0.1f; // КПД усвоения еды
inline constexpr float reproduction_min_biomass = 1.0f; // Деление только при массе 1.0
inline constexpr float reproduction_chance = 0.2f; // Шанс деления 20% за тик
inline constexpr float child_biomass_ratio = 0.5f; // 50/50 распределение при делении
inline constexpr int steps_for_nonactivating = 800; // 800 тиков голодания до спячки
inline constexpr int steps_to_live_forward = 1600; // 1600 тиков голодания до смерти
inline constexpr float nonactive_resistance_multiplier = 10.0f;
inline constexpr float nonactive_food_usage_multiplier = 0.1f;
inline constexpr float nonactive_max_life_multiplier = 10.0f;
inline constexpr float nonactive_biomass_loss_multiplier = 0.1f;
inline constexpr int dead_steps_to_disappearance = 200; // 200 тиков на разложение
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
inline constexpr int header_bottom = 92;
}

namespace visualization {
inline constexpr const int number_of_step_to_diffuse = 1; 
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
inline constexpr float standard_nutrition_normalizer = 0.5f;
inline constexpr float modified_nutrition_normalizer = 0.5f;

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
inline constexpr unsigned char nonactive_cell_g = 255;
inline constexpr unsigned char nonactive_cell_b = 0;

inline constexpr unsigned char dead_cell_r = 255;
inline constexpr unsigned char dead_cell_g = 0;
inline constexpr unsigned char dead_cell_b = 0;
}

}  // namespace simulation_config
