/**
 * AS - the open source Automotive Software on https://github.com/parai
 *
 * Copyright (C) 2018  AS <parai@foxmail.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
#ifdef USE_XCP
/* ============================ [ INCLUDES  ] ====================================================== */
#include "bootloader.h"
#include "Flash.h"
#include "Os.h"
#include "asdebug.h"
/* ============================ [ MACROS    ] ====================================================== */
/* ============================ [ TYPES     ] ====================================================== */
/* ============================ [ DECLARES  ] ====================================================== */
static void PBL_wdTriggerFct(void);
extern void application_main(void); /* Symbol exposed in linker.lds */
/* ============================ [ DATAS     ] ====================================================== */
static tFlashParam pblFlashParam =
{
    .patchlevel  = FLASH_DRIVER_VERSION_PATCH,
    .minornumber = FLASH_DRIVER_VERSION_MINOR,
    .majornumber = FLASH_DRIVER_VERSION_MAJOR,
    .wdTriggerFct = PBL_wdTriggerFct,
    .errorcode   = kFlashFailed,
};
/* ============================ [ LOCALS    ] ====================================================== */
static void PBL_wdTriggerFct(void)
{
	/* feed the watchdog */
}
/* ============================ [ FUNCTIONS ] ====================================================== */
Std_ReturnType Xcp_UnlockFn(Xcp_ProtectType res, const uint8* seed,
            uint8 seed_len, const uint8* key, uint8 key_len)
{
	Std_ReturnType ercd = E_OK;
	if( (4==seed_len) && (4==key_len))
	{
		if( 0 == memcmp(seed,key,4))
		{
			if(XCP_PROTECT_PGM == res)
			{
				pblFlashParam.errorcode = kFlashFailed;
			}
		}
		else
		{
			ercd = E_NOT_OK;
		}
	}
	else
	{
		ercd = E_NOT_OK;
	}

	return ercd;
}
uint8 Xcp_SeedFn(Xcp_ProtectType res, uint8* seed)
{
	uint8 seed_r[4];    /* use stack local random value */
	memcpy(seed,seed_r,4);
	return 4;
}

Std_ReturnType Xcp_FlashErase(uint32 address, uint32 length)
{
	Std_ReturnType ercd = E_OK;

	if(kFlashOk != pblFlashParam.errorcode)
	{
		FLASH_DRIVER_INIT(FLASH_DRIVER_STARTADDRESS,&pblFlashParam);
		if(kFlashOk != pblFlashParam.errorcode)
		{
			ercd = E_NOT_OK;
		}
		else
		{
			pblFlashParam.address = address;
			pblFlashParam.length  = length;
			pblFlashParam.data    = NULL;
			FLASH_DRIVER_ERASE(FLASH_DRIVER_STARTADDRESS,&pblFlashParam);
			if(kFlashOk != pblFlashParam.errorcode)
			{
				ercd = E_NOT_OK;
			}
		}
	}
	return ercd;
}

Std_ReturnType Xcp_ProgramReset(void* data, int len)
{
	(void)data;
	(void)len;
	if((*(uint32_t*)application_main) != 0)
	{
		application_main();
	}
	else
	{
		ASLOG(PBL,"invalid application entry point, stay in bootloader.\n");
	}
	return E_OK;
}

void TaskIdleHook(void)
{
	Xcp_MainFunction();
}
#endif /* USE_XCP */
