/* TinyWM is written by Nick Welch <mack@incise.org>, 2005.
 *
 * This software is in the public domain
 * and is provided AS IS, with NO WARRANTY. */

#include <X11/Xlib.h>
#include <iostream>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main(int, char*[]) {
    Display * dpy;
    Window root;
    XWindowAttributes attr;
    XButtonEvent start;
    XEvent ev;

    if(!(dpy = XOpenDisplay(0x0))) return 1;

    root = DefaultRootWindow(dpy);

    std::cout << "main" << std::endl;

    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("F1")), Mod1Mask, root,
            True, GrabModeAsync, GrabModeAsync);
    XGrabButton(dpy, 1, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync,
            GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync,
            GrabModeAsync, None, None);

    for(;;)
    {
        XNextEvent(dpy, &ev);
        if(ev.type == KeyPress && ev.xkey.subwindow != None) {
          std::cout << "BUTTON PRESS" << std::endl;
          XRaiseWindow(dpy, ev.xkey.subwindow);
        } else if(ev.type == ButtonPress && ev.xbutton.subwindow != None) {
            XGrabPointer(
              dpy, ev.xbutton.subwindow, True,
              PointerMotionMask|ButtonReleaseMask, GrabModeAsync,
              GrabModeAsync, None, None, CurrentTime
            );

            XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
            start = ev.xbutton;
        } else if(ev.type == MotionNotify) {
            std::cout << "XMOTION NOTIFY" << std::endl;
            int xdiff, ydiff;
            while(XCheckTypedEvent(dpy, MotionNotify, &ev));
            std::cout << "XMOTION NOTIFY AFTER" << std::endl;
            xdiff = ev.xbutton.x_root - start.x_root;
            ydiff = ev.xbutton.y_root - start.y_root;
            XMoveResizeWindow(dpy, ev.xmotion.window,
                attr.x + (start.button==1 ? xdiff : 0),
                attr.y + (start.button==1 ? ydiff : 0),
                MAX(1, attr.width + (start.button==3 ? xdiff : 0)),
                MAX(1, attr.height + (start.button==3 ? ydiff : 0)));
        } else if(ev.type == ButtonRelease) {
          XUngrabPointer(dpy, CurrentTime);
        }
    }
}
