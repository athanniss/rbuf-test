#include "ring-buffer.h"
#include <stdbool.h>

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

    // No issues detected, continue with RB logic
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