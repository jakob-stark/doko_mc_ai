add_executable(ares
    ares.cpp

    analysis.c
    mc.c
    random.c
    simulate.c
)

target_link_libraries(ares protocol)
