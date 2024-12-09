# Take config file template (config.hpp.in) and copy it into build directory
# Add the created file to include path
set(CONFIG_INCLUDE_DIRECTORY "${CMAKE_BINARY_DIR}/configuration/include")
configure_file(
    "Config.hpp.in"
    "${CONFIG_INCLUDE_DIRECTORY}/Config.hpp" ESCAPE_QUOTES
)
target_include_directories(${LIBRARY_NAME} PUBLIC ${CONFIG_INCLUDE_DIRECTORY})
