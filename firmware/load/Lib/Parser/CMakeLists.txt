# Define the library target
add_library(parser STATIC)

# Add source files for the library
target_sources(parser PRIVATE
    src/parser.c
)

# Add include directories
target_include_directories(parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Add compiler flags (from your Makefile)
target_compile_options(parser PRIVATE
    -Wall -Wextra -Wshadow -Wunsafe-loop-optimizations -Waddress -Wlogical-op
    -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes
    -Wno-missing-braces -Wmissing-field-initializers -Wmissing-declarations
    -Wpacked -Wundef -Wredundant-decls -Wunreachable-code -Woverlength-strings
    -Wpointer-sign -Wdisabled-optimization -Wvariadic-macros -fstrict-aliasing
    -Wstrict-overflow=5 -fstrict-overflow -funsigned-char -funsigned-bitfields
    -fshort-enums -Wformat=2 -ffunction-sections -fdata-sections -std=gnu18
    -Werror=switch -Werror=implicit-fallthrough=4
)
