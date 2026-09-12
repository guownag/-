lab_add_lesson(l05_exercises
    exercises.cpp
    tests.cpp
)
add_test(NAME l05_exercises COMMAND l05_exercises)

if(LAB_BUILD_SOLUTIONS)
    # ------------------------------------------------------------------
    # 这一课的参考答案需要自己的一份 exercises.h，因为它是个"模板头文件"，
    # 实现都在里面，不能和桩版共用。
    #
    # 但有个麻烦：C++ 的 #include "xxx.h" 会【先搜当前文件所在目录】，
    # 而 tests.cpp 和桩版 exercises.h 正好在同一个目录里，所以它会
    # 优先找到桩版，include 路径设了也没用。
    #
    # 解法：把 tests.cpp 复制到 build 目录再编译。那份拷贝旁边没有
    # exercises.h，于是编译器乖乖按 include 路径去找，
    # 就能优先命中 solutions 目录里的版本。
    # ------------------------------------------------------------------
    configure_file(tests.cpp
                   ${CMAKE_CURRENT_BINARY_DIR}/tests_solution_copy.cpp
                   COPYONLY)

    lab_add_lesson(l05_solution
        ${CMAKE_SOURCE_DIR}/solutions/05-templates-and-compile-time/exercises.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/tests_solution_copy.cpp
    )
    target_include_directories(l05_solution BEFORE PRIVATE
        ${CMAKE_SOURCE_DIR}/solutions/05-templates-and-compile-time
    )
endif()
