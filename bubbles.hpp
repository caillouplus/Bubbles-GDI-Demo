#pragma once
#include "pch.hpp"
#include "macros.hpp"
#include "config.hpp"

class Canvas;
class Bubble;

class Canvas {
  unsigned size_x;
  unsigned size_y;

  HBRUSH const background;

  std::vector <Bubble*> bubbles;

  enum class CanvasSide : unsigned {Top = 0B0U, Bottom = 0B1U, Left = 0B10U, Right = 0B11U, None = 0B100U};

public:
  Canvas(unsigned, unsigned, unsigned long);
  bool AddBubble(Bubble&);
  bool CheckCanvasOutOfBounds(Bubble const&) const;
  bool CheckSpecificSideOutOfBounds(Bubble const&, CanvasSide) const;
  bool CheckSpecificSideOutOfBoundsNext(Bubble const&, CanvasSide) const;
  bool CheckTwoBubblesShock(Bubble const&, Bubble const&) const;
  bool CheckTwoBubblesNextShock(Bubble const&, Bubble const&) const;

  double WhenWillBubbleReachWall(Bubble const&, CanvasSide) const;
//  double WhenWillTwoBubblesShock(Bubble const& bubble_a, Bubble const& bubble_b) const;

  void HandleBubbleWallShock(Bubble&, double);
  void HandleTwoBubblesShock(Bubble&, Bubble&, double);

  void AdvanceTime(double);
  void PaintEverything(HDC) const;
};

class Bubble {
  double radius;
  double pos_x;
  double pos_y;
  double speed_x;
  double speed_y;

  bool invert_x = false;
  bool invert_y = false;

  HPEN const outline;
  HBRUSH const fill;

  struct {
    double pos_x;
    double pos_y;
    bool conflict{false};
    unsigned wall_conflict{4U};
    Bubble* bubble_conflict{nullptr};
  } pre_next;

  friend class Canvas;

public:
  Bubble(double, double, double, double, double, unsigned long, unsigned long);
  void SetPreNextPos(double);
  bool ConflictSetAndRet();
  void NoConflictAdvance();
  void CleanPreNext();

  static double WhenWillTwoBubblesShock(Bubble const&,Bubble const&);
};


bool CheckTwoBubblesShockWorker(double, double, double, double, double, double);
//bool CheckLineAndBubbleShock(bool, double, double, double, double);
