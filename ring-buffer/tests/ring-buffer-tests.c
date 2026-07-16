////////////////////////////////////////////////////////////////////////////////
/**
* @file     ring-buffer-tests.c
* @brief    Test cases for the ring buffer implementation.
*           Tests are executed via a function pointer table.
*           Each test prints its own [PASS]/[FAIL] result.
*           ResetTestEnvironment() is called before every test.
* @author   ben
* @date     16.07.2024
*
*///////////////////////////////////////////////////////////////////////////////

#include "ring-buffer-tests.h"
#include "../ring-buffer.h"
#include <stdio.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Test configuration
////////////////////////////////////////////////////////////////////////////////

#define TEST_BUFFER_SIZE    (16u)

////////////////////////////////////////////////////////////////////////////////
// Private variables
////////////////////////////////////////////////////////////////////////////////

static uint8_t      gTestBuffer[TEST_BUFFER_SIZE];  // Ring buffer backing storage
static uint8_t      gReadBuffer[TEST_BUFFER_SIZE];  // Destination buffer for RB_Get calls
static RB_Handle_t  gHandle;                        // Ring buffer handle under test

static int          gPassCount = 0;
static int          gFailCount = 0;

////////////////////////////////////////////////////////////////////////////////
// Private macros
////////////////////////////////////////////////////////////////////////////////

#define TEST_RESULT(name, pass)                                             \
    do {                                                                    \
        if((pass)) { gPassCount++; printf("[PASS] %s\n", (name)); }        \
        else       { gFailCount++; printf("[FAIL] %s\n", (name)); }        \
    } while(0)

////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
////////////////////////////////////////////////////////////////////////////////

static void ResetTestEnvironment(void);

// RB_Init tests
static void test_RB_Init_NullHandle(void);
static void test_RB_Init_NullStorage(void);
static void test_RB_Init_ZeroSize(void);
static void test_RB_Init_Valid(void);

// RB_Put tests
static void test_RB_Put_NullHandle(void);
static void test_RB_Put_NullBuf(void);
static void test_RB_Put_ZeroElements(void);
static void test_RB_Put_InvalidInstance(void);
static void test_RB_Put_SingleWrite(void);
static void test_RB_Put_ExactlyFull(void);
#if (RB_CONF_ALLOW_OVERWRITE == 0u)
static void test_RB_Put_OverCapacity(void);
static void test_RB_Put_FullThenOne(void);
#endif
static void test_RB_Put_WrapAround(void);

// RB_Get tests
static void test_RB_Get_NullHandle(void);
static void test_RB_Get_NullBuf(void);
static void test_RB_Get_ZeroElements(void);
static void test_RB_Get_TooManyElements(void);
static void test_RB_Get_SingleRead(void);
static void test_RB_Get_ReadAll(void);
static void test_RB_Get_WrapAround(void);
static void test_RB_Get_Interleaved(void);

#if (RB_CONF_ALLOW_OVERWRITE == 1u)
// RB_Put overwrite mode tests
static void test_RB_Put_OW_WriteToFull(void);
static void test_RB_Put_OW_ElementCountCapped(void);
static void test_RB_Put_OW_HeadFollowsTail(void);
static void test_RB_Put_OW_LargerThanBuffer(void);

// RB_Get overwrite mode tests
static void test_RB_Get_OW_ReadAfterPartialOverwrite(void);
static void test_RB_Get_OW_ReadAfterFullReplace(void);
#endif // RB_CONF_ALLOW_OVERWRITE

////////////////////////////////////////////////////////////////////////////////
// Function table
////////////////////////////////////////////////////////////////////////////////

typedef void (*TestFunc_t)(void);

static const TestFunc_t gTests[] =
{
    // RB_Init
    test_RB_Init_NullHandle,
    test_RB_Init_NullStorage,
    test_RB_Init_ZeroSize,
    test_RB_Init_Valid,

    // RB_Put
    test_RB_Put_NullHandle,
    test_RB_Put_NullBuf,
    test_RB_Put_ZeroElements,
    test_RB_Put_InvalidInstance,
    test_RB_Put_SingleWrite,
    test_RB_Put_ExactlyFull,
#if (RB_CONF_ALLOW_OVERWRITE == 0u)
    test_RB_Put_OverCapacity,
    test_RB_Put_FullThenOne,
#endif
    test_RB_Put_WrapAround,

    // RB_Get
    test_RB_Get_NullHandle,
    test_RB_Get_NullBuf,
    test_RB_Get_ZeroElements,
    test_RB_Get_TooManyElements,
    test_RB_Get_SingleRead,
    test_RB_Get_ReadAll,
    test_RB_Get_WrapAround,
    test_RB_Get_Interleaved,
};

#if (RB_CONF_ALLOW_OVERWRITE == 1u)
static const TestFunc_t gOverwriteTests[] =
{
    test_RB_Put_OW_WriteToFull,
    test_RB_Put_OW_ElementCountCapped,
    test_RB_Put_OW_HeadFollowsTail,
    test_RB_Put_OW_LargerThanBuffer,
    test_RB_Get_OW_ReadAfterPartialOverwrite,
    test_RB_Get_OW_ReadAfterFullReplace,
};
#endif // RB_CONF_ALLOW_OVERWRITE

////////////////////////////////////////////////////////////////////////////////
// Private function definitions
////////////////////////////////////////////////////////////////////////////////

static void ResetTestEnvironment(void)
{
    memset(gTestBuffer, 0, sizeof(gTestBuffer));
    memset(gReadBuffer, 0, sizeof(gReadBuffer));
    memset(&gHandle, 0, sizeof(gHandle));
    RB_Init(&gHandle, TEST_BUFFER_SIZE, gTestBuffer);
}

////////////////////////////////////////////////////////////////////////////////
// RB_Init tests
////////////////////////////////////////////////////////////////////////////////

static void test_RB_Init_NullHandle(void)
{
    error_t result = RB_Init(NULL, TEST_BUFFER_SIZE, gTestBuffer);

    TEST_RESULT("RB_Init: NULL handle returns eERROR_INVALID_PARAMETERS",
                result == eERROR_INVALID_PARAMETERS);
}

static void test_RB_Init_NullStorage(void)
{
    error_t result = RB_Init(&gHandle, TEST_BUFFER_SIZE, NULL);

    TEST_RESULT("RB_Init: NULL storage returns eERROR_INVALID_PARAMETERS",
                result == eERROR_INVALID_PARAMETERS);
}

static void test_RB_Init_ZeroSize(void)
{
    error_t result = RB_Init(&gHandle, 0u, gTestBuffer);

    TEST_RESULT("RB_Init: Zero size returns eERROR_INVALID_PARAMETERS",
                result == eERROR_INVALID_PARAMETERS);
}

static void test_RB_Init_Valid(void)
{
    error_t result = RB_Init(&gHandle, TEST_BUFFER_SIZE, gTestBuffer);

    bool fields_ok = (gHandle.bufferSize   == TEST_BUFFER_SIZE) &&
                     (gHandle.pBuffer      == gTestBuffer)      &&
                     (gHandle.headIndex    == 0u)               &&
                     (gHandle.tailIndex    == 0u)               &&
                     (gHandle.elementCount == 0u);

    TEST_RESULT("RB_Init: Valid parameters return eERROR_NO_ERROR and fields are set correctly",
                (result == eERROR_NO_ERROR) && fields_ok);
}

////////////////////////////////////////////////////////////////////////////////
// RB_Put tests
////////////////////////////////////////////////////////////////////////////////

static void test_RB_Put_NullHandle(void)
{
    static const uint8_t data[] = {0x01, 0x02, 0x03};

    error_t result = RB_Put(NULL, sizeof(data), data);

    TEST_RESULT("RB_Put: NULL handle returns eERROR_INSTANCE_INVALID",
                result == eERROR_INSTANCE_INVALID);
}

static void test_RB_Put_NullBuf(void)
{
    error_t result = RB_Put(&gHandle, 3u, NULL);

    TEST_RESULT("RB_Put: NULL source buffer returns eERROR_INVALID_PARAMETERS",
                result == eERROR_INVALID_PARAMETERS);
}

static void test_RB_Put_ZeroElements(void)
{
    static const uint8_t data[] = {0x01};

    error_t result = RB_Put(&gHandle, 0u, data);

    TEST_RESULT("RB_Put: Zero element count returns eERROR_INVALID_PARAMETERS",
                result == eERROR_INVALID_PARAMETERS);
}

static void test_RB_Put_InvalidInstance(void)
{
    // Corrupt the handle to simulate an uninitialized or corrupted instance
    gHandle.bufferSize = 0u;

    static const uint8_t data[] = {0x01};
    error_t result = RB_Put(&gHandle, sizeof(data), data);

    TEST_RESULT("RB_Put: Corrupted instance returns eERROR_INSTANCE_INVALID",
                result == eERROR_INSTANCE_INVALID);
}

static void test_RB_Put_SingleWrite(void)
{
    static const uint8_t data[] = {0xAB};

    error_t result = RB_Put(&gHandle, sizeof(data), data);

    bool data_ok  = (gTestBuffer[0]       == 0xAB);
    bool state_ok = (gHandle.tailIndex    == 1u) &&
                    (gHandle.elementCount == 1u);

    TEST_RESULT("RB_Put: Single byte write stores correct data and advances tail",
                (result == eERROR_NO_ERROR) && data_ok && state_ok);
}

static void test_RB_Put_ExactlyFull(void)
{
    // Writing exactly bufferSize bytes into an empty buffer must succeed.
    // Edge case: elementCount(0) + nelements(16) > bufferSize(16) is false,
    // so no BUFFER_FULL error. Tail wraps back to 0.
    static const uint8_t data[TEST_BUFFER_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };

    error_t result = RB_Put(&gHandle, TEST_BUFFER_SIZE, data);

    bool state_ok = (gHandle.elementCount == TEST_BUFFER_SIZE) &&
                    (gHandle.tailIndex    == 0u);   // Wrapped back to start

    TEST_RESULT("RB_Put: Write exactly bufferSize bytes into empty buffer succeeds, tail wraps to 0",
                (result == eERROR_NO_ERROR) && state_ok);
}

#if (RB_CONF_ALLOW_OVERWRITE == 0u)
static void test_RB_Put_OverCapacity(void)
{
    // Writing more bytes than the buffer can hold must be rejected immediately
    static const uint8_t data[TEST_BUFFER_SIZE + 1u] = {0};

    error_t result = RB_Put(&gHandle, TEST_BUFFER_SIZE + 1u, data);

    TEST_RESULT("RB_Put: Write more than bufferSize bytes returns eERROR_BUFFER_FULL",
                result == eERROR_BUFFER_FULL);
}

static void test_RB_Put_FullThenOne(void)
{
    // Fill the buffer completely, then verify that adding one more byte is rejected
    static const uint8_t fill[TEST_BUFFER_SIZE] = {0};
    static const uint8_t extra                  = 0xFF;

    RB_Put(&gHandle, TEST_BUFFER_SIZE, fill);

    error_t result = RB_Put(&gHandle, 1u, &extra);

    TEST_RESULT("RB_Put: Write one byte to a full buffer returns eERROR_BUFFER_FULL",
                result == eERROR_BUFFER_FULL);
}
#endif // RB_CONF_ALLOW_OVERWRITE

static void test_RB_Put_WrapAround(void)
{
    // Setup: fill 10 bytes and drain them so head=10, tail=10, count=0.
    // Then write 10 bytes. Tail must wrap: first 6 bytes go to [10..15],
    // remaining 4 bytes go to [0..3], tail lands at 4.
    static const uint8_t fill[10]      = {0};
    static const uint8_t wrap_data[10] = {
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9
    };

    RB_Put(&gHandle, 10u, fill);
    RB_Get(&gHandle, 10u, gReadBuffer);

    error_t result = RB_Put(&gHandle, 10u, wrap_data);

    bool state_ok  = (gHandle.tailIndex    == 4u) &&
                     (gHandle.elementCount == 10u);

    // Verify physical layout in backing buffer
    bool layout_ok = (gTestBuffer[10] == 0xA0) &&   // First part starts at index 10
                     (gTestBuffer[15] == 0xA5) &&   // First part ends at index 15
                     (gTestBuffer[0]  == 0xA6) &&   // Second part starts at index 0
                     (gTestBuffer[3]  == 0xA9);     // Second part ends at index 3

    TEST_RESULT("RB_Put: Wrap-around write stores data correctly across buffer boundary",
                (result == eERROR_NO_ERROR) && state_ok && layout_ok);
}

////////////////////////////////////////////////////////////////////////////////
// RB_Get tests
////////////////////////////////////////////////////////////////////////////////

static void test_RB_Get_NullHandle(void)
{
    error_t result = RB_Get(NULL, 1u, gReadBuffer);

    TEST_RESULT("RB_Get: NULL handle returns eERROR_INSTANCE_INVALID",
                result == eERROR_INSTANCE_INVALID);
}

static void test_RB_Get_NullBuf(void)
{
    error_t result = RB_Get(&gHandle, 1u, NULL);

    TEST_RESULT("RB_Get: NULL destination buffer returns eERROR_INVALID_PARAMETERS",
                result == eERROR_INVALID_PARAMETERS);
}

static void test_RB_Get_ZeroElements(void)
{
    error_t result = RB_Get(&gHandle, 0u, gReadBuffer);

    TEST_RESULT("RB_Get: Zero element count returns eERROR_INVALID_PARAMETERS",
                result == eERROR_INVALID_PARAMETERS);
}

static void test_RB_Get_TooManyElements(void)
{
    // Put 4 bytes, then request 5 — must be rejected
    static const uint8_t data[4] = {0x01, 0x02, 0x03, 0x04};

    RB_Put(&gHandle, 4u, data);

    error_t result = RB_Get(&gHandle, 5u, gReadBuffer);

    TEST_RESULT("RB_Get: Request more elements than available returns eERROR_READ_COUNT_TOO_LARGE",
                result == eERROR_READ_COUNT_TOO_LARGE);
}

static void test_RB_Get_SingleRead(void)
{
    static const uint8_t data[] = {0xCD};

    RB_Put(&gHandle, 1u, data);
    error_t result = RB_Get(&gHandle, 1u, gReadBuffer);

    bool data_ok  = (gReadBuffer[0]       == 0xCD);
    bool state_ok = (gHandle.headIndex    == 1u) &&
                    (gHandle.elementCount == 0u);

    TEST_RESULT("RB_Get: Single byte read returns correct data and advances head",
                (result == eERROR_NO_ERROR) && data_ok && state_ok);
}

static void test_RB_Get_ReadAll(void)
{
    static const uint8_t data[4] = {0x10, 0x20, 0x30, 0x40};

    RB_Put(&gHandle, 4u, data);
    error_t result = RB_Get(&gHandle, 4u, gReadBuffer);

    bool data_ok  = (memcmp(gReadBuffer, data, 4u) == 0);
    bool state_ok = (gHandle.elementCount == 0u);

    TEST_RESULT("RB_Get: Reading all elements produces correct data and empty buffer",
                (result == eERROR_NO_ERROR) && data_ok && state_ok);
}

static void test_RB_Get_WrapAround(void)
{
    // Use identical setup as test_RB_Put_WrapAround to produce a wrapped state,
    // then verify that reading back reconstructs the original data in order.
    static const uint8_t fill[10]      = {0};
    static const uint8_t wrap_data[10] = {
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9
    };

    RB_Put(&gHandle, 10u, fill);
    RB_Get(&gHandle, 10u, gReadBuffer);         // Drain; head=10, tail=10
    RB_Put(&gHandle, 10u, wrap_data);           // Wrap; head=10, tail=4

    memset(gReadBuffer, 0, sizeof(gReadBuffer));
    error_t result = RB_Get(&gHandle, 10u, gReadBuffer);

    bool data_ok  = (memcmp(gReadBuffer, wrap_data, 10u) == 0);
    bool state_ok = (gHandle.elementCount == 0u);

    TEST_RESULT("RB_Get: Wrap-around read reconstructs data in correct order",
                (result == eERROR_NO_ERROR) && data_ok && state_ok);
}

static void test_RB_Get_Interleaved(void)
{
    // Interleaved sequence of puts and gets verifying that FIFO order and
    // element count remain correct throughout.
    static const uint8_t data_a[4]   = {0x01, 0x02, 0x03, 0x04};
    static const uint8_t data_b[4]   = {0x05, 0x06, 0x07, 0x08};
    static const uint8_t expected[6] = {0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    bool pass = true;

    RB_Put(&gHandle, 4u, data_a);
    pass = pass && (gHandle.elementCount == 4u);

    RB_Get(&gHandle, 2u, gReadBuffer);
    pass = pass && (gReadBuffer[0] == 0x01) && (gReadBuffer[1] == 0x02);
    pass = pass && (gHandle.elementCount == 2u);

    RB_Put(&gHandle, 4u, data_b);
    pass = pass && (gHandle.elementCount == 6u);

    RB_Get(&gHandle, 6u, gReadBuffer);
    pass = pass && (memcmp(gReadBuffer, expected, 6u) == 0);
    pass = pass && (gHandle.elementCount == 0u);

    TEST_RESULT("RB_Get: Interleaved put/get preserves FIFO order and element count",
                pass);
}

#if (RB_CONF_ALLOW_OVERWRITE == 1u)

////////////////////////////////////////////////////////////////////////////////
// RB_Put overwrite mode tests
////////////////////////////////////////////////////////////////////////////////

static void test_RB_Put_OW_WriteToFull(void)
{
    // Fill the buffer completely then write one more byte.
    // In overwrite mode this must succeed (not return BUFFER_FULL).
    static const uint8_t fill[TEST_BUFFER_SIZE] = {0};
    static const uint8_t extra                  = 0xFF;

    RB_Put(&gHandle, TEST_BUFFER_SIZE, fill);

    error_t result = RB_Put(&gHandle, 1u, &extra);

    TEST_RESULT("RB_Put (OW): Write one byte to full buffer succeeds",
                result == eERROR_NO_ERROR);
}

static void test_RB_Put_OW_ElementCountCapped(void)
{
    // After any overwrite, elementCount must never exceed bufferSize.
    static const uint8_t fill[TEST_BUFFER_SIZE] = {0};
    static const uint8_t extra                  = 0xFF;

    RB_Put(&gHandle, TEST_BUFFER_SIZE, fill);
    RB_Put(&gHandle, 1u, &extra);

    TEST_RESULT("RB_Put (OW): elementCount is capped at bufferSize after overwrite",
                gHandle.elementCount == TEST_BUFFER_SIZE);
}

static void test_RB_Put_OW_HeadFollowsTail(void)
{
    // After an overwrite, headIndex must equal tailIndex
    // so that head always points at the oldest surviving element.
    static const uint8_t fill[TEST_BUFFER_SIZE] = {0};
    static const uint8_t extra                  = 0xFF;

    RB_Put(&gHandle, TEST_BUFFER_SIZE, fill);
    RB_Put(&gHandle, 1u, &extra);

    TEST_RESULT("RB_Put (OW): headIndex equals tailIndex after overwrite",
                gHandle.headIndex == gHandle.tailIndex);
}

static void test_RB_Put_OW_LargerThanBuffer(void)
{
    // Writing more than bufferSize bytes must keep only the last bufferSize bytes.
    // Source: {0x00..0x13} (20 bytes). Discarded: first 4. Retained: {0x04..0x13}.
    static const uint8_t big_data[TEST_BUFFER_SIZE + 4u] = {
        0x00, 0x01, 0x02, 0x03,                           // Discarded (oldest)
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,  // Retained (newest)
        0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13
    };
    static const uint8_t expected[TEST_BUFFER_SIZE] = {
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13
    };

    error_t result = RB_Put(&gHandle, TEST_BUFFER_SIZE + 4u, big_data);

    RB_Get(&gHandle, TEST_BUFFER_SIZE, gReadBuffer);
    bool data_ok  = (memcmp(gReadBuffer, expected, TEST_BUFFER_SIZE) == 0);
    bool state_ok = (gHandle.elementCount == 0u);

    TEST_RESULT("RB_Put (OW): Write larger than bufferSize retains only last bufferSize bytes",
                (result == eERROR_NO_ERROR) && data_ok && state_ok);
}

////////////////////////////////////////////////////////////////////////////////
// RB_Get overwrite mode tests
////////////////////////////////////////////////////////////////////////////////

static void test_RB_Get_OW_ReadAfterPartialOverwrite(void)
{
    // Fill the buffer then overwrite 1 byte. Read all 16 bytes.
    // The oldest byte (0x00) must be dropped; the new byte (0xFF) appears at the end.
    static const uint8_t fill[TEST_BUFFER_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };
    static const uint8_t extra    = 0xFF;
    static const uint8_t expected[TEST_BUFFER_SIZE] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF
    };

    RB_Put(&gHandle, TEST_BUFFER_SIZE, fill);
    RB_Put(&gHandle, 1u, &extra);

    error_t result = RB_Get(&gHandle, TEST_BUFFER_SIZE, gReadBuffer);
    bool data_ok   = (memcmp(gReadBuffer, expected, TEST_BUFFER_SIZE) == 0);

    TEST_RESULT("RB_Get (OW): Read after 1-byte overwrite drops oldest and appends newest",
                (result == eERROR_NO_ERROR) && data_ok);
}

