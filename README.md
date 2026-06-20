# Разбор проекта `project_but_splitting_into_files`

## Что делает программа в целом

Это простая клеточная симуляция на двумерном поле.

На поле есть:

- клетки;
- запас пищи в каждой ячейке;
- заготовка под антибиотик в каждой ячейке;
- консольная визуализация состояния поля.

Программа стартует с полосой живых клеток в нижней строке поля, после чего на каждом шаге:

1. проверяет, какие клетки должны умереть;
2. старит выжившие клетки;
3. даёт живым клеткам забрать еду из своей ячейки;
4. списывает у них расход пищи на жизнь;
5. пытается размножить их в соседние свободные ячейки;
6. обрабатывает исчезновение уже мёртвых клеток;
7. печатает новое состояние в консоль.

Симуляция идёт, пока на поле остаётся хотя бы одна живая клетка.

---

## Главный принцип работы модели

### 1. Поле

Поле хранится как двумерный массив объектов `Nucleus`:

- ширина задаётся в `main.cpp`;
- высота задаётся в `main.cpp`;
- каждая ячейка знает свои координаты;
- каждая ячейка может хранить:
  - указатель на клетку;
  - объект `Food`;
  - объект `Antibiotic`.

По сути `Nucleus` является контейнером состояния одной позиции поля.

### 2. Что считается соседями

Соседи определяются только по четырём направлениям:

- вверх;
- вниз;
- влево;
- вправо.

Диагоналей нет.

Это реализовано в `Field::get_neighbours()` в [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:98).

### 3. Как устроены границы поля

Границы по `x` циклические:

- если уйти влево за `x = 0`, попадём на правый край;
- если уйти вправо за последний столбец, попадём на левый край.

Это делается через `Field::normalize_x()` в [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:74).

Границы по `y` обычные:

- выше верхней строки выйти нельзя;
- ниже нижней строки выйти нельзя.

Это проверяется в `Field::is_y_inside()` в [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:78).

Итог:

- мир "замкнут" по горизонтали;
- мир "открыт с ограничением" по вертикали.

### 4. Как происходит размножение

Размножение описано в `active_Cell::reproduction()` в [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:68).

Условия:

- клетка должна быть живой;
- у неё должно быть `food_inside > 8`;
- рядом должна быть хотя бы одна свободная соседняя ячейка.

Алгоритм:

1. Берутся только свободные соседи через `Field::get_free_neighbours()`.
2. Из них случайно выбирается одна ячейка равновероятно.
3. Дочерней клетке передаётся половина текущей еды родителя.
4. Родитель теряет эту половину.
5. В выбранную ячейку помещается новый `active_Cell`.

### 5. Размножение равномерное или направленное?

Если смотреть только на код выбора направления, то размножение локально не имеет встроенного предпочтения вверх, вниз, влево или вправо:

- все свободные ортогональные соседи выбираются случайно;
- выбор производится равновероятно среди доступных вариантов.

Но глобально картина несимметрична из-за начальных условий и границ:

- стартовая популяция стоит на нижней строке (`y = height - 1`) в `main.cpp`;
- у клеток на нижней границе нет соседа снизу;
- значит в начале размножение реально возможно только:
  - вверх;
  - влево;
  - вправо.

Следствие:

- в самом начале рост будет преимущественно вверх и в стороны;
- вниз рост с нижней строки невозможен;
- позже, когда клетки появятся выше, новые клетки уже смогут размножаться и вниз тоже;
- из-за этого общий фронт роста начинается снизу и распространяется прежде всего вверх, но локально каждый конкретный акт деления всё равно случайный среди доступных 4 направлений.

Краткий вывод:

- не "равномерно во все стороны" в масштабе всей колонии;
- не "жёстко преимущественно вверх" на уровне формулы;
- фактически рост стартует снизу и поэтому сначала идёт вверх/вбок, а затем заполняет доступные соседние клетки случайным образом.

