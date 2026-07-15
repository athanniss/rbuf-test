#include "ring-buffer.h"

//////////////////////////////////////////////////////////////////////////////////
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
* @return   error       eERROR_NO_ERROR if RB instance was successfuly initialized.
*/////////////////////////////////////////////////////////////////////////////////
error_t  RB_Init(RB_Handle_t* const _handle, size_t _size, uint8_t* const _storage)
{
    // Input parameter validity check
    if((_handle == NULL) || (_size == 0u) || (_storage == NULL))
    {
        return eERROR_INVALID_PARAMETERS;
    }
}

error_t RB_Put(RB_Handle_t* const _handle, const size_t _nelements, const uint8_t* const _buf)
{
    // Input parameter validity check
    if((_handle == NULL) || (_nelements == 0u) || (_buf == NULL))
    {
        return eERROR_INVALID_PARAMETERS;
    }
}

error_t RB_Get(RB_Handle_t* const _handle, const size_t _nelements, uint8_t* const _buf)
{
    // Input parameter validity check
    if((_handle == NULL) || (_nelements == 0u) || (_buf == NULL))
    {
        return eERROR_INVALID_PARAMETERS;
    }
}