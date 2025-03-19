# Take config file template (config.hpp.in) and copy it into build directory
set(CONFIG_INCLUDE_DIRECTORY "${CMAKE_BINARY_DIR}/config/include")
configure_file( "Config.hpp.in"
                "${CONFIG_INCLUDE_DIRECTORY}/Config.hpp"
                ESCAPE_QUOTES
)

# Add the created file to include path
target_include_directories(${TEST_LIBRARY_NAME} PUBLIC ${CONFIG_INCLUDE_DIRECTORY})

# Copy the created file to include directory when installing
install(FILES "${CONFIG_INCLUDE_DIRECTORY}/Config.hpp" DESTINATION include)
