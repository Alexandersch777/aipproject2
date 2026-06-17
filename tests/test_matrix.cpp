#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "matrix.hpp"

using namespace mtx;

namespace {

/// @brief Вспомогательная функция: строит матрицу из списка значений по строкам.
Matrix build(std::size_t rows, std::size_t cols, std::initializer_list<long long> values) {
    Matrix result(rows, cols);
    auto it = values.begin();
    for (std::size_t i = 0; i < rows; ++i) {
        for (std::size_t j = 0; j < cols; ++j) {
            result.at(i, j) = *it++;
        }
    }
    return result;
}

}  // namespace

TEST_CASE("Умножение матриц: корректный результат и проверка размеров") {
    const Matrix a = build(2, 3, {1, 2, 3, 4, 5, 6});
    const Matrix b = build(3, 2, {7, 8, 9, 10, 11, 12});
    const Matrix product = a.multiply(b);
    CHECK(product.rows() == 2);
    CHECK(product.cols() == 2);
    CHECK(product.at(0, 0) == 58);
    CHECK(product.at(1, 1) == 154);

    // Отрицательный случай: несовместимые размеры.
    const Matrix bad = build(2, 2, {1, 2, 3, 4});
    CHECK_THROWS_AS(a.multiply(bad), MatrixError);
}

TEST_CASE("Определитель: известное значение и неквадратная матрица") {
    const Matrix square = build(3, 3, {2, 0, 1, 3, 0, 0, 5, 1, 1});
    CHECK(square.determinant() == 3);

    const Matrix singular = build(2, 2, {2, 4, 1, 2});
    CHECK(singular.determinant() == 0);

    // Отрицательный случай: определитель неквадратной матрицы не определён.
    const Matrix rectangular = build(2, 3, {1, 2, 3, 4, 5, 6});
    CHECK_THROWS_AS(rectangular.determinant(), MatrixError);
}

TEST_CASE("Обратная матрица: произведение даёт единичную, особые случаи бросают") {
    const Matrix unimodular = build(2, 2, {1, 2, 1, 3});
    const Matrix inverse = unimodular.inverse();
    const Matrix identity = unimodular.multiply(inverse);
    CHECK(identity.at(0, 0) == 1);
    CHECK(identity.at(0, 1) == 0);
    CHECK(identity.at(1, 0) == 0);
    CHECK(identity.at(1, 1) == 1);

    // Отрицательный случай: вырожденная матрица не имеет обратной.
    const Matrix singular = build(2, 2, {2, 4, 1, 2});
    CHECK_THROWS_AS(singular.inverse(), MatrixError);

    // Отрицательный случай: обратная не целочисленна (определитель = 2).
    const Matrix nonUnit = build(2, 2, {2, 0, 0, 1});
    CHECK_THROWS_AS(nonUnit.inverse(), MatrixError);
}

TEST_CASE("Генератор: унимодулярная матрица имеет определитель 1") {
    Generator generator(123);
    for (std::size_t size = 2; size <= 4; ++size) {
        const Matrix m = generator.randomUnimodular(size, 2);
        CHECK(m.determinant() == 1);
    }

    // Отрицательный случай: некорректный диапазон.
    CHECK_THROWS_AS(generator.nextInt(5, 1), MatrixError);
}

TEST_CASE("Задания: эталонный ответ согласован с условием") {
    Generator generator(42);

    const Problem multiplication = makeProblem(ProblemType::Multiplication, Difficulty::Medium, generator);
    const Matrix expected = multiplication.inputs[0].multiply(multiplication.inputs[1]);
    CHECK(checkMatrix(expected, multiplication.expectedMatrix));

    const Problem gauss = makeProblem(ProblemType::Gauss, Difficulty::Medium, generator);
    const Matrix rightSide = gauss.inputs[0].multiply(gauss.expectedMatrix);
    CHECK(checkMatrix(rightSide, gauss.inputs[1]));

    const Problem determinant = makeProblem(ProblemType::Determinant, Difficulty::Hard, generator);
    CHECK(determinant.scalarAnswer);
    CHECK(checkScalar(determinant.inputs[0].determinant(), determinant.expectedScalar));
}

TEST_CASE("Проверка ответов: положительные и отрицательные исходы") {
    const Matrix a = build(2, 2, {1, 0, 0, 1});
    const Matrix b = build(2, 2, {1, 0, 0, 1});
    const Matrix c = build(2, 2, {1, 0, 0, 2});
    CHECK(checkMatrix(a, b));
    CHECK_FALSE(checkMatrix(a, c));
    CHECK(checkScalar(7, 7));
    CHECK_FALSE(checkScalar(7, 8));
}
