/**
 * 字体解析器窗口及主入口。
 * @date 2026-02-05
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#define SDL_MAIN_USE_CALLBACKS 1
#include "type.h"

#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

/**
 * 窗体指针。
 */
static SDL_Window* window=NULL;

/**
 * 渲染器指针。
 */
static SDL_Renderer* renderer=NULL;

/**
 * 网格边长。
 */
#define GRID_BASE 64

/**
 * 窗体参考大小。
 */
#define WINDOW_HEIGHT_MIN 450
#define WINDOW_HEIGHT_MAX 900

/**
 * 图像逻辑基本参数。
 */
#define BORDER_LINE 9
#define GRID_LINE 1
#define GRID_WIDTH 9

/**
 * 画布设计大小。
 */
#define LOGICAL_WIDTH (BORDER_LINE*3+(GRID_BASE*GRID_WIDTH+(GRID_BASE-1)*GRID_LINE)*2)
#define LOGICAL_HEIGHT (BORDER_LINE*2+(GRID_BASE*GRID_WIDTH+(GRID_BASE-1)*GRID_LINE))

/**
 * 应用初始化生命周期函数。
 * 
 * @param appstate 可选的应用空间指针。
 * @param argc     参数数目。
 * @param argv     参数数组。
 * 
 * @return 应用运行结果。
 */
SDL_AppResult SDL_AppInit(void **appstate,int argc,char* argv[])
{
    SDL_SetAppMetadata("Font Parser","0.0.1","aos.research.font_parser");

    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Couldn't initialize SDL: %s",SDL_GetError());
        return SDL_APP_FAILURE;
    }

    int width=LOGICAL_WIDTH;
    int height=LOGICAL_HEIGHT;

    while(height<WINDOW_HEIGHT_MIN)
    {
        height=height+LOGICAL_HEIGHT/2;
        width=width+LOGICAL_WIDTH/2;
    }

    while(height>WINDOW_HEIGHT_MAX)
    {
        height=height-LOGICAL_HEIGHT/2;
        width=width-LOGICAL_WIDTH/2;
    }

    if(!SDL_CreateWindowAndRenderer("Font Parser",width,height,SDL_WINDOW_RESIZABLE,
        &window,&renderer))
    {
        SDL_Log("Couldn't create window/renderer: %s",SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer,LOGICAL_WIDTH,LOGICAL_HEIGHT,
        SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return SDL_APP_CONTINUE;
}

/**
 * 应用事件生命周期函数。
 * 
 * @param appstate 可选的应用空间指针。
 * @param enevt    应用事件。
 * 
 * @return 应用运行结果。
 */
SDL_AppResult SDL_AppEvent(void *appstate,SDL_Event *event)
{
    if(event->type==SDL_EVENT_QUIT)
    {
        /*成功代表要退出应用了*/
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

/**
 * 设置网格颜色。需要已知前提才能使用该函数。
 * 
 * @param x0 x=0在窗口的逻辑坐标。
 * @param y0 y=0在窗口的逻辑坐标。
 * @param x  被绘制网格的x坐标。
 * @param y  被绘制网格的y坐标。
 * @param r  绘制红色值。
 * @param g  绘制绿色值。
 * @param b  绘制蓝色值。
 * @param a  绘制不透明值。
 * 
 * @return 应用运行结果。
 */
static void set_grid_color(int x0,int y0,int x,int y,Uint8 r,Uint8 g,Uint8 b,Uint8 a)
{
    SDL_FRect rect={x0+(GRID_WIDTH+GRID_LINE)*x,y0+(GRID_WIDTH+GRID_LINE)*y,GRID_WIDTH,GRID_WIDTH};
    SDL_SetRenderDrawColor(renderer,r,g,b,a);
    SDL_RenderFillRect(renderer,&rect);
}

/**
 * 应用帧迭代生命周期函数。
 * 
 * @param appstate 可选的应用空间指针。
 * 
 * @return 应用运行结果。
 */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    SDL_FRect rects[16];

    /*清屏本质是完全填充颜色，所以要设置色彩*/
    SDL_SetRenderDrawColor(renderer,0,0,0,SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer,0xB2,0xBB,0xBE,SDL_ALPHA_OPAQUE);
    SDL_FRect rect={0,0,LOGICAL_WIDTH,LOGICAL_HEIGHT};
    SDL_RenderFillRect(renderer,&rect);

    int left_x=BORDER_LINE;
    int left_y=BORDER_LINE;

    int right_x=BORDER_LINE*2+GRID_BASE*GRID_WIDTH+(GRID_BASE-1)*GRID_LINE;
    int right_y=BORDER_LINE;

    for(int i=0;i<GRID_BASE;i++)
    {
        for(int j=0;j<GRID_BASE;j++)
        {
            set_grid_color(left_x,left_y,i,j,0xFF,0xFF,0xFF,0xFF);
        }
    }

    for(int i=0;i<GRID_BASE;i++)
    {
        for(int j=0;j<GRID_BASE;j++)
        {
            set_grid_color(right_x,right_y,i,j,0xFF,0xFF,0xFF,0xFF);
        }
    }

    /*提交渲染*/
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

/**
 * 应用退出生命周期函数。
 * 
 * @param appstate 可选的应用空间指针。
 * @param result   应用运行结果。
 * 
 * @return 无返回值。
 */
void SDL_AppQuit(void *appstate,SDL_AppResult result)
{
    /*由SDL负责清除窗口和渲染器*/
}