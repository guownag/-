lab_add_lesson(l03_exercises
    exercises.cpp
    linkage_demo.cpp
    tests.cpp
)
add_test(NAME l03_exercises COMMAND l03_exercises)

if(LAB_BUILD_SOLUTIONS)
    lab_add_lesson(l03_solution
        ${CMAKE_SOURCE_DIR}/solutions/03-functions-and-modules/exercises.cpp
        linkage_demo.cpp
        tests.cpp
    )
    target_include_directories(l03_solution PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
