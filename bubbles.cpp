#include "bubbles.hpp"

Canvas::Canvas(unsigned size_x, unsigned size_y, unsigned long background_color) :
  size_x(size_x), size_y(size_y), background(CreateSolidBrush(background_color)) {}

bool Canvas::AddBubble(Bubble& candidate_bubble) {
  if (CheckCanvasOutOfBounds(candidate_bubble)) return false;
  if (bubbles.empty()) goto AddIt;
  for (auto existant_bubble : bubbles) {
    if (CheckTwoBubblesShock(candidate_bubble, *existant_bubble)) return false;
  }

AddIt:
  bubbles.push_back(&candidate_bubble);
  return true;
}

bool Canvas::CheckCanvasOutOfBounds(Bubble const& bubble) const {
  REPEAT(side_id, 4) {
    if (CheckSpecificSideOutOfBounds(bubble, (CanvasSide)side_id)) return true;
  }
  return false;
}

bool Canvas::CheckSpecificSideOutOfBounds(Bubble const& bubble, CanvasSide side_id) const {
  bool direction = ((side_id == CanvasSide::Left) || (side_id == CanvasSide::Right));
  bool position = ((side_id == CanvasSide::Bottom) || (side_id == CanvasSide::Right));

  if (position) return (((direction) ? bubble.pos_x : bubble.pos_y) + bubble.radius) > ((direction) ? size_x : size_y);
  else return (((direction) ? bubble.pos_x : bubble.pos_y) < bubble.radius);

//  return (CheckLineAndBubbleShock(direction, ((position) ? 0.0 : 1.0) * ((direction) ? size_x : size_y), bubble.pos_x, bubble.pos_y, bubble.radius));
}

bool Canvas::CheckSpecificSideOutOfBoundsNext(Bubble const& bubble, CanvasSide side_id) const {
  bool direction = ((side_id == CanvasSide::Left) || (side_id == CanvasSide::Right));
  bool position = ((side_id == CanvasSide::Bottom) || (side_id == CanvasSide::Right));

  if (position) return (((direction) ? bubble.pre_next.pos_x : bubble.pre_next.pos_y) + bubble.radius) > ((direction) ? size_x : size_y);
  else return (((direction) ? bubble.pre_next.pos_x : bubble.pre_next.pos_y) < bubble.radius);
}

bool Canvas::CheckTwoBubblesShock(Bubble const& bubble_a, Bubble const& bubble_b) const {
  return CheckTwoBubblesShockWorker(bubble_a.pos_x, bubble_a.pos_y, bubble_b.pos_x, bubble_b.pos_y, bubble_a.radius, bubble_b.radius);
}

bool Canvas::CheckTwoBubblesNextShock(Bubble const& bubble_a, Bubble const& bubble_b) const {
  return CheckTwoBubblesShockWorker(bubble_a.pre_next.pos_x, bubble_a.pre_next.pos_y, bubble_b.pre_next.pos_x, bubble_b.pre_next.pos_y, bubble_a.radius, bubble_b.radius);
}




double Canvas::WhenWillBubbleReachWall(Bubble const& bubble, CanvasSide side_id) const {
  bool direction = ((side_id == CanvasSide::Left) || (side_id == CanvasSide::Right));
  bool position = ((side_id == CanvasSide::Bottom) || (side_id == CanvasSide::Right));
  double goal = (position) ? ((direction) ? (size_x) : (size_y)) : (0.0);
  double closer_atom = (direction) ? (bubble.pos_x) : (bubble.pos_y) + bubble.radius * ((position) ? 1.0 : -1.0);
  return ((goal) - (closer_atom)) / ((direction) ? bubble.speed_x : bubble.speed_y);
}





void Canvas::HandleBubbleWallShock(Bubble& bubble, double time) {
  double when_will_bubble_reach_wall = WhenWillBubbleReachWall(bubble, (CanvasSide)bubble.pre_next.wall_conflict);
  bubble.pos_x = bubble.pos_x + bubble.speed_x * time * ((bubble.invert_x) ? -1.0 : 1.0);
  bubble.pos_y = bubble.pos_y + bubble.speed_y * time * ((bubble.invert_y) ? -1.0 : 1.0);
  if ((unsigned)bubble.pre_next.wall_conflict & 2U) bubble.invert_x = !(bubble.invert_x);
  else bubble.invert_y = !(bubble.invert_y);
  bubble.pos_x = bubble.pos_x + bubble.speed_x * time * ((bubble.invert_x) ? -1.0 : 1.0);
  bubble.pos_y = bubble.pos_y + bubble.speed_y * time * ((bubble.invert_y) ? -1.0 : 1.0);
  bubble.CleanPreNext();
}

void Canvas::HandleTwoBubblesShock(Bubble& bubble_a, Bubble& bubble_b, double time) {
  double _time = Bubble::WhenWillTwoBubblesShock(bubble_a, bubble_b);

  bubble_a.pos_x = bubble_a.pos_x + bubble_a.speed_x * time * ((bubble_a.invert_x) ? -1.0 : 1.0);
  bubble_b.pos_y = bubble_b.pos_y + bubble_b.speed_y * time * ((bubble_b.invert_y) ? -1.0 : 1.0);

  std::swap(bubble_a.speed_x, bubble_b.speed_x);
  std::swap(bubble_a.speed_y, bubble_b.speed_y);
  std::swap(bubble_a.invert_x, bubble_b.invert_x);
  std::swap(bubble_a.invert_y, bubble_b.invert_y);

  bubble_a.pos_x = bubble_a.pos_x + bubble_a.speed_x * time * ((bubble_a.invert_x) ? -1.0 : 1.0);
  bubble_b.pos_y = bubble_b.pos_y + bubble_b.speed_y * time * ((bubble_b.invert_y) ? -1.0 : 1.0);

  bubble_a.CleanPreNext();
  bubble_b.CleanPreNext();
}







void Canvas::AdvanceTime(double time) {
  if (bubbles.empty()) return;
  std::for_each(bubbles.begin(), bubbles.end(), [this, time](Bubble* bubble) mutable -> void {bubble->SetPreNextPos(time);});
  bool no_conf = true;
  for (auto bubble_a_ref = bubbles.begin(); bubble_a_ref < bubbles.end(); ++bubble_a_ref) {
    Bubble& bubble_a = **bubble_a_ref;
    REPEAT(side_id, 4) {
      if (CheckSpecificSideOutOfBoundsNext(bubble_a, (CanvasSide)side_id)) {
        if (bubble_a.ConflictSetAndRet()) goto TooManyConflicts;
        no_conf = false;
        bubble_a.pre_next.wall_conflict = side_id;
      }
    }
    for (auto bubble_b_ref = bubble_a_ref + 1; bubble_b_ref < bubbles.end(); ++bubble_b_ref) {
      Bubble& bubble_b = **bubble_b_ref;
      if (CheckTwoBubblesNextShock(bubble_a, bubble_b)) {
        if (bubble_a.ConflictSetAndRet()) goto TooManyConflicts;
        if (bubble_b.ConflictSetAndRet()) goto TooManyConflicts;
        bubble_a.pre_next.bubble_conflict = &bubble_b;
      }
    }
  }
  std::for_each(bubbles.begin(), bubbles.end(), [this, time](Bubble* bubble) mutable -> void {
    if (!bubble->pre_next.conflict) {
      bubble->NoConflictAdvance();
    } else {
      if (bubble->pre_next.wall_conflict < 4) {
        HandleBubbleWallShock(*bubble, time);
      } else if (bubble->pre_next.bubble_conflict) {
        HandleTwoBubblesShock(*bubble, *(bubble->pre_next.bubble_conflict), time);
      }
    }
  });
  return;
TooManyConflicts:
  std::for_each(bubbles.begin(), bubbles.end(), [](Bubble* bubble) mutable -> void {
    bubble->CleanPreNext();
  });
  AdvanceTime(time / 2.0);
  std::for_each(bubbles.begin(), bubbles.end(), [](Bubble* bubble) mutable -> void {
    bubble->CleanPreNext();
  });
  AdvanceTime(time / 2.0);
}



void Canvas::PaintEverything(HDC hdc) const {
  SelectObject(hdc, background);
  SelectObject(hdc, GetStockObject(NULL_PEN));
  Rectangle(hdc, 0, 0, WINDOW_SIZE_X, WINDOW_SIZE_Y);
  std::for_each(bubbles.begin(), bubbles.end(), [hdc, this](Bubble* bubble) mutable -> void {
    SelectObject(hdc, bubble->fill);
    SelectObject(hdc, bubble->outline);
    Ellipse(hdc, bubble->pos_x - bubble->radius, bubble->pos_y - bubble->radius, bubble->pos_x + bubble->radius, bubble->pos_y + bubble->radius);
  });
}












Bubble::Bubble(double radius, double pos_x, double pos_y, double speed_x, double speed_y, unsigned long outline_color, unsigned long fill_color) :
  radius(radius), pos_x(pos_x), pos_y(pos_y), speed_x(speed_x), speed_y(speed_y), outline(CreatePen(PS_SOLID, 1, outline_color)), fill(CreateSolidBrush(fill_color)) {}

void Bubble::SetPreNextPos(double time) {
  pre_next.pos_x = pos_x + speed_x * time * ((invert_x) ? -1.0 : 1.0);
  pre_next.pos_y = pos_y + speed_y * time * ((invert_y) ? -1.0 : 1.0);
}










