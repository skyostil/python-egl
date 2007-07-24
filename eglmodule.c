#include <Python.h>
#include <GLES/egl.h>
EGLDisplay display;
EGLSurface surface;
EGLint major, minor;
EGLContext context;

Window root_window;
Window xwin;
Display* xdisplay;

void createX11window() {
    xdisplay = XOpenDisplay(NULL);
    if (xdisplay == NULL) {
        fprintf(stderr, "Cannot connect to X server %s\n", ":0");
    }
    
    int screen_num;

    /* these variables will store the size of the screen, in pixels.    */
    int screen_width;
    int screen_height;
    
    /* this variable will be used to store the ID of the root window of our */
    /* screen. Each screen always has a root window that covers the whole   */
    /* screen, and always exists.                                           */
    
    /* these variables will be used to store the IDs of the black and white */
    /* colors of the given screen. More on this will be explained later.    */
    unsigned long white_pixel;
    unsigned long black_pixel;
    
    /* check the number of the default screen for our X server. */
    screen_num = DefaultScreen(xdisplay);
    
    /* find the width of the default screen of our X server, in pixels. */
    screen_width = DisplayWidth(xdisplay, screen_num);
    
    /* find the height of the default screen of our X server, in pixels. */
    screen_height = DisplayHeight(xdisplay, screen_num);
    
    /* find the ID of the root window of the screen. */
    root_window = RootWindow(xdisplay, screen_num);
    
    /* find the value of a white pixel on this screen. */
    white_pixel = WhitePixel(xdisplay, screen_num);
    
    /* find the value of a black pixel on this screen. */
    black_pixel = BlackPixel(xdisplay, screen_num);
    
    
    
    xwin = XCreateSimpleWindow(
            xdisplay,
            RootWindow(xdisplay, screen_num),
            0, 0,
            800, 480,
            0, BlackPixel(xdisplay, screen_num),
            WhitePixel(xdisplay, screen_num)
        );
    XMapWindow(xdisplay, xwin);
}

PyObject *egl_create(PyObject *self, PyObject *args) {
    //createX11window();
    //Display *xdisplay;
    
    //Window win;
    if(!PyArg_ParseTuple(args, "i",&xwin)) {
        return NULL;
    }
    
    xdisplay = XOpenDisplay(NULL);
    if (xdisplay == NULL) {
        fprintf(stderr, "Cannot connect to X server\n");
    }
    
    display = eglGetDisplay(xdisplay);
    
    if (!eglInitialize(display, &major, &minor)) {
        // could not initialize display
        PyErr_SetString(PyExc_RuntimeError, "eglInitialize failed");
        return NULL;
    }
    
    
    
    EGLConfig configs[10];
    EGLint matchingConfigs;
    EGLint attribList[] = {
            EGL_DEPTH_SIZE, 16,
            0, 0 };	// extend this
            
    if (!eglChooseConfig(display, attribList, &configs, 10, &matchingConfigs)) {
		// could not choose config
	}

	if (matchingConfigs < 1) {
		// did not find a suitable configuration
	}

	EGLConfig config = configs[0];	// pick any

    
    // Create X11 window
    
	surface = eglCreateWindowSurface(display, config, xwin, attribList);
	
	// test for error 

	context = eglCreateContext(display, config, 0, attribList);
    eglMakeCurrent(display, surface, surface, context);
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *egl_swapbuffers(PyObject *self, PyObject *args) {
    eglWaitGL();
    eglSwapBuffers(display, surface);
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *egl_close(PyObject *self, PyObject *args) {
    if (!eglDestroyContext(display, context)) {
		// error deallocating context
        PyErr_SetString(PyExc_RuntimeError, "eglDestroyContext failed");
        return NULL;
	}
	
	if (!eglDestroySurface(display, surface)) {
		// error deallocating surface
        PyErr_SetString(PyExc_RuntimeError, "eglDestroySurface failed");
        return NULL;
	}

	if (!eglTerminate(display)) {
		// error while cleaning up display
        PyErr_SetString(PyExc_RuntimeError, "eglTerminate failed");
        return NULL;
	}
    
    XCloseDisplay(display);
    
    Py_INCREF(Py_None);
    return Py_None;
}

  static const PyMethodDef egl_methods[] = {
    {"create",          (PyCFunction)egl_create,           METH_VARARGS, NULL},
    {"swapbuffers",          (PyCFunction)egl_swapbuffers,           METH_NOARGS, NULL},
    {"close",          (PyCFunction)egl_close,           METH_NOARGS, NULL},
    {NULL,              NULL}           /* sentinel */
};


DL_EXPORT(void) initegl(void) {
    PyObject *m, *d; // Pointer for module object and it's dictionary
    // Initialize the module
    m = Py_InitModule("egl", (PyMethodDef*)egl_methods);
    d = PyModule_GetDict(m);
}
