#include "bubbles.hpp"

Canvas::Canvas(unsigned size_x, unsigned size_y, unsigned long background_color, unsigned bubbles_count) :
    size_x(size_x), size_y(size_y), background(CreateSolidBrush(background_color)), bubbles(new Bubble*[bubbles_count]), bubbles_end(bubbles) {
  AddBubble(bubbles_count);
}

void Canvas::PreDraw(HDC hdc_base) {
  std::for_each(bubbles, bubbles_end, [hdc_base](Bubble* bubble) mutable -> void {bubble->PreDraw(GETCOLOR(), GETCOLOR(), hdc_base);});
}

void Canvas::AddBubble(unsigned count) {
  REPEAT(i, count) {
Start:
    Bubble* candidate_bubble = new Bubble(MYRANDOM <double> (RADIUS_MIN, RADIUS_MAX), MYRANDOM <double> (0.0, (double)WINDOW_SIZE_X), MYRANDOM <double> (0.0, (double)WINDOW_SIZE_Y), MYRANDOM <double> (- SPEED_MAX, SPEED_MAX), MYRANDOM <double> (- SPEED_MAX, SPEED_MAX));
    if (!CheckCanvasOutOfBounds(*candidate_bubble)) {
      for (auto existant_bubble = bubbles; existant_bubble < bubbles_end; ++existant_bubble) {
        if (CheckTwoBubblesShock(*candidate_bubble, **existant_bubble)) {
          --i;
          delete candidate_bubble;
          goto Start;
        }
      }
      *bubbles_end = candidate_bubble;
      ++bubbles_end;
    }
  }
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

void Canvas::HandleBubbleWallShock(Bubble& bubble, double time) const {
  double when_will_bubble_reach_wall = WhenWillBubbleReachWall(bubble, (CanvasSide)bubble.pre_next.wall_conflict);
  bubble.pos_x = bubble.pos_x + bubble.speed_x * time;
  bubble.pos_y = bubble.pos_y + bubble.speed_y * time;
  if ((unsigned)bubble.pre_next.wall_conflict & 2U) bubble.speed_x = - bubble.speed_x;
  else bubble.speed_y = - bubble.speed_y;
  bubble.pos_x = bubble.pos_x + bubble.speed_x * time;
  bubble.pos_y = bubble.pos_y + bubble.speed_y * time;
  bubble.CleanPreNext();
}

void Canvas::HandleTwoBubblesShock(Bubble& bubble_a, Bubble& bubble_b, double time) const {

  double collision_time = Bubble::WhenWillTwoBubblesShock(bubble_a, bubble_b);

  bubble_a.pos_x = bubble_a.pos_x + bubble_a.speed_x * collision_time;
  bubble_b.pos_y = bubble_b.pos_y + bubble_b.speed_y * collision_time;
  bubble_b.pos_x = bubble_b.pos_x + bubble_b.speed_x * collision_time;
  bubble_a.pos_y = bubble_a.pos_y + bubble_a.speed_y * collision_time;

  double collision_angle = atan2(bubble_a.pos_y - bubble_b.pos_y, bubble_a.pos_x - bubble_b.pos_x);

  double bubble_a_speed_x;
  double bubble_a_speed_y;
  double bubble_b_speed_x;
  double bubble_b_speed_y;

  double bubble_a_speed_range;
  double bubble_a_speed_angle;
  double bubble_b_speed_range;
  double bubble_b_speed_angle;

  CartesianToPolar(bubble_a.speed_x, bubble_a.speed_y, bubble_a_speed_angle, bubble_a_speed_range);
  CartesianToPolar(bubble_b.speed_x, bubble_b.speed_y, bubble_b_speed_angle, bubble_b_speed_range);

////////////////////////////////////////////////////////////////////////////////////////////////////////////

  bubble_a_speed_angle -= collision_angle;
  bubble_b_speed_angle -= collision_angle;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

  PolarToCartesian(bubble_a_speed_angle, bubble_a_speed_range, bubble_a_speed_x, bubble_a_speed_y);
  PolarToCartesian(bubble_b_speed_angle, bubble_b_speed_range, bubble_b_speed_x, bubble_b_speed_y);

  std::swap(bubble_a_speed_x, bubble_b_speed_x);

  CartesianToPolar(bubble_a_speed_x, bubble_a_speed_y, bubble_a_speed_angle, bubble_a_speed_range);
  CartesianToPolar(bubble_b_speed_x, bubble_b_speed_y, bubble_b_speed_angle, bubble_b_speed_range);

////////////////////////////////////////////////////////////////////////////////////////////////////////////

  bubble_a_speed_angle += collision_angle;
  bubble_b_speed_angle += collision_angle;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

  PolarToCartesian(bubble_a_speed_angle, bubble_a_speed_range, bubble_a_speed_x, bubble_a_speed_y);
  PolarToCartesian(bubble_b_speed_angle, bubble_b_speed_range, bubble_b_speed_x, bubble_b_speed_y);

  bubble_a.speed_x = bubble_a_speed_x;
  bubble_a.speed_y = bubble_a_speed_y;
  bubble_b.speed_x = bubble_b_speed_x;
  bubble_b.speed_y = bubble_b_speed_y;


/*
  double bubble_a_speed_direct = bubble_a_speed_combined * cos(bubble_a_angle - bubble_a_collision_angle);
  double bubble_a_speed_oblique = bubble_a_speed_combined * sin(bubble_a_angle - bubble_a_collision_angle);

  double bubble_b_speed_direct = bubble_b_speed_combined * cos(bubble_b_angle - bubble_b_collision_angle);
  double bubble_b_speed_oblique = bubble_b_speed_combined * sin(bubble_b_angle - bubble_b_collision_angle);

  //////////////////////////////////////////////////////////////////////////////////////////////////////

  double bubble_a_new_speed_x = bubble_a_speed_direct * cos(bubble_a_angle) + bubble_a_speed_oblique * cos(bubble_a_collision_angle);
  double bubble_a_new_speed_y = bubble_a_speed_direct * sin(bubble_a_angle) + bubble_a_speed_oblique * sin(bubble_a_collision_angle);
  double bubble_b_new_speed_x = bubble_b_speed_direct * cos(bubble_b_angle) + bubble_b_speed_oblique * cos(bubble_b_collision_angle);
  double bubble_b_new_speed_y = bubble_b_speed_direct * sin(bubble_b_angle) + bubble_b_speed_oblique * sin(bubble_b_collision_angle);

  bubble_a.speed_x = bubble_a_new_speed_x;
  bubble_a.speed_y = bubble_a_new_speed_y;
  bubble_b.speed_x = bubble_b_new_speed_x;
  bubble_b.speed_y = bubble_b_new_speed_y;
*/
//  double bubble_a_angle = atan2(bubble_a.speed_y, bubble_a.speed_x);
//  double bubble_b_angle = atan2(bubble_b.speed_y, bubble_b.speed_x); 
/*7
  double collision_angle = atan2(bubble_a.pos_y - bubble_b.pos_y, bubble_a.pos_x - bubble_b.pos_y);
  double collision_cos = cos(collision_angle);
  double collision_sin = sin(collision_angle);

  double bubble_a_speed_x_parallel = - collision_cos * bubble_a.speed_x;
  double bubble_a_speed_x_perpendicular = - collision_sin * bubble_a.speed_x;
  double bubble_a_speed_y_parallel = - collision_sin * bubble_a.speed_y;
  double bubble_a_speed_y_perpendicular = - collision_cos * bubble_a.speed_y;
  double bubble_b_speed_x_parallel = collision_cos * bubble_b.speed_x;
  double bubble_b_speed_x_perpendicular = collision_sin * bubble_b.speed_x;
  double bubble_b_speed_y_parallel = collision_sin * bubble_b.speed_y;
  double bubble_b_speed_y_perpendicular = collision_cos * bubble_b.speed_y;

  std::swap(bubble_a_speed_x_parallel, bubble_b_speed_x_parallel);
  std::swap(bubble_a_speed_y_parallel, bubble_b_speed_y_parallel);

  bubble_a.speed_x = bubble_a_speed_x_parallel + bubble_a_speed_x_perpendicular;
  bubble_a.speed_y = bubble_a_speed_y_parallel + bubble_a_speed_y_perpendicular;
  bubble_b.speed_x = bubble_b_speed_x_parallel + bubble_b_speed_x_perpendicular;
  bubble_b.speed_y = bubble_b_speed_y_parallel + bubble_b_speed_y_perpendicular;
*/
/*
  double bubble_a_new_angle = NormalizeArc(collision_angle * 2.0 - bubble_a_angle);
  double bubble_b_new_angle = NormalizeArc(collision_angle * 2.0 - bubble_b_angle);

//  double bubble_a_speed_s = sqrt(Pow2(bubble_a.speed_x) + Pow2(bubble_a.speed_y));
//  double bubble_b_speed_s = sqrt(Pow2(bubble_b.speed_x) + Pow2(bubble_b.speed_y));
  double bubble_a_mass = Pow2(bubble_a.radius);
  double bubble_b_mass = Pow2(bubble_b.radius);

  double total_x_movement = bubble_a.speed_x * bubble_a_mass + bubble_b.speed_x * bubble_b_mass;
  double total_y_movement = bubble_a.speed_y * bubble_a_mass + bubble_b.speed_y * bubble_b_mass;
//  double total_energy = bubble_a_mass * Pow2(bubble_a_speed_s) + bubble_b_mass * Pow2(bubble_b_speed_s);

  double bubble_a_new_speed_x_ratio = cos(bubble_a_new_angle);
  double bubble_a_new_speed_y_ratio = sin(bubble_a_new_angle);
  double bubble_b_new_speed_x_ratio = cos(bubble_b_new_angle);
  double bubble_b_new_speed_y_ratio = sin(bubble_b_new_angle);
/*
  double magnitude_a;
  double magnitude_b;

  double bubble_a_new_speed_x = bubble_a_new_speed_x_ratio * magnitude_a;
  double bubble_a_new_speed_y = bubble_a_new_speed_y_ratio * magnitude_a;
  double bubble_b_new_speed_x = bubble_b_new_speed_x_ratio * magnitude_b;
  double bubble_b_new_speed_y = bubble_b_new_speed_y_ratio * magnitude_b;

  bubble_a_new_speed_x * bubble_a_mass + bubble_b_new_speed_x * bubble_b_mass == total_x_movement;
  bubble_a_new_speed_y * bubble_a_mass + bubble_b_new_speed_y * bubble_b_mass == total_y_movement;
  double bubble_a_new_speed_s = sqrt(Pow2(bubble_a_new_speed_x) + Pow2(bubble_a_new_speed_y));
  double bubble_b_new_speed_s = sqrt(Pow2(bubble_b_new_speed_x) + Pow2(bubble_b_new_speed_y));
  bubble_a_mass * Pow2(bubble_a_new_speed_s) + bubble_b_mass * Pow2(bubble_b_new_speed_s) == total_energy;

  bubble_a_new_speed_x_ratio * magnitude_a * bubble_a_mass +
  bubble_b_new_speed_x_ratio * magnitude_b * bubble_b_mass == total_x_movement;

  bubble_a_new_speed_y_ratio * magnitude_a * bubble_a_mass +
  bubble_b_new_speed_y_ratio * magnitude_b * bubble_b_mass == total_y_movement;

  bubble_a_mass * (Pow2(bubble_a_new_speed_x_ratio * magnitude_a) + Pow2(bubble_a_new_speed_y_ratio * magnitude_a)) +
  bubble_b_mass * (Pow2(bubble_b_new_speed_x_ratio * magnitude_b) + Pow2(bubble_b_new_speed_y_ratio * magnitude_b)) == total_energy;

  magnitude_a * (bubble_a_new_speed_x_ratio * bubble_a_mass) +
  magnitude_b * (bubble_b_new_speed_x_ratio * bubble_b_mass) == total_x_movement;

  magnitude_a * (bubble_a_new_speed_y_ratio * bubble_a_mass) +
  magnitude_b * (bubble_b_new_speed_y_ratio * bubble_b_mass) == total_y_movement;
*//*
  auto result = SolveLinearEquation(bubble_a_new_speed_x_ratio * bubble_a_mass,
                                    bubble_b_new_speed_x_ratio * bubble_b_mass,
                                    total_x_movement,
                                    bubble_a_new_speed_y_ratio * bubble_a_mass,
                                    bubble_b_new_speed_y_ratio * bubble_b_mass,
                                    total_y_movement);

  bubble_a.speed_x = result.first * bubble_a_new_speed_x_ratio;
  bubble_a.speed_y = result.first * bubble_a_new_speed_y_ratio;
  bubble_b.speed_x = result.second * bubble_b_new_speed_x_ratio;
  bubble_b.speed_y = result.second * bubble_b_new_speed_y_ratio;
*/
  bubble_a.pos_x = bubble_a.pos_x + bubble_a.speed_x * (time - collision_time);
  bubble_b.pos_y = bubble_b.pos_y + bubble_b.speed_y * (time - collision_time);
  bubble_b.pos_x = bubble_b.pos_x + bubble_b.speed_x * (time - collision_time);
  bubble_a.pos_y = bubble_a.pos_y + bubble_a.speed_y * (time - collision_time);

  bubble_a.CleanPreNext();
  bubble_b.CleanPreNext();
}

void Canvas::AdvanceTime(double time) const {
  double once_time = time;
  double done_time = 0.0;
StartPoint:
  std::for_each(bubbles, bubbles_end, [once_time](Bubble* const bubble) mutable -> void {
    bubble->SetPreNextPos(once_time);
  });
  for (auto bubble_a_ref = bubbles; bubble_a_ref != bubbles_end; ++bubble_a_ref) {
    Bubble& bubble_a = **bubble_a_ref;
    REPEAT(side_id, 4) {
      if (CheckSpecificSideOutOfBoundsNext(bubble_a, (CanvasSide)side_id)) {
        if (bubble_a.ConflictSetAndRet()) goto TooManyConflicts;
        bubble_a.pre_next.wall_conflict = side_id;
      }
    }
    for (auto bubble_b_ref = bubble_a_ref + 1; bubble_b_ref != bubbles_end; ++bubble_b_ref) {
      Bubble& bubble_b = **bubble_b_ref;
      if (CheckTwoBubblesNextShock(bubble_a, bubble_b)) {
        if (bubble_a.ConflictSetAndRet()) goto TooManyConflicts;
        if (bubble_b.ConflictSetAndRet()) goto TooManyConflicts;
        bubble_a.pre_next.bubble_conflict = &bubble_b;
      }
    }
  }
  std::for_each(bubbles, bubbles_end, [this, once_time](Bubble* bubble) mutable -> void {
    if (!bubble->pre_next.conflict) {
      bubble->NoConflictAdvance();
    } else {
      if (bubble->pre_next.wall_conflict < 4) {
        HandleBubbleWallShock(*bubble, once_time);
      } else if (bubble->pre_next.bubble_conflict) {
        HandleTwoBubblesShock(*bubble, *(bubble->pre_next.bubble_conflict), once_time);
      }
    }
  });
  done_time += once_time;
  if (done_time >= time) return;
  else {
    once_time = time - done_time;
  }
TooManyConflicts:
  std::for_each(bubbles, bubbles_end, [](Bubble* bubble) mutable -> void {
    bubble->CleanPreNext();
  });
  once_time = (time - done_time) / 2.0;
  goto StartPoint;
}


#pragma comment(lib, "msimg32")

void Canvas::PaintEverything(HDC hdc) const {
  SelectObject(hdc, background);
  SelectObject(hdc, GetStockObject(NULL_PEN));
  Rectangle(hdc, 0, 0, WINDOW_SIZE_X, WINDOW_SIZE_Y);
  std::for_each(bubbles, bubbles_end, [hdc, this](Bubble* bubble) mutable -> void {
    HDC old = (HDC)SelectObject(hdc, bubble->drawing);
    TransparentBlt(hdc, bubble->pos_x - bubble->radius, bubble->pos_y - bubble->radius, bubble->radius * 2, bubble->radius * 2, bubble->drawing, 0, 0, bubble->radius * 2, bubble->radius * 2, RGB(255, 255, 255));
//    BitBlt(hdc, bubble->pos_x - bubble->radius, bubble->pos_y - bubble->radius, bubble->radius * 2, bubble->radius * 2, bubble->drawing, 0, 0, SRCPAINT);
    SelectObject(hdc, old);
/*
    SelectObject(hdc, bubble->fill);
    SelectObject(hdc, bubble->outline);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    Ellipse(hdc, bubble->pos_x - bubble->radius, bubble->pos_y - bubble->radius, bubble->pos_x + bubble->radius, bubble->pos_y + bubble->radius);
    Ellipse(hdc, bubble->pre_next.pos_x - bubble->radius, bubble->pre_next.pos_y - bubble->radius, bubble->pre_next.pos_x + bubble->radius, bubble->pre_next.pos_y + bubble->radius);*/
  });
}


Bubble::Bubble(double radius, double pos_x, double pos_y, double speed_x, double speed_y) :
    radius(radius), pos_x(pos_x), pos_y(pos_y), speed_x(speed_x), speed_y(speed_y) {}

void Bubble::PreDraw(unsigned outline_color, unsigned fill_color, HDC base_hdc) {
  HPEN outline = CreatePen(PS_SOLID, 1, outline_color);
  HBRUSH fill = CreateSolidBrush(fill_color);
  HBITMAP bitmap = CreateCompatibleBitmap(base_hdc, radius * 2, radius * 2);
  drawing = CreateCompatibleDC(base_hdc);
  SelectObject(drawing, bitmap);
  RECT rect{0, 0, radius * 2.0, radius * 2.0};
  FillRect(drawing, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
  SelectObject(drawing, fill);
  SelectObject(drawing, outline);
  Ellipse(drawing, 0, 0, radius * 2.0, radius * 2.0);
  DeleteObject(outline);
  DeleteObject(fill);
  DeleteObject(bitmap);
}

Bubble::~Bubble() {
  if (drawing) DeleteObject(drawing);
}


void Bubble::SetPreNextPos(double time) {
  pre_next.pos_x = pos_x + speed_x * time;
  pre_next.pos_y = pos_y + speed_y * time;
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

  double bhaskara_a = Pow2(bubble_a.speed_x - bubble_b.speed_x)
                    + Pow2(bubble_a.speed_y - bubble_b.speed_y);

  double bhaskara_b = 2.0 * (  (bubble_a.speed_x - bubble_b.speed_x * (bubble_a.pos_x - bubble_b.pos_x))
                             + (bubble_a.speed_y - bubble_b.speed_y * (bubble_a.pos_y - bubble_b.pos_y)));

  double bhaskara_c = Pow2(bubble_a.pos_x - bubble_b.pos_x) + Pow2(bubble_a.pos_y - bubble_b.pos_y) - Pow2(bubble_a.radius + bubble_b.radius);

  double bhaskara_delta_sqrt = sqrt(Pow2(bhaskara_b) - 4 * bhaskara_a * bhaskara_c);

  double radix_1 = - (- bhaskara_b + bhaskara_delta_sqrt) / (2 * bhaskara_a);

  double radix_2 = - (- bhaskara_b - bhaskara_delta_sqrt) / (2 * bhaskara_a);


  return 0.5;
  return radix_1;

}


bool CheckTwoBubblesShockWorker(double a_pos_x, double a_pos_y, double b_pos_x, double b_pos_y, double a_radius, double b_radius) {
  if (((a_pos_x + a_radius < b_pos_x - b_radius) || (a_pos_x - a_radius > b_pos_x + b_radius)) && ((a_pos_y + a_radius < b_pos_y - b_radius) || (a_pos_y - a_radius > b_pos_y + b_radius))) return false;
  else return (Pow2(a_pos_x - b_pos_x) + Pow2(a_pos_y - b_pos_y) < Pow2(a_radius + b_radius));
}

