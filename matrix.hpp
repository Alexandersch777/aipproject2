#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace mtx {

/**
 * @brief Ошибка при работе с матрицами (несовпадение размеров, вырожденность и т.п.).
 */
class MatrixError : public std::runtime_error {
   public:
    /**
     * @brief Создаёт ошибку с текстовым описанием.
     * @param message Текст сообщения об ошибке.
     */
    explicit MatrixError(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief Матрица с целыми числами.
 *
 * Числа хранятся в виде таблицы: data_[номер строки][номер столбца].
 * Класс умеет умножать матрицы, считать определитель и обратную матрицу.
 */
class Matrix {
   public:
    /**
     * @brief Создаёт матрицу нужного размера, заполненную нулями.
     * @param rows Количество строк (больше нуля).
     * @param cols Количество столбцов (больше нуля).
     * @throws MatrixError Если размер нулевой.
     */
    Matrix(std::size_t rows, std::size_t cols);

    /**
     * @brief Возвращает количество строк.
     * @return Число строк.
     */
    std::size_t rows() const { return rows_; }

    /**
     * @brief Возвращает количество столбцов.
     * @return Число столбцов.
     */
    std::size_t cols() const { return cols_; }

    /**
     * @brief Доступ к элементу для чтения и записи.
     * @param row Номер строки.
     * @param col Номер столбца.
     * @return Ссылка на элемент.
     * @throws MatrixError Если номер строки или столбца вне матрицы.
     */
    long long& at(std::size_t row, std::size_t col);

    /**
     * @brief Доступ к элементу только для чтения.
     * @param row Номер строки.
     * @param col Номер столбца.
     * @return Значение элемента.
     * @throws MatrixError Если номер строки или столбца вне матрицы.
     */
    long long at(std::size_t row, std::size_t col) const;

    /**
     * @brief Умножает эту матрицу на другую.
     * @param other Правая матрица.
     * @return Матрица-произведение.
     * @throws MatrixError Если столбцов слева не столько же, сколько строк справа.
     */
    Matrix multiply(const Matrix& other) const;

    /**
     * @brief Считает определитель разложением по первой строке.
     * @return Значение определителя (целое число).
     * @throws MatrixError Если матрица не квадратная.
     */
    long long determinant() const;

    /**
     * @brief Считает обратную матрицу (только если она целочисленная).
     * @return Обратная матрица.
     * @throws MatrixError Если матрица не квадратная, вырождена или обратная дробная.
     */
    Matrix inverse() const;

    /**
     * @brief Сравнивает две матрицы по всем элементам.
     * @param other Матрица для сравнения.
     * @return true, если размеры и все числа совпадают.
     */
    bool equals(const Matrix& other) const;

   private:
    /**
     * @brief Возвращает матрицу без одной строки и одного столбца.
     * @param skipRow Номер выбрасываемой строки.
     * @param skipCol Номер выбрасываемого столбца.
     * @return Матрица на единицу меньшего размера.
     */
    Matrix minor(std::size_t skipRow, std::size_t skipCol) const;

    std::size_t rows_;                            ///< Количество строк.
    std::size_t cols_;                            ///< Количество столбцов.
    std::vector<std::vector<long long>> data_;    ///< Числа: data_[строка][столбец].
};

/**
 * @brief Генератор случайных матриц.
 *
 * Хранит источник случайных чисел. Зерно (seed) задаётся вручную,
 * поэтому при одинаковом зерне получаются одинаковые матрицы (удобно для тестов).
 */
class Generator {
   public:
    /**
     * @brief Создаёт генератор с заданным зерном.
     * @param seed Зерно для случайных чисел.
     */
    explicit Generator(std::uint_fast32_t seed);

    /**
     * @brief Возвращает случайное целое число от low до high включительно.
     * @param low Нижняя граница.
     * @param high Верхняя граница.
     * @return Случайное целое число.
     * @throws MatrixError Если low больше high.
     */
    long long nextInt(long long low, long long high);

    /**
     * @brief Создаёт матрицу со случайными числами от low до high.
     * @param rows Число строк.
     * @param cols Число столбцов.
     * @param low Минимальное число.
     * @param high Максимальное число.
     * @return Случайная матрица.
     */
    Matrix randomMatrix(std::size_t rows, std::size_t cols, long long low, long long high);

    /**
     * @brief Создаёт матрицу с определителем 1 (унимодулярную).
     *
     * Начинает с единичной матрицы и много раз прибавляет к одной строке
     * другую, умноженную на число. Такое действие не меняет определитель,
     * поэтому он остаётся равным 1, а обратная матрица выходит целой.
     *
     * @param size Размер квадратной матрицы.
     * @param maxCoefficient Наибольший множитель в преобразованиях.
     * @return Матрица с определителем 1.
     */
    Matrix randomUnimodular(std::size_t size, long long maxCoefficient);

   private:
    std::mt19937 engine_;  ///< Источник случайных чисел.
};

/**
 * @brief Уровень сложности (от него зависит размер матриц).
 */
enum class Difficulty { Easy, Medium, Hard };

/**
 * @brief Тип задания.
 */
enum class ProblemType { Multiplication, Determinant, Gauss, Inverse };

/**
 * @brief Готовое задание вместе с правильным ответом.
 *
 * Если scalarAnswer == true, ответ — это число expectedScalar (определитель).
 * Иначе ответ — матрица expectedMatrix. Для системы Гаусса inputs[1] — это
 * столбец свободных членов, а expectedMatrix — столбец решения.
 */
struct Problem {
    ProblemType type;              ///< Тип задания.
    std::vector<Matrix> inputs;    ///< Матрицы, которые показываем пользователю.
    Matrix expectedMatrix;         ///< Правильный ответ-матрица.
    long long expectedScalar = 0;  ///< Правильный ответ-число (для определителя).
    bool scalarAnswer = false;     ///< true, если ответ — одно число.
};

/**
 * @brief Возвращает размер матрицы для уровня сложности.
 * @param level Уровень сложности.
 * @return Размер матрицы (2, 3 или 4).
 */
std::size_t baseSize(Difficulty level);

/**
 * @brief Создаёт задание нужного типа и сложности с правильным ответом.
 * @param type Тип задания.
 * @param level Уровень сложности.
 * @param generator Генератор случайных матриц.
 * @return Готовое задание.
 */
Problem makeProblem(ProblemType type, Difficulty level, Generator& generator);

/**
 * @brief Проверяет ответ-матрицу.
 * @param expected Правильная матрица.
 * @param given Матрица пользователя.
 * @return true, если совпадают.
 */
bool checkMatrix(const Matrix& expected, const Matrix& given);

/**
 * @brief Проверяет ответ-число.
 * @param expected Правильное число.
 * @param given Число пользователя.
 * @return true, если совпадают.
 */
bool checkScalar(long long expected, long long given);

}  // namespace mtx
