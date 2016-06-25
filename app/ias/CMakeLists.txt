# ias

add_executable(ias ias.c)

# link libraries
target_link_libraries(ias vm mem util)
