#include <Arduino.h>
#include "src/Snake.h"
#include <CircuitOS.h>
#include <Input/I2cExpander.h>
#include <Input/InputI2C.h>
#include <Loop/LoopManager.h>
#include <ByteBoi.h>
//#include "Nibble.hpp"
#include <Audio/Piezo.h>
#include <SleepService.h>
Snake::Snake* game;

void setup()
{
	Serial.begin(115200);
	ByteBoi.begin();
	Sleep.begin();
	ByteBoi.bindMenu();
	BatteryPopup.enablePopups(true);
	ByteBoi.setGameID("Snake");
	game=new Snake::Snake(ByteBoi.getDisplay());
	game->unpack();
	ByteBoi.splash();
	game->start();
}

void loop()
{
	LoopManager::loop();
}