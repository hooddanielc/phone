#include <iostream>
#include <exception>
#include <memory>
#include <ostream>
#include <sstream>
#include <mutex>
#include <algorithm>
#include <unordered_map>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace phone {


  // SOME STRANGS
  std::string XRequestCodeToString(unsigned char request_code) {
    static const char* const X_REQUEST_CODE_NAMES[] = {
        "",
        "CreateWindow",
        "ChangeWindowAttributes",
        "GetWindowAttributes",
        "DestroyWindow",
        "DestroySubwindows",
        "ChangeSaveSet",
        "ReparentWindow",
        "MapWindow",
        "MapSubwindows",
        "UnmapWindow",
        "UnmapSubwindows",
        "ConfigureWindow",
        "CirculateWindow",
        "GetGeometry",
        "QueryTree",
        "InternAtom",
        "GetAtomName",
        "ChangeProperty",
        "DeleteProperty",
        "GetProperty",
        "ListProperties",
        "SetSelectionOwner",
        "GetSelectionOwner",
        "ConvertSelection",
        "SendEvent",
        "GrabPointer",
        "UngrabPointer",
        "GrabButton",
        "UngrabButton",
        "ChangeActivePointerGrab",
        "GrabKeyboard",
        "UngrabKeyboard",
        "GrabKey",
        "UngrabKey",
        "AllowEvents",
        "GrabServer",
        "UngrabServer",
        "QueryPointer",
        "GetMotionEvents",
        "TranslateCoords",
        "WarpPointer",
        "SetInputFocus",
        "GetInputFocus",
        "QueryKeymap",
        "OpenFont",
        "CloseFont",
        "QueryFont",
        "QueryTextExtents",
        "ListFonts",
        "ListFontsWithInfo",
        "SetFontPath",
        "GetFontPath",
        "CreatePixmap",
        "FreePixmap",
        "CreateGC",
        "ChangeGC",
        "CopyGC",
        "SetDashes",
        "SetClipRectangles",
        "FreeGC",
        "ClearArea",
        "CopyArea",
        "CopyPlane",
        "PolyPoint",
        "PolyLine",
        "PolySegment",
        "PolyRectangle",
        "PolyArc",
        "FillPoly",
        "PolyFillRectangle",
        "PolyFillArc",
        "PutImage",
        "GetImage",
        "PolyText8",
        "PolyText16",
        "ImageText8",
        "ImageText16",
        "CreateColormap",
        "FreeColormap",
        "CopyColormapAndFree",
        "InstallColormap",
        "UninstallColormap",
        "ListInstalledColormaps",
        "AllocColor",
        "AllocNamedColor",
        "AllocColorCells",
        "AllocColorPlanes",
        "FreeColors",
        "StoreColors",
        "StoreNamedColor",
        "QueryColors",
        "LookupColor",
        "CreateCursor",
        "CreateGlyphCursor",
        "FreeCursor",
        "RecolorCursor",
        "QueryBestSize",
        "QueryExtension",
        "ListExtensions",
        "ChangeKeyboardMapping",
        "GetKeyboardMapping",
        "ChangeKeyboardControl",
        "GetKeyboardControl",
        "Bell",
        "ChangePointerControl",
        "GetPointerControl",
        "SetScreenSaver",
        "GetScreenSaver",
        "ChangeHosts",
        "ListHosts",
        "SetAccessControl",
        "SetCloseDownMode",
        "KillClient",
        "RotateProperties",
        "ForceScreenSaver",
        "SetPointerMapping",
        "GetPointerMapping",
        "SetModifierMapping",
        "GetModifierMapping",
        "NoOperation",
    };
    return X_REQUEST_CODE_NAMES[request_code];
  }



















  // Whether an existing window manager has been detected
  bool wm_detected = false;

  // Represents a 2D size.
  template <typename T>
  struct Size {
    T width, height;

    Size() = default;
    Size(T w, T h)
        : width(w), height(h) {
    }

    std::string to_string() const;
  };

  template <typename T>
  std::string Size<T>::to_string() const {
    std::ostringstream out;
    out << width << 'x' << height;
    return out.str();
  }

  template <typename T>
  std::ostream& operator << (std::ostream &out, const Size<T> &size) {
    return out << size.to_string();
  }

  // Represents a 2D position.
  template <typename T>
  struct Position {
    T x, y;

    Position() = default;
    Position(T _x, T _y)
        : x(_x), y(_y) {
    }

    std::string to_string() const;
  };

  template <typename T>
  std::string Position<T>::to_string() const {
    std::ostringstream out;
    out << "(" << x << ", " << y << ")";
    return out.str();
  }

  // Represents a 2D vector.
  template <typename T>
  struct Vector2D {
    T x, y;

    Vector2D() = default;
    Vector2D(T _x, T _y)
        : x(_x), y(_y) {
    }

    std::string to_string() const;
  };

  template <typename T>
  std::string Vector2D<T>::to_string() const {
    std::ostringstream out;
    out << "(" << x << ", " << y << ")";
    return out.str();
  }


  // Position operators
  template <typename T>
  Vector2D<T> operator - (const Position<T>& a, const Position<T>& b) {
    return Vector2D<T>(a.x - b.x, a.y - b.y);
  }

  template <typename T>
  Position<T> operator + (const Position<T>& a, const Vector2D<T> &v) {
    return Position<T>(a.x + v.x, a.y + v.y);
  }

  template <typename T>
  Position<T> operator + (const Vector2D<T> &v, const Position<T>& a) {
    return Position<T>(a.x + v.x, a.y + v.y);
  }

  template <typename T>
  Position<T> operator - (const Position<T>& a, const Vector2D<T> &v) {
    return Position<T>(a.x - v.x, a.y - v.y);
  }

  // Size operators.
  template <typename T>
  Vector2D<T> operator - (const Size<T>& a, const Size<T>& b) {
    return Vector2D<T>(a.width - b.width, a.height - b.height);
  }
  template <typename T>
  Size<T> operator + (const Size<T>& a, const Vector2D<T> &v) {
    return Size<T>(a.width + v.x, a.height + v.y);
  }

  template <typename T>
  Size<T> operator + (const Vector2D<T> &v, const Size<T>& a) {
    return Size<T>(a.width + v.x, a.height + v.y);
  }

  template <typename T>
  Size<T> operator - (const Size<T>& a, const Vector2D<T> &v) {
    return Size<T>(a.width - v.x, a.height - v.y);
  }

  class window_manager_t {
      // Atom constants.
      const Atom WM_PROTOCOLS;
      const Atom WM_DELETE_WINDOW;
      // A mutex for protecting  wm_detected
      std::mutex wm_detected_mutex;
      // Maps top-level windows to their frame windows.
      std::unordered_map<Window, Window> clients;

      // The cursor position at the start of a window move/resize.
      Position<int> drag_start_pos_;
      // The position of the affected window at the start of a window
      // move/resize.
      Position<int> drag_start_frame_pos_;
      // The size of the affected window at the start of a window move/resize.
      Size<int> drag_start_frame_size_;

      // unframes a client window.
      void unframe(Window w) {
        std::cout << "The clients count is " << clients.count(w) << std::endl;

        // We reverse the steps taken in Frame().
        const Window frame = clients[w];
        // 1. Unmap frame.
        XUnmapWindow(display, frame);
        // 2. Reparent client window.
        XReparentWindow(
            display,
            w,
            root_window,
            0, 0);  // Offset of client window within root.
        // 3. Remove client window from save set, as it is now unrelated to us.
        XRemoveFromSaveSet(display, w);
        // 4. Destroy frame.
        XDestroyWindow(display, frame);
        // 5. Drop reference to frame handle.
        clients.erase(w);

        std::cout << "Unframed window " << w << " [" << frame << "]" << std::endl;
      }

      void frame(Window w) {
        std::cout << "FRAME " << std::endl;
        // Visual properties of the frame to create.
        const unsigned int BORDER_WIDTH = 6;
        const unsigned long BORDER_COLOR = 0xff0000;
        const unsigned long BG_COLOR = 0x0000ff;

        std::cout << "client.count(w) === " << clients.count(w) << std::endl;

        // 1. Retrieve attributes of window to frame.
        XWindowAttributes x_window_attrs;

        std::cout << "Check XGetWindowAttributes = " << XGetWindowAttributes(display, w, &x_window_attrs) << std::endl;

        // 2. Create frame.
        const Window frame = XCreateSimpleWindow(
            display,
            root_window,
            x_window_attrs.x,
            x_window_attrs.y,
            x_window_attrs.width,
            x_window_attrs.height,
            BORDER_WIDTH,
            BORDER_COLOR,
            BG_COLOR);
        // 3. Select events on frame.
        XSelectInput(
            display,
            frame,
            SubstructureRedirectMask | SubstructureNotifyMask);
        // 4. Add client to save set, so that it will be restored and kept alive if we
        // crash.
        XAddToSaveSet(display, w);
        // 5. Reparent client window.
        XReparentWindow(
            display,
            w,
            frame,
            0, 0);  // Offset of client window within frame.
        // 6. Map frame.
        XMapWindow(display, frame);
        // 7. Save frame handle.
        clients[w] = frame;
        // 8. Grab universal window management actions on client window.
        //   a. Move windows with alt + left button.
        std::cout << "Grab Btn" << std::endl;
        XGrabButton(
            display,
            Button1,
            Mod1Mask,
            w,
            false,
            ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
            GrabModeAsync,
            GrabModeAsync,
            None,
            None);
        //   b. Resize windows with alt + right button.
        XGrabButton(
            display,
            Button3,
            Mod1Mask,
            w,
            false,
            ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
            GrabModeAsync,
            GrabModeAsync,
            None,
            None);
        //   c. Kill windows with alt + f4.
        XGrabKey(
            display,
            XKeysymToKeycode(display, XK_F5),
            Mod1Mask,
            w,
            false,
            GrabModeAsync,
            GrabModeAsync);
        //   d. Switch windows with alt + tab.
        XGrabKey(
            display,
            XKeysymToKeycode(display, XK_Tab),
            Mod1Mask,
            w,
            false,
            GrabModeAsync,
            GrabModeAsync);

        std::cout << "Framed window " << w << " [" << frame << "]" << std::endl;
      }

    public:
      Display *display;
      const Window root_window;

      window_manager_t(const window_manager_t &) = delete;
      window_manager_t &operator=(const window_manager_t &) = delete;

      // constructor
      window_manager_t(Display *argdisplay) :
        display(argdisplay),
        root_window(DefaultRootWindow(argdisplay)),
        WM_PROTOCOLS(XInternAtom(argdisplay, "WM_PROTOCOLS", false)),
        WM_DELETE_WINDOW(XInternAtom(argdisplay, "WM_DELETE_WINDOW", false)) {}

      // destructor
      ~window_manager_t() {
        XCloseDisplay(display);
      }

      // ALl the event handlers in the world
      // Event handlers.
      void OnCreateNotify(const XCreateWindowEvent& e) {
        std::cout << "EVENT HANDLER OnCreateNotify" << std::endl;
      }

      void OnDestroyNotify(const XDestroyWindowEvent& e) {
        std::cout << "EVENT HANDLER OnDestroyNotify" << std::endl;
      }

      void OnReparentNotify(const XReparentEvent& e) {
        std::cout << "EVENT HANDLER OnReparentNotify" << std::endl;
      }

      void OnMapNotify(const XMapEvent& e) {
        std::cout << "EVENT HANDLER OnMapNotify" << std::endl;
      }

      void OnUnmapNotify(const XUnmapEvent& e) {
        // If the window is a client window we manage, unframe it upon UnmapNotify. We
        // need the check because other than a client window, we can receive an
        // UnmapNotify for
        //     - A frame we just destroyed ourselves.
        //     - A pre-existing and mapped top-level window we reparented.
        if (!clients.count(e.window)) {
          std::cout << "Ignore UnmapNotify for non-client window " << e.window << std::endl;
          return;
        }
        if (e.event == root_window) {
          std::cout << "Ignore UnmapNotify for reparented pre-existing window " << e.window << std::endl;
          return;
        }
        unframe(e.window);
        std::cout << "EVENT HANDLER OnUnmapNotify" << std::endl;
      }

      void OnConfigureNotify(const XConfigureEvent &e) {
        std::cout << "The X Value is : " << e.x << std::endl;
        std::cout << "The Y Value is : " << e.y << std::endl;
        XMoveWindow(display, e.window, e.x, e.y);
        std::cout << "EVENT HANDLER OnConfigureNotify" << std::endl;
      }

      void OnMapRequest(const XMapRequestEvent &e) {
        std::cout << "EVENT HANDLER OnMapRequest" << std::endl;

        // 1. Frame or re-frame window.
        frame(e.window);
        // 2. Actually map window.
        XMapWindow(display, e.window);
      }

      void OnConfigureRequest(const XConfigureRequestEvent &e) {
        XWindowChanges changes;
        changes.x = e.x;
        changes.y = e.y;
        changes.width = e.width;
        changes.height = e.height;
        changes.border_width = e.border_width;
        changes.sibling = e.above;
        changes.stack_mode = e.detail;

        if (clients.count(e.window)) {
          const Window frame = clients[e.window];
          XConfigureWindow(display, frame, e.value_mask, &changes);
          std::cout << "Resize [" << frame << "] to " << Size<int>(e.width, e.height) << std::endl;
        }

        XConfigureWindow(display, e.window, e.value_mask, &changes);
        std::cout << "Resize " << e.window << " to " << Size<int>(e.width, e.height) << std::endl;
        std::cout << "EVENT HANDLER OnConfigureRequest" << std::endl;
      }

      void OnButtonPress(const XButtonEvent& e) {

        std::cout << "clients.cout ok? " << clients.count(e.window) << std::endl;
        const Window frame = clients[e.window];

        // 1. Save initial cursor position.
        drag_start_pos_ = Position<int>(e.x_root, e.y_root);

        // 2. Save initial window info.
        Window returned_root;
        int x, y;
        unsigned width, height, border_width, depth;

        std::cout << "Check geometry ok? " <<
        XGetGeometry(
            display,
            frame,
            &returned_root,
            &x, &y,
            &width, &height,
            &border_width,
            &depth);
        drag_start_frame_pos_ = Position<int>(x, y);
        drag_start_frame_size_ = Size<int>(width, height);
        std::cout << "EVENT HANDLER OnButtonPress" << std::endl;
      }

      void OnButtonRelease(const XButtonEvent& e) {
        std::cout << "EVENT HANDLER OnButtonRelease" << std::endl;
      }

      void OnMotionNotify(const XMotionEvent& e) {
        std::cout << "Check clients count window? "<< clients.count(e.window) << std::endl;
        const Window frame = clients[e.window];
        const Position<int> drag_pos(e.x_root, e.y_root);
        const Vector2D<int> delta = drag_pos - drag_start_pos_;

        if (e.state & Button1Mask ) {
          std::cout << "Button1Mask" << std::endl;
          // alt + left button: Move window.
          const Position<int> dest_frame_pos = drag_start_frame_pos_ + delta;
          XMoveWindow(
              display,
              frame,
              dest_frame_pos.x, dest_frame_pos.y);
        } else if (e.state & Button3Mask) {
          std::cout << "Button 3 mask yo" << std::endl;
          // alt + right button: Resize window.
          // Window dimensions cannot be negative.
          const Vector2D<int> size_delta(
              std::max(delta.x, -drag_start_frame_size_.width),
              std::max(delta.y, -drag_start_frame_size_.height));
          const Size<int> dest_frame_size = drag_start_frame_size_ + size_delta;
          // 1. Resize frame.
          XResizeWindow(
              display,
              frame,
              dest_frame_size.width, dest_frame_size.height);
          // 2. Resize client window.
          XResizeWindow(
              display,
              e.window,
              dest_frame_size.width, dest_frame_size.height);
        }
        std::cout << "EVENT HANDLER OnMotionNotify" << std::endl;
      }

      void OnKeyPress(const XKeyEvent& e) {
        if ((e.state & Mod1Mask) && (e.keycode == XKeysymToKeycode(display, XK_F4))) {
          // alt + f4: Close window.
          //
          // There are two ways to tell an X window to close. The first is to send it
          // a message of type WM_PROTOCOLS and value WM_DELETE_WINDOW. If the client
          // has not explicitly marked itself as supporting this more civilized
          // behavior (using XSetWMProtocols()), we kill it with XKillClient().
          Atom *supported_protocols;
          int num_supported_protocols;
          if (
            XGetWMProtocols(
              display,
              e.window,
              &supported_protocols,
              &num_supported_protocols) &&
            (std::find(
              supported_protocols,
              supported_protocols + num_supported_protocols,
              WM_DELETE_WINDOW) != supported_protocols + num_supported_protocols)
          ) {
            std::cout << "Gracefully deleting window " << e.window << std::endl;
            // 1. Construct message.
            XEvent msg;
            memset(&msg, 0, sizeof(msg));
            msg.xclient.type = ClientMessage;
            msg.xclient.message_type = WM_PROTOCOLS;
            msg.xclient.window = e.window;
            msg.xclient.format = 32;
            msg.xclient.data.l[0] = WM_DELETE_WINDOW;
            // 2. Send message to window to be closed.
            std::cout << "SEND MESSAGE TO WINDOW CHECK? "<< XSendEvent(display, e.window, false, 0, &msg) << std::endl;
          } else {
            std::cout << "Killing window " << e.window << std::endl;
            XKillClient(display, e.window);
          }
        } else if ((e.state & Mod1Mask) && (e.keycode == XKeysymToKeycode(display, XK_Tab))) {
          // alt + tab: Switch window.
          // 1. Find next window.
          auto i = clients.find(e.window);
          std::cout << "Check i != clients.end()? " << (i != clients.end()) << std::endl;
          ++i;
          if (i == clients.end()) {
            i = clients.begin();
          }
          // 2. Raise and set focus.
          XRaiseWindow(display, i->second);
          XSetInputFocus(display, i->first, RevertToPointerRoot, CurrentTime);
        }
        std::cout << "EVENT HANDLER OnKeyPress" << std::endl;
      }

      void OnKeyRelease(const XKeyEvent& e) {
        std::cout << "EVENT HANDLER OnKeyRelease" << std::endl;
      }

      static int on_x_error(Display *display, XErrorEvent *e) {
        const int MAX_ERROR_TEXT_LENGTH = 1024;
        char error_text[MAX_ERROR_TEXT_LENGTH];
        XGetErrorText(display, e->error_code, error_text, sizeof(error_text));
        std::cout << "ERROR: " << "Received X error:\n"
                   << "    Request: " << int(e->request_code)
                   << " - " << XRequestCodeToString(e->request_code) << "\n"
                   << "    Error code: " << int(e->error_code)
                   << " - " << error_text << "\n"
                   << "    Resource ID: " << e->resourceid;
        // The return value is ignored.
        return 0;
      }

      // run
      int run() {
        std::cout << "Running" << std::endl;

        {
          std::lock_guard<std::mutex> lock(window_manager_t::wm_detected_mutex);

          wm_detected = false;
          XSetErrorHandler(&window_manager_t::on_wm_detected);
          XSelectInput(
            display,
            root_window,
            SubstructureRedirectMask | SubstructureNotifyMask
          );
          XSync(display, false);
          if (wm_detected) {
            std::cout << "Detected another window manager on display " << XDisplayString(display);
            return 1;
          }
        }

        XSetErrorHandler(&window_manager_t::on_x_error);
        //   c. Grab X server to prevent windows from changing under us.
        XGrabServer(display);

        //   d. Reparent existing top-level windows.
        //     i. Query existing top-level windows.
        Window returned_root, returned_parent;
        Window *top_level_windows;
        unsigned int num_top_level_windows;
        std::cout << "RESULT Xquery Tree " << XQueryTree(
            display,
            root_window,
            &returned_root,
            &returned_parent,
            &top_level_windows,
            &num_top_level_windows) << std::endl;

        if (returned_root == root_window) {
          std::cout << "Root window is equal" << std::endl;
        } else {
          std::cout << "Root window is not equal" << std::endl;
        }

        //     ii. Frame each top-level window.
        for (unsigned int i = 0; i < num_top_level_windows; ++i) {
          frame(top_level_windows[i]);
        }

        //     iii. Free top-level window array.
        XFree(top_level_windows);
        //   e. Ungrab X server.
        XUngrabServer(display);

        for (;;) {
          XEvent e;
          XNextEvent(display, &e);
          std::cout << "Received Event" << std::endl;

          switch (e.type) {
            case CreateNotify:
              OnCreateNotify(e.xcreatewindow);
              break;
            case DestroyNotify:
              OnDestroyNotify(e.xdestroywindow);
              break;
            case ReparentNotify:
              OnReparentNotify(e.xreparent);
              break;
            case MapNotify:
              OnMapNotify(e.xmap);
              break;
            case UnmapNotify:
              OnUnmapNotify(e.xunmap);
              break;
            case ConfigureNotify:
              OnConfigureNotify(e.xconfigure);
              break;
            case MapRequest:
              OnMapRequest(e.xmaprequest);
              break;
            case ConfigureRequest:
              OnConfigureRequest(e.xconfigurerequest);
              break;
            case ButtonPress:
              OnButtonPress(e.xbutton);
              break;
            case ButtonRelease:
              OnButtonRelease(e.xbutton);
              break;
            case MotionNotify:
              // Skip any already pending motion events.
              while (XCheckTypedWindowEvent(
                  display, e.xmotion.window, MotionNotify, &e)) {}
              OnMotionNotify(e.xmotion);
              break;
            case KeyPress:
              OnKeyPress(e.xkey);
              break;
            case KeyRelease:
              OnKeyRelease(e.xkey);
              break;
            default:
              std::cout << "Ignored event" << std::endl;
          }
        }

        return 0;
      }

      // factgory method to return unique_ptr
      static void make(std::unique_ptr<window_manager_t> &ptr) {
        Display *display = XOpenDisplay(nullptr);
        if (display == nullptr) {
          std::stringstream strm;
          strm << "Could not open display ";
          strm << XDisplayName(nullptr);
          throw std::runtime_error(strm.str());
        }

        ptr.reset(new window_manager_t(display));
      }

      // called when another window manager
      // is already running
      static int on_error(Display *display, XErrorEvent *e) {
        std::cout << "ERROR" << std::endl;
        return 0;
        // TODO
      }

      // Whether an existing window manager has been detected. Set by on_wm_detected
      static int on_wm_detected(Display *display, XErrorEvent *e) {
        std::cout << "ON WM DETECTED" << std::endl;
        if (static_cast<int>(e->error_code) == BadAccess) {
          std::cout << "Bad Access" << std::endl;
        }

        wm_detected = true;
        return 0;
        // TODO
      }
  };
}

using namespace phone;

int main(int, char*[]) {
  std::unique_ptr<window_manager_t> wm;
  window_manager_t::make(wm);
  return wm->run();
}
