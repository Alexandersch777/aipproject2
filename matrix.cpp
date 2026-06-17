#include "matrix.hpp"

namespace mtx {

Matrix::Matrix(std::size_t rows, std::size_t cols)
    : rows_(rows), cols_(cols), data_(rows, std::vector<long long>(cols, 0)) {
    if (rows == 0 || cols == 0) {
        throw MatrixError("Размер матрицы должен быть больше нуля");
    }
}

long long& Matrix::at(std::size_t row, std::size_t col) {
    if (row >= rows_ || col >= cols_) {
        throw MatrixError("Обращение за пределы матрицы");
    }
    return data_[row][col];
}

long long Matrix::at(std::size_t row, std::size_t col) const {
    if (row >= rows_ || col >= cols_) {
        throw MatrixError("Обращение за пределы матрицы");
    }
    return data_[row][col];
}

Matrix Matrix::multiply(const Matrix& other) const {
    if (cols_ != other.rows_) {
        throw MatrixError("Размеры матриц не подходят для умножения");
    }
    Matrix result(rows_, other.cols_);
    for (std::size_t i = 0; i < rows_; ++i) {
        for (std::size_t j = 0; j < other.cols_; ++j) {
            long long sum = 0;
            for (std::size_t k = 0; k < cols_; ++k) {
                sum += at(i, k) * other.at(k, j);
            }
            result.at(i, j) = sum;
        }
    }
    return result;
}

Matrix Matrix::minor(std::size_t skipRow, std::size_t skipCol) const {
    Matrix result(rows_ - 1, cols_ - 1);
    std::size_t targetRow = 0;
    for (std::size_t i = 0; i < rows_; ++i) {
        if (i == skipRow) {
            continue;
        }
        std::size_t targetCol = 0;
        for (std::size_t j = 0; j < cols_; ++j) {
            if (j == skipCol) {
                continue;
            }
            result.at(targetRow, targetCol) = at(i, j);
            ++targetCol;
        }
        ++targetRow;
    }
    return result;
}

long long Matrix::determinant() const {
    if (rows_ != cols_) {
        throw MatrixError("Определитель есть только у квадратной матрицы");
    }
    if (rows_ == 1) {
        return at(0, 0);
    }
    // Разложение по первой строке: суммируем элементы строки, умноженные
    // на определители их миноров, с чередующимся знаком (+ - + - ...).
    long long result = 0;
    long long sign = 1;
    for (std::size_t col = 0; col < cols_; ++col) {
        result += sign * at(0, col) * minor(0, col).determinant();
        sign = -sign;
    }
    return result;
}

Matrix Matrix::inverse() const {
    if (rows_ != cols_) {
        throw MatrixError("Обратная матрица есть только у квадратной матрицы");
    }
    long long det = determinant();
    if (det == 0) {
        throw MatrixError("Матрица вырождена, обратной не существует");
    }
    const std::size_t n = rows_;
    // Строим присоединённую матрицу (алгебраические дополнения с перестановкой),
    // затем делим её на определитель.
    Matrix adjugate(n, n);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            long long sign = ((i + j) % 2 == 0) ? 1 : -1;
            adjugate.at(j, i) = sign * minor(i, j).determinant();
        }
    }
    Matrix result(n, n);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            if (adjugate.at(i, j) % det != 0) {
                throw MatrixError("Обратная матрица получается дробной");
            }
            result.at(i, j) = adjugate.at(i, j) / det;
        }
    }
    return result;
}

bool Matrix::equals(const Matrix& other) const {
    return rows_ == other.rows_ && cols_ == other.cols_ && data_ == other.data_;
}

Generator::Generator(std::uint_fast32_t seed) : engine_(seed) {}

long long Generator::nextInt(long long low, long long high) {
    if (low > high) {
        throw MatrixError("Неверный диапазон случайных чисел");
    }
    std::uniform_int_distribution<long long> distribution(low, high);
    return distribution(engine_);
}

Matrix Generator::randomMatrix(std::size_t rows, std::size_t cols, long long low, long long high) {
    Matrix result(rows, cols);
    for (std::size_t i = 0; i < rows; ++i) {
        for (std::size_t j = 0; j < cols; ++j) {
            result.at(i, j) = nextInt(low, high);
        }
    }
    return result;
}

Matrix Generator::randomUnimodular(std::size_t size, long long maxCoefficient) {
    Matrix result(size, size);
    for (std::size_t i = 0; i < size; ++i) {
        result.at(i, i) = 1;
    }
    if (size == 1) {
        return result;
    }
    long long lastIndex = static_cast<long long>(size) - 1;
    // Много раз прибавляем к одной строке другую, умноженную на число.
    // Это не меняет определитель, поэтому он остаётся равным 1.
    for (std::size_t step = 0; step < size * 4; ++step) {
        std::size_t target = static_cast<std::size_t>(nextInt(0, lastIndex));
        std::size_t source = static_cast<std::size_t>(nextInt(0, lastIndex));
        while (source == target) {
            source = static_cast<std::size_t>(nextInt(0, lastIndex));
        }
        long long factor = nextInt(-maxCoefficient, maxCoefficient);
        if (factor == 0) {
            factor = 1;
        }
        for (std::size_t col = 0; col < size; ++col) {
            result.at(target, col) += factor * result.at(source, col);
        }
    }
    return result;
}

std::size_t baseSize(Difficulty level) {
    switch (level) {
        case Difficulty::Easy:
            return 2;
        case Difficulty::Medium:
            return 3;
        case Difficulty::Hard:
            return 4;
    }
    throw MatrixError("Неизвестный уровень сложности");
}

Problem makeProblem(ProblemType type, Difficulty level, Generator& generator) {
    std::size_t n = baseSize(level);
    if (type == ProblemType::Multiplication) {
        // Размеры n x (n+1) и (n+1) x n — показываем, что матрицы не обязаны быть квадратными.
        Matrix left = generator.randomMatrix(n, n + 1, -5, 5);
        Matrix right = generator.randomMatrix(n + 1, n, -5, 5);
        Problem problem{ProblemType::Multiplication, {left, right}, left.multiply(right)};
        return problem;
    }
    if (type == ProblemType::Determinant) {
        Matrix square = generator.randomMatrix(n, n, -4, 4);
        Problem problem{ProblemType::Determinant, {square}, Matrix(1, 1)};
        problem.expectedScalar = square.determinant();
        problem.scalarAnswer = true;
        return problem;
    }
    if (type == ProblemType::Gauss) {
        Matrix coefficients = generator.randomUnimodular(n, 2);
        Matrix solution = generator.randomMatrix(n, 1, -5, 5);
        Matrix rightSide = coefficients.multiply(solution);
        Problem problem{ProblemType::Gauss, {coefficients, rightSide}, solution};
        return problem;
    }
    Matrix unimodular = generator.randomUnimodular(n, 2);
    Problem problem{ProblemType::Inverse, {unimodular}, unimodular.inverse()};
    return problem;
}

bool checkMatrix(const Matrix& expected, const Matrix& given) {
    return expected.equals(given);
}

bool checkScalar(long long expected, long long given) {
    return expected == given;
}

}  // namespace mtx