static void test_RB_Get_OW_ReadAfterFullReplace(void)
{
    // Write bufferSize bytes on top of a half-full buffer.
    // All old data must be displaced; read back must return only the new data.
    static const uint8_t old_data[8] = {
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA
    };
    static const uint8_t new_data[TEST_BUFFER_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };

    RB_Put(&gHandle, 8u, old_data);              // Half-fill with old data
    RB_Put(&gHandle, TEST_BUFFER_SIZE, new_data); // Overwrite with new data

    error_t result = RB_Get(&gHandle, TEST_BUFFER_SIZE, gReadBuffer);
    bool data_ok   = (memcmp(gReadBuffer, new_data, TEST_BUFFER_SIZE) == 0);

    TEST_RESULT("RB_Get (OW): Read after full-buffer overwrite returns only new data",
                (result == eERROR_NO_ERROR) && data_ok);
}

#endif // RB_CONF_ALLOW_OVERWRITE

////////////////////////////////////////////////////////////////////////////////
// Public function definitions
////////////////////////////////////////////////////////////////////////////////

void RB_Tests_Run(void)
{
    const size_t numTests = sizeof(gTests) / sizeof(gTests[0]);

    printf("\n=== Ring Buffer Tests ===\n\n");

    for(size_t i = 0u; i < numTests; i++)
    {
        ResetTestEnvironment();
        gTests[i]();
    }

#if (RB_CONF_ALLOW_OVERWRITE == 1u)
    const size_t numOverwriteTests = sizeof(gOverwriteTests) / sizeof(gOverwriteTests[0]);

    printf("\n=== Ring Buffer Overwrite Mode Tests ===\n\n");

    for(size_t i = 0u; i < numOverwriteTests; i++)
    {
        ResetTestEnvironment();
        gOverwriteTests[i]();
    }
#endif // RB_CONF_ALLOW_OVERWRITE

    printf("\n=== Results: %d passed, %d failed ===\n\n",
           gPassCount, gFailCount);
}
