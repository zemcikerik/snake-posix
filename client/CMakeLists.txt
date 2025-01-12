add_executable(client)
target_link_libraries(client PRIVATE libsnake libtermbox2)
target_sources(
        client PRIVATE
        input.c main.c server_launcher.c shm_client.c socket_client.c terminal.c
        menu/connect_menu.c menu/host_menu.c menu/main_menu.c
        renderer/renderer.c renderer/renderer_command_queue.c
)
