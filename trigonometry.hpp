#pragma once
#include "pch.hpp"

template <typename T>
T NormalizeArc(T arc) {
  if (arc < - M_PI) return NormalizeArc(arc + M_PI * 2);
  else if (arc > M_PI) return NormalizeArc(arc - M_PI * 2);
  else return arc;
}

template <typename T>
T InvertArc(T arc) {
  return NormalizeArc(arc + M_PI);
}