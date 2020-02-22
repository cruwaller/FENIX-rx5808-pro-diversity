#ifndef RECEIVER_SPI_H
#define RECEIVER_SPI_H

#include <stdint.h>
#include <SPI.h>
#include "receiver.h"

namespace ReceiverSpi {
  void setSynthRegisterB(uint16_t value);
  void setPowerDownRegister(uint32_t value);
  void setStateRegister(uint32_t value);

  void rxStandby(Receiver::ReceiverId ReceiverId);
  void rxPowerOn(Receiver::ReceiverId ReceiverId);
};


#endif
