add_library(task_status STATIC task_model.c task_status.c)
target_include_directories(task_status PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(task_status PRIVATE -Wall -Wextra -Werror)
target_link_libraries(task_status PUBLIC cjson curl pthread m)
