#ifndef RECEIVER_SPI_H
#define RECEIVER_SPI_H

#include <stdint.h>
#include "receiver.h"

namespace ReceiverSpi {
  void setup(void);

  void IRAM_ATTR setSynthRegisterB(uint32_t freq, Receiver::ReceiverId ReceiverId);

  void IRAM_ATTR rxVideoOff(Receiver::ReceiverId ReceiverId);
  void IRAM_ATTR rxPowerOff(Receiver::ReceiverId ReceiverId);
  void IRAM_ATTR rxPowerUp(Receiver::ReceiverId ReceiverId);
  void IRAM_ATTR rxStandby(Receiver::ReceiverId ReceiverId);
  void IRAM_ATTR rxWakeup(Receiver::ReceiverId ReceiverId);
};


#endif
