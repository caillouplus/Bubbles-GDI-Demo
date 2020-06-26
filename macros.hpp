#pragma once
#include "pch.hpp"

#define REPEAT(I, X) for (unsigned I = 0; ((I) < (X)); ++(I))

static COLORREF MYRGB(BYTE r, BYTE g, BYTE b) {return ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)));}

//constexpr COLORREF SATURATED(double I) { return (((I) < (1.0/3.0)) ? (MYRGB(255, ((I) * 255), 0)) : ((I) < (2.0/3.0)) ? (MYRGB(0, 255, (I - (1.0/3.0)) * 255)) : (MYRGB(((I - (2.0/3.0)) * 255), (0), (255)))); }
constexpr COLORREF DEFAULT_COLORS[]{
  RGB(0, 0, 255),
  RGB(255, 0, 255),
  RGB(0, 255, 255),
  RGB(0, 255, 0),
  RGB(0, 255, 255),
  RGB(255, 255, 0),
  RGB(255, 0, 0),
  RGB(255, 255, 0),
  RGB(255, 0, 255),

  RGB(127, 0, 255),
  RGB(0, 127, 255),
  RGB(0, 255, 127),
  RGB(127, 255, 0),
  RGB(255, 127, 0),
  RGB(255, 0, 127),

  RGB(127, 127, 127),
  RGB(255, 255, 255)
};

static std::random_device device;
static std::uniform_int_distribution <unsigned long long> distribution(0ULL, ULLONG_MAX);



template<typename T, typename T1>
T1 MYRANDOM(T1 MIN, T1 MAX) {
  return (T)((long double)(distribution(device) * (long double)(MAX - MIN) / (long double)ULLONG_MAX + (long double)MIN));
}

/* {
  return (T)((double)rand() * (double)(MAX - MIN) / (double)(RAND_MAX) + MIN);
}*/

static COLORREF GETCOLOR() {
  return DEFAULT_COLORS[MYRANDOM <unsigned> (0U, (unsigned)(sizeof DEFAULT_COLORS / sizeof(COLORREF)) - 1U)];
}

template <typename T>
inline T Pow2(T number) {
  return number * number;
}

inline std::pair <double, double> SolveLinearEquation(double a1, double b1, double c1, double a2, double b2, double c2) {
  double determinant = a1 * b2 - b1 * a2;
  double x = (c1 * b2 - b1 * c2) / determinant;
  double y = (a1 * c2 - c1 * a2) / determinant;
  return std::pair <double, double> {x, y};
}

inline void PolarToCartesian(double angle, double range, double& x, double& y) {
  x = range * cos(angle);
  y = range * sin(angle);
}

inline void CartesianToPolar(double x, double y, double& angle, double& range) {
  angle = atan2(y, x);
  range = sqrt(Pow2(x) + Pow2(y));
}

template <typename T>
void Debug(T number) {
  std::wstringstream buffer;
  buffer << number;
  std::wstring out = buffer.str();
  OutputDebugStringW(out.c_str());
  OutputDebugStringW(L"\n");
}

static void Debug(wchar_t const* str) {
  OutputDebugStringW(str);
  OutputDebugStringW(L"\n");
}

static void Debug(char const* str) {
  OutputDebugStringA(str);
  OutputDebugStringA("\n");
}