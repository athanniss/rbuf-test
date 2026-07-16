#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stddef.h> // Include mandatory because it contains size_t type
#include <stdbool.h>

/*
* NOTE: Suggested API improvements
*   - RB_Get: Currently there is no way for the user to check the number of elements in the ring buffer
*     If the user attempts to read more element that exist the RB_Get function will throw an error.
*     This is not ideal since there is no way for the API user to know how much data can be read.
*     Suggestion: Add an API function to check the number of elements in the buffer. It can be checked
*     Directly from the ring buffer instance but this muddles the API.
*     Alternatively, RB_Get could be modified that it returns the maximum number of items possible and
*     pass back the value through a pointer parameter.
*
*   - RB_Init / RB_Put: The behaviour of the ring buffer when written
*     with more data than it can hold is not defined. Right now the code returns
*     an error and no write is performed. 
*     Suggestion: Add a configuration flag to ring buffer init to define the behaviour
*     whether an overwrite of the old data is allowed or not. If it's not allowed, then
*     the current behaviour is fine. If it is, the RB_Put needs to check this flag
*     and do possible multiple memcpy's to overwrite the old data (e.g. memcpy needs
*     to be done in a loop until all new data is written).
*/

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
    eERROR_READ_COUNT_TOO_LARGE,        // User requested to read more elements than are currently in the buffer
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
    size_t elementCount;        // Number of elements currently in the buffer
}RB_Handle_t;

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////

error_t RB_Init(RB_Handle_t* const _handle, size_t _size, uint8_t* const _storage);
error_t RB_Put(RB_Handle_t* const _handle, const size_t _nelements, const uint8_t* const _buf);
error_t RB_Get(RB_Handle_t* const _handle, const size_t _nelements, uint8_t* const _buf);

#endif // RING_BUFFER_H