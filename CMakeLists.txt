cmake_minimum_required(VERSION 3.16)
project(MatrixTrainer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Под MinGW линкуем рантайм статически, чтобы исполняемый файл был
# самодостаточным и запускался двойным кликом без внешних DLL.
if(MINGW)
    add_link_options(-static -static-libgcc -static-libstdc++)
endif()

# Библиотека с матричной логикой — из неё собираются и приложение, и тесты.
add_library(matrix_core matrix.cpp)
target_include_directories(matrix_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Консольное приложение.
add_executable(matrix_trainer main.cpp)
target_link_libraries(matrix_trainer PRIVATE matrix_core)

# Тесты можно отключить, например при сборке только приложения.
option(BUILD_TESTING "Собирать тесты" ON)
if(BUILD_TESTING)
    # Doctest объявляет старый минимум CMake; разрешаем сборку под новым CMake.
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
    include(FetchContent)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.11
    )
    FetchContent_MakeAvailable(doctest)

    enable_testing()
    add_executable(matrix_tests tests/test_matrix.cpp)
    target_link_libraries(matrix_tests PRIVATE matrix_core doctest::doctest)

    include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
    doctest_discover_tests(matrix_tests)
endif()
