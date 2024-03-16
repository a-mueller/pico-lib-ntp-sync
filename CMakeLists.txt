add_library(pico_ntp_sync INTERFACE)

target_sources(pico_ntp_sync INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/src/pico_ntp_sync.cpp
)

target_include_directories(pico_ntp_sync INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_CURRENT_LIST_DIR}/include)

# Pull in pico libraries that we need
target_link_libraries(pico_ntp_sync INTERFACE pico_stdlib pico_cyw43_arch_lwip_threadsafe_background)
