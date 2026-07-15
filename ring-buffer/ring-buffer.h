#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stddef.h> // Include mandatory because it contains size_t type
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

/*
* Error enum for ring buffer.
*/
typedef enum
{
    eERROR_NO_ERROR             = 0,    // No error
    eERROR_INVALID_PARAMETERS,          // Invalid input parameters for API functions
    eERROR_INSTANCE_INVALID,            // Ring buffer instance struct is corrupted or uninitialized
    eERROR_BUFFER_FULL,                 // Ring buffer is full (applicable only if overwrite setting is false)
}error_t;

/*
* Structure containing members for ring buffer instance.
*/
typedef struct
{
    size_t  headIndex;          // Head index - element that will be read next
    size_t  tailIndex;          // Tail index - element that will be written next
    uint8_t *pBuffer;           // Pointer to the start of the allocated ring buffer memory
    size_t  bufferSize;         // Size of the allocated memory
    bool   overwriteOldValues;  // Flag to indicate if new data should overwrite old data when buffer is full
    size_t elementCount;        // Number of elements currently in the buffer
}RB_Handle_t;

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////
error_t RB_Init(RB_Handle_t* const _handle, size_t _size, uint8_t* const _storage);
error_t RB_Put(RB_Handle_t* const _handle, const size_t _nelements, const uint8_t* const _buf);
error_t RB_Get(RB_Handle_t* const _handle, const size_t _nelements, uint8_t* const _buf);

#endif // RING_BUFFER_H