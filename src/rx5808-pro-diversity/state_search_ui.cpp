#include "state_search.h"

#include "receiver.h"
#include "channels.h"
#include "ui.h"
#include "pstr_helper.h"
#include "settings_eeprom.h"


#define BORDER_GRAPH_L_X 59

#define CHANNEL_TEXT_SIZE 4
#define CHANNEL_TEXT_X 3
#define CHANNEL_TEXT_Y 4
#define CHANNEL_TEXT_H (CHAR_HEIGHT * CHANNEL_TEXT_SIZE)

#define FREQUENCY_TEXT_SIZE 2
#define FREQUENCY_TEXT_X 2
#define FREQUENCY_TEXT_Y (SCREEN_HEIGHT - (CHAR_HEIGHT * 3)-4)
#define FREQUENCY_TEXT_H (CHAR_HEIGHT * FREQUENCY_TEXT_SIZE)

#define SCANBAR_BORDER_X 0
#define SCANBAR_BORDER_Y SCREEN_HEIGHT - 1 - 6 //(CHANNEL_TEXT_H + 4)
#define SCANBAR_BORDER_W (BORDER_GRAPH_L_X - 8)
#define SCANBAR_BORDER_H 7

#define SCANBAR_X (SCANBAR_BORDER_X + 2)
#define SCANBAR_Y (SCANBAR_BORDER_Y + 2)
#define SCANBAR_W (SCANBAR_BORDER_W - 4)
#define SCANBAR_H (SCANBAR_BORDER_H - 4)

#define GRAPH_SEPERATOR_Y SCREEN_HEIGHT_MID
#define GRAPH_SEPERATOR_W (SCREEN_WIDTH - BORDER_GRAPH_L_X)
#define GRAPH_SEPERATOR_STEP 3

#define GRAPH_X (BORDER_GRAPH_L_X + 0)
#define GRAPH_W (SCREEN_WIDTH - BORDER_GRAPH_L_X)

uint8_t GRAPH_H;
uint8_t GRAPH_A_Y;
uint8_t GRAPH_B_Y;
uint8_t GRAPH_C_Y;
uint8_t GRAPH_D_Y;

uint8_t RX_TEXT_SIZE;
uint8_t RX_TEXT_X;
uint8_t RX_TEXT_H;
uint8_t RX_TEXT_A_Y;
uint8_t RX_TEXT_B_Y;
uint8_t RX_TEXT_C_Y;
uint8_t RX_TEXT_D_Y;
 

void StateMachine::SearchStateHandler::onInitialDraw() {
     
if (!EepromSettings.quadversity) {
    GRAPH_H = (GRAPH_SEPERATOR_Y - 2);
    GRAPH_A_Y = 0;
    GRAPH_B_Y = (SCREEN_HEIGHT - GRAPH_H - 1);

    RX_TEXT_SIZE = 1;
    RX_TEXT_X = (BORDER_GRAPH_L_X + 4);
    RX_TEXT_H = (CHAR_HEIGHT * RX_TEXT_SIZE);
    RX_TEXT_A_Y = ((GRAPH_A_Y + GRAPH_H / 2) - (RX_TEXT_H / 2));
    RX_TEXT_B_Y = ((GRAPH_B_Y + GRAPH_H / 2) - (RX_TEXT_H / 2));
}

if (EepromSettings.quadversity) {
    GRAPH_H = SCREEN_HEIGHT / 4;
    GRAPH_A_Y = 0;
    GRAPH_B_Y = (1 * GRAPH_H - 1);
    GRAPH_C_Y = (2 * GRAPH_H - 1);
    GRAPH_D_Y = (3 * GRAPH_H - 1);

    RX_TEXT_SIZE = 1;
    RX_TEXT_X = (BORDER_GRAPH_L_X + 4);
    RX_TEXT_H = (CHAR_HEIGHT * RX_TEXT_SIZE);
    RX_TEXT_A_Y = ((GRAPH_A_Y + GRAPH_H / 2) - (RX_TEXT_H / 2));
    RX_TEXT_B_Y = ((GRAPH_B_Y + GRAPH_H / 2) - (RX_TEXT_H / 2));
    RX_TEXT_C_Y = ((GRAPH_C_Y + GRAPH_H / 2) - (RX_TEXT_H / 2));
    RX_TEXT_D_Y = ((GRAPH_D_Y + GRAPH_H / 2) - (RX_TEXT_H / 2));
}

    Ui::clear();

    Ui::setTextSize(RX_TEXT_SIZE);
//    Ui::setTextColor(INVERSE);

    Ui::setCursor(RX_TEXT_X-11, RX_TEXT_A_Y-4);
    Ui::display.print(PSTR2("A"));

    Ui::setCursor(RX_TEXT_X-11, RX_TEXT_B_Y-4);
    Ui::display.print(PSTR2("B"));

    if (EepromSettings.quadversity) {
      Ui::setCursor(RX_TEXT_X-11, RX_TEXT_C_Y-4);
      Ui::display.print(PSTR2("C"));
  
      Ui::setCursor(RX_TEXT_X-11, RX_TEXT_D_Y-4);
      Ui::display.print(PSTR2("D"));
    }

    drawBorders();

    drawChannelText();
    drawFrequencyText();
    drawScanBar();
    drawRssiGraph();

    Ui::needDisplay();
}

