cmake_minimum_required(VERSION 3.10)
project(GraphGenerator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ищем Python
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

# Получаем путь к заголовочным файлам NumPy
execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import numpy; print(numpy.get_include())"
    OUTPUT_VARIABLE NUMPY_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Добавляем все необходимые пути
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Python3_INCLUDE_DIRS}
    ${NUMPY_INCLUDE_DIR}
)

add_executable(graph_generator main.cpp)

# Линкуемся с Python
target_link_libraries(graph_generator ${Python3_LIBRARIES})

# Если нужно, добавляем флаги компиляции
target_compile_options(graph_generator PRIVATE -Wno-deprecated-declarations)

# Для отладки выводим найденные пути
message(STATUS "Python include dirs: ${Python3_INCLUDE_DIRS}")
message(STATUS "NumPy include dir: ${NUMPY_INCLUDE_DIR}")