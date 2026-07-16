#include <stdio.h>
#include "./ring-buffer/ring-buffer.h"
#include "./ring-buffer/tests/ring-buffer-tests.h"

// Build with: gcc main.c -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wcast-qual -Werror -g -I ring-buffer ring-buffer/ring-buffer.c ring-buffer/tests/ring-buffer-tests.c -o build/main
// Overwrite mode: add -DRB_CONF_ALLOW_OVERWRITE=1 to the build command

int main(void)
{
    RB_Tests_Run();

    return 0;
}