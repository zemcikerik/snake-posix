add_executable(client)
target_link_libraries(client PRIVATE libsnake libtermbox2)
target_sources(client PRIVATE host_menu.c main.c main_menu.c map_menu.c terminal.c)
