#include "perception/filter/ball_filter.h"

#include "raw_ball.h"

namespace perception {

BallFilter::BallFilter(const RawBall& ball) : last_ball_(ball) { update(ball); }

bool BallFilter::update(const RawBall& raw_ball) {
  // TODO(#ISSUE_N): Implement the ball filter update to determine if the raw_ball should be
  // accepted as the same ball it was tracking before.
  last_ball_ = raw_ball;
  return true;
}

RawBall BallFilter::getBall() const { return last_ball_; }

} // namespace perception
