#include <Arduino.h>
#include <stdint.h>

#include "receiver_spi.h"
#include "settings.h"

static inline void sendRegister(uint8_t address, uint32_t data, Receiver::ReceiverId rx_id=Receiver::ReceiverId::ALL);

#define SPI_ADDRESS_SYNTH_B 0x01
#define SPI_ADDRESS_POWER   0x0A
#define SPI_ADDRESS_STATE   0x0F

namespace ReceiverSpi {
    //
    // Sends SPI command to receiver module to change frequency.
    //
    // Format is LSB first, with the following bits in order:
    //     4 bits - address
    //     1 bit  - read/write enable
    //    20 bits - data
    //
    // Address for frequency select (Synth Register B) is 0x1
    // Expected data is (LSB):
    //     7 bits - A counter divider ratio
    //      1 bit - seperator
    //    12 bits - N counter divder ratio
    //
    // Forumla for calculating N and A is:/
    //    F_lo = 2 * (N * 32 + A) * (F_osc / R)
    //    where:
    //        F_osc = 8 Mhz
    //        R = 8
    //
    // Refer to RTC6715 datasheet for further details.
    //
    void setSynthRegisterB(uint16_t value) {
        sendRegister(SPI_ADDRESS_SYNTH_B, value);
    }

    void setPowerDownRegister(uint32_t value) {
        sendRegister(SPI_ADDRESS_POWER, value);
    }

    void setStateRegister(uint32_t value) {
        sendRegister(SPI_ADDRESS_STATE, value);
    }

    void rxStandby(Receiver::ReceiverId ReceiverId) {
        sendRegister(SPI_ADDRESS_STATE, 0b00000000000000000011, ReceiverId);
    }

    void rxPowerOn(Receiver::ReceiverId ReceiverId) {
        sendRegister(SPI_ADDRESS_STATE, 0b00000000000000000001, ReceiverId);
    }
}


static inline void sendRegister(uint8_t addressBits, uint32_t dataBits, Receiver::ReceiverId rx_id) {

    uint32_t data = addressBits | (1 << 4) | (dataBits << 5);

    switch (rx_id) {
    case Receiver::ReceiverId::A:
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_A, LOW);
        break;
    case Receiver::ReceiverId::B:
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_B, LOW);
        break;
    default:
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_A, LOW);
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_B, LOW);
        break;
    }

    SPI.beginTransaction(SPISettings(1000000, LSBFIRST, SPI_MODE0));
    SPI.transferBits(data, NULL, 25);
    SPI.endTransaction();

    switch (rx_id) {
    case Receiver::ReceiverId::A:
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_A, HIGH);
        break;
    case Receiver::ReceiverId::B:
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_B, HIGH);
        break;
    default:
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_A, HIGH);
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_B, HIGH);
        break;
    }
}