### 6. Что ограничивает рост

Рост ограничен тремя механизмами:

1. Возраст.
   Клетка умирает, если `age_of_cell >= max_age_of_cell`.

2. Голод.
   Клетка умирает, если `food_inside <= 0`.

3. Отсутствие места.
   Даже если еды много, размножение невозможно, если рядом нет свободной ячейки.

Проверка смерти происходит в `abstract_Cell::must_he_die()` в [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:23).

---

## Жизненный цикл клетки за один шаг

Полный шаг симуляции описан в `Field::make_one_step()` в [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:185).

### Фаза 1. Сбор списка живых клеток

Сначала поле полностью обходится, и координаты всех живых клеток копируются в отдельный список `cells_for_this_step`.

Это важно, потому что:

- новые клетки, появившиеся в ходе текущего шага, не начинают действовать сразу;
- они начнут участвовать только на следующем шаге.

Это делает шаг более стабильным и предсказуемым.

### Фаза 2. Смерть или старение

Для каждой клетки из списка:

- если `must_he_die()` возвращает `true`, живая клетка заменяется объектом `dead_Cell`;
- иначе ей увеличивают возраст через `increase_age()`.

Важно:

- мёртвая клетка не удаляется мгновенно;
- сначала она превращается в отдельный объект `dead_Cell`.

### Фаза 3. Поглощение пищи

Все ещё живые клетки вызывают `food_consumption_from_environment()` и забирают еду из своей ячейки.

Правила:

- клетка не может превысить внутренний предел `max_food_inside`;
- за один шаг не может съесть больше `max_amount_of_food_consumed`;
- ячейка отдаёт только фактически доступное количество еды.

### Фаза 4. Внутренний расход пищи

После еды каждая живая клетка тратит `using_food_for_step` единиц внутреннего запаса через `depletion_of_savings()`.

### Фаза 5. Размножение

Каждая живая клетка вызывает `reproduction()`.

Для `active_Cell` это реальный механизм деления.
Для базового `abstract_Cell` метод по умолчанию просто возвращает `false`.

### Фаза 6. Исчезновение трупов

В конце вызывается `Field::process_dead_cells_disappearance()`:

- у каждой мёртвой клетки вызывается `step_after_death()`;
- затем проверяется `should_be_removed_from_field()`;
- если ответ `true`, ячейка освобождается.

В текущей версии:

- `dead_Cell` хранится ровно один шаг;
- после этого удаляется из ячейки.

Это задаётся полем `count_of_steps_to_disappearance = 1` в [Cell.hpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.hpp:60).

---

## Роли файлов

### `main.cpp`

Файл запуска программы.

Содержит:

- создание поля `Field simulation_field(width, height)`;
- начальное размещение живых клеток;
- главный цикл симуляции;
- вызов визуализации.

Ключевые строки:

- создание поля: [main.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/main.cpp:11)
- начальная засевка клеток: [main.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/main.cpp:13)
- цикл симуляции: [main.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/main.cpp:21)

Начальное состояние:

- ширина поля: `100`;
- высота поля: `50`;
- в нижней строке заполняется только левая половина поля, потому что `x` идёт от `0` до `width / 2 - 1`.

Это означает, что стартовая колония:

- не занимает всю нижнюю строку;
- начинается как плотная полоса внизу слева;
- затем расширяется вправо через циклическую горизонталь и через размножение.

### `Field.hpp` и `Field.cpp`

Это центральный модуль симуляции поля.

Он отвечает за:

- хранение сетки;
- доступ к ячейкам;
- поиск соседей;
- постановку клеток;
- выполнение одного шага модели;
- удаление окончательно исчезнувших мёртвых клеток.

Кроме `Field`, здесь же находится класс `Nucleus`.

### `Cell.hpp` и `Cell.cpp`

Это логика самих клеток.

Здесь описаны:

- базовый абстрактный класс `abstract_Cell`;
- живая активная клетка `active_Cell`;
- живая, но пока неиспользуемая `nonactive_Cell`;
- мёртвая клетка `dead_Cell`.

### `Food.hpp` и `Food.cpp`

Минимальный класс для хранения ресурса пищи в ячейке.

### `Antibiotic.hpp` и `Antibiotic.cpp`

Минимальный класс для хранения концентрации антибиотика в ячейке.

Важно:

- в текущей версии антибиотик нигде не влияет на судьбу клеток;
- он только хранится и может быть получен из `Nucleus`;
- никакой логики "клетка погибает от антибиотика" пока нет.

### `visualization.hpp` и `visualization.cpp`

Консольный вывод состояния поля.

Обозначения:

- `.` — пустая ячейка;
- `O` — живая клетка;
- `x` — мёртвая клетка, ещё не исчезнувшая.

После вывода кадра есть задержка 100 мс.

### `simulation`

Это уже собранный исполняемый файл, а не исходный код.

---

## Подробно по классам и функциям

## Класс `Nucleus`

Объявлен в [Field.hpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.hpp:13), реализован в [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:8).

### Что хранит

- `std::shared_ptr<abstract_Cell> cell` — клетка в ячейке;
- `std::array<int, 2> cell_coordinates` — координаты ячейки;
- `Food food` — запас пищи;
- `Antibiotic antibiotic` — концентрация антибиотика.

### Методы `Nucleus`

`Nucleus(int x, int y, float start_food = 30.0f, float start_antibiotic = 0.0f)`

- Где объявлен: [Field.hpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.hpp:25)
- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:8)
- Что делает: создаёт ячейку с координатами, стартовой едой и стартовым антибиотиком.
- Где используется: в `Field::Field()` при заполнении сетки через `emplace_back`.

`bool is_this_nucleus_free() const`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:18)
- Что делает: проверяет, что в ячейке нет клетки.
- Где используется:
  - `Field::get_free_neighbours()`;
  - `Field::place_cell()`.

`void set_cell(std::shared_ptr<abstract_Cell> new_cell)`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:22)
- Что делает: помещает клетку в ячейку.
- Где используется:
  - `Field::place_cell()`;
  - `Field::make_one_step()` при замене живой клетки на `dead_Cell`;
  - `active_Cell::reproduction()` при рождении дочерней клетки.

`std::shared_ptr<abstract_Cell> get_cell() const`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:26)
- Что делает: возвращает указатель на клетку.
- Где используется:
  - `Field::has_living_cells()`;
  - `Field::process_dead_cells_disappearance()`;
  - `Field::make_one_step()`;
  - `visualize_field()`.

`void remove_cell()`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:30)
- Что делает: очищает ячейку.
- Где используется:
  - `Field::process_dead_cells_disappearance()`.

`std::pair<float, float> situation_in_the_environment() const`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:34)
- Что делает: возвращает пару `(food, antibiotic)`.
- Где используется: в текущей версии нигде не используется.
- Назначение: заготовка для будущей логики анализа среды.

`std::array<int, 2> coordinates() const`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:41)
- Что делает: возвращает координаты ячейки.
- Где используется: в текущей версии нигде не используется.

`Food& get_food()` / `const Food& get_food() const`

- Где реализованы: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:45), [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:49)
- Что делают: дают доступ к еде в ячейке.
- Где используются:
  - `Field::make_one_step()` для кормления клетки через `food_consumption_from_environment()`.

`Antibiotic& get_antibiotic()` / `const Antibiotic& get_antibiotic() const`

- Где реализованы: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:53), [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:57)
- Что делают: дают доступ к антибиотику в ячейке.
- Где используются: в текущей версии нигде не используются.

---

## Класс `Field`

Объявлен в [Field.hpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.hpp:48), реализован в [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:61).

### Что хранит

- `width`;
- `height`;
- `std::vector<std::vector<Nucleus>> field`.

### Методы `Field`

`Field(int width, int height)`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:61)
- Что делает:
  - создаёт `height` строк;
  - в каждой строке создаёт `width` объектов `Nucleus`;
  - каждая ячейка получает координаты и стартовую еду `30.0f`.
- Где используется:
  - `main.cpp` при создании поля.

`int normalize_x(int x) const`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:74)
- Что делает: переводит `x` в диапазон `[0, width - 1]` с циклическим оборачиванием.
- Где используется:
  - `Field::get_nucleus()`;
  - `Field::get_neighbours()`.

`bool is_y_inside(int y) const`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:78)
- Что делает: проверяет, что `y` лежит внутри поля.
- Где используется:
  - `Field::get_neighbours()`;
  - `Field::place_cell()`.

`int get_width() const`, `int get_height() const`

- Где реализованы: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:82), [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:86)
- Что делают: возвращают размеры поля.
- Где используются:
  - `visualize_field()`.

`Nucleus& get_nucleus(int x, int y)` / `const Nucleus& get_nucleus(int x, int y) const`

- Где реализованы: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:90), [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:94)
- Что делают: возвращают ссылку на ячейку по координатам, нормализуя `x`.
- Где используются:
  - `Field::get_neighbours()`;
  - `Field::place_cell()`;
  - `Field::process_dead_cells_disappearance()`;
  - `Field::make_one_step()`;
  - `visualize_field()`.

`std::vector<Nucleus*> get_neighbours(int x, int y)`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:98)
- Что делает: возвращает существующих соседей по 4 направлениям.
- Где используется:
  - `Field::get_free_neighbours()`.

`std::vector<Nucleus*> get_free_neighbours(int x, int y)`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:120)
- Что делает: фильтрует список соседей и оставляет только пустые ячейки.
- Где используется:
  - `active_Cell::reproduction()`.

`bool place_cell(int x, int y, std::shared_ptr<abstract_Cell> cell)`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:132)
- Что делает:
  - проверяет вертикальные границы;
  - проверяет, что ячейка свободна;
  - помещает клетку.
- Где используется:
  - `main.cpp` для начальной расстановки.

`bool has_living_cells() const`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:148)
- Что делает: проверяет, есть ли на поле хотя бы одна живая клетка.
- Где используется:
  - `main.cpp` в условии главного цикла.

`void process_dead_cells_disappearance()`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:162)
- Что делает:
  - обходит всё поле;
  - для мёртвых клеток увеличивает счётчик шагов после смерти;
  - удаляет их, если пора.
- Где используется:
  - только внутри `Field::make_one_step()`.

`void make_one_step()`

- Где реализован: [Field.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Field.cpp:185)
- Что делает: выполняет полный такт симуляции.
- Где используется:
  - `main.cpp` в главном цикле.

---

## Класс `abstract_Cell`

Объявлен в [Cell.hpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.hpp:6), реализован в [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:11).

Это базовый класс для всех типов клеток.

### Поля

Защищённые:

- `age_of_cell` — возраст;
- `max_food_inside` — максимальный внутренний запас еды;
- `max_amount_of_food_consumed` — максимум еды, съедаемой за шаг;
- `using_food_for_step` — расход еды за шаг.

Публичные:

- `max_age_of_cell` — предел возраста;
- `food_inside` — текущий внутренний запас;
- `level_of_resistance` — уровень устойчивости.

Замечание:

- `level_of_resistance` пока нигде не участвует в расчётах;
- это ещё одна заготовка под будущую биологическую модель.

### Методы `abstract_Cell`

`int get_age() const`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:11)
- Что делает: возвращает возраст.
- Где используется: в текущей версии нигде не используется.

`float get_level_of_resistance() const`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:15)
- Что делает: возвращает устойчивость.
- Где используется: в текущей версии нигде не используется.

`int get_food_inside() const`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:19)
- Что делает: возвращает внутренний запас еды.
- Где используется: в текущей версии нигде не используется.

