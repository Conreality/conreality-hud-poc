/* This is free and unencumbered software released into the public domain. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef LEAPFUNCS_H
#define LEAPFUNCS_H

#include "Leap.h"

class leap_event_listener : public Leap::Listener {
public:
  virtual void onConnect(const Leap::Controller&);
  virtual void onDisconnect(const Leap::Controller&);
  virtual void onFrame(const Leap::Controller&);
};

#endif //LEAPFUNCS_H
