#include <Arduino.h>
#include <stdint.h>

#include "receiver_spi.h"
#include "settings.h"


static void sendRegister(uint8_t address, uint32_t data, Receiver::ReceiverId rx_id=Receiver::ReceiverId::ALL);
static uint32_t converteFreqToSynthRegisterB(uint32_t freq);

#define SPI_ADDRESS_SYNTH_B 0x01
#define SPI_ADDRESS_SYNTH_C 0x02
#define SPI_ADDRESS_SYNTH_D 0x03
#define SPI_ADDRESS_VCO     0x04
#define SPI_ADDRESS_DFC     0x05
#define SPI_ADDRESS_6M      0x06 // 6M Audio Demodulator Control Register
#define SPI_ADDRESS_6M5     0x07 // 6M5 Audio Demodulator Control Register
#define SPI_ADDRESS_RCVR1   0x08 // Receiver Control Register 1
#define SPI_ADDRESS_RCVR2   0x09 // Receiver Control Register 2
#define SPI_ADDRESS_POWER   0x0A // Power Down Control Register
#define SPI_ADDRESS_STATE   0x0F // State Register


#define RX5808_STATE_RESET      0b000
#define RX5808_STATE_PWRON_CAL  0b001
#define RX5808_STATE_STBY       0b010
#define RX5808_STATE_VCO_CAL    0b011

namespace ReceiverSpi {

    static void setPowerDownRegister(uint32_t value, Receiver::ReceiverId ReceiverId)
    {
#ifdef DISABLE_AUDIO
        value |= 0b00000000000111100000; // Put PD_6M5, PD_AU6M5, PD_6M, PD_AU6M to off state
#endif
        sendRegister(SPI_ADDRESS_POWER, value, ReceiverId);
    }

    static void setStateRegister(uint32_t value, Receiver::ReceiverId ReceiverId)
    {
        sendRegister(SPI_ADDRESS_STATE, value, ReceiverId);
    }

    void setup(void)
    {
        /* Init receivers registers */
    }

    void setSynthRegisterB(uint32_t freq)
    {
        uint32_t value = converteFreqToSynthRegisterB(freq);
        sendRegister(SPI_ADDRESS_SYNTH_B, value);
    }

    void rxVideoOff(Receiver::ReceiverId ReceiverId)
    {
        setPowerDownRegister(0b01010000110000010011, ReceiverId); // Put PD_VAMP to off state
    }

    void rxPowerOff(Receiver::ReceiverId ReceiverId)
    {
        setPowerDownRegister(0b11111111111111111111, ReceiverId);
    }

    void rxPowerUp(Receiver::ReceiverId ReceiverId)
    {
        setPowerDownRegister(0b00010000110000010011, ReceiverId);
    }

    void rxStandby(Receiver::ReceiverId ReceiverId)
    {
        //setStateRegister(RX5808_STATE_VCO_CAL, ReceiverId);
        setStateRegister(RX5808_STATE_STBY, ReceiverId);
    }

    void rxWakeup(Receiver::ReceiverId ReceiverId)
    {
        setStateRegister(RX5808_STATE_PWRON_CAL, ReceiverId);
    }
}


static void sendRegister(uint8_t addressBits, uint32_t dataBits, Receiver::ReceiverId rx_id)
{
    // LSB first
    uint32_t data = addressBits | (0x1 << 4) | (dataBits << 5);

    switch (rx_id) {
    case Receiver::ReceiverId::A:
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_A, LOW);
        break;
    case Receiver::ReceiverId::B:
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_B, LOW);
        break;
    case Receiver::ReceiverId::C:
#if defined(PIN_SPI_SLAVE_SELECT_RX_C)
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_C, LOW);
#endif
        break;
    case Receiver::ReceiverId::D:
#if defined(PIN_SPI_SLAVE_SELECT_RX_D)
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_D, LOW);
#endif
        break;
    default:
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_A, LOW);
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_B, LOW);
#if defined(PIN_SPI_SLAVE_SELECT_RX_C) && defined(PIN_SPI_SLAVE_SELECT_RX_D)
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_C, LOW);
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_D, LOW);
#endif
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
    case Receiver::ReceiverId::C:
#if defined(PIN_SPI_SLAVE_SELECT_RX_C)
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_C, HIGH);
#endif
        break;
    case Receiver::ReceiverId::D:
#if defined(PIN_SPI_SLAVE_SELECT_RX_D)
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_D, HIGH);
#endif
    default:
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_A, HIGH);
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_B, HIGH);
#if defined(PIN_SPI_SLAVE_SELECT_RX_C) && defined(PIN_SPI_SLAVE_SELECT_RX_D)
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_C, HIGH);
        digitalWrite(PIN_SPI_SLAVE_SELECT_RX_D, HIGH);
#endif
        break;
    }
}

static uint32_t converteFreqToSynthRegisterB(uint32_t freq)
{
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
    return ((((freq - 479) / 2) / 32) << 7) | (((freq - 479) / 2) % 32);
}
