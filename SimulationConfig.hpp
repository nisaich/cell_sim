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
    // МИКРООРГАНИЗМ: Pseudomonas putida KT2440
    // Источники литературных значений:
    //   [M1] Hintermayer & Weuster-Botz (2017), Eng. Life Sci. 17:1112 - кинетика роста P. putida на глюкозе
    //   [M2] Nelson et al. (2002), Environ. Microbiol. 4:799 - геном KT2440
    //   [A1] Biswas et al. (2026), Heliyon 12:e45084 - MIC левофлоксацина для P. aeruginosa
    //   [D1] Li & Gregory (1974), Geochim. Cosmochim. Acta 38:703 - D_glucose в воде при 25°C
    //   [D2] Wilke & Chang (1955), AIChE J. 1:264 - D для антибиотиков-фторхинолонов
    //   [E1] Fernandez et al. (2000), J. Bacteriol. 182:6552 - помпа TtgABC P. putida
    // ============================================================

    namespace monod {
        // Один тик симуляции = 60 секунд
        inline constexpr double delta_t = 60.0;

        // --- ПАРАМЕТРЫ РОСТА P. putida KT2440 (комнатная температура ~22°C) ---
        //
        // Максимальная удельная скорость роста при 30°C: μ_max = 0.48 h⁻¹ [M1]
        // Поправка на 22°C (Q10 = 2 для бактерий, Arrhenius):
        //   μ_max(22°C) ≈ 0.48 × 2^((22-30)/10) ≈ 0.276 h⁻¹
        // Коэффициент выхода биомассы Y = 0.45 г_биомассы / г_глюкозы [M1]
        // Скорость потребления глюкозы: q_s = μ_max / Y = 0.276 / 0.45 ≈ 0.613 h⁻¹
        inline constexpr double U_max = 0.613 / 3600.0;  // ~1.70e-4 с⁻¹

        // Коэффициент обслуживания (maintenance) при 22°C [M1]:
        // m_s(30°C) ≈ 0.042 г_глюкозы/(г_биомассы × ч) → при 22°C ≈ 0.024 h⁻¹
        inline constexpr double m_act    = 0.024 / 3600.0;  // ~6.67e-6 с⁻¹ (активная клетка)
        inline constexpr double m_inactiv = 0.024 / 360000.0; // в 100× меньше для спящей клетки

        // Константа Моно для глюкозы (K_s) у P. putida: ~40–80 мг/л [M1]
        // Используем 50 мг/л = 50 food units
        inline constexpr double K_F = 50.0;  // мг/л

        // Коэффициент выхода биомассы Y = 0.45 г_биомассы / г_глюкозы [M1]
        inline constexpr double Y_B_F = 0.45;

        // Порог биомассы для перехода в спящее состояние
        inline constexpr double starvation_biomass_threshold = 0.2;
        // Насколько потенциальный доход должен превышать расходы для пробуждения
        inline constexpr double greed_coefficient = 1.3;
        // Время активации спящей клетки (~5 мин = 5 тиков)
        inline constexpr int steps_for_waking_up = 60;
    }

    namespace field {
        // 1 ячейка = 0.1 мкм. Поле 100 × 200 = 10 мкм × 20 мкм
        // (микроколония на начальном этапе роста)
        inline constexpr int width  = 100;
        inline constexpr int height = 200;

        // Начальная концентрация глюкозы в среде: 280 мг/л
        // Это типичное значение для минимальной питательной среды MSM
        // (в лабораторных опытах с P. putida используют 50–500 мг/л)
        inline constexpr double initial_food         = 280.0; // мг/л
        inline constexpr double default_initial_food = 0.0;

        // Диффузия глюкозы в биоплёнке:
        // D_глюкоза в воде при 25°C = 6.7×10⁻¹⁰ м²/с [D1]
        // D_глюкоза в биоплёнке ≈ 0.7 × D_вода ≈ 4.69×10⁻¹⁰ м²/с [D1]
        // Масштабирование: ячейка = 10⁻⁷ м, тик = 60 с
        // α = D × dt / dx² = 4.69e-10 × 60 / (1e-7)² ≈ 2.8×10⁷ >> 1
        // Глюкоза равномерно распределяется за <0.1 с → физически почти мгновенно.
        // Используем α = 0.20 (предел стабильности явной схемы ≈ правильный физический ответ)
        inline constexpr double D = 1.0 / 600.0;
        inline constexpr double food_diffusion_coeff = 0.20;

        inline constexpr int    steps_for_adding_food  = 1000;
        inline constexpr double count_of_adding_food   = 20.0;
    }



    namespace biomass {
        inline constexpr double dispersion_chance  = 0.001;
        inline constexpr int    dispersion_radius  = 10;

        inline constexpr int    max_count_reps     = 10000;
        inline constexpr double initial_biomass    = 0.5;  // первоначальный засев
        inline constexpr double max_biomass        = 1.0;
        inline constexpr double child_biomass_ratio      = 0.5;  // деление строго пополам
        inline constexpr double reproduction_min_biomass = 0.7;  // порог деления
        inline constexpr int    default_max_age    = 1000000;
        inline constexpr double default_resistance = 0.000015;

        // Параметры спящего состояния (дормантность)
        inline constexpr double nonactive_resistance_multiplier = 2.0; // спящие устойчивее
        inline constexpr double nonactive_max_life_multiplier   = 2.0; // спящие живут дольше
        inline constexpr int    dead_steps_to_disappearance     = 100; // шагов до исчезновения трупа
    }

    namespace antibiotic {
        // ============================================================
        // АНТИБИОТИК: Левофлоксацин (Levofloxacin, фторхинолон)
        // ============================================================
        // Левофлоксацин выбран, поскольку:
        //   1. Изучался на P. aeruginosa в статье [A1] (наш ближайший аналог P. putida)
        //   2. Ингибирует ДНК-гиразу (topoisomerase II/IV) — ключевой фермент P. putida
        //   3. В P. putida активна помпа эффлюкса TtgABC [E1] (аналог MexAB-OprM)
        //
        // MIC для P. aeruginosa (wild-type): ≤6 мкг/мл [A1]
        // P. putida несколько чувствительнее: MIC ≈ 1–4 мкг/мл [E1]
        // Используем death_threshold = 2.0 мкг/мл = 2.0 antibiotic units

        inline constexpr double death_threshold = 2.0; // мкг/мл ≈ MIC для P. putida

        inline constexpr double reproduction_penalty   = 0.5;
        inline constexpr double stress_transition_chance = 0.05; // вероятность перехода в спячку под стрессом

        // Диффузия левофлоксацина:
        // D_lev в воде при 25°C ≈ 5.0×10⁻¹⁰ м²/с [D2]
        // В биоплёнке лекарство дополнительно задерживается EPS-матриксом и
        // связывается с отрицательно заряженными полисахаридами (retardation factor ~5–10×).
        // D_eff ≈ 0.1 × D_вода ≈ 5×10⁻¹¹ м²/с → α ≈ 1.8×10⁶ >> 1
        // Физически — тоже мгновенно, но используем малый коэффициент (0.01)
        // чтобы моделировать МЕДЛЕННЫЙ фронт проникновения через биоплёнку
        // (biological retardation + EPS binding effect)
        inline constexpr double diffusion_coeff = 0.01;

        // Деградация левофлоксацина:
        // Левофлоксацин химически стабилен в среде (t½ > 24 ч при нейтральном pH, 22°C)
        // Очень медленный распад: k_deg ≈ 1.6×10⁻⁵ мин⁻¹ → ≈ 1.0×10⁻³ за тик (60 с)
        // Источник: фотолитическое разложение незначительно при отсутствии UV
        inline constexpr double decay_rate = 0.0001; // за тик

        // Добавление антибиотика (выключено по умолчанию)
        inline constexpr double concetration_for_next_step     = 0.0;
        inline constexpr double middle_value_of_antibiotic     = 0.0;
        inline constexpr double visualization_normalizer       = antibiotic::middle_value_of_antibiotic;

        // Параметры адаптивной резистентности — помпа эффлюкса TtgABC [E1]
        // (TtgABC является основной помпой резистентности к фторхинолонам у P. putida)
        //
        // k_ind: скорость индукции помпы.
        // Экспрессия tctC полностью индуцируется за ~10–20 мин при MIC [E1].
        // При концентрации = K_ind: время полуиндукции ≈ ln(2)/k_ind ≈ 7 тиков ≈ 7 мин
        // → k_ind = ln(2)/7 ≈ 0.10 за тик
        inline constexpr double k_ind = 0.10;

        // K_ind: концентрация антибиотика, при которой помпа индуцируется на 50%.
        // Устанавливается примерно на уровне 0.5 × MIC [E1]:
        // K_ind ≈ 0.5 × 2.0 мкг/мл = 1.0 мкг/мл
        inline constexpr double K_ind = 1.0;

        // k_rec: скорость обратного расслабления (деиндукции помпы).
        // Релаксация TtgABC занимает ~30–60 мин (в 5–10 раз медленнее индукции) [E1].
        // → k_rec ≈ k_ind / 10 = 0.01 за тик
        inline constexpr double k_rec = 0.01;

        // Фитнес-штраф за активные эффлюксные помпы:
        // Экспрессия TtgABC снижает скорость роста на ~15–25% при полной активации [E1]
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
