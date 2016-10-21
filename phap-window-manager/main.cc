#include <iostream>
#include <exception>
#include <memory>
#include <ostream>
#include <sstream>
#include <mutex>
#include <unordered_map>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace phone {
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

  class window_manager_t {
      // A mutex for protecting  wm_detected
      std::mutex wm_detected_mutex;
      // Maps top-level windows to their frame windows.
      std::unordered_map<Window, Window> clients;

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
        // Visual properties of the frame to create.
        const unsigned int BORDER_WIDTH = 3;
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
            XKeysymToKeycode(display, XK_F4),
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
        root_window(DefaultRootWindow(display)) {}

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
        std::cout << "EVENT HANDLER OnButtonPress" << std::endl;
      }

      void OnButtonRelease(const XButtonEvent& e) {
        std::cout << "EVENT HANDLER OnButtonRelease" << std::endl;
      }

      void OnMotionNotify(const XMotionEvent& e) {
        std::cout << "EVENT HANDLER OnMotionNotify" << std::endl;
      }

      void OnKeyPress(const XKeyEvent& e) {
        std::cout << "EVENT HANDLER OnKeyPress" << std::endl;
      }

      void OnKeyRelease(const XKeyEvent& e) {
        std::cout << "EVENT HANDLER OnKeyRelease" << std::endl;
      }















      // run
      int run() {
        std::cout << "Running" << std::endl;
        std::lock_guard<std::mutex> lock(window_manager_t::wm_detected_mutex);

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

        XSetErrorHandler(&window_manager_t::on_wm_detected);
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