void StateMachine::SearchStateHandler::onUpdateDraw() {
    
    Ui::clearRect(
        CHANNEL_TEXT_X,
        CHANNEL_TEXT_Y,
        BORDER_GRAPH_L_X - 10,
        CHANNEL_TEXT_H
    );

    Ui::clearRect(
        FREQUENCY_TEXT_X,
        FREQUENCY_TEXT_Y,
        BORDER_GRAPH_L_X - 10,
        CHAR_HEIGHT * 2
    );

    Ui::clearRect(
        SCANBAR_X,
        SCANBAR_Y,
        SCANBAR_W,
        SCANBAR_H
    );

    drawScanBar();
    drawRssiGraph();
    drawChannelText();
    drawFrequencyText();
    menu.draw();

    Ui::needDisplay();
}

void StateMachine::SearchStateHandler::drawBorders() {
    Ui::drawRoundRect(
        SCANBAR_BORDER_X,
        SCANBAR_BORDER_Y,
        SCANBAR_BORDER_W,
        SCANBAR_BORDER_H,
        2,
        WHITE
    );

    Ui::drawDashedVLine(
        BORDER_GRAPH_L_X-1,
        0,
        SCREEN_HEIGHT,
        GRAPH_SEPERATOR_STEP
    );

    if (!EepromSettings.quadversity) {
        Ui::drawDashedHLine(
            GRAPH_X,
            GRAPH_SEPERATOR_Y,
            GRAPH_SEPERATOR_W,
            GRAPH_SEPERATOR_STEP
        );

        Ui::drawDashedHLine(
            GRAPH_X,
            GRAPH_SEPERATOR_Y + GRAPH_SEPERATOR_Y + 1,
            GRAPH_SEPERATOR_W,
            GRAPH_SEPERATOR_STEP
        );
    }
    if (EepromSettings.quadversity) {
        Ui::drawDashedHLine(
            GRAPH_X,
            GRAPH_SEPERATOR_Y + GRAPH_SEPERATOR_Y / 2 + 1,
            GRAPH_SEPERATOR_W,
            GRAPH_SEPERATOR_STEP
        );

        Ui::drawDashedHLine(
            GRAPH_X,
            GRAPH_SEPERATOR_Y,
            GRAPH_SEPERATOR_W,
            GRAPH_SEPERATOR_STEP
        );

        Ui::drawDashedHLine(
            GRAPH_X,
            GRAPH_SEPERATOR_Y - GRAPH_SEPERATOR_Y / 2 - 1,
            GRAPH_SEPERATOR_W,
            GRAPH_SEPERATOR_STEP
        );

        Ui::drawDashedHLine(
            GRAPH_X,
            GRAPH_SEPERATOR_Y + GRAPH_SEPERATOR_Y + 1,
            GRAPH_SEPERATOR_W,
            GRAPH_SEPERATOR_STEP
        );
    }
}

void StateMachine::SearchStateHandler::drawChannelText() {
    Ui::setTextSize(CHANNEL_TEXT_SIZE);
    Ui::setTextColor(WHITE);
    Ui::setCursor(CHANNEL_TEXT_X, CHANNEL_TEXT_Y);

    Ui::display.print(Channels::getName(Receiver::activeChannel));
}

void StateMachine::SearchStateHandler::drawFrequencyText() {
    Ui::setTextSize(FREQUENCY_TEXT_SIZE);
    Ui::setTextColor(WHITE);
    Ui::setCursor(FREQUENCY_TEXT_X, FREQUENCY_TEXT_Y);

    Ui::display.print(Channels::getFrequency(Receiver::activeChannel));
}

void StateMachine::SearchStateHandler::drawScanBar() {
    uint8_t scanWidth = orderedChanelIndex * SCANBAR_W / CHANNELS_SIZE;
  
    Ui::fillRect(
        SCANBAR_X + scanWidth + 1,
        SCANBAR_Y,
        SCANBAR_W - scanWidth,
        SCANBAR_H,
        BLACK
    ); 
     
    Ui::fillRect(
        SCANBAR_X,
        SCANBAR_Y,
        scanWidth,
        SCANBAR_H,
        WHITE
    );
}

void StateMachine::SearchStateHandler::drawRssiGraph() {
    if (!EepromSettings.quadversity) {
        Ui::drawGraph(
            Receiver::rssiBLast,
            RECEIVER_LAST_DATA_SIZE,
            100,
            GRAPH_X,
            GRAPH_A_Y+1,
            GRAPH_W+1,
            GRAPH_H
        );
  
        Ui::drawGraph(
            Receiver::rssiALast,
            RECEIVER_LAST_DATA_SIZE,
            100,
            GRAPH_X,
            GRAPH_B_Y-1,
            GRAPH_W+1,
            GRAPH_H
        );

        Ui::setTextSize(RX_TEXT_SIZE);
//        Ui::setTextColor(INVERSE);

    }      
    if (EepromSettings.quadversity) {
          
        Ui::drawGraph(
            Receiver::rssiALast,
            RECEIVER_LAST_DATA_SIZE,
            100,
            GRAPH_X,
            GRAPH_A_Y,
            GRAPH_W - 1,
            GRAPH_H - 2
        );

        Ui::drawGraph(
            Receiver::rssiBLast,
            RECEIVER_LAST_DATA_SIZE,
            100,
            GRAPH_X,
            GRAPH_B_Y + 1,
            GRAPH_W - 1,
            GRAPH_H - 2
        );

        Ui::drawGraph(
            Receiver::rssiCLast,
            RECEIVER_LAST_DATA_SIZE,
            100,
            GRAPH_X,
            GRAPH_C_Y + 1,
            GRAPH_W - 1,
            GRAPH_H - 2
        );

        Ui::drawGraph(
            Receiver::rssiDLast,
            RECEIVER_LAST_DATA_SIZE,
            100,
            GRAPH_X,
            GRAPH_D_Y + 1,
            GRAPH_W - 1,
            GRAPH_H - 2
        );
    }
}

void StateMachine::SearchStateHandler::drawMenu() {
    this->menu.draw();
}
