add_library(libtermbox2 INTERFACE)
target_include_directories(libtermbox2 INTERFACE .)
target_compile_definitions(libsnake PUBLIC _DEFAULT_SOURCE _XOPEN_SOURCE=500)
