#include "application/FeedController.h"

FeedController::FeedController(ILogger& logger, IFeederActuator& actuator)
    : logger_(logger), actuator_(actuator) {}

bool FeedController::serve(const FeedRequest& request) {
  if (request.portionCount == 0) {
    logger_.error("La cantidad de porciones debe ser mayor a cero.");
    return false;
  }

  logger_.info("Solicitud de alimento recibida.");
  const bool wasDispensed = actuator_.dispense(request.portionCount);
  if (!wasDispensed) {
    logger_.error("No se pudo dispensar alimento.");
  }
  return wasDispensed;
}
