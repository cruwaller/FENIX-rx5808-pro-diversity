#ifndef RECEIVER_SPI_H
#define RECEIVER_SPI_H

#include <stdint.h>
#include <SPI.h>
#include "receiver.h"

namespace ReceiverSpi {
  void setup(void);

  void setSynthRegisterB(uint32_t freq);

  void rxVideoOff(Receiver::ReceiverId ReceiverId);
  void rxPowerOff(Receiver::ReceiverId ReceiverId);
  void rxPowerUp(Receiver::ReceiverId ReceiverId);
  void rxStandby(Receiver::ReceiverId ReceiverId);
  void rxWakeup(Receiver::ReceiverId ReceiverId);
};


#endif
