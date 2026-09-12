lab_add_lesson(l02_exercises
    exercises.cpp
    tests.cpp
)
add_test(NAME l02_exercises COMMAND l02_exercises)

if(LAB_BUILD_SOLUTIONS)
    lab_add_lesson(l02_solution
        ${CMAKE_SOURCE_DIR}/solutions/02-pointers-and-memory/exercises.cpp
        tests.cpp
    )
    target_include_directories(l02_solution PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
