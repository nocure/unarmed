
#include <string.h>
#include "platform/YL_SYS.h"
#include "platform/tq_bsp.h"
#include <../hmi/osd.h>

#include <minigui/common.h>
#include "../mgui/include_priv/newgal.h"
//#include "../sysvideo.h"
//#include "../pixels_c.h"


#include "2440emu_video.h"

#define RGB32_VID_DRIVER_NAME "RGB32"

/* Initialization/Query functions */
static int RGB32_VideoInit(_THIS, GAL_PixelFormat *vformat);
static GAL_Rect **RGB32_ListModes(_THIS, GAL_PixelFormat *format, Uint32 flags);
static GAL_Surface *RGB32_SetVideoMode(_THIS, GAL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int RGB32_SetColors(_THIS, int firstcolor, int ncolors, GAL_Color *colors);
static void RGB32_VideoQuit(_THIS);

/* Hardware surface functions */
static int RGB32_AllocHWSurface(_THIS, GAL_Surface *surface);
static void RGB32_FreeHWSurface(_THIS, GAL_Surface *surface);


static const struct GAL_PrivateVideoData PrivateVideoData[2]={
		// width, height, bpp, bufferaddr
		{VSCR0_XSIZE_TFT,VSCR0_YSIZE_TFT,32,(unsigned int *)0},
		{VSCR1_XSIZE_TFT,VSCR1_YSIZE_TFT,32,(unsigned int *)(0+(VSCR0_YSIZE_TFT*VSCR0_XSIZE_TFT*4))}
};

static int RGB32_Available(void)
{
    return(2);
}

static void RGB32_DeleteDevice(GAL_VideoDevice *device)
{
    free(device->hidden);
    free(device);
}

static GAL_VideoDevice *RGB32_CreateDevice(int devindex)
{
    GAL_VideoDevice *device;

    if (devindex>=RGB32_Available()) return 0;
    /* Initialize all variables that we clean on shutdown */
    device = (GAL_VideoDevice *)malloc(sizeof(GAL_VideoDevice));
    if ( device ) {
        memset(device, 0, (sizeof *device));
        device->hidden = (struct GAL_PrivateVideoData *)
                malloc((sizeof *device->hidden));
    }
    if ( (device == NULL) || (device->hidden == NULL) ) {
        GAL_OutOfMemory();
        if ( device ) {
            free(device);
        }
        return(0);
    }
//    memset(device->hidden, 0, (sizeof *device->hidden));
    memcpy(device->hidden,&PrivateVideoData[devindex],(sizeof *device->hidden));

    /* Set the function pointers */
    device->VideoInit = RGB32_VideoInit;
    device->ListModes = RGB32_ListModes;
    device->SetVideoMode = RGB32_SetVideoMode;
    device->CreateYUVOverlay = NULL;
    device->SetColors = RGB32_SetColors;
    device->VideoQuit = RGB32_VideoQuit;
#ifdef _LITE_VERSION
    device->RequestHWSurface = NULL;
#endif
    device->AllocHWSurface = RGB32_AllocHWSurface;
    device->CheckHWBlit = NULL;
    device->FillHWRect = NULL;
    device->SetHWColorKey = NULL;
    device->SetHWAlpha = NULL;
    device->FreeHWSurface = RGB32_FreeHWSurface;

    device->free = RGB32_DeleteDevice;

    return device;
}

VideoBootStrap RGB32_bootstrap = {
	RGB32_VID_DRIVER_NAME, "RGB32 video driver",
    RGB32_Available, RGB32_CreateDevice
};


static int RGB32_VideoInit(_THIS, GAL_PixelFormat *vformat)
{
    dbgout ("NEWGAL>RGB32: Calling init method!\n");

    /* Determine the screen depth (use default 32-bit depth) */
    /* we change this during the GAL_SetVideoMode implementation... */
//    this->hidden->w = SCR_XSIZE_TFT;
//    this->hidden->h = SCR_YSIZE_TFT;
//    this->hidden->bpp = 32;		/* TODO: no hard coded */
//    this->hidden->fb = LCD_BUFFER;

    vformat->BitsPerPixel = 32;
    switch (vformat->BitsPerPixel) {
		case 8:
			vformat->BytesPerPixel = 1;
            break;
		case 12:
			vformat->BitsPerPixel = 16;
			vformat->BytesPerPixel = 2;
			vformat->Rmask = 0x00000F00;
			vformat->Gmask = 0x000000F0;
			vformat->Bmask = 0x0000000F;
			break;
		case 16:
			vformat->BytesPerPixel = 2;
			vformat->Rmask = 0x0000F800;
			vformat->Gmask = 0x000007E0;
			vformat->Bmask = 0x0000001F;
			break;
		case 32:
			vformat->BytesPerPixel = 4;
			vformat->Rmask = 0x00FF0000;
			vformat->Gmask = 0x0000FF00;
			vformat->Bmask = 0x000000FF;
			break;
		default:
			GAL_SetError ("NEWGAL>2440 EMU: Not supported depth: %d, "
					"please try to use Shadow NEWGAL engine with targetname.\n", vformat->BitsPerPixel);
			return -1;
    }
    /* We're done! */
    return(0);
}

static GAL_Rect **RGB32_ListModes(_THIS, GAL_PixelFormat *format, Uint32 flags)
{
    return (GAL_Rect**) -1;
}

static GAL_Surface *RGB32_SetVideoMode(_THIS, GAL_Surface *current,
                int width, int height, int bpp, Uint32 flags)
{
    int pitch;
    void *p;

    pitch = width * ((bpp + 7) / 8);
    pitch = (pitch + 3) & ~3;

    p=tq_lcdGetBufAddr(0,0);

    if (p) {

        p+=(DWORD)this->hidden->fb;
//    yl_lcdcClearScr( (DWORD)0x00000000);

		/* Set up the new mode framebuffer */
		current->flags = flags & GAL_FULLSCREEN;
		this->hidden->w = current->w = width;
		this->hidden->h = current->h = height;
		current->pitch=pitch;
		this->hidden->fb = current->pixels = p;
    	memset (p, 0, pitch * height);
    }
    else {
		current->flags = 0;
		this->hidden->w = current->w = 0;
		this->hidden->h = current->h = 0;
		current->pitch=0;
		this->hidden->fb = current->pixels = 0;
    }

    /* We're done */
    return(current);
}

/* We don't actually allow hardware surfaces other than the main one */
static int RGB32_AllocHWSurface(_THIS, GAL_Surface *surface)
{
    return(-1);
}
static void RGB32_FreeHWSurface(_THIS, GAL_Surface *surface)
{
    surface->pixels = NULL;
}

static int RGB32_SetColors(_THIS, int firstcolor, int ncolors, GAL_Color *colors)
{
    /* do nothing of note. */
    return(1);
}

/* Note:  If we are terminated, this could be called in the middle of
   another video routine -- notably UpdateRects.
*/
static void RGB32_VideoQuit(_THIS)
{
    if (this->screen->pixels != NULL)
    {
        this->screen->pixels = NULL;
    }
}
