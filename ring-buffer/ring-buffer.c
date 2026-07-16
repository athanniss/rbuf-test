////////////////////////////////////////////////////////////////////////////////
/**
* @file ring-buffer.c
* @brief Implements a byte-size (uint8_t) ring buffer
* @author ben
* @date 16.06.2024
*
*///////////////////////////////////////////////////////////////////////////////

#include "ring-buffer.h"
#include <string.h> // For memcpy

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////

static bool RB_IsInstanceValid(RB_Handle_t* const _handle);

////////////////////////////////////////////////////////////////////////////////
// Function Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
* @brief    Initialize ring buffer instance
*
* @note     This function initializes ring buffer instance.
*           It verifies input parameters for validity (no null pointers, non-zero size)
*           and if the parameters are valid it populates the ring buffer instance to
*           initial state.
*
* @param[in,out] _handle    Pointer to the pre-allocated ring buffer instance
* @param[in] _size          Size of the available underlying memory
* @param[in] _storage       Pointer to the underlying memory
*
* @retval eERROR_NO_ERROR               Ring Buffer instance was successfully initialized.
* @retval eERROR_INVALID_PARAMETERS     Input parameters are invalid (null pointers, zero size)
*///////////////////////////////////////////////////////////////////////////////
error_t RB_Init(RB_Handle_t* const _handle, size_t _size, uint8_t* const _storage)
{
    // Input parameter validity check
    if((_handle == NULL) || (_size == 0u) || (_storage == NULL))
    {
        return eERROR_INVALID_PARAMETERS;
    }
    else
    {
        _handle->bufferSize = _size;
        _handle->pBuffer    = _storage;

        _handle->headIndex = 0;
        _handle->tailIndex = 0;
        _handle->elementCount = 0;
    }

    return eERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/**
* @brief    Add new data to the ring buffer.
*
* @note     This function adds new data to the ring buffer.
*           It verifies input parameters for validity (no null pointers, non-zero size)
*           and if the parameters are valid it updates the ring buffer instance accordingly.
*
* @param[in] _handle    Pointer to the pre-allocated ring buffer instance
* @param[in] _nelements Number of elements to add
* @param[in] _buf       Pointer to the data to add
*
* @retval   eERROR_NO_ERROR               Data was successfully added to the ring buffer.
* @retval   eERROR_INVALID_PARAMETERS     Input parameters are invalid (null pointers, zero size).
* @retval   eERROR_INSTANCE_INVALID       Ring buffer instance is invalid
* @retval   eERROR_BUFFER_FULL            Ring buffer is full and overwrite is not allowed
*///////////////////////////////////////////////////////////////////////////////
error_t RB_Put(RB_Handle_t* const _handle, const size_t _nelements, const uint8_t* const _buf)
{
    error_t retVal = eERROR_NO_ERROR;

    if(RB_IsInstanceValid(_handle) == false)
    {
        return eERROR_INSTANCE_INVALID;
    }

    // Input parameter validity check (_handle is checked separately)
    if((_nelements == 0u) || (_buf == NULL))
    {
        return eERROR_INVALID_PARAMETERS;
    }

    // Then check if the buffer is full and if overwrite is allowed
    if( (_handle->elementCount + _nelements > _handle->bufferSize) && 
        (_handle->overwriteOldValues == false))
    {
        return eERROR_BUFFER_FULL;
    }

    // No issues detected, continue with RB logic. For put we advance tail index
    // We need to check for possible overflow of array size. Two options here:
    // 1. The tail index + number of elements to add is less than the buffer size, then we can just copy the data
    // 2. The tail index + number of elements to add is greater than the buffer size, then we need to wrap around and copy the data in two parts
    if(_handle->tailIndex + _nelements < _handle->bufferSize)
    {
        // Copy data directly
        memcpy(&_handle->pBuffer[_handle->tailIndex], _buf, _nelements);
        _handle->tailIndex += _nelements;
    }
    else
    {
        // Copy data in two parts
        // First we calculate the number of elements that can be copied before the end of the buffer
        const size_t firstPartSize = _handle->bufferSize - _handle->tailIndex;

        // Then we do two memcopies.
        memcpy(&_handle->pBuffer[_handle->tailIndex], _buf, firstPartSize);
        memcpy(&_handle->pBuffer[0], &_buf[firstPartSize], _nelements - firstPartSize);

        // Advance the tail index
        _handle->tailIndex = (_nelements - firstPartSize);
    }

    // Update the element count
    _handle->elementCount += _nelements;

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
/**
* @brief    Retrieve data from the ring buffer.
*
* @note     This function retrieves data from the ring buffer.
*           It verifies input parameters for validity (no null pointers, non-zero size)
*           and if the parameters are valid it copies the data to user provided buffer and
*           updates the ring buffer instance accordingly.
*           Important: The function will return an error if the user tries to read
*           more elements than are currently in the buffer. Continuous read is not
*           allowed. The user should check the number of elements in the buffer before reading.
*
* @param[in] _handle    Pointer to the pre-allocated ring buffer instance
* @param[in] _nelements Number of elements to retrieve
* @param[in] _buf       Pointer to the buffer to store retrieved data
*
* @retval   eERROR_NO_ERROR               Data was successfully retrieved from the ring buffer.
* @retval   eERROR_INVALID_PARAMETERS     Input parameters are invalid (null pointers, zero size).
* @retval   eERROR_INSTANCE_INVALID       Ring buffer instance is invalid
* @retval   eERROR_READ_COUNT_TOO_LARGE   User requested to read more elements than are currently in the buffer
*///////////////////////////////////////////////////////////////////////////////
error_t RB_Get(RB_Handle_t* const _handle, const size_t _nelements, uint8_t* const _buf)
{
    if(RB_IsInstanceValid(_handle) == false)
    {
        return eERROR_INSTANCE_INVALID;
    }

    // Input parameter validity check (_handle is checked separately)
    if( (_nelements == 0u) || (_buf == NULL))
    {
        return eERROR_INVALID_PARAMETERS;
    }

    if(_nelements > _handle->elementCount)
    {
        return eERROR_READ_COUNT_TOO_LARGE;
    }

    // Two options.
    // 1. The head index + number of elements to retrieve is < the buffer size. In this case direct memcpy is possible
    // 2. The head index + number of elements to retrieve is >= the buffer size, then we need to wrap around and copy the data in two parts
    if(_handle->headIndex + _nelements < _handle->bufferSize)
    {
        // Copy data directly
        memcpy(_buf, &_handle->pBuffer[_handle->headIndex], _nelements);
        _handle->headIndex += _nelements;
    }
    else
    {
        // Copy data in two parts
        const size_t firstPartSize = _handle->bufferSize - _handle->headIndex;

        memcpy(_buf, &_handle->pBuffer[_handle->headIndex], firstPartSize);
        memcpy(&_buf[firstPartSize], &_handle->pBuffer[0], _nelements - firstPartSize);

        _handle->headIndex = (_nelements - firstPartSize);
    }

    // Update the element count
    _handle->elementCount -= _nelements;

    return eERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/**
* @brief    Verify if ring buffer instance is valid
*
* @note     This function checks if the ring buffer instance is initialized
*           (e.g non-zero size, non-null pointer to buffer) and if the head and
*           tail indexes are within the bounds of the allocated memory.
*
* @param[in] _handle    Pointer to the pre-allocated ring buffer instance
*
* @retval   true    Ring buffer instance is valid.
* @retval   false   Ring buffer instance is invalid
*///////////////////////////////////////////////////////////////////////////////
static bool RB_IsInstanceValid(RB_Handle_t* const _handle)
{
    if( (_handle == NULL) ||                            // The buffer is not initialized (need to check this first, if handle is NULL then the rest of the checks will cause a crash)
        (_handle->bufferSize == 0)  ||                  // Indicates that instance is uninitialized or corrupted
        (_handle->headIndex >= _handle->bufferSize) ||  // If either of the indexes is larger than the size, it's corrupted
        (_handle->tailIndex >= _handle->bufferSize) ||
        (_handle->pBuffer == NULL))                     // If pointer to buffer is null, it's uninitialized
    {
        return false;
    }

    return true;
}