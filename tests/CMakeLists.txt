
message("Adding testing configuration")

find_package(Catch2 3 REQUIRED)

add_compile_definitions(private=public)

# Set testing binary filename
set(TEST_BINARY rematch_test)

set(TEST_SOURCES
        output_enumeration/garbage_collector.cpp
        output_enumeration/ecs_node.cpp
        output_enumeration/ecs.cpp
        output_enumeration/enumerator.cpp
        parsing/parser.cpp
        parsing/logical_va.cpp
        parsing/exceptions.cpp
        filtering_module/segment_identification.cpp
        filtering_module/search_nfa.cpp
        filtering_module/search_dfa.cpp
        evaluation/extended_va.cpp
        evaluation/extended_det_va.cpp
        evaluation/dummy_mapping.hpp
        evaluation/mapping_helpers.hpp
        evaluation/algorithm_class.cpp
        evaluation/algorithm_utf8.cpp
        mediator/mediator.cpp
        library_interface/match.cpp
        library_interface/match_iterator.cpp
        library_interface/regex.cpp
        library_interface/rematch.cpp
        library_interface/flags.cpp
        )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(${TEST_BINARY} ${TEST_SOURCES})

target_link_libraries(${TEST_BINARY} PUBLIC ${PROJECT_NAME}lib PRIVATE Catch2::Catch2WithMain)

include(Catch)

catch_discover_tests(${TEST_BINARY})
