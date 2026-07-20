#pragma once

#include "domain/FeedRequest.h"
#include "ports/IFeederActuator.h"
#include "ports/ILogger.h"

class FeedController {
 public:
  FeedController(ILogger& logger, IFeederActuator& actuator);
  bool serve(const FeedRequest& request);

 private:
  ILogger& logger_;
  IFeederActuator& actuator_;
};
