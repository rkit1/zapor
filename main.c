#define XUTIL_DEFINE_FUNCTIONS
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <X11/xpm.h>
#include <string.h>
#include "Arrows.h"
#include <time.h>

/*
void dump_img(XImage* img) {
    bmpfile_t* bmp = bmp_create(img->width, img->height, img->depth);
    rgb_pixel_t px = {0, 0, 0, 0};
    
    for (int a = 0; a < img->width * img->height; a++) {
        unsigned long spx = XGetPixel(img, a % img->width, a / img->width);
        px.red = (spx & img->red_mask) >> (img->depth / 3 * 2);
        px.green = (spx & img->green_mask) >> (img->depth / 3);
        px.blue = spx & img->blue_mask;
        bmp_set_pixel(bmp, a % img->width, a / img->width, px);
    }
  
    bmp_save(bmp, "/home/victor/dump.bmp");
    bmp_destroy(bmp);
}
*/

void dump_xpm(Display* dis, XImage* img) {
    printf("%d", XpmWriteFileFromImage(dis, "/home/tori/dump.xpm", img, NULL, NULL));
}

XImage* capture_screen(Display* dis) {
    Window wind = XRootWindow(dis, XDefaultScreen(dis));
    XWindowAttributes attrs;
    XGetWindowAttributes(dis, wind, &attrs);
    XImage* img = XGetImage(dis, wind, 0, 0, attrs.width, attrs.height
            , AllPlanes, ZPixmap);
    return img;
}

XImage* capture_screen_region(Display* dis, int x, int y, int x1, int y1) {
    Window wind = XRootWindow(dis, XDefaultScreen(dis));
    XImage* img = XGetImage(dis, wind, x, y, x1 - x, y1 - y, AllPlanes, ZPixmap);
    return img;
}

void get_pointer(Display* dis, int* x, int* y) {
    Window root_return, child_return;
    int win_x_return, win_y_return;
    unsigned int mask_return;
    XQueryPointer(dis, XRootWindow(dis, XDefaultScreen(dis)), 
            &root_return, &child_return, x, y, 
            &win_x_return, &win_y_return, &mask_return);
}

void setup_listen_key(Display* dis, int key, unsigned long keyModifier) {
    Window wind = XRootWindow(dis, XDefaultScreen(dis));
    
    XGrabKey(dis, key, keyModifier|Mod2Mask,
            wind, True,
            GrabModeAsync, GrabModeAsync);		
    //scrolllock
    XGrabKey(dis, key, keyModifier|Mod5Mask,
            wind, True,
            GrabModeAsync, GrabModeAsync);	
    //capslock
    XGrabKey(dis, key, keyModifier|LockMask,
            wind, True,
            GrabModeAsync, GrabModeAsync);

    //capslock+numlock
    XGrabKey(dis, key, keyModifier|LockMask|Mod2Mask,
            wind, True,
            GrabModeAsync, GrabModeAsync);

    //capslock+scrolllock
    XGrabKey(dis, key, keyModifier|LockMask|Mod5Mask,
            wind, True,
            GrabModeAsync, GrabModeAsync);						

    //capslock+numlock+scrolllock
    XGrabKey(dis, key, keyModifier|Mod2Mask|Mod5Mask|LockMask,
            wind, True,
            GrabModeAsync, GrabModeAsync);						

    //numlock+scrollLock
    XGrabKey(dis, key, keyModifier|Mod2Mask|Mod5Mask,
            wind, True,
            GrabModeAsync, GrabModeAsync);
    
    XSelectInput(dis, wind, KeyPressMask);
}

void capture_key_region(Display* dis) {
    unsigned int f12 = XKeysymToKeycode(dis, XK_F12);
    unsigned int f11 = XKeysymToKeycode(dis, XK_F11);
    unsigned int f10 = XKeysymToKeycode(dis, XK_F10);
    unsigned int f9 = XKeysymToKeycode(dis, XK_F9);
    setup_listen_key(dis, f9, 0);
    setup_listen_key(dis, f10, 0);
    setup_listen_key(dis, f11, 0);
    setup_listen_key(dis, f12, 0);
    XEvent ev;
    int x = 0, y = 0, x1 = 1, y1 = 1;
    while(True) {
        XNextEvent(dis, &ev);
        
        if (ev.type == KeyPress) {
            printf("%x\n", ev.xkey.keycode);
            if (ev.xkey.keycode == f10) break;
            if (ev.xkey.keycode == f11) { 
                get_pointer(dis, &x, &y);
                printf("x:%d, y:%d\n", x, y);
            }
            if (ev.xkey.keycode == f12) {
                get_pointer(dis, &x1, &y1);
                printf("x1:%d, y1:%d\n", x1, y1);
            }
        }
    }
    dump_xpm(dis, capture_screen_region(dis, x, y, x1, y1));    
}

void click(Display* dis, int button, int x, int y) {
    XTestFakeMotionEvent(dis, -1, x, y, 0);
    XSync(dis,0);
    usleep(30000);
    XTestFakeButtonEvent(dis, button, True, 0);
    XSync(dis,0);
    usleep(30000);
    XTestFakeButtonEvent(dis, button, False, 0);
    XSync(dis,0);
    usleep(30000);
}

Bool check_for_image(XImage* haystack, int x, int y, XImage* needle) {
    for (int ty = 0; ty < needle->height; ty++) {
        for (int tx = 0; tx < needle->width; tx++) {
            unsigned long tpx = XGetPixel(needle, tx, ty);
            if (tpx > 0) {
                if (tpx != XGetPixel(haystack, tx + x, ty + y)) {
                    return False;
                }
            }
        }
    }
    return True;
}

int image_loop(Display* dis) {
    XImage* img;
    XImage* imgcur;
    XImage* tri;
    XImage* trib;
    Bool exit_error = False;
    XpmCreateImageFromData(dis, arrow_small, &tri, NULL, NULL);
    XpmCreateImageFromData(dis, arrow_big, &trib, NULL, NULL);
    img = capture_screen(dis);
    
    for (int iy = 0; iy < img->height - tri->height ; iy++) {
        for (int ix = 0; ix < img->width - tri->width ; ix++) {
            
            if (check_for_image(img, ix, iy, tri)) {
                imgcur = capture_screen_region(dis, ix, iy, ix + trib->width, iy + trib->height);
		if (check_for_image(imgcur, 0, 0, tri)) {
                    printf("s %d %d\n", ix, iy);
                    click(dis, 1, ix + 5, iy + 5);
		} else if(check_for_image(imgcur, 0, 0, trib)) {
                    printf("b %d %d\n", ix, iy);
                    click(dis, 1, ix + 5, iy + 5);
		} else
                    exit_error = True;
                XFree(imgcur);
            }
            
        }
    }
    
    XFree(img);
    XFree(tri);
    XFree(trib);
    if (exit_error) return -1;
    return 0;
}

int main(int argc, char** argv) {
    Display* dis = XOpenDisplay(0);
    image_loop(dis);
    return (EXIT_SUCCESS);
    unsigned long f9 = XKeysymToKeycode(dis, XK_9);
    printf("%d\n", f9);
    setup_listen_key(dis, f9, 0);
    XEvent ev;
    while(True) {
        XNextEvent(dis, &ev);
	printf("xevent\n");
        if (ev.type == KeyPress) {
	    printf("kp\n");
            if (ev.xkey.keycode == f9) {
                if (image_loop(dis) == -1) 
                    printf("exited with error\n");
            }
        }
    }
    XFree(dis);
    return (EXIT_SUCCESS);
}

