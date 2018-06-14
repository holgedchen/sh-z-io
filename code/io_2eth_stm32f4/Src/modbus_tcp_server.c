#include <string.h>
#include "cmsis_os.h"
#include "lwip.h"
#include "mb.h"
#include "sh_z_002.h"
#include "mb_tcp_server.h"
#include "di_monitor.h"
#include "ai_monitor.h"
#include "spiffs.h"
#include "fs_handling.h"

#define PROG                    	"FreeModbus"
extern spiffs SPI_FFS_fs;
extern char SH_Z_002_SN[SH_Z_SN_LEN + 1];
/**********************  AI part  *************************/
static uint16_t unADCxConvertedValueBuf[SH_Z_002_AI_NUM];

/**********************  DI part  *************************/
#define MB_REG_DI_CNT_OVF_ADDR			97

static uint32_t unDI_CNT_FreqValueBuf[SH_Z_002_DI_NUM];
static uint32_t DI_ValuesBuf;
static uint32_t DI_EnableCNT_Buf;
static uint32_t DI_ClearCNT_Buf;
static uint32_t DI_CNT_Overflow_Buf;
static uint32_t DI_LatchSet_Buf;
static uint32_t DI_LatchStatus_Buf;

/**********************  FS part  *************************/
#define FS_ERASE_ALL_FILES_PWD			0x1A0CA544			//'D'
#define FS_FORMAT_PWD					0x1A0CA546			//'F'
static uint32_t FS_EraseAllFilesPwd;
static uint32_t FS_FormatPwd;
//static DI_ConfTypeDef tDI_ConfBuf[SH_Z_002_DI_NUM];

static eMBErrorCode get_DI_value_buf( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils ) {
	DI_ValuesBuf = DI_get_DI_values();
	return 	MB_ENOERR;
}

static eMBErrorCode get_AI_value_buf( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
	AI_get_AI_values(unADCxConvertedValueBuf);
	return 	MB_ENOERR;
}


static eMBErrorCode get_DI_cnt_freq_buf( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
	DI_get_DI_cnt_freq(unDI_CNT_FreqValueBuf, SH_Z_002_DI_NUM);
	return 	MB_ENOERR;
}

static eMBErrorCode get_DI_enable_CNT_buf( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
	DI_EnableCNT_Buf = DI_get_DI_enable_CNT();
	return 	MB_ENOERR;
}

static eMBErrorCode set_DI_enable_CNT( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
	DI_set_DI_enable_CNT(DI_EnableCNT_Buf);
	return 	MB_ENOERR;
}

static eMBErrorCode clear_DI_CNT( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
	uint8_t unDI_Index;
	for (unDI_Index = 0; unDI_Index < SH_Z_002_DI_NUM; unDI_Index++) {
		if (READ_BIT(DI_ClearCNT_Buf, 0x01 << unDI_Index)) 
			DI_clear_DI_CNT(unDI_Index);{
		}
	}
	DI_ClearCNT_Buf = 0;
	return 	MB_ENOERR;
}

static eMBErrorCode get_DI_latch_set_buf( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
	DI_LatchSet_Buf = DI_get_DI_latch_set();
	return 	MB_ENOERR;
}

static eMBErrorCode set_DI_latch_set_buf( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
	DI_set_DI_latch_set(DI_LatchSet_Buf);
	return 	MB_ENOERR;
}

static eMBErrorCode clear_DI_latch( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
//	uint8_t unDI_Index;
//	for (unDI_Index = 0; unDI_Index < SH_Z_002_DI_NUM; unDI_Index++) {
//		if (READ_BIT(DI_LatchStatus_Buf, 0x01 << unDI_Index)) 
//			DI_clear_DI_latch(unDI_Index);{
//		}
//	}
//	DI_LatchStatus_Buf = 0;
	DI_set_DI_latch_status(DI_LatchStatus_Buf);
	return 	MB_ENOERR;
}

static eMBErrorCode get_DI_latch_status_buf( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
	DI_LatchStatus_Buf = DI_get_DI_latch_status();
	return 	MB_ENOERR;
}

static eMBErrorCode get_DI_CNT_overflow_buf( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils ) {
	if ((usAddress < MB_REG_DI_CNT_OVF_ADDR) || ((usAddress + usNCoils) > (MB_REG_DI_CNT_OVF_ADDR + SH_Z_002_DI_NUM))) {
		return MB_ENOREG;
	} else {
		DI_CNT_Overflow_Buf = DI_get_DI_CNT_overflow();
		return 	MB_ENOERR;		
	}
}

static eMBErrorCode clear_DI_CNT_overflow( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils ) {
	uint8_t unDI_Index;
	if ((usAddress < MB_REG_DI_CNT_OVF_ADDR) || ((usAddress + usNCoils) > (MB_REG_DI_CNT_OVF_ADDR + SH_Z_002_DI_NUM))) {
		return MB_ENOREG;
	} else {
		for (unDI_Index = (usAddress - MB_REG_DI_CNT_OVF_ADDR); unDI_Index < (usAddress + usNCoils); unDI_Index++) {
			if (READ_BIT(DI_CNT_Overflow_Buf, 0x01 << unDI_Index)) 
				DI_clear_DI_CNT_oveflow(unDI_Index);{
			}
		}
	}
	return 	MB_ENOERR;
}

static eMBErrorCode erase_all_spiffs_files( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils ) {
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;
	
	if (FS_ERASE_ALL_FILES_PWD == FS_EraseAllFilesPwd) {
		xReturned = xTaskCreate(start_erase_all_files_task, "FILES_ERASE", 128, NULL, tskIDLE_PRIORITY, &xHandle );
		if( xReturned != pdPASS ){
			return MB_ENORES;
		} else {
			return MB_ENOERR;
		}
	} else {		
		return MB_EINVAL;
	}
}

static eMBErrorCode format_spiffs_flash( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils ) {
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;
	
	if (FS_FORMAT_PWD == FS_FormatPwd) {
		xReturned = xTaskCreate(start_format_fs_task, "FS_FORMAT", 128, NULL, tskIDLE_PRIORITY, &xHandle );
		if( xReturned != pdPASS ){
			return MB_ENORES;
		} else {
			return MB_ENOERR;
		}
	} else {		
		return MB_EINVAL;
	}
}

const MB_RegAccessTypeDef SH_Z_X_MB_REG[] = {
	{1, sizeof(DI_ValuesBuf), &DI_ValuesBuf, MB_TCP_SVR_FUNC_RD_COLIS_BIT, get_DI_value_buf, NULL, NULL, NULL},	
	{33, sizeof(DI_EnableCNT_Buf), &DI_EnableCNT_Buf, MB_TCP_SVR_FUNC_RD_COLIS_BIT | MB_TCP_SVR_FUNC_WR_COLIS_BIT, get_DI_enable_CNT_buf, NULL, NULL, set_DI_enable_CNT},	
	{65, sizeof(DI_ClearCNT_Buf), &DI_ClearCNT_Buf, MB_TCP_SVR_FUNC_WR_COLIS_BIT, NULL, NULL, NULL, clear_DI_CNT},
	{MB_REG_DI_CNT_OVF_ADDR, sizeof(DI_CNT_Overflow_Buf), &DI_CNT_Overflow_Buf, MB_TCP_SVR_FUNC_RD_COLIS_BIT, get_DI_CNT_overflow_buf, clear_DI_CNT_overflow, NULL, NULL},
	{129, sizeof(DI_LatchSet_Buf), &DI_LatchSet_Buf, MB_TCP_SVR_FUNC_RD_COLIS_BIT | MB_TCP_SVR_FUNC_WR_COLIS_BIT, get_DI_latch_set_buf, NULL, NULL, set_DI_latch_set_buf},
	{161, sizeof(DI_LatchStatus_Buf), &DI_LatchStatus_Buf, MB_TCP_SVR_FUNC_RD_COLIS_BIT | MB_TCP_SVR_FUNC_WR_COLIS_BIT, get_DI_latch_status_buf, NULL, NULL, clear_DI_latch},
	{40001, sizeof(unDI_CNT_FreqValueBuf), unDI_CNT_FreqValueBuf , MB_TCP_SVR_FUNC_RD_INPUT_BIT, get_DI_cnt_freq_buf, NULL, NULL, NULL},
	{40101, sizeof(unADCxConvertedValueBuf), unADCxConvertedValueBuf , MB_TCP_SVR_FUNC_RD_INPUT_BIT, get_AI_value_buf, NULL, NULL, NULL},
	{40501, sizeof(DI_ValuesBuf), &DI_ValuesBuf , MB_TCP_SVR_FUNC_RD_INPUT_BIT, get_DI_value_buf, NULL, NULL, NULL},
	{50001, sizeof(FS_EraseAllFilesPwd), &FS_EraseAllFilesPwd , MB_TCP_SVR_FUNC_WR_HOLDING_BIT, NULL, NULL, NULL, erase_all_spiffs_files},
	{50003, sizeof(FS_FormatPwd), &FS_FormatPwd , MB_TCP_SVR_FUNC_WR_HOLDING_BIT, NULL, NULL, NULL, format_spiffs_flash},
	{0, 0, NULL, 0, NULL, NULL, NULL, NULL}
};

static const MB_RegAccessTypeDef* get_reg_coil_access(USHORT usAddress, USHORT usNCoils, MB_TCP_ServerFuncBit eFunc) {
	const MB_RegAccessTypeDef* pRegAccess = SH_Z_X_MB_REG;
	USHORT usNBytes = GET_DI_BYTE_NUM(usNCoils);
	while (pRegAccess->unRegByteLen != 0) {
		if ((usAddress >= pRegAccess->unRegAddr) && 
			(usAddress < (pRegAccess->unRegAddr + pRegAccess->unRegByteLen)) && 
			((usAddress + usNBytes) <= (pRegAccess->unRegAddr + pRegAccess->unRegByteLen))) {
			if ((pRegAccess->unMB_RegFuncEnBits & eFunc) != 0) {
				return pRegAccess;
			} else {
				return NULL;
			}		
		}
		pRegAccess++;
	}
	return NULL;
}

static const MB_RegAccessTypeDef* get_reg_access(USHORT usAddress, USHORT usNRegs, MB_TCP_ServerFuncBit eFunc) {
	const MB_RegAccessTypeDef* pRegAccess = SH_Z_X_MB_REG;
	while (pRegAccess->unRegByteLen != 0) {
		if ((usAddress >= pRegAccess->unRegAddr) && 
			(usAddress < (pRegAccess->unRegAddr + pRegAccess->unRegByteLen)) && 
			((usAddress + usNRegs * 2) <= (pRegAccess->unRegAddr + pRegAccess->unRegByteLen))) {
			if ((pRegAccess->unMB_RegFuncEnBits & eFunc) != 0) {
				return pRegAccess;
			} else {
				return NULL;
			}		
		}
		pRegAccess++;
	}
	return NULL;
}


eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs ) {
	const MB_RegAccessTypeDef* pRegAccess = get_reg_access(usAddress, usNRegs, MB_TCP_SVR_FUNC_RD_INPUT_BIT);
	eMBErrorCode eErrCode;
	
	if (pRegAccess != NULL) {
		if (pRegAccess->preReadF != NULL ) {
			eErrCode = pRegAccess->preReadF(pucRegBuffer, usAddress, usNRegs * 2);
			if (MB_ENOERR != eErrCode ) {
				return eErrCode;
			}
		}
		memcpy(pucRegBuffer, (uint8_t *)(pRegAccess->pRegValue) + usAddress - pRegAccess->unRegAddr, usNRegs * 2);
		
		if (pRegAccess->postReadF != NULL ) {
			eErrCode = pRegAccess->postReadF(pucRegBuffer, usAddress, usNRegs * 2);
			if (MB_ENOERR != eErrCode ) {
				return eErrCode;
			}
		}
		return MB_ENOERR;
	} else {
		return MB_ENOREG;
	}
}

eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode ) {
	const MB_RegAccessTypeDef* pRegAccess;
	eMBErrorCode eErrCode;
	int i;
	
	pRegAccess = get_reg_coil_access(usAddress, usNCoils, (eMode == MB_REG_READ) ? (MB_TCP_SVR_FUNC_RD_COLIS_BIT):(MB_TCP_SVR_FUNC_WR_COLIS_BIT));
	if (pRegAccess != NULL) {
		if ((usAddress < pRegAccess->unRegAddr) || ((GET_DI_BYTE_NUM(usAddress - pRegAccess->unRegAddr + usNCoils)) > pRegAccess->unRegByteLen)) {
			return MB_ENOREG;
		}
		if (MB_REG_READ == eMode) {
			if (pRegAccess->preReadF != NULL ) {
				eErrCode = pRegAccess->preReadF(pucRegBuffer, usAddress, usNCoils);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}
			*(uint32_t *)(pRegAccess->pRegValue) = *(uint32_t *)(pRegAccess->pRegValue) >> (usAddress - pRegAccess->unRegAddr);
			memcpy(pucRegBuffer, (uint8_t *)(pRegAccess->pRegValue), GET_DI_BYTE_NUM(usNCoils));
			
			if (pRegAccess->postReadF != NULL ) {
				eErrCode = pRegAccess->postReadF(pucRegBuffer, usAddress, usNCoils);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}
			return MB_ENOERR;
		} else {
			if (pRegAccess->preWriteF != NULL ) {
				eErrCode = pRegAccess->preWriteF(pucRegBuffer, usAddress, usNCoils);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}
			for (i = 0; i < usNCoils; i++) {
				if (READ_BIT(*(pucRegBuffer + i/8), (0x01 << (i%8)))) {
					SET_BIT(*((uint8_t *)(pRegAccess->pRegValue) + (i + (usAddress - pRegAccess->unRegAddr))/8), (0x01 << ((i + (usAddress - pRegAccess->unRegAddr))%8)));
				} else {
					CLEAR_BIT(*((uint8_t *)(pRegAccess->pRegValue) + (i + (usAddress - pRegAccess->unRegAddr))/8), (0x01 << ((i + (usAddress - pRegAccess->unRegAddr))%8)));
				}
			}
			
			if (pRegAccess->postWriteF != NULL ) {
				eErrCode = pRegAccess->postWriteF(pucRegBuffer, usAddress, usNCoils);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}
			return	MB_ENOERR;
		}
	} else {
		return	MB_EINVAL;
	}
}

eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode ) {
	const MB_RegAccessTypeDef* pRegAccess = 
		get_reg_access(usAddress, usNRegs, (eMode == MB_REG_READ) ? (MB_TCP_SVR_FUNC_RD_HOLDING_BIT):(MB_TCP_SVR_FUNC_WR_HOLDING_BIT));
	eMBErrorCode eErrCode;
	
	if (pRegAccess != NULL) {
		if (MB_REG_READ == eMode) {
			if (pRegAccess->preReadF != NULL ) {
				eErrCode = pRegAccess->preReadF(pucRegBuffer, usAddress, usNRegs * 2);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}
			memcpy(pucRegBuffer, (uint8_t *)(pRegAccess->pRegValue) + usAddress - pRegAccess->unRegAddr, usNRegs * 2);
			
			if (pRegAccess->postReadF != NULL ) {
				eErrCode = pRegAccess->postReadF(pucRegBuffer, usAddress, usNRegs * 2);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}					
		} else {
			if (pRegAccess->preWriteF != NULL ) {
				eErrCode = pRegAccess->preWriteF(pucRegBuffer, usAddress, usNRegs * 2);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}
			memcpy((uint8_t *)(pRegAccess->pRegValue) + usAddress - pRegAccess->unRegAddr, pucRegBuffer, usNRegs * 2);
			
			if (pRegAccess->postWriteF != NULL ) {
				eErrCode = pRegAccess->postWriteF(pucRegBuffer, usAddress, usNRegs * 2);
				if (MB_ENOERR != eErrCode ) {
					return eErrCode;
				}
			}			
		}
		return MB_ENOERR;
	} else {
		return MB_ENOREG;
	}
}

eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete ) {
		
	return 	MB_EILLSTATE;
}			   
							   
void start_modbus_tcp_server(void const * argument) {
	static char cReportSlaveID[SH_Z_SN_LEN + 2 + 1];
	eMBErrorCode    xStatus;

	snprintf(cReportSlaveID, sizeof(cReportSlaveID), "%02X%s", SH_Z_002_VERSION, SH_Z_002_SN);
	cReportSlaveID[sizeof(cReportSlaveID) - 1] = '\0';
	eMBSetSlaveID(SH_Z_002_SLAVE_ID, TRUE, (uint8_t *)cReportSlaveID, strlen(cReportSlaveID));
	
	if( eMBTCPInit( MB_TCP_PORT_USE_DEFAULT ) != MB_ENOERR ) {
		printf( "%s: can't initialize modbus stack!\r\n", PROG );
	} else if( eMBEnable(  ) != MB_ENOERR ) {
		printf( "%s: can't enable modbus stack!\r\n", PROG );
	} else {
		do {
			xStatus = eMBPoll(  );
		}
		while( xStatus == MB_ENOERR );
	}
	/* An error occured. Maybe we can restart. */
	( void )eMBDisable(  );
	( void )eMBClose(  );			

}

