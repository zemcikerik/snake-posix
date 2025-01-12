add_executable(client)
target_link_libraries(client PRIVATE libsnake libtermbox2)
target_sources(client PRIVATE connect_menu.c host_menu.c input.c main.c main_menu.c renderer/renderer.c renderer/renderer_command_queue.c server_launcher.c shm_client.c socket_client.c terminal.c)
