/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "globals.h"
#include "leapfuncs.h"

extern Globals global;

void leap_event_listener::onConnect(const Leap::Controller& controller) {
  std::cout << "LeapMotion connected" << std::endl;
}

void leap_event_listener::onDisconnect(const Leap::Controller& controller) {
  std::cout << "LeapMotion disconnected" << std::endl;
}

void leap_event_listener::onFrame(const Leap::Controller& controller) {
  Leap::Frame frame = controller.frame();
  Leap::HandList hands = frame.hands();
  Leap::PointableList pointables = frame.pointables();
  Leap::FingerList fingers = frame.fingers();
  Leap::ToolList tools = frame.tools();
  Leap::GestureList gestures = frame.gestures();

  int count = 0;
  Leap::Vector average = Leap::Vector();
  Leap::Finger finger_to_average = frame.fingers()[0];
  for (int i = 0; i < 10; i++) {
    Leap::Finger finger_from_frame = controller.frame(i).finger(finger_to_average.id());
    if (finger_from_frame.isValid()) {
      average += finger_from_frame.tipPosition();
      count++;
    }
  }

/*for (Leap::GestureList::const_iterator i = gestures.begin(); i != gestures.end(); i ++) {
    std::cout << (*i).type() << std::endl;
    if ((*i).type() == Leap::Gesture::TYPE_SWIPE) { global.flags.show_items = true; }
  }
*/

  average /= count;
/*std::cout << "average position: " << average << std::endl;
  std::cout << "hand count: " << hands.count() << std::endl;

  for (Leap::HandList::const_iterator i = hands.begin(); i != hands.end(); i++) {
    std::cout << "pinch strength: " << (*i).pinchStrength() << std::endl;
    if ((*i).pinchStrength() == 1) { global.flags.is_running = false; }
    std::cout << "palm direction: " << (*i).palmNormal() << std::endl;
  }
  std::cout << "--------------------" << std::endl;
*/
}


