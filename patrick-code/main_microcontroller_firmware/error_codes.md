# Error Code Integer Values

## A list of common error codes used by the API

### By convention zero represents no-error/success, and negative values represent error cases

### These error codes are defined by the MSDK in [mxc_errors.h](https://github.com/analogdevicesinc/msdk/blob/main/Libraries/PeriphDrivers/Include/MAX32665/mxc_errors.h)

|  Error Code     | Integer Value |
|-----------------|---------------|
| E_NO_ERROR      |    0          |
| E_SUCCESS       |    0          |
| E_NULL_PTR      |   -1          |
| E_NO_DEVICE     |   -2          |
| E_BAD_PARAM     |   -3          |
| E_INVALID       |   -4          |
| E_UNINITIALIZED |   -5          |
| E_BUSY          |   -6          |
| E_BAD_STATE     |   -7          |
| E_UNKNOWN       |   -8          |
| E_COMM_ERR      |   -9          |
| E_TIME_OUT      |  -10          |
| E_NO_RESPONSE   |  -11          |
| E_OVERFLOW      |  -12          |
| E_UNDERFLOW     |  -13          |
| E_NONE_AVAIL    |  -14          |
| E_SHUTDOWN      |  -15          |
| E_ABORT         |  -16          |
| E_NOT_SUPPORTED |  -17          |
| E_FAIL          | -255          |
