add_executable(ares
    ares.cpp

    analysis.c
    mc.c
    random.c
    simulate.c
)

target_link_libraries(ares protocol)


add_executable(poseidon
    poseidon.cpp

    analysis.c
    mc.c
    random.c
    simulate.c
)

target_link_libraries(poseidon protocol)


add_executable(apollo
    apollo.cpp

    analysis.c
    mc.c
    random.c
    simulate.c
    ismcts.c
)

target_link_libraries(apollo protocol)

add_executable(test
    test.c

    mc.c
    random.c
    simulate.c
    ismcts.c
)

target_link_libraries(test m)
