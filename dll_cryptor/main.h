/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#pragma once

#include <windows.h>
#include <wininet.h>
#include <shellapi.h>
#include <commctrl.h>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <vector>
#include <map>
#include <io.h>


#include "pefile\pe_base.h"
#include "pefile\pe_file.h"
#include "pefile\pe_section.h"
#include "pefile\pe_export.h"
#include "pefile\pe_import.h"
#include "pefile\pe_relocs.h"
#include "pefile\pe_tls.h"
#include "pefile\pe_rsrc.h"

#include "crypt.h"
#include "misc.h"

#include "rsrc\resource.h"
