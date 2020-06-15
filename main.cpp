#include "main.hpp"

int wWinMain(HINSTANCE main_instance, HINSTANCE, wchar_t*, int show_args) {
  main_class = {
    sizeof main_class,
    0U,
    MainWindowProc,
    0,
    0,
    main_instance,
    LoadIconW(nullptr, MAKEINTRESOURCEW(IDI_APPLICATION)),
    LoadCursorW(nullptr, MAKEINTRESOURCEW(IDC_ARROW)),
    (HBRUSH)GetStockObject(WHITE_BRUSH),
    nullptr,
    main_class_name,
    nullptr
  };
  RegisterClassExW(&main_class);

  AdjustWindowRectEx(&window_size, my_style, FALSE, 0);
  main_window_hwnd = CreateWindowExW(0, main_class_name, main_window_name, my_style, window_size.left, window_size.top, window_size.right - window_size.left, window_size.bottom - window_size.top, nullptr, nullptr, main_instance, nullptr);
  if (!main_window_hwnd) return 0;

  ShowWindow(main_window_hwnd, show_args);
  UpdateWindow(main_window_hwnd);

  while (GetMessageW(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  return msg.lParam;
}

long long MainWindowProc(HWND hwnd, unsigned int msg, unsigned long long wparam, long long lparam) {
  switch (msg) {
    case WM_CREATE: {
      main_canvas = new Canvas(WINDOW_SIZE_X, WINDOW_SIZE_Y, GETCOLOR());
      REPEAT(i, BUBBLE_COUNT) {
        Bubble* bubble = new Bubble(MYRANDOM <double> (RADIUS_MIN, RADIUS_MAX), MYRANDOM <double> (0.0, (double)WINDOW_SIZE_X), MYRANDOM <double> (0.0, (double)WINDOW_SIZE_Y), MYRANDOM <double> (- SPEED_MAX, SPEED_MAX), MYRANDOM <double> (- SPEED_MAX, SPEED_MAX), GETCOLOR(), GETCOLOR());
        if (main_canvas->AddBubble(*bubble)) continue;
        else {
          delete bubble;
          --i;
        }
      }
      SetTimer(hwnd, 0, 15, nullptr);
    } return 0;
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      main_canvas->PaintEverything(hdc);
      EndPaint(hwnd, &ps);
    } return 0;
    case WM_TIMER: {
      main_canvas->AdvanceTime(1.0);
      InvalidateRect(hwnd, nullptr, FALSE);
    } return 0;
    case WM_CLOSE: {
      DestroyWindow(hwnd);
      PostQuitMessage(0);
    } return 0;
    default: return DefWindowProcW(hwnd, msg, wparam, lparam);
  }
}