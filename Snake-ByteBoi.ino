#include <Arduino.h>
#include "src/Snake.h"
#include <CircuitOS.h>
#include <Input/I2cExpander.h>
#include <Input/InputI2C.h>
#include <Loop/LoopManager.h>
#include <ByteBoi.h>
//#include "Nibble.hpp"
#include <Audio/Piezo.h>

Snake::Snake* game;

void setup()
{
	Serial.begin(115200);
	ByteBoi.begin();
	ByteBoi.getDisplay()->commit();
	Piezo.begin(25);
	LoopManager::addListener(Input::getInstance());
	game=new Snake::Snake(ByteBoi.getDisplay());
	LoopManager::addListener(game);
	game->unpack();
	game->start();
}

void loop()
{
	LoopManager::loop();
    yield();
}