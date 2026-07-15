#include "ring-buffer.h"

////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////////////////////////////////////////////

static bool RB_IsInstanceValid(RB_Handle_t* const _handle);

////////////////////////////////////////////////////////////////////////////////
/**
* @brief    Initialize ring buffer instance
*
* @note     This function initializes ring buffer instance.
*           It verifies input parameters for validity (no null pointers, non-zero size)
*           and if the parameters are valid it populates the ring buffer pointers to
*           initial state.
*
* @param[in] _handle    Pointer to the pre-allocated ring buffer instance
* @param[in] _size      Size of the available underlying memory
* @param[in] _storage   Pointer to the underlying memory
*
* @retval eERROR_NO_ERROR               Ring Buffer instance was successfully initialized.
* @retval eERROR_INVALID_PARAMETERS     Input parameters are invalid (null pointers, zero size)
*///////////////////////////////////////////////////////////////////////////////
error_t RB_Init(RB_Handle_t* const _handle, size_t _size, uint8_t* const _storage)
{
    error_t retVal = eERROR_NO_ERROR;

    // Input parameter validity check
    if((_handle == NULL) || (_size == 0u) || (_storage == NULL))
    {
        retVal = eERROR_INVALID_PARAMETERS;
    }
    else
    {
        // Initialize instance directly
        _handle->bufferSize = _size;
        _handle->pBuffer    = _storage;

        _handle->headIndex = 0;
        _handle->tailIndex = 0;
    }

    return retVal;
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

    // Input parameter validity check
    if((_handle == NULL) || (_nelements == 0u) || (_buf == NULL))
    {
        return eERROR_INVALID_PARAMETERS;
    }

    if(RB_IsInstanceValid(_handle) == false)
    {
        return eERROR_INSTANCE_INVALID;
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
    if(_handle->tailIndex + _nelements <= _handle->bufferSize)
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
*           and if the parameters are valid it updates the ring buffer instance accordingly.
*
* @param[in] _handle    Pointer to the pre-allocated ring buffer instance
* @param[in] _nelements Number of elements to retrieve
* @param[in] _buf       Pointer to the buffer to store retrieved data
*
* @retval   eERROR_NO_ERROR               Data was successfully retrieved from the ring buffer.
* @retval   eERROR_INVALID_PARAMETERS     Input parameters are invalid (null pointers, zero size).
* @retval   eERROR_INSTANCE_INVALID       Ring buffer instance is invalid
*///////////////////////////////////////////////////////////////////////////////
error_t RB_Get(RB_Handle_t* const _handle, const size_t _nelements, uint8_t* const _buf)
{
    error_t retVal = eERROR_NO_ERROR;

    // Input parameter validity check
    if((_handle == NULL) || (_nelements == 0u) || (_buf == NULL))
    {
        return eERROR_INVALID_PARAMETERS;
    }

    if(RB_IsInstanceValid(_handle) == false)
    {
        return eERROR_INSTANCE_INVALID;
    }

    // No issues detected, continue with RB logic
    return retVal;
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
    bool retVal = true;

    if( (_handle == NULL) ||                            // The buffer is not initalized (need to check this first)
        (_handle->bufferSize == 0)  ||                  // If buffer size is 0 it's most likely uninitialized or corrupted
        (_handle->headIndex >= _handle->bufferSize) ||  // If either of the indexes is larger than the size, it's corrupted
        (_handle->tailIndex >= _handle->bufferSize) ||
        (_handle->pBuffer == NULL))                     // If pointer to buffer is null, it's uninitialized)
    {
        retVal = false;
    }

    return retVal;
}