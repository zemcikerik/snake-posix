add_library(libsnake)
target_sources(libsnake PRIVATE changelog/changelog.c changelog/syn_changelog.c coordinates.c game_state.c map/map.c map/map_template.c map/syn_map.c player/player_manager.c rng.c player/syn_player.c shm_game_state.c syn_helpers.c ticker.c)
target_link_libraries(libsnake PRIVATE m pthread rt)
target_compile_definitions(libsnake PUBLIC _XOPEN_SOURCE=500)