`bool must_he_die() const`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:23)
- Что делает: определяет смерть по возрасту или голоду.
- Где используется:
  - `Field::make_one_step()`.

`void increase_age()`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:27)
- Что делает: увеличивает возраст на 1.
- Где используется:
  - `Field::make_one_step()`.

`void food_consumption_from_environment(Food& food)`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:39)
- Что делает:
  - считает свободное место внутри клетки;
  - ограничивает аппетит параметром `max_amount_of_food_consumed`;
  - просит у `Food` выдать нужное количество;
  - добавляет полученную еду во внутренний запас.
- Где используется:
  - `Field::make_one_step()`.

`void depletion_of_savings()`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:50)
- Что делает: тратит еду на поддержание жизни.
- Где используется:
  - `Field::make_one_step()`.

`virtual bool is_alive() const = 0`

- Абстрактный метод.
- Назначение: различать живые и мёртвые клетки.
- Где используется:
  - `Field::has_living_cells()`;
  - `Field::process_dead_cells_disappearance()`;
  - `Field::make_one_step()`;
  - `visualize_field()`.

`virtual bool reproduction(Field& current_field, int x, int y)`

- Где реализован по умолчанию: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:54)
- Что делает: по умолчанию ничего, возвращает `false`.
- Где используется:
  - `Field::make_one_step()`.
- Кто переопределяет:
  - `active_Cell`.

`virtual void step_after_death()`

- Где реализован по умолчанию: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:31)
- Что делает: для базового класса ничего не делает.
- Где используется:
  - `Field::process_dead_cells_disappearance()`.
- Кто переопределяет:
  - `dead_Cell`.

`virtual bool should_be_removed_from_field() const`

- Где реализован по умолчанию: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:35)
- Что делает: по умолчанию возвращает `false`.
- Где используется:
  - `Field::process_dead_cells_disappearance()`.
- Кто переопределяет:
  - `dead_Cell`.

---

## Класс `active_Cell`

Объявлен в [Cell.hpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.hpp:40), реализован в [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:58).

Это основной реально используемый тип живой клетки.

### Методы `active_Cell`

`active_Cell(int start_food, float resistance = 0.0f, int max_age = 20)`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:58)
- Что делает: задаёт стартовый запас еды, устойчивость и максимальный возраст.
- Где используется:
  - `main.cpp` при начальной засевке через конструктор по умолчанию `active_Cell()`;
  - `active_Cell::reproduction()` при создании дочерней клетки через параметризованный конструктор.

Важно:

- в `main.cpp` используется `std::make_shared<active_Cell>()`, то есть конструктор по умолчанию;
- поэтому стартовая клетка получает базовые значения из `abstract_Cell`: `food_inside = 10`, `max_age_of_cell = 20`, `level_of_resistance = 0.0f`.

`bool is_alive() const`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:64)
- Что делает: всегда возвращает `true`.
- Где используется:
  - во всех проверках "живая ли клетка".

`bool reproduction(Field& current_field, int x, int y)`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:68)
- Что делает:
  - запрещает деление при `food_inside <= 8`;
  - получает свободных соседей;
  - случайно выбирает одного;
  - создаёт дочернюю клетку с половиной еды;
  - копирует в неё параметры питания;
  - уменьшает запас еды родителя;
  - помещает ребёнка в выбранную ячейку.
- Где используется:
  - `Field::make_one_step()`.

Замечание по вероятностям:

- случайность берётся через `std::mt19937` и `std::uniform_int_distribution`;
- при каждом делении все свободные соседние клетки равноправны.

---

## Класс `nonactive_Cell`

Объявлен в [Cell.hpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.hpp:49), частично реализован в [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:107).

Сейчас это недореализованная заготовка.

Что есть:

- два приватных множителя:
  - `resistance_multiplier`;
  - `food_usage_multiplier`;
- метод `is_alive()`, который просто возвращает `true`.

Чего нет:

