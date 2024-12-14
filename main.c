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

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "pairui.h"
#include "regmgr.h"

#define WINDOW_TITLE  "LinkDev"
#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1280


int SDL_main(int argc, char* args[])
{
    SDL_Renderer* renderer;
    SDL_Window* window;
    TTF_Font* font;
    int rp_enable;
    int err;

    printf("%s %s was compiled at %s %s\n",
           WINDOW_TITLE, VERSION_TAG, __DATE__, __TIME__);

    if((err=SCE_REGMGR_ENT_GET(REMOTEPLAY_rp_enable, &rp_enable))) {
        printf("SCE_REGMGR: unable to get REMOTEPLAY_rp_enable (0x%x)\n", err);
        return -1;
    } else if (rp_enable != 1) {
        rp_enable = 1;
        if((err=SCE_REGMGR_ENT_SET(REMOTEPLAY_rp_enable, &rp_enable))) {
            printf("SCE_REGMGR: unable to set REMOTEPLAY_rp_enable (0x%x)\n", err);
            return -1;
        }
    }

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("SDL_Init: %s\n", SDL_GetError());
	return -1;
    }

    if(!(window=SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED,
				 SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
				 SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN))) {
        printf("SDL_CreateWindow: %s\n", SDL_GetError());
	return -1;
    }

    if(!(renderer=SDL_CreateRenderer(window, -1, (SDL_RENDERER_PRESENTVSYNC |
						  SDL_RENDERER_SOFTWARE)))) {
        printf("SDL_CreateRenderer: %s\n", SDL_GetError());
	return -1;
    }

    SDL_GameControllerOpen(0);

    if(TTF_Init() < 0) {
        printf("TTF_Init: %s\n", TTF_GetError());
	return -1;
    }

    if(!(font=TTF_OpenFont("/preinst/common/font/n023055ms.ttf", 84))) {
        printf("TTF_OpenFontRW: %s\n", TTF_GetError());
        return 1;
    }

    err = PairUI_Main(renderer, font);

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return err;
}


/* Local Variables: */
/* tab-width: 8 */
/* c-basic-offset: 4 */
/* End: */
