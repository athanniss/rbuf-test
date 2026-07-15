#include <stdio.h>
#include "./ring-buffer/ring-buffer.h"

// Build with: gcc main.c -Wall -Wextra -g -I ring-buffer ring-buffer/ring-buffer.c -o build/main

// Possible questions
// Q: What changes do you need to do to the ring buffer implementation to support multiple producers and consumers?
// Q: What changes do you need to do to support variable type sizes in the ring buffer implementation?
// Q: Implement peek.
// Q: Explain the difference between a circular buffer and a ring buffer. Are they the same? If not, what are the differences?
// Q: Implement a ring buffer that supports overwriting old data when the buffer is full. How would you handle this situation?
// Q: Implement a ring buffer that supports dynamic resizing. How would you handle this situation?
// Q: What about thread safety? Which operations are critical?

int main(void)
{
    printf("Hello World\n");

    return 0;
}