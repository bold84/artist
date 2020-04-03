#include "../../app.hpp"

#include <SDKDDKVer.h>
#include <windows.h>
#include <gl/gl.h>
#include "gl/glext.h"
#include "gl/wglext.h"

# include "GrContext.h"
# include "gl/GrGLInterface.h"
# include "SkImage.h"
# include "SkSurface.h"

# include "SkBitmap.h"
# include "SkData.h"
# include "SkImage.h"
# include "SkPicture.h"
# include "SkSurface.h"
# include "SkCanvas.h"
# include "SkPath.h"
# include "GrBackendSurface.h"

#include <stdexcept>

using namespace cycfi::artist;

class window
{
public:

               window(extent size, color bkd);
               ~window();

   void        render();
   void        swapBuffers();

private:

   ATOM        registerClass(HINSTANCE hInstance);
   void        destroy();

   extent      _size;

   LPTSTR      windowClass;	// Window Class
   HGLRC       RC;			// Rendering Context
   HDC	      DC;				// Device Context
   HWND        WND;			// Window

};

window::window(extent size, color bkd)
{;
   _size = size;
   auto style = WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

   windowClass = MAKEINTATOM(registerClass(nullptr));
   if (windowClass == 0)
      throw std::runtime_error("Error: registerClass() failed.");

   // create temporary window

   HWND fakeWND = CreateWindow(
      windowClass, L"Fake window",
      style,
      0, 0,						// position x, y
      1, 1,						// width, height
      NULL, NULL,					// parent window, menu
      nullptr, NULL);			// instance, param

   HDC fakeDC = GetDC(fakeWND);	// Device Context

   PIXELFORMATDESCRIPTOR fakePFD;
   ZeroMemory(&fakePFD, sizeof(fakePFD));
   fakePFD.nSize = sizeof(fakePFD);
   fakePFD.nVersion = 1;
   fakePFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   fakePFD.iPixelType = PFD_TYPE_RGBA;
   fakePFD.cColorBits = 32;
   fakePFD.cAlphaBits = 8;
   fakePFD.cDepthBits = 24;

   const int fakePFDID = ChoosePixelFormat(fakeDC, &fakePFD);
   if (fakePFDID == 0)
      throw std::runtime_error("Error: ChoosePixelFormat() failed.");

   if (!SetPixelFormat(fakeDC, fakePFDID, &fakePFD))
      throw std::runtime_error("Error: SetPixelFormat() failed.");

   HGLRC fakeRC = wglCreateContext(fakeDC);	// Rendering Contex

   if (fakeRC == nullptr)
      throw std::runtime_error("Error: wglCreateContext() failed.");

   if (!wglMakeCurrent(fakeDC, fakeRC))
      throw std::runtime_error("Error: wglMakeCurrent() failed.");

   // get pointers to functions (or init opengl loader here)

   PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
   wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
   if (wglChoosePixelFormatARB == nullptr)
      throw std::runtime_error("Error: wglGetProcAddress() failed.");

   PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
   wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
   if (wglCreateContextAttribsARB == nullptr)
      throw std::runtime_error("Error: wglGetProcAddress() failed.");


   RECT rect = { 0, 0, LONG(size.x), LONG(size.y) };
   AdjustWindowRect(&rect, style, false);
   size.x = rect.right - rect.left;
   size.y = rect.bottom - rect.top;

	RECT primaryDisplaySize;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &primaryDisplaySize, 0);	// system taskbar and application desktop toolbars not included
	auto posX = (primaryDisplaySize.right - size.x) / 2;
	auto posY = (primaryDisplaySize.bottom - size.y) / 2;

   // create a new window and context

   WND = CreateWindow(
      windowClass, L"OpenGL window",	// class name, window name
      style,							// styles
       posX, posY,		// posx, posy. If x is set to CW_USEDEFAULT y is ignored
      size.x, size.y,	// width, height
      NULL, NULL,						// parent window, menu
      nullptr, NULL);				// instance, param

   DC = GetDC(WND);

   const int pixelAttribs[] = {
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
      WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
      WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
      WGL_COLOR_BITS_ARB, 32,
      WGL_ALPHA_BITS_ARB, 8,
      WGL_DEPTH_BITS_ARB, 24,
      WGL_STENCIL_BITS_ARB, 8,
      WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
      WGL_SAMPLES_ARB, 4,
      0
   };

   int pixelFormatID; UINT numFormats;
   const bool status = wglChoosePixelFormatARB(DC, pixelAttribs, NULL, 1, &pixelFormatID, &numFormats);

   if (status == false || numFormats == 0)
      throw std::runtime_error("Error: wglChoosePixelFormatARB() failed.");

   PIXELFORMATDESCRIPTOR PFD;
   DescribePixelFormat(DC, pixelFormatID, sizeof(PFD), &PFD);
   SetPixelFormat(DC, pixelFormatID, &PFD);

   const int major_min = 4, minor_min = 0;
   const int contextAttribs[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
      WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
      WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
//		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
      0
   };

   RC = wglCreateContextAttribsARB(DC, 0, contextAttribs);
   if (RC == nullptr)
      throw std::runtime_error("Error: wglCreateContextAttribsARB() failed.");

   // delete temporary context and window

   wglMakeCurrent(NULL, NULL);
   wglDeleteContext(fakeRC);
   ReleaseDC(fakeWND, fakeDC);
   DestroyWindow(fakeWND);
   if (!wglMakeCurrent(DC, RC))
      throw std::runtime_error("Error: wglMakeCurrent() failed.");

   // init opengl loader here (extra safe version)

   SetBkColor(DC, RGB(bkd.red, bkd.green, bkd.blue));

   SetWindowLongPtrW(WND, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
   SetWindowText(WND, L"Hello Skia");
   ShowWindow(WND, SW_RESTORE);
}

window::~window()
{
   wglMakeCurrent(NULL, NULL);
   if (RC)
      wglDeleteContext(RC);
   if (DC)
      ReleaseDC(WND, DC);
   if (WND)
      DestroyWindow(WND);
}

LRESULT CALLBACK handle_event(
   HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
      case WM_PAINT:
         {
            auto param = GetWindowLongPtrW(hWnd, GWLP_USERDATA);
            auto win = reinterpret_cast<window*>(param);
            win->render();
            win->swapBuffers();
         }
         break;

      case WM_KEYDOWN:
         if (wParam == VK_ESCAPE)
            PostQuitMessage(0);
         break;

      case WM_CLOSE:
         PostQuitMessage(0);
         break;

      default:
         return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

ATOM window::registerClass(HINSTANCE hInstance)
{
   WNDCLASSEX wcex;
   ZeroMemory(&wcex, sizeof(wcex));
   wcex.cbSize = sizeof(wcex);
   wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
   wcex.lpfnWndProc = handle_event;
   wcex.hInstance = hInstance;
   wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
   wcex.lpszClassName = L"Core";

   return RegisterClassEx(&wcex);
}

///////////////////////////////////////////////////////////

void window::render() {

   // glClearColor(0.0f, 0.0f, 1.0f, 1.0f);	// rgb(33,150,243)
   // glClear(GL_COLOR_BUFFER_BIT);

   PAINTSTRUCT ps;
   HDC hdc = BeginPaint(WND, &ps);

   auto interface = GrGLMakeNativeInterface();
   sk_sp<GrContext> ctx = GrContext::MakeGL(interface);

   GrGLint buffer;
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);
   GrGLFramebufferInfo info;
   info.fFBOID = (GrGLuint) buffer;
   SkColorType colorType = kRGBA_8888_SkColorType;

   info.fFormat = GL_RGBA8;
   GrBackendRenderTarget target(_size.x, _size.y, 0, 8, info);

   sk_sp<SkSurface> surface(
      SkSurface::MakeFromBackendRenderTarget(ctx.get(), target,
      kBottomLeft_GrSurfaceOrigin, colorType, nullptr, nullptr));

   if (!surface)
      throw std::runtime_error("Error: SkSurface::MakeRenderTarget returned null");

   SkCanvas* gpu_canvas = surface->getCanvas();
   auto cnv = canvas{ gpu_canvas };

   auto scale = GetDpiForWindow(WND) / 96.0;
   cnv.pre_scale({ float(scale), float(scale) });
   draw(cnv);

   // SkCanvas* canvas = surface->getCanvas();

   // canvas->clear(SK_ColorRED);

   // SkPaint paint;
   // paint.setColor(SK_ColorBLUE);

   // SkRect rect = SkRect::MakeXYWH(10, 10, 128, 128);
   // canvas->drawRect(rect, paint);

   gpu_canvas->flush();

   EndPaint(WND, &ps);
}

void window::swapBuffers()
{
   SwapBuffers(DC);
}

void window::destroy()
{
   wglMakeCurrent(NULL, NULL);
   if (RC)
      wglDeleteContext(RC);
   if (DC)
      ReleaseDC(WND, DC);
   if (WND)
      DestroyWindow(WND);
}

namespace cycfi::artist
{
   void init_paths()
   {
   }
}

int run_app(
   int argc
 , const char* argv[]
 , extent window_size
 , color bkd
 , bool animate
)
{
   window win{ window_size, bkd };

   MSG msg;
   bool active = true;
   while (active)
   {
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
         if (msg.message == WM_QUIT)
            active = false;
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
      if (animate)
      {
         win.render();
         win.swapBuffers();
      }
   }

   return msg.wParam;
}

void stop_app()
{
   PostQuitMessage(0);
}


