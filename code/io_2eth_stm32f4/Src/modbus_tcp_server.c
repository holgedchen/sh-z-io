
#include "cmsis_os.h"
#include "lwip.h"
#include "mb.h"

eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
		
	return 	MB_ENOERR;
}

eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode ) {
		
	return 	MB_ENOERR;
}

eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode ) {
		
	return 	MB_ENOERR;
}

eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete ) {
		
	return 	MB_ENOERR;
}
							   
							   
void start_modbus_tcp_server(void const * argument) {
	
}

