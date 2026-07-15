#include <stdint.h>
#include <stddef.h> // Include mandatory because it contains size_t type

typedef enum
{
    eERROR_NO_ERROR = 0,
    eERROR_INVALID_PARAMETERS,
    eERROR_BUFFER_FULL,
}error_t;

typedef struct
{
    size_t  headIndex;      // Head index
    size_t  tailIndex;      // Tail index
    uint8_t *pBuffer;       // Pointer to the start of the allocated ring buffer memory
    size_t  bufferSize;     // Size of the allocated memory
}RB_Handle_t;

error_t RB_Init(RB_Handle_t* const _handle, size_t _size, uint8_t* const _storage);
error_t RB_Put(RB_Handle_t* const _handle, const size_t _nelements, const uint8_t* const _buf);
error_t RB_Get(RB_Handle_t* const _handle, const size_t _nelements, uint8_t* const _buf);