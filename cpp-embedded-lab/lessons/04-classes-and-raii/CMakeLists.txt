lab_add_lesson(l04_exercises
    exercises.cpp
    tests.cpp
)
add_test(NAME l04_exercises COMMAND l04_exercises)

if(LAB_BUILD_SOLUTIONS)
    lab_add_lesson(l04_solution
        ${CMAKE_SOURCE_DIR}/solutions/04-classes-and-raii/exercises.cpp
        tests.cpp
    )
    target_include_directories(l04_solution PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

