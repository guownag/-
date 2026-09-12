lab_add_lesson(l01_exercises
    exercises.cpp
    tests.cpp
)
add_test(NAME l01_exercises COMMAND l01_exercises)

if(LAB_BUILD_SOLUTIONS)
    lab_add_lesson(l01_solution
        ${CMAKE_SOURCE_DIR}/solutions/01-types-and-bits/exercises.cpp
        tests.cpp
    )
    target_include_directories(l01_solution PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