- собственной логики поведения;
- использования этих множителей;
- создания объектов этого типа где-либо в проекте.

Итог:

- класс существует, но в текущей симуляции никак не участвует.

---

## Класс `dead_Cell`

Объявлен в [Cell.hpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.hpp:58), реализован в [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:111).

Это модель трупа клетки, который ещё один шаг остаётся на поле.

### Поля

- `count_of_steps_to_disappearance = 1` — сколько шагов труп держится на поле;
- `count_of_steps_from_death = 0` — сколько уже прошло после смерти.

### Методы `dead_Cell`

`void step_after_death()`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:111)
- Что делает: увеличивает число шагов после смерти.
- Где используется:
  - `Field::process_dead_cells_disappearance()`.

`bool should_be_removed_from_field() const`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:115)
- Что делает: говорит, пора ли удалить труп.
- Где используется:
  - `Field::process_dead_cells_disappearance()`.

`bool is_it_still_there() const`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:119)
- Что делает: проверяет, не исчез ли труп.
- Где используется: в текущей версии нигде не используется.

`bool is_alive() const`

- Где реализован: [Cell.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Cell.cpp:123)
- Что делает: возвращает `false`.
- Где используется:
  - во всех местах, где нужно отличить живую клетку от мёртвой.

---

## Класс `Food`

Объявлен в [Food.hpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Food.hpp:3), реализован в [Food.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Food.cpp:5).

Хранит количество еды в ячейке.

### Методы `Food`

`Food(float start_amount)`

- Где реализован: [Food.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Food.cpp:5)
- Что делает: задаёт стартовое количество еды.
- Где используется:
  - внутри `Nucleus`.

`float get_amount() const`

- Где реализован: [Food.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Food.cpp:8)
- Что делает: возвращает текущий запас еды.
- Где используется:
  - `Nucleus::situation_in_the_environment()`.

`void add(float value)`

- Где реализован: [Food.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Food.cpp:12)
- Что делает: прибавляет положительное количество еды.
- Где используется: в текущей версии нигде не используется.

`int take(int wanted_amount)`

- Где реализован: [Food.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Food.cpp:18)
- Что делает:
  - округляет доступную еду вниз до целого;
  - отдаёт минимум из доступного и запрошенного;
  - уменьшает запас в ячейке.
- Где используется:
  - `abstract_Cell::food_consumption_from_environment()`.

Важно:

- еда в ячейке не восстанавливается;
- значит ресурсы поля только убывают.

---

## Класс `Antibiotic`

Объявлен в [Antibiotic.hpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Antibiotic.hpp:3), реализован в [Antibiotic.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Antibiotic.cpp:3).

Хранит концентрацию антибиотика в ячейке.

### Методы `Antibiotic`

`Antibiotic(float start_concentration)`

- Где реализован: [Antibiotic.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Antibiotic.cpp:3)
- Что делает: задаёт стартовую концентрацию.
- Где используется:
  - внутри `Nucleus`.

`float get_concentration() const`

- Где реализован: [Antibiotic.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Antibiotic.cpp:6)
- Что делает: возвращает концентрацию.
- Где используется:
  - `Nucleus::situation_in_the_environment()`.

`void add(float value)`

- Где реализован: [Antibiotic.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Antibiotic.cpp:10)
- Что делает: увеличивает концентрацию.
- Где используется: в текущей версии нигде не используется.

`void decrease(float value)`

- Где реализован: [Antibiotic.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/Antibiotic.cpp:16)
- Что делает:
  - уменьшает концентрацию;
  - не даёт ей уйти ниже нуля.
- Где используется: в текущей версии нигде не используется.

---

## Визуализация

Функция `visualize_field(const Field& current_field)` объявлена в [visualization.hpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/visualization.hpp:5) и реализована в [visualization.cpp](/home/isaich/innopopykaki/summer_project/project_but_splitting_into_files/visualization.cpp:12).

Что делает:

1. очищает консоль через ANSI escape-последовательность;
2. проходит по всему полю;
3. печатает символ для каждой ячейки;
4. делает задержку 100 мс.

Где используется:

- `main.cpp` после каждого шага симуляции.

---

## Карта вызовов между файлами

Ниже упрощённая схема потока управления.

`main.cpp`

- создаёт `Field`
- вызывает `Field::place_cell()`
- в цикле вызывает `Field::has_living_cells()`
- в цикле вызывает `Field::make_one_step()`
- в цикле вызывает `visualize_field()`

`Field::make_one_step()`

- читает клетки из `Nucleus::get_cell()`
- проверяет состояние через `abstract_Cell::is_alive()`
- проверяет смерть через `abstract_Cell::must_he_die()`
- старит через `abstract_Cell::increase_age()`
- кормит через `abstract_Cell::food_consumption_from_environment()`
- получает еду через `Nucleus::get_food()`
- уменьшает запас через `abstract_Cell::depletion_of_savings()`
- размножает через `abstract_Cell::reproduction()`
- затем вызывает `Field::process_dead_cells_disappearance()`

`active_Cell::reproduction()`

- вызывает `Field::get_free_neighbours()`
- та вызывает `Field::get_neighbours()`
- та использует `Field::normalize_x()` и `Field::is_y_inside()`
- затем ребёнок помещается через `Nucleus::set_cell()`

`Field::process_dead_cells_disappearance()`

- получает клетки через `Nucleus::get_cell()`
- вызывает `dead_Cell::step_after_death()` полиморфно
- вызывает `dead_Cell::should_be_removed_from_field()` полиморфно
- очищает ячейку через `Nucleus::remove_cell()`

`visualize_field()`

- читает размеры через `Field::get_width()` и `Field::get_height()`
- читает клетки через `Field::get_nucleus()`
- читает указатель клетки через `Nucleus::get_cell()`
- определяет символ через `abstract_Cell::is_alive()`

---

## Что в текущей модели уже реализовано, а что пока только заготовлено

### Реально работает

- поле фиксированного размера;
- горизонтальное зацикливание;
- вертикальные границы;
- хранение еды в каждой ячейке;
- поедание еды клетками;
- старение клеток;
- смерть от голода и возраста;
- размножение активных клеток;
- временное хранение мёртвых клеток;
- консольная визуализация.

### Пока не используется или не доведено

- влияние антибиотика на клетки;
- влияние устойчивости `level_of_resistance`;
- поведение `nonactive_Cell`;
- анализ среды через `situation_in_the_environment()`;
- пополнение еды;
- превращение живой клетки в неактивную;
- какие-либо правила конкуренции кроме "ячейка занята / свободна".

---

## Важные наблюдения по поведению модели

### 1. Еда на поле конечная

Каждая ячейка создаётся с `INF` еды.

### 2. Новые клетки не действуют в тот же шаг

Это следствие отдельного списка `cells_for_this_step`.

Плюс:

- нет бесконечной цепочки мгновенных делений за один такт.

### 3. Мёртвая клетка не блокирует поле надолго

Она остаётся лишь на один шаг, потом исчезает.

Это значит:

- освободившееся пространство довольно быстро снова становится доступным для размножения.

### 4. Поле по горизонтали замкнуто

Рост с левого края может выйти на правый край и наоборот.

Это сильно влияет на форму колонии:

- геометрически это скорее цилиндр, чем обычный прямоугольник.

### 5. Антибиотик пока декоративный

Объект класса есть, но биологического эффекта нет.

---

## Короткий итог

Текущий проект моделирует колонию клеток на двумерной сетке с ограниченной пищей.

Основной рабочий цикл такой:

- клетки живут, стареют, едят и тратят запас;
- при достатке еды делятся в случайную свободную соседнюю клетку;
- умирают от возраста или голода;
- после смерти кратко остаются на поле и исчезают.
