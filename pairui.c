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

#include <time.h>

#include "notify.h"
#include "pairui.h"
#include "regmgr.h"


int sceKernelDlsym(int, const char*, void*);
int sceKernelLoadStartModule(const char*, unsigned long, const void*,
                             unsigned int, void*, int*);
int sceUserServiceGetForegroundUser(int *);

int (*sceRemoteplayInitialize)(void*, size_t) = 0;
int (*sceRemoteplayGeneratePinCode)(uint32_t*) = 0;
int (*sceRemoteplayConfirmDeviceRegist)(int*, int*) = 0;
int (*sceRemoteplayNotifyPinCodeError)(int) = 0;


static int RemoteplayInit(void)
{
    int handle;

    if(sceRemoteplayInitialize) {
        return 0;
    }
    if((handle=sceKernelLoadStartModule("/system/common/lib/libSceRemoteplay.sprx",
                                        0, 0, 0, 0, 0)) < 0) {
        return -1;
    }

    if(sceKernelDlsym(handle, "sceRemoteplayInitialize",
                      &sceRemoteplayInitialize)) {
        return -1;
    }
    sceRemoteplayInitialize(0, 0);

    if(sceKernelDlsym(handle, "sceRemoteplayGeneratePinCode",
                      &sceRemoteplayGeneratePinCode)) {
        return -1;
    }
    if(sceKernelDlsym(handle, "sceRemoteplayConfirmDeviceRegist",
                      &sceRemoteplayConfirmDeviceRegist)) {
        return -1;
    }
    if(sceKernelDlsym(handle, "sceRemoteplayNotifyPinCodeError",
                      &sceRemoteplayNotifyPinCodeError)) {
        return -1;
    }

    return 0;
}

static void Base64Encode(uint64_t input, char* output) {
    char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    uint8_t* data = (uint8_t*)&input;

    output[0] = charset[(data[0] >> 2) & 0x3F];
    output[1] = charset[((data[0] & 0x03) << 4) | ((data[1] >> 4) & 0x0F)];
    output[2] = charset[((data[1] & 0x0F) << 2) | ((data[2] >> 6) & 0x03)];
    output[3] = charset[data[2] & 0x3F];
    output[4] = charset[(data[3] >> 2) & 0x3F];
    output[5] = charset[((data[3] & 0x03) << 4) | ((data[4] >> 4) & 0x0F)];
    output[6] = charset[((data[4] & 0x0F) << 2) | ((data[5] >> 6) & 0x03)];
    output[7] = charset[data[5] & 0x3F];
    output[8] = charset[(data[6] >> 2) & 0x3F];
    output[9] = charset[((data[6] & 0x03) << 4) | ((data[7] >> 4) & 0x0F)];
    output[10] = charset[(data[7] & 0x0F) << 2];
    output[11] = '=';
    output[12] = '=';
    output[13] = '\0';
}

static void DrawText(SDL_Renderer* renderer, const char* text,
			      TTF_Font* font, int x, int y, SDL_Color color)
{
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};

    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}


static int GetAccountId(char* str)
{
    uint64_t account_id;
    int numb = 0;
    int user_id;
    int err;

    if((err=sceUserServiceGetForegroundUser(&user_id))) {
        printf("sceUserServiceGetForegroundUser: %x", err);
        return -1;
    }

    for(int i=1, uid=0; i<=16; i++) {
        if((err=SCE_REGMGR_ENT_GET_n(i, USER_01_16_user_id, &uid))) {
            printf("SCE_REGMGR: unable to get USER_01_16_user_id(%d) (0x%x)\n",
                   i, err);
            return -1;
        } else if(uid == user_id) {
            numb = i;
            break;
        }
    }

    if(!numb) {
        printf("Unable to find the account id of the currently logged in user\n");
        return -1;
    }

    if((err=SCE_REGMGR_ENT_GET_n(numb, USER_01_16_account_id, &account_id))) {
        printf("SCE_REGMGR: unable to get USER_01_16_account_id(%d) (0x%x)\n",
               numb, err);
        return -1;
    }

    Base64Encode(account_id, str);

    return 0;
}


int PairUI_Main(SDL_Renderer* renderer, TTF_Font* font)
{
    SDL_Color color = {240, 240, 240, 255};
    uint32_t pin = 0;
    SDL_Event event;
    int pair_status;
    int pair_error;
    char buf[255];
    int idle = 0;
    char id[65];
    int xoff;
    int yoff;
    int err;
    time_t t;

    if(GetAccountId(id)) {
        return -1;
    }
    if(RemoteplayInit()) {
        return -1;
    }

    sceRemoteplayNotifyPinCodeError(1);
    if((err=sceRemoteplayGeneratePinCode(&pin))) {
        printf("sceRemoteplayGeneratePinCode: %x", err);
        return -1;
    }

    t = time(0) + 300;
    while(1) {
        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
		case SDLK_BACKSPACE:
                    return 0;
                }
            } else if(event.type == SDL_CONTROLLERBUTTONDOWN) {
		switch(event.cbutton.button) {
                case SDL_CONTROLLER_BUTTON_B:
                    return 0;
		}
	    }
	}

        if(idle) {
            continue;
        }

        SDL_SetRenderDrawColor(renderer, 0x05, 0x0d, 0x1c, 0xff);
        SDL_RenderClear(renderer);

        xoff = (int)TTF_FontHeight(font);
        yoff = (int)TTF_FontHeight(font);

        DrawText(renderer, "Link Device", font, xoff, yoff, color);

        yoff += (int)TTF_FontHeight(font) * 8;
        xoff += (int)TTF_FontHeight(font) * 10;

        if(t-time(0) < 0) {
            idle = 1;
            DrawText(renderer, "        Error: timeout", font, xoff, yoff, color);
        } else if((err=sceRemoteplayConfirmDeviceRegist(&pair_status, &pair_error))) {
            idle = 1;
            sprintf(buf, "        Error: 0x%x", err);
            DrawText(renderer, buf, font, xoff, yoff, color);

        } else if(pair_status == 2) {
            idle = 1;
            DrawText(renderer, "        Pairing complete",
                     font, xoff, yoff, color);

        } else if(pair_status == 3) {
            idle = 1;
            if(pair_error == 0x80FC1047) {
                DrawText(renderer, "        Error: incorrect PIN",
                         font, xoff, yoff, color);
            } else if(pair_error == 0x80FC1040) {
                DrawText(renderer, "        Error: incorrect Account Id",
                         font, xoff, yoff, color);
            } else {
                sprintf(buf, "        Error: 0x%x", pair_error);
                DrawText(renderer, buf, font, xoff, yoff, color);
            }
        } else {
            sprintf(buf, "Accound Id    : %s", id);
            DrawText(renderer, buf, font, xoff, yoff, color);

            yoff += (int)TTF_FontHeight(font);
            sprintf(buf, "PIN           : %04d %04d", pin / 10000, pin % 10000);
            DrawText(renderer, buf, font, xoff, yoff, color);

            yoff += (int)TTF_FontHeight(font);
            sprintf(buf, "Remaining time: %ld seconds", t-time(0));
            DrawText(renderer, buf, font, xoff, yoff, color);
        }

        SDL_RenderPresent(renderer);
    }

    return 0;
}


/* Local Variables: */
/* tab-width: 8 */
/* c-basic-offset: 4 */
/* End: */

