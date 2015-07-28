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
    printf("%d", XpmWriteFileFromImage(dis, "/home/victor/dump.xpm", img, NULL, NULL));
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
    XTestFakeButtonEvent(dis, button, True, 0);
    XTestFakeButtonEvent(dis, button, False, 0);
    XSync(dis,0);
}

typedef struct pixel {
    unsigned long red;
    unsigned long green;
    unsigned long blue;
} pixel;

pixel getColors(XImage* img, int x, int y) {
    unsigned long px = XGetPixel(img, x, y);
    pixel pxout;
    pxout.red = (px & img->red_mask) >> (img->depth / 3 * 2);
    pxout.green = (px & img->green_mask) >> (img->depth / 3);
    pxout.blue = px & img->blue_mask;
    return pxout;
}

int percent_to_pixel_threshold(XImage* img, int percents) {
    int signifigant_count = 0;
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            if (XGetPixel(img, x, y) > 0) signifigant_count++;
        }
    }
    return (signifigant_count * percents) / 100;
}

Bool check_for_image(XImage* haystack, int x, int y, XImage* needle, int treshold) {
    int fail_count = 0;
    for (int ty = 0; ty < needle->height; ty++) {
        for (int tx = 0; tx < needle->width; tx++) {
            unsigned long tpx = XGetPixel(needle, tx, ty);
            if (tpx > 0 && tpx != XGetPixel(haystack, tx + x, ty + y)) {
                fail_count++;
                if (fail_count >= treshold)
                    return False;
            }
        }
    }
    return True;
}

void image_loop(Display* dis) {
    XImage* img;
    XImage* tri;
    XImage* trib;
    XpmCreateImageFromData(dis, arrow_small, &trib, NULL, NULL);
    XpmCreateImageFromData(dis, arrow_big, &tri, NULL, NULL);
    int tri_tresh = percent_to_pixel_threshold(tri, 10);
    int trib_tresh = percent_to_pixel_threshold(trib, 10);
    img = capture_screen(dis);
    
    for (int iy = 0; iy < img->height - tri->height ; iy++) {
        for (int ix = 0; ix < img->width - tri->width ; ix++) {
            
            if (check_for_image(img, ix, iy, tri, tri_tresh)) {
                printf("s %d %d\n", ix, iy);
                click(dis, 1, ix + 5, iy + 5);
            } else if (check_for_image(img, ix, iy, trib, trib_tresh)) {
                printf("b %d %d\n", ix, iy);
                click(dis, 1, ix + 5, iy + 5);
            }
            
        }
    }
    
    XFree(img);
    XFree(tri);
    XFree(trib);
}

void key_loop(Display* dis) {
    unsigned long f9 = XKeysymToKeycode(dis, XK_F9);
    setup_listen_key(dis, f9, ControlMask);
    XEvent ev;
    while(True) {
        XNextEvent(dis, &ev);
        if (ev.type == KeyPress) {
            if (ev.xkey.keycode == f9) {
                image_loop(dis);
            }
        }
    }
}

int main(int argc, char** argv) {
    Display* dis = XOpenDisplay(0);
    image_loop(dis);
    XFree(dis);
    return (EXIT_SUCCESS);
}