bool Bubble::ConflictSetAndRet() {
  if (pre_next.conflict) return true;
  else {
    pre_next.conflict = true;
    return false;
  }
}

void Bubble::NoConflictAdvance() {
  pos_x = pre_next.pos_x;
  pos_y = pre_next.pos_y;
  CleanPreNext();
}

void Bubble::CleanPreNext() {
  pre_next.conflict = false;
  pre_next.bubble_conflict = nullptr;
  pre_next.wall_conflict = 4U;
}


double Bubble::WhenWillTwoBubblesShock(Bubble const& bubble_a, Bubble const& bubble_b) {

  double bhaskara_a = pow((bubble_a.speed_x * (bubble_a.invert_x ? 1.0 : -1.0) - (bubble_b.speed_x * (bubble_b.invert_x ? 1.0 : -1.0))), 2.0)
                    + pow((bubble_a.speed_y * (bubble_a.invert_y ? 1.0 : -1.0) - (bubble_b.speed_y * (bubble_b.invert_y ? 1.0 : -1.0))), 2);

  double bhaskara_b = 2.0 * (  (bubble_a.speed_x * (bubble_a.invert_x ? 1.0 : -1.0) - bubble_b.speed_x * (bubble_b.invert_x ? 1.0 : -1.0)) * (bubble_a.pos_x - bubble_b.pos_x)
                             + (bubble_a.speed_y * (bubble_a.invert_y ? 1.0 : -1.0) - bubble_b.speed_y * (bubble_b.invert_y ? 1.0 : -1.0)) * (bubble_a.pos_y - bubble_b.pos_y));

  double bhaskara_c = pow((bubble_a.pos_x - bubble_b.pos_x), 2.0) + pow((bubble_a.pos_y - bubble_b.pos_y), 2.0) - pow((bubble_a.radius + bubble_b.radius), 2);

  double bhaskara_delta_sqrt = sqrt(pow(bhaskara_b, 2.0) - 4 * bhaskara_a * bhaskara_c);

  double radix_1 = (- bhaskara_b + bhaskara_delta_sqrt) / (2 * bhaskara_a);

  double radix_2 = (- bhaskara_b - bhaskara_delta_sqrt) / (2 * bhaskara_a);

  Debug(radix_1);
  Debug(radix_2);


  return min(abs(radix_1), abs(radix_2));
/*
  double diff_pos_x = bubble_a.pos_x - bubble_b.pos_x;
  double diff_speed_x = bubble_a.speed_x * (bubble_a.invert_x ? - 1.0 : 1.0) - bubble_b.speed_x * (bubble_b.invert_x ? - 1.0 : 1.0);
  double diff_pos_y = bubble_a.pos_y - bubble_b.pos_y;
  double diff_speed_y = bubble_a.speed_y * (bubble_a.invert_y ? - 1.0 : 1.0) - bubble_b.speed_y * (bubble_b.invert_y ? - 1.0 : 1.0);

  double bhaskara_a = pow(diff_speed_x, 2) + pow(diff_speed_y, 2);
  double bhaskara_b = 2.0 * (diff_pos_x * diff_speed_x + diff_pos_y * diff_speed_y);
  double bhaskara_c = pow(diff_pos_x, 2) + pow(diff_pos_y, 2) - pow(bubble_a.radius + bubble_b.radius, 2);
  double bhaskara_delta_sqrt = sqrt(pow(bhaskara_b, 2) - 4 * bhaskara_a * bhaskara_c);

  double radix_1 = (- bhaskara_b + bhaskara_delta_sqrt) / (2.0 * bhaskara_a);
  double radix_2 = (- bhaskara_b - bhaskara_delta_sqrt) / (2.0 * bhaskara_a);
  return 0.0;*/
}





bool CheckTwoBubblesShockWorker(double a_pos_x, double a_pos_y, double b_pos_x, double b_pos_y, double a_radius, double b_radius) {
  bool result = (pow(a_pos_x - b_pos_x, 2.0) + pow(a_pos_y - b_pos_y, 2.0)) < pow(a_radius + b_radius, 2.0);
  if (result) {
    Sleep(0);
  }
  return result;
}

/*bool CheckLineAndBubbleShock(bool direction, double line_distance_from_zero, double bubble_pos_x, double bubble_pos_y, double bubble_radius) {
  if (direction) {
    if (line_distance_from_zero) {
      return bubble_pos_x + bubble_radius < line_distance_from_zero;
    } else {
      return bubble_pos_x - bubble_radius < line_distance_from_zero;
    }
  } else {
    if (line_distance_from_zero) {
      return bubble_pos_y + bubble_radius > line_distance_from_zero;
    } else {
      return bubble_pos_y - bubble_radius < line_distance_from_zero;
    }
  }
}*/