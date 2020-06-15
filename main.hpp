#pragma once
#include "pch.hpp"
#include "config.hpp"
#include "macros.hpp"
#include "bubbles.hpp"

wchar_t constexpr main_window_name[]{L"MainWindow"};
wchar_t constexpr main_class_name[]{L"Main Class"};
WNDCLASSEXW main_class;
HWND main_window_hwnd;

int __stdcall wWinMain(HINSTANCE, HINSTANCE, wchar_t*, int);
long long __stdcall MainWindowProc(HWND, unsigned int, unsigned long long, long long);

static DWORD constexpr my_style{WS_CAPTION | WS_MINIMIZEBOX | WS_BORDER | WS_MINIMIZEBOX | WS_SYSMENU};
RECT window_size{WINDOW_POS_X, WINDOW_POS_Y, WINDOW_SIZE_X + WINDOW_POS_X, WINDOW_SIZE_Y + WINDOW_POS_Y};

MSG msg;

Canvas* main_canvas;