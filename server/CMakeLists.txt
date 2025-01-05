add_executable(server)
target_sources(server PRIVATE main.c map_loader.c player_data_cache.c server.c)
target_link_libraries(server PRIVATE libargs libsnake)
