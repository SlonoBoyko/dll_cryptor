#pragma once

#include <windows.h>
#include <stdint.h>
#include <process.h>

#include "pe_load.h"

static char data[] = { 
	0
};

extern uint32_t module_base;

//void Log(const char *text, ...);