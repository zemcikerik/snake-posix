add_executable(server)
target_sources(server PRIVATE argument_parser.c client_connection.c client_connection_handler.c main.c map_loader.c player_data_cache.c server.c)
target_link_libraries(server PRIVATE libargs libsnake)
