////////////////////////////////////////////////////////////////////////////////
/**
* @file ring-buffer.c
* @brief Implements a byte-size (uint8_t) ring buffer
* @author ben
* @date 16.07.2026
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
* @param[in,out]    _handle    Pointer to the pre-allocated ring buffer instance
* @param[in]        _nelements Number of elements to add
* @param[in]        _buf       Pointer to the data to add
*
* @retval   eERROR_NO_ERROR               Data was successfully added to the ring buffer.
* @retval   eERROR_INVALID_PARAMETERS     Input parameters are invalid (null pointers, zero size).
* @retval   eERROR_INSTANCE_INVALID       Ring buffer instance is invalid
* @retval   eERROR_BUFFER_FULL            Ring buffer is full and overwrite is not allowed
*///////////////////////////////////////////////////////////////////////////////
error_t RB_Put(RB_Handle_t* const _handle, const size_t _nelements, const uint8_t* const _buf)
{
    if(RB_IsInstanceValid(_handle) == false)
    {
        return eERROR_INSTANCE_INVALID;
    }

    // Input parameter validity check (_handle is checked separately)
    if((_nelements == 0u) || (_buf == NULL))
    {
        return eERROR_INVALID_PARAMETERS;
    }

    // Variables below are only necessary if the overwrite is allowed
    // and the user tries to write more data than the buffer can hold.
    // In this case we need to copy only the last part of the data and discard the rest.
    // To do that we need to modify the source pointer and the number of elements to copy.

    const uint8_t *pSourceBuffer = _buf; // Mutable pointer to immutable data
    size_t  numOfElementsToCopy = _nelements;

#if (RB_CONF_ALLOW_OVERWRITE == 0u)
    // Check if the data overwrites the old data.
    // Since we are observing the element count, we can check if
    // the number of elements to add + current element count is larger than the buffer size.
    // No need to compare indexes.
    if( ((_handle->elementCount + numOfElementsToCopy) > _handle->bufferSize))
    {
        return eERROR_BUFFER_FULL;
    }
#else
    if(numOfElementsToCopy >= _handle->bufferSize)
    {
        // In this case we need to copy only the last part of the buffer.
        // We might still need to do a split copy, so we only have to modify 
        // the source pointer to point to the last part of the data.
        pSourceBuffer += numOfElementsToCopy - _handle->bufferSize;  // Advance using original count
        numOfElementsToCopy = _handle->bufferSize;                   // Cap the copy element count to the buffer size       
    }
#endif

    // For Put we advance tail index. There are three cases:
    // 1. If the tail index + number of elements to add is < the buffer size. In this case direct memcpy is possible
    // 2. If the tail index + number of elements to add is >= the buffer size, then we need to wrap around and copy the data in two parts
    // 3. If the number of elements to add is larger than the buffer size, then we need to copy only the last part of the data (the part that fits in the buffer) and discard the rest. This is only applicable if overwrite is allowed.
    if(_handle->tailIndex + numOfElementsToCopy < _handle->bufferSize)
    {
        // Copy data directly
        memcpy(&_handle->pBuffer[_handle->tailIndex], pSourceBuffer, numOfElementsToCopy);
        _handle->tailIndex += numOfElementsToCopy;
    }
    else
    {
        // Copy data in two parts
        const size_t firstPartSize = _handle->bufferSize - _handle->tailIndex;

        // Do a split copy
        memcpy(&_handle->pBuffer[_handle->tailIndex], pSourceBuffer, firstPartSize);
        memcpy(&_handle->pBuffer[0], &pSourceBuffer[firstPartSize], numOfElementsToCopy - firstPartSize);

        // Advance the tail index
        _handle->tailIndex = (numOfElementsToCopy - firstPartSize);
    }

    // If the write has overrun the head, advance head to stay just ahead of tail
    if(_handle->elementCount + numOfElementsToCopy > _handle->bufferSize)
    {
        // Buffer is now full; head must follow tail
        _handle->headIndex = _handle->tailIndex;
        _handle->elementCount = _handle->bufferSize;
    }
    else
    {
        _handle->elementCount += numOfElementsToCopy;
    }

    return eERROR_NO_ERROR;
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