/* Copyright (C) 2024 John Törnblom

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING. If not, see
<http://www.gnu.org/licenses/>.  */

#include <stdlib.h>
#include <string.h>

#include "regmgr.h"


int sceRegMgrGetInt(long, int*);
int sceRegMgrGetStr(long, char*, int);
int sceRegMgrGetBin(long, void*, int);

int sceRegMgrSetInt(long, int);
int sceRegMgrSetBin(long, const void*, int);
int sceRegMgrSetStr(long, const char*, int);


int RegMgr_Get(long key, int type, int size, void* val)
{
    switch(type) {
    case SCE_REGMGR_TYPE_INTEGER:
        return sceRegMgrGetInt(key, (int*)val);

    case SCE_REGMGR_TYPE_STRING:
        return sceRegMgrGetStr(key, (char*)val, size);

    case SCE_REGMGR_TYPE_BINARY:
        return sceRegMgrGetBin(key, val, size);
    }

    return -1;
}

int RegMgr_Set(long key, int type, int size, const void* val)
{
    char str[size];

    switch(type) {
    case SCE_REGMGR_TYPE_INTEGER:
        return sceRegMgrSetInt(key, *(const int*)val);

    case SCE_REGMGR_TYPE_STRING:
        memset(str, 0, size);
        strcpy(str, (const char*)val);
        return sceRegMgrSetStr(key, str, size);

    case SCE_REGMGR_TYPE_BINARY:
        return sceRegMgrSetBin(key, (const void*)val, size);
    }

    return -1;
}


/* Local Variables: */
/* tab-width: 8 */
/* c-basic-offset: 4 */
/* End: */
