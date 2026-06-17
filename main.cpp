#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#include "matrix.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

using mtx::Difficulty;
using mtx::Generator;
using mtx::Matrix;
using mtx::Problem;
using mtx::ProblemType;

/**
 * @brief Ошибка ввода со стороны пользователя.
 */
class InputError : public std::runtime_error {
   public:
    explicit InputError(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief Читает одно целое число, повторяя запрос при неверном вводе.
 * @param prompt Текст-приглашение для пользователя.
 * @return Введённое целое число.
 * @throws InputError Если ввод закончился (например, нажали Ctrl+Z).
 */
long long readInteger(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        long long value = 0;
        if (std::cin >> value) {
            return value;
        }
        if (std::cin.eof()) {
            throw InputError("Ввод закончился");
        }
        // Ввели не число: очищаем поток и пропускаем испорченную строку.
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Это не число, попробуйте снова.\n";
    }
}

/**
 * @brief Читает целое число и требует, чтобы оно было от low до high.
 * @param prompt Текст-приглашение.
 * @param low Минимально допустимое значение.
 * @param high Максимально допустимое значение.
 * @return Число в диапазоне [low, high].
 */
long long readInRange(const std::string& prompt, long long low, long long high) {
    while (true) {
        long long value = readInteger(prompt);
        if (value >= low && value <= high) {
            return value;
        }
        std::cout << "Введите число от " << low << " до " << high << ".\n";
    }
}

/**
 * @brief Печатает матрицу с выравниванием столбцов.
 * @param matrix Матрица для вывода.
 */
void printMatrix(const Matrix& matrix) {
    std::size_t width = 1;
    for (std::size_t i = 0; i < matrix.rows(); ++i) {
        for (std::size_t j = 0; j < matrix.cols(); ++j) {
            std::size_t len = std::to_string(matrix.at(i, j)).size();
            if (len > width) {
                width = len;
            }
        }
    }
    for (std::size_t i = 0; i < matrix.rows(); ++i) {
        std::cout << "  [ ";
        for (std::size_t j = 0; j < matrix.cols(); ++j) {
            std::string cell = std::to_string(matrix.at(i, j));
            std::cout << std::string(width - cell.size(), ' ') << cell << ' ';
        }
        std::cout << "]\n";
    }
}

/**
 * @brief Читает матрицу заданного размера с клавиатуры.
 * @param rows Число строк.
 * @param cols Число столбцов.
 * @return Введённая матрица.
 */
Matrix readMatrix(std::size_t rows, std::size_t cols) {
    Matrix result(rows, cols);
    std::cout << "Введите " << rows << "x" << cols << " чисел (по строкам):\n";
    for (std::size_t i = 0; i < rows; ++i) {
        for (std::size_t j = 0; j < cols; ++j) {
            result.at(i, j) = readInteger("");
        }
    }
    return result;
}

/**
 * @brief Превращает номер пункта меню в уровень сложности.
 * @param choice Номер пункта (1..3).
 * @return Уровень сложности.
 */
Difficulty toDifficulty(long long choice) {
    if (choice == 1) {
        return Difficulty::Easy;
    }
    if (choice == 2) {
        return Difficulty::Medium;
    }
    return Difficulty::Hard;
}

/**
 * @brief Превращает номер пункта меню в тип задания.
 * @param choice Номер пункта (1..4).
 * @return Тип задания.
 */
ProblemType toProblemType(long long choice) {
    if (choice == 1) {
        return ProblemType::Multiplication;
    }
    if (choice == 2) {
        return ProblemType::Determinant;
    }
    if (choice == 3) {
        return ProblemType::Gauss;
    }
    return ProblemType::Inverse;
}

/**
 * @brief Печатает условие задания.
 * @param problem Задание, которое нужно показать.
 */
void showProblem(const Problem& problem) {
    std::cout << "\n=== Условие ===\n";
    if (problem.type == ProblemType::Multiplication) {
        std::cout << "Перемножьте матрицы A и B:\nA =\n";
        printMatrix(problem.inputs[0]);
        std::cout << "B =\n";
        printMatrix(problem.inputs[1]);
    } else if (problem.type == ProblemType::Determinant) {
        std::cout << "Вычислите определитель матрицы:\n";
        printMatrix(problem.inputs[0]);
    } else if (problem.type == ProblemType::Gauss) {
        std::cout << "Решите систему A*x = b методом Гаусса.\nA =\n";
        printMatrix(problem.inputs[0]);
        std::cout << "b =\n";
        printMatrix(problem.inputs[1]);
    } else {
        std::cout << "Найдите обратную матрицу для:\n";
        printMatrix(problem.inputs[0]);
    }
}

/**
 * @brief Читает ответ пользователя и сравнивает его с правильным.
 * @param problem Задание с правильным ответом.
 * @return true, если ответ верный.
 */
bool readAndCheckAnswer(const Problem& problem) {
    if (problem.scalarAnswer) {
        long long answer = readInteger("Ваш ответ (определитель): ");
        return mtx::checkScalar(problem.expectedScalar, answer);
    }
    const Matrix& expected = problem.expectedMatrix;
    Matrix given = readMatrix(expected.rows(), expected.cols());
    return mtx::checkMatrix(expected, given);
}

/**
 * @brief Дописывает строку в файл журнала, если путь задан.
 * @param logPath Путь к файлу (пустая строка — журнал не вести).
 * @param line Строка для записи.
 * @throws InputError Если файл не удалось открыть.
 */
void appendToLog(const std::string& logPath, const std::string& line) {
    if (logPath.empty()) {
        return;
    }
    std::ofstream out(logPath, std::ios::app);
    if (!out) {
        throw InputError("Не удалось открыть файл журнала: " + logPath);
    }
    out << line << '\n';
}

/**
 * @brief Проводит один раунд: меню, задание, проверка ответа.
 * @param generator Генератор случайных матриц.
 * @param logPath Путь к журналу (может быть пустым).
 * @return true, если пользователь хочет продолжить.
 */
bool playRound(Generator& generator, const std::string& logPath) {
    std::cout << "\nУровень сложности: 1) Лёгкий  2) Средний  3) Сложный\n";
    Difficulty level = toDifficulty(readInRange("Выбор: ", 1, 3));
    std::cout << "Тип задания: 1) Умножение  2) Определитель  3) Гаусс  4) Обратная\n";
    ProblemType type = toProblemType(readInRange("Выбор: ", 1, 4));

    Problem problem = mtx::makeProblem(type, level, generator);
    showProblem(problem);

    bool correct = readAndCheckAnswer(problem);
    if (correct) {
        std::cout << "Верно! Отличная работа.\n";
        appendToLog(logPath, "result=correct");
    } else {
        std::cout << "Неверно. Попробуйте ещё раз в следующем раунде.\n";
        appendToLog(logPath, "result=wrong");
    }

    std::cout << "Продолжить? 1) Да  0) Нет\n";
    return readInRange("Выбор: ", 0, 1) == 1;
}

}  // namespace

/**
 * @brief Точка входа: разбирает аргументы и запускает цикл заданий.
 * @param argc Число аргументов командной строки.
 * @param argv Аргументы командной строки (--seed N, --log путь).
 * @return 0 при нормальном завершении, 1 при ошибке.
 */
int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);  // показывать русские буквы в консоли Windows
#endif
    try {
        // По умолчанию зерно — текущее время, чтобы задания были разными.
        std::uint_fast32_t seed = static_cast<std::uint_fast32_t>(std::time(nullptr));
        std::string logPath;
        for (int i = 1; i < argc; ++i) {
            std::string flag = argv[i];
            if (flag == "--seed" && i + 1 < argc) {
                seed = static_cast<std::uint_fast32_t>(std::stoul(argv[i + 1]));
                ++i;
            } else if (flag == "--log" && i + 1 < argc) {
                logPath = argv[i + 1];
                ++i;
            } else {
                throw InputError("Неизвестный аргумент: " + flag);
            }
        }

        Generator generator(seed);
        std::cout << "Тренажёр по матричной алгебре. Все ответы — целые числа.\n";
        while (playRound(generator, logPath)) {
            // Повторяем раунды, пока пользователь не выберет выход.
        }
        std::cout << "До встречи!\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
}
