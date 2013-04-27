/*============================================================================*/
/*                   PROJECT INCLUDE FILES                                    */
/*============================================================================*/
#define CUSTOMER_IMPLEMENTATION

#include <stdarg.h>

#include "tmNxTypes.h"
#include "tmCompId.h"
#include "tmFrontEnd.h"
#include "tmbslFrontEndTypes.h"
#include "tmsysFrontEndTypes.h"
#include "tmUnitParams.h"


#include "tmsysOM3912.h"
#include "tmsysOM3912local.h"
#include "tmsysOM3912Instance.h"

#ifndef CUSTOMER_IMPLEMENTATION
#include "tmdlI2C.h"
#include "tmptkDefaultDbgMsgUnManaged.h"
#include "tmsysOM3912Dbg.h"
#endif

/*============================================================================*/
/*                   TYPE DEFINITION                                          */
/*============================================================================*/

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#define P_DBGPRINTEx tmsysOM3912Print
#define P_DBGPRINTVALID (True)
#define OM3912_INVALID_DEVICE_ADDRESS (0xFF)

/*============================================================================*/
/*                   CUSTOMER SPECIFIC IMPLEMENTATION                         */
/*============================================================================*/
#ifdef CUSTOMER_IMPLEMENTATION

#define I2C_BUFFER_SIZE (500)
#define PRINT_BUFFER_SIZE (1040)

/* Link your I�C Read function to this function */
/*============================================================================*/
/* tmdlI2CRead                                                                */
/*============================================================================*/
tmErrorCode_t tmdlI2CRead(tmUnitSelect_t tUnit, UInt32 uHwAddress, UInt32 AddrSize, UInt8* pAddr, UInt32 ReadLen, UInt8* pData)
{
    tmErrorCode_t   err = TM_OK;

    /* Place your code here */

    return err;
}

/* Link your I�C Write function to this function */
/*============================================================================*/
/* tmdlI2CWrite                                                               */
/*============================================================================*/
tmErrorCode_t tmdlI2CWrite(tmUnitSelect_t tUnit, UInt32 uHwAddress, UInt32 AddrSize, UInt8* pAddr, UInt32 WriteLen, UInt8* pData)
{
    tmErrorCode_t   err = TM_OK;

    /* Place your code here */

    return err;
}

/* Link your Wait function to this function */
/*============================================================================*/
/* tmdlWait                                                                   */
/*============================================================================*/
tmErrorCode_t tmdlWait(UInt32 tms)
{
    tmErrorCode_t   err = TM_OK;

    /* Place your code here */

    return err;
}

/* Link your Print function to this function */
/*============================================================================*/
/* tmdlPrint                                                                  */
/*============================================================================*/
tmErrorCode_t tmdlPrint(char *buf)
{
    tmErrorCode_t   err = TM_OK;

    /* Place your code here */

    return err;
}

#else

#define I2C_BUFFER_SIZE (500)
#define PRINT_BUFFER_SIZE (1040)

#endif




/*
/*============================================================================*/
/*                   PUBLIC FUNCTIONS DEFINITIONS                             */
/*============================================================================*/

tmErrorCode_t tmsysOM3912I2CRead(tmUnitSelect_t tUnit, UInt32 AddrSize, UInt8* pAddr, UInt32 ReadLen, UInt8* pData)
{
    ptmOM3912Object_t       pObj = Null;
    tmErrorCode_t           err = TM_OK;
    tmErrorCode_t           errI2C = TM_OK;
    UInt8                   uBytesBuffer[I2C_BUFFER_SIZE];
    UInt8                   uSubAddress = 0;
    UInt32                  i = 0;
    tmUnitSelect_t          tUnitOM3912 = GET_SYSTEM_TUNIT(tUnit);

    if(GET_XFER_DISABLED_FLAG_TUNIT(tUnit) == False)
    {
        err = OM3912GetInstance(tUnitOM3912, &pObj);
        tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "OM3912GetInstance(0x%08X) failed.", tUnit));

        if(err == TM_OK)
        {
            if(pObj->uHwAddress[UNIT_PATH_TYPE_GET(tUnit)] != OM3912_INVALID_DEVICE_ADDRESS)
            {          
                switch(AddrSize)
                {
                    default:
                    case 0:
                        uSubAddress = 0;
                        break;

                    case 1:
                        uSubAddress = (*pAddr)&0xFF;
                        break;
                }

                if (ReadLen > I2C_BUFFER_SIZE)
                {
                    err = OM3912_ERR_BAD_PARAMETER;
                }
                else
                {
                    switch(UNIT_PATH_TYPE_GET(tUnit))
                    {
                        case tmOM3912UnitDeviceTypeTuner:
                            errI2C = tmdlI2CRead(tUnit, pObj->uHwAddress[UNIT_PATH_TYPE_GET(tUnit)], AddrSize, &uSubAddress, ReadLen, uBytesBuffer);
                            break;

                        default:
                            tmDBGPRINTEx(DEBUGLVL_ERROR, "tmsysOM3912I2CRead(%08X) failed. Unknown Device Type.", tUnit);
                            err = OM3912_ERR_BAD_PARAMETER;
                            break;
                    }
                }

                if(errI2C == TM_OK && err == TM_OK)
                {
                    for (i=0 ; i<ReadLen ; i++) 
                    {
                        *(pData + i) = (UInt8)uBytesBuffer[i];
                    }
                }
                else
                {
                    tmDBGPRINTEx(DEBUGLVL_ERROR, "tmsysOM3912I2CRead(0x%08X, [sub@0x%02x] %d) failed.", tUnit, uSubAddress, ReadLen);
                    err = OM3912_ERR_HW;
                }
            }
        }
    }

    return err;
}

tmErrorCode_t tmsysOM3912I2CWrite(tmUnitSelect_t tUnit, UInt32 AddrSize, UInt8* pAddr, UInt32 WriteLen, UInt8* pData)
{
    ptmOM3912Object_t       pObj = Null;
    tmErrorCode_t           err = TM_OK;
    tmErrorCode_t           errI2C = TM_OK;
    Bool                    bIncSubAddress = True;
    UInt8                   uSubAddress = 0;
    UInt32                  uCounter = 0;
    UInt8                   WriteBuffer[I2C_BUFFER_SIZE+1] = {0};
    UInt32                  count = 0;
    UInt32                  remain = 0;
    tmUnitSelect_t          tUnitOM3912 = GET_SYSTEM_TUNIT(tUnit);

    if(GET_XFER_DISABLED_FLAG_TUNIT(tUnit) == False)
    {
        err = OM3912GetInstance(tUnitOM3912, &pObj);
        tmASSERTExT(err, TM_OK, (DEBUGLVL_ERROR, "OM3912GetInstance(0x%08X) failed.", tUnit));

        if(err == TM_OK)
        {
            if(pObj->uHwAddress[UNIT_PATH_TYPE_GET(tUnit)] != OM3912_INVALID_DEVICE_ADDRESS)
            {
                switch(AddrSize)
                {
                    default:
                    case 0:
                        uSubAddress = 0;
                        break;

                    case 1:
                        uSubAddress = (*pAddr)&0xFF;
                        break;
                }

                /* Copy uSubAddress to first data byte */
                WriteBuffer[0] = uSubAddress;

                for(count=0; count<WriteLen; count += remain)
                {
                    remain = min(WriteLen-count, I2C_BUFFER_SIZE);

                    /* copy I2CMap data in WriteBuffer */
                    for (uCounter = 0; uCounter < remain; uCounter++)
                    {
                        WriteBuffer[uCounter+1] = pData[count+uCounter];
                    }

                    switch(UNIT_PATH_TYPE_GET(tUnit))
                    {
                        case tmOM3912UnitDeviceTypeTuner:
                            errI2C = tmdlI2CWrite(tUnit, pObj->uHwAddress[UNIT_PATH_TYPE_GET(tUnit)], AddrSize, &uSubAddress, remain, WriteBuffer+1);
                            break;

                        default:
                            tmDBGPRINTEx(DEBUGLVL_ERROR, "tmsysOM3912I2CWrite(0x%08X) failed. Unknown Device Type.", tUnit);
                            err = OM3912_ERR_BAD_PARAMETER;
                            break;
                    }

                    if(errI2C != TM_OK)
                    {
                        tmDBGPRINTEx(DEBUGLVL_ERROR, "tmsysOM3912I2CWrite(0x%08X, [sub@0x%02x] %d/%d) failed.", tUnit, uSubAddress, count, WriteLen);
                        err = OM3912_ERR_HW;
                    }

                    if(err != TM_OK)
                        break;

                    if(bIncSubAddress)
                    {
                        uSubAddress = (UInt8)(uSubAddress+remain);
                        WriteBuffer[0] = uSubAddress;
                    }
                }
            }
        }
    }

    return err;
}

/**  Wait tms ms without blocking scheduler, warning this function 
don't schedule others frontend instance */
tmErrorCode_t tmsysOM3912Wait(tmUnitSelect_t tUnit, UInt32 tms)
{
#ifndef CUSTOMER_IMPLEMENTATION
    LARGE_INTEGER freq;
    LARGE_INTEGER count1;
    LARGE_INTEGER count2;
    UInt32 time_ms = 0;

    freq.QuadPart = 0;
    count1.QuadPart = 0;
    count2.QuadPart = 0;

    if (QueryPerformanceFrequency(&freq) != 0)
    {

        if (QueryPerformanceCounter(&count1) != 0)
        {
            while(time_ms<tms)
            {
                Sleep(0);
                QueryPerformanceCounter(&count2);
                time_ms = (UInt32)((count2.QuadPart - count1.QuadPart) * 1000 / freq.QuadPart);
            }
        }
        else
        {
            Sleep(1);
            Sleep(tms);
        }
    }
#else
    /* Customer implementation */
    tmdlWait(tms);
#endif

    return TM_OK;
}

#ifdef NXPFEV2_DBG_MSG

w_tmdlDbgMsgCallBack    pDbgMsgCallBack = Null;

w_tmdlDbgMsgCallBack *tmsysOM3912AllocDbgMsgUnManaged()
{
    return &pDbgMsgCallBack;
}

tmErrorCode_t tmsysOM3912DeAllocDbgMsgUnManaged()
{
    pDbgMsgCallBack = Null;

    return TM_OK;
}

#endif

tmErrorCode_t tmsysOM3912Print(UInt32 level, const char* format, ...)
{
    tmErrorCode_t   err = TM_OK;

#ifdef NXPFEV2_DBG_MSG
    va_list         args;
    int             len = 0;
    char            *buffer = Null;

    va_start( args, format );
    len = _vscprintf(format, args ) + 1;
    buffer = (char *)malloc(len * sizeof(char));

    (void)vsprintf_s(buffer, len, format, args);

    if(pDbgMsgCallBack != Null)
    {
        err = pDbgMsgCallBack(level, buffer);
    }

    free(buffer);
#else
    static char     buf[PRINT_BUFFER_SIZE] = "\0";
    va_list         ap;

    if(level<DEBUGLVL_VERBOSE)
    {
        va_start(ap, format);
        (void)vsnprintf(buf, PRINT_BUFFER_SIZE, format, ap);
        va_end(ap);

#ifdef CUSTOMER_IMPLEMENTATION
        /* Customer implementation */
        tmdlPrint(buf);
#endif
    }
#endif

    return err;
}
