#include "Snake.h"
#include "Highscore.h"
#include <ByteBoi.h>
#include <SD.h>
#include <SPI.h>
#include <Playback/PlaybackSystem.h>

Snake::Snake *Snake::Snake::instance = nullptr;
Snake::Snake::Snake(Display *display) : Context(*display), baseSprite(screen.getSprite()),
										buttons(Input::getInstance())
{
	instance = this;
	gamestatus = "title";
	deadTime = 0;
	memset(snakeX, 0, sizeof(snakeX));
	memset(snakeY, 0, sizeof(snakeY));
	snakeLength = 0;

	SD.begin(SD_CS, SPI);

	Samples.game = new Sample(SD.open(ByteBoi.getSDPath() + "/Music/Game.aac"));
	Samples.game->setLooping(true);

	Samples.menu = new Sample(SD.open(ByteBoi.getSDPath() + "/Music/Menu.aac"));
	Samples.menu->setLooping(true);
}
void Snake::Snake::start()
{
	Highscore.begin();
	prevGamestatus = "";
	draw();
	LoopManager::addListener(this);

	if(instance->gamestatus == "oldgame"){
		Playback.open(instance->Samples.game);
		Playback.start();
	}
}
void Snake::Snake::stop()
{
	Playback.stop();
	clearButtonCallbacks();
	LoopManager::removeListener(this);
}
void Snake::Snake::pack(){

}
void Snake::Snake::draw()
{
	if (gamestatus == "title")
	{
		titleScreen();
		drawSnake();
	}
	else if (gamestatus == "newgame")
	{
		oldgame();
	}
	else if (gamestatus == "oldgame")
	{
		oldgame();
	}
	else if(gamestatus == "dead")
	{
		if(deadTime <= 500000)
		{
			oldgame();
		}
		else
		{
			drawDead();
		}
	}
	else if (gamestatus == "paused")
	{
		paused();
	}
	else if(gamestatus == "eraseData")
	{
		eraseDataDraw();
	}
	else if(gamestatus == "dataDisplay")
	{
		dataDisplay();
	}
	else if(gamestatus == "enterInitials")
	{
		enterInitialsDraw();
	}
}
void Snake::Snake::titleScreen()
{
	baseSprite->setTextDatum(textdatum_t::top_center);

	baseSprite->clear(TFT_BLACK);
	baseSprite->setTextColor(TFT_GREEN);
	baseSprite->setFont(&fonts::Font0);
	baseSprite->setTextSize(2);
	baseSprite->drawString("SNAKE", baseSprite->width()/2, 8);
	baseSprite->setTextColor(TFT_LIGHTGREY);
	if (menuSignal == 0 && borderFlag == 1)
		baseSprite->setTextColor(TFT_YELLOW);
	baseSprite->setCursor(25, 30);
	baseSprite->setTextSize(2);
	baseSprite->print("WALL ");
	baseSprite->setTextColor(TFT_LIGHTGREY);
	if (menuSignal == 0 && borderFlag == 0)
		baseSprite->setTextColor(TFT_YELLOW);
	baseSprite->print(" FREE");
	baseSprite->setTextColor(TFT_LIGHTGREY);
	if (menuSignal == 1)
		baseSprite->setTextColor(TFT_YELLOW);
	baseSprite->drawString(String(String("SPEED: ") + String(instance->speed)), baseSprite->width()/2, 52);
	baseSprite->setTextColor(TFT_LIGHTGREY);
	if (menuSignal == 2)
		baseSprite->setTextColor(TFT_YELLOW);
	baseSprite->drawString("SCORES", baseSprite->width()/2, 73);
	baseSprite->setTextColor(TFT_LIGHTGREY);
	if (menuSignal == 3)
		baseSprite->setTextColor(TFT_YELLOW);
	baseSprite->drawString("EXIT", baseSprite->width()/2, 95);
	/*if (speedB)
		speed = 1;
	else
		speed = 2;*/
}
void Snake::Snake::titleSetup()
{
	menuSignal = 0;
	clearButtonCallbacks();
	snakeMenu();
	buttons->setBtnPressCallback(BTN_UP, []() {
		if (instance->menuSignal > 0)
		{
			instance->menuSignal--;
		}
		else if (instance->menuSignal == 0)
		{
			instance->menuSignal = 3;
		}
		Playback.tone(400, 100);
	});

	buttons->setBtnPressCallback(BTN_DOWN, []() {
		if (instance->menuSignal < 3)
		{
			instance->menuSignal++;
		}
		else if (instance->menuSignal == 3)
		{
			instance->menuSignal = 0;
		}
		Playback.tone(400, 100);
	});

	buttons->setBtnPressCallback(BTN_RIGHT, []() {
		if (instance->menuSignal == 0)
		{
			instance->borderFlag = !instance->borderFlag;
		}
		if (instance->menuSignal == 1)
		{
			if (instance->speed < 3)
				instance->speed += 1;
			else
				instance->speed = 1;
		}
		Playback.tone(400, 100);
	});
	buttons->setBtnPressCallback(BTN_LEFT, []() {
		if (instance->menuSignal == 0)
		{
			instance->borderFlag = !instance->borderFlag;
		}
		if (instance->menuSignal == 1)
		{
			if (instance->speed > 1)
				instance->speed -= 1;
			else
				instance->speed = 3;
		}
		Playback.tone(400, 100);
	});

	buttons->setBtnPressCallback(BTN_A, []() {
		if(instance->gamestatus != "title") return ;
		switch (instance->menuSignal)
		{
		case 0:
		case 1:
			if(instance->gamestatus == "title" || instance->gamestatus == "enterInitials"){
				Playback.play(instance->Samples.game);
			}

			instance->gamestatus = "newgame";
			instance->screenChange = true;
			return;
		case 2:
			instance->gamestatus = "dataDisplay";
			break;
		case 3:
			ByteBoi.backToLauncher();
			break;
		}
		Playback.tone(600, 100);
	});

	Playback.play(instance->Samples.menu);
}
void Snake::Snake::drawHead()
{
	if (dirX > 0)
	{
		baseSprite->drawRect(snakeX[0] + 3, snakeY[0] + 1, 2, 3, TFT_RED);
		baseSprite->drawPixel(snakeX[0], snakeY[0] + 1, EYE_COLOR);
		baseSprite->drawPixel(snakeX[0] + 1, snakeY[0] + 1, TFT_BLACK);
		baseSprite->drawPixel(snakeX[0], snakeY[0] + 3, TFT_BLACK);
		baseSprite->drawPixel(snakeX[0] + 1, snakeY[0] + 3, TFT_BLACK);
	}
	else if (instance->dirX < 0)
	{
		baseSprite->fillRect(snakeX[0], snakeY[0] + 1, 2, 3, TFT_RED);
		baseSprite->drawPixel(snakeX[0] + 3, snakeY[0] + 1, TFT_BLACK);
		baseSprite->drawPixel(snakeX[0] + 3, snakeY[0] + 3, TFT_BLACK);
		baseSprite->drawPixel(snakeX[0] + 4, snakeY[0] + 1, TFT_BLACK);
		baseSprite->drawPixel(snakeX[0] + 4, snakeY[0] + 3, TFT_BLACK);
	}
	else if (instance->dirY > 0)
	{
		baseSprite->fillRect(snakeX[0] + 1, snakeY[0] + 3, 3, 2, TFT_RED);
		baseSprite->drawPixel(snakeX[0] + 1, snakeY[0], TFT_BLACK);
		baseSprite->drawPixel(snakeX[0] + 3, snakeY[0], TFT_BLACK);
		baseSprite->drawPixel(snakeX[0] + 1, snakeY[0] + 1, TFT_BLACK);
		baseSprite->drawPixel(snakeX[0] + 3, snakeY[0] + 1, TFT_BLACK);
	}
	else if (instance->dirY < 0)
	{
		baseSprite->fillRect(snakeX[0] + 1, snakeY[0], 3, 2, TFT_RED);
		baseSprite->drawPixel(snakeX[0] + 1, snakeY[0] + 3, TFT_BLACK);
		baseSprite->drawPixel(snakeX[0] + 3, snakeY[0] + 3, TFT_BLACK);
		baseSprite->drawPixel(snakeX[0] + 1, snakeY[0] + 4, TFT_BLACK);
		baseSprite->drawPixel(snakeX[0] + 3, snakeY[0] + 4, TFT_BLACK);
	}
}
void Snake::Snake::loop(uint _time)
{
	if (gamestatus != prevGamestatus)
	{
		screenChange = 1;
		prevGamestatus = gamestatus;
	}
	else
	{
		screenChange = 0;
	}
	if (gamestatus == "title")
	{
		if (screenChange)
		{
			titleSetup();
		}
		titleScreen();
		control();
		snakeMenuControl();
	}
	if (gamestatus == "newgame")
	{
		if (screenChange)
		{
			newGameSetup();
		}
		gamestatus = "oldgame";
	}
	if (gamestatus == "paused")
	{
		if (screenChange)
			pausedSetup();
		paused();
	}
	if (gamestatus == "oldgame")
	{
		if (screenChange)
		{
			setButtonCallbacksGame();
		}
		control();
		if (speed == 3){
			control();
		}
		crash();
		if (eaten){
			drawFood();
		}
		foodCheck();
	}

	if (gamestatus == "dead")
	{
		if(screenChange)
		{
			clearButtonCallbacks();
		}
		deadTime+=_time;
		dead();
	}
	if(gamestatus == "eraseData")
	{
		if(screenChange){
			eraseDataSetup();
		}
		eraseDataUpdate();
	}
	if(gamestatus == "dataDisplay")
	{
		if(screenChange){
			dataDisplaySetup();
		}
	}
	if(gamestatus == "enterInitials")
	{
		if(screenChange){
			enterInitialsSetup();
		}
		enterInitialsUpdate();
	}
	draw();
	screen.commit();
}
void Snake::Snake::control()
{
	if (bigger)
	{
		for (int i = snakeLength - 6; i > 0; i--)
		{
			snakeX[i] = snakeX[i - 1];
			snakeY[i] = snakeY[i - 1];
		}
		for (int i = snakeLength; i > snakeLength - 6; i--)
		{
			snakeX[i] = snakeX[snakeLength - 7];
			snakeY[i] = snakeY[snakeLength - 7];
		}
		bigger = false;
	}
	else
	{
		for (int i = snakeLength; i > 0; i--)
		{
			snakeX[i] = snakeX[i - 1];
			snakeY[i] = snakeY[i - 1];
		}
	}
	snakeX[0] += instance->dirX;
	snakeY[0] += instance->dirY;
}
void Snake::Snake::clearButtonCallbacks()
{
	for (uint8_t i = 0; i < 6; i++)
	{
		buttons->setBtnReleaseCallback(i, nullptr);
		buttons->setBtnPressCallback(i, nullptr);
		buttons->setButtonHeldRepeatCallback(i, 0, nullptr);
		buttons->setButtonHeldCallback(i, 0, nullptr);
	}
}
void Snake::Snake::snakeMenu()
{
	memset(snakeX, 0, sizeof(snakeX));
	memset(snakeY, 0, sizeof(snakeY));
	dirX = 1.3; dirY = 0;
	snakeLength = 30;
}
void Snake::Snake::snakeMenuControl()
{
	if (snakeX[0] < baseSprite->width() - 7 && snakeY[0] <= 1)
	{
		dirX = 1.3 * speed;
		dirY = 0;
	}
	else if (snakeX[0] >= baseSprite->width() - 7 && snakeY[0] < baseSprite->height() - 7)
	{
		dirX = 0;
		dirY = 1.3 * speed;
	}
	else if (snakeX[0] > 3 && snakeY[0] >= baseSprite->height() - 7)
	{
		dirX = -1.3 * speed;
		dirY = 0;
	}
	else if (snakeX[0] <= 1 && snakeY[0] > 3)
	{
		dirX = 0;
		dirY = -1.3 * speed;
	}
}
void Snake::Snake::newGameSetup()
{
	deadTime = 0;
	setButtonCallbacksGame();
	for (int i = 400; i > 0; i--)
	{
		snakeX[i] = baseSprite->width();
		snakeY[i] = baseSprite->height();
		baseSprite->fillRect(snakeX[i], snakeY[i], tileSize, tileSize, TFT_BLACK);
	}
	snakeX[0] = 10;
	snakeY[0] = 63;
	foodX = random(3, baseSprite->width() - 6);
	foodY = random(3, baseSprite->height() - 6);

	dirX = 1.3 * speed;
	dirY = 0;
	snakeLength = 12;
	hScore = 90;
	gamestatus = "oldgame";
}
void Snake::Snake::drawFood()
{
	while (!foodCoolFlag)
	{
		foodX = random(3, baseSprite->width() - 6);
		foodY = random(3, baseSprite->height() - 6);
		for (uint8_t chX = 0; chX < 4; chX++)
		{
			for (uint8_t chY = 0; chY < 4; chY++)
			{
				if (baseSprite->readPixel(chX + foodX, chY + foodY) == 0)
				{
					foodCoolFlag = 1;
				}
				else
				{
					foodCoolFlag = 0;
					break;
				}
			}
			if (foodCoolFlag == 0)
				break;
		}
	}
	foodCoolFlag = 0;
	eaten = false;
}
void Snake::Snake::drawSnake()
{
	if(snakeLength == 0) return;
	for (int i = (snakeLength)-1; i >= 0; i--)
	{
		baseSprite->fillRect(snakeX[i], snakeY[i], tileSize, tileSize, TFT_BLACK);
		if (i > (snakeLength)-10 && i < snakeLength)
			//snakeColor = false;
			snakeColorVal = 1;
		//else if(i < 4 || (i <= (snakeLength) - 6 &&  i % 2 == 1)) snakeColor = 1;
		else if (i >= 0 && i <= (snakeLength - 10))
			//snakeColor = true;
			snakeColorVal = 2;
		else
			snakeColorVal = 3;
		/* if (!foodX == snakeX[i] && !foodY == snakeY[i]) drawFood(); */
		//else snakeColor = 0;
		if (snakeColorVal == 2)
			baseSprite->fillRect(snakeX[i], snakeY[i], tileSize, tileSize, TFT_GREEN);
		else if (snakeColorVal == 1)
			baseSprite->fillRect(snakeX[i], snakeY[i], tileSize, tileSize, TFT_DARKGREEN);
		else
			baseSprite->fillRect(snakeX[i], snakeY[i], tileSize, tileSize, TFT_BLACK);
	}
	drawHead();
}
void Snake::Snake::setButtonCallbacksGame()
{
	clearButtonCallbacks();
	buttons->setBtnPressCallback(BTN_UP, []() {
		if (instance->dirY == 0)
		{
			instance->dirX = 0;
			instance->dirY = -1.3 * instance->speed;
		}
	});
	buttons->setBtnPressCallback(BTN_DOWN, []() {
		if (instance->dirY == 0)
		{
			instance->dirX = 0;
			instance->dirY = 1.3 * instance->speed;
		}
	});
	buttons->setBtnPressCallback(BTN_RIGHT, []() {
		if (instance->dirX == 0)
		{
			instance->dirX = 1.3 * instance->speed;
			instance->dirY = 0;
		}
	});
	buttons->setBtnPressCallback(BTN_LEFT, []() {
		if (instance->dirX == 0)
		{
			instance->dirX = -1.3 * instance->speed;
			instance->dirY = 0;
		}
	});
	buttons->setBtnPressCallback(BTN_B, []() {
		Serial.println("paused");
		instance->gamestatus = "paused";
		Playback.tone(400, 100);
	});
}
void Snake::Snake::foodCheck()
{
	if (baseSprite->readPixel(snakeX[0], snakeY[0]) == TFT_YELLOW)
		eaten = true;
	else if (baseSprite->readPixel(snakeX[0] + 4, snakeY[0]) == TFT_YELLOW)
		eaten = true;
	else if (baseSprite->readPixel(snakeX[0], snakeY[0] + 4) == TFT_YELLOW)
		eaten = true;
	else if (baseSprite->readPixel(snakeX[0] + 4, snakeY[0] + 4) == TFT_YELLOW)
		eaten = true;
	else
		eaten = false;

	if (eaten)
	{
		LED.setRGB(static_cast<LEDColor>(LEDColor::YELLOW));
		snakeLength += 6;
		hScore += (1 * speed);
		Playback.tone(600, 100);
		bigger = true;
	}
	else{
		LED.setRGB(OFF);
	}
}
void Snake::Snake::crash()
{
	if (borderFlag)
	{
		if ((snakeX[0] <= 1 || snakeY[0] <= 1 || snakeX[0] >= baseSprite->width() - 4 || snakeY[0] >= baseSprite->height() - 4)){
			gamestatus = "dead";
			buttons->removeBtnPressCallback(BTN_A);
			buttons->removeBtnPressCallback(BTN_B);
			buttons->removeBtnPressCallback(BTN_UP);
			buttons->removeBtnPressCallback(BTN_DOWN);
			buttons->removeBtnPressCallback(BTN_LEFT);
			buttons->removeBtnPressCallback(BTN_RIGHT);
			Playback.tone(200, 300);
		}
	}
	else
	{
		for (int i = 0; i < snakeLength; i++)
		{
			if (snakeX[i] <= 0 && snakeX[i] > -8)
				snakeX[i] = baseSprite->width() - 1;
			else if (snakeX[i] < -5)
				snakeX[i] = baseSprite->width() - 1;
			else
				snakeX[i] = snakeX[i] % baseSprite->width();
			if (snakeY[i] <= 0 && snakeY[i] > -8)
				snakeY[i] = baseSprite->height() - 1;
			else if (snakeY[i] < -5)
				snakeY[i] = baseSprite->height() - 1;
			else
				snakeY[i] = snakeY[i] % baseSprite->height();
		}
	}
	// itself
	for (int i = 1; i < snakeLength; i++)
	{
		if (((snakeX[0] == snakeX[i + 10] || snakeX[0] == snakeX[i + 10] + 4) && (snakeY[0] == snakeY[i + 10] || snakeY[0] == snakeY[i + 10] + 4))
		|| ((snakeX[0] + 4 == snakeX[i + 10] || snakeX[0] + 4 == snakeX[i + 10] + 4) && (snakeY[0] + 4 == snakeY[i + 10] || snakeY[0] + 4 == snakeY[i + 10] + 4))){
			gamestatus = "dead";
			Playback.tone(200, 300);
		}
	}
}
void Snake::Snake::drawDead()
{
	baseSprite->clear(TFT_BLACK);
	baseSprite->setTextSize(2);
	baseSprite->setFont(&fonts::Font0);
	baseSprite->setTextColor(TFT_RED);
	baseSprite->setTextDatum(textdatum_t::top_center);
	baseSprite->drawString("GAME OVER", baseSprite->width()/2, 5);
	baseSprite->setFont(&fonts::Font2);
	baseSprite->setTextColor(TFT_YELLOW);
	baseSprite->setTextSize(1);
	baseSprite->drawString("Your score:", baseSprite->width()/2, 48);
	baseSprite->setTextSize(2);
	baseSprite->drawString(String(hScore), baseSprite->width()/2, 62);

}
void Snake::Snake::dead()
{
	if(deadTime > 2500000)
	{
		screenChange = true;
		gamestatus = "enterInitials";
	}
}
void Snake::Snake::oldgame()
{
		baseSprite->clear(TFT_BLACK);
		if (borderFlag){
			baseSprite->drawRect(0, 0, baseSprite->width(), baseSprite->height(), TFT_WHITE);
		}

		if (eaten){
			baseSprite->fillRect(foodX, foodY, foodSize, foodSize, TFT_BLACK);
		}
		drawSnake();
		if (eaten){
			drawFood();
		}
		baseSprite->fillRect(foodX, foodY, foodSize, foodSize, TFT_YELLOW);
		baseSprite->setCursor(6, baseSprite->height() - 15);
		baseSprite->setTextSize(1);
		baseSprite->setTextFont(1);
		baseSprite->setTextColor(TFT_LIGHTGREY);
		baseSprite->print("SCORE:");
		baseSprite->print(hScore);
}
void Snake::Snake::pausedSetup()
{
	clearButtonCallbacks();
	buttons->setBtnPressCallback(BTN_B, []() {
		Serial.println("title");
		instance->gamestatus = "title";
		Playback.tone(400, 100);
	});
	buttons->setBtnPressCallback(BTN_A, []() {
		Serial.println("oldgame");
		instance->gamestatus = "oldgame";
		Playback.tone(600, 100);
	});
}
void Snake::Snake::paused()
{
	baseSprite->clear(TFT_BLACK);
	drawSnake();
	baseSprite->fillRect(foodX, foodY, foodSize, foodSize, TFT_YELLOW);
	if (borderFlag)
		baseSprite->drawRect(0, 0, baseSprite->width(), baseSprite->height(), TFT_WHITE);
	baseSprite->setFont(&fonts::Font2);
	baseSprite->setCursor(6, baseSprite->width() - 18);
	baseSprite->setTextSize(1);
	baseSprite->setTextColor(TFT_LIGHTGREY);
	baseSprite->print("SCORE:");
	baseSprite->print(hScore);
	baseSprite->setTextColor(TFT_LIGHTGREY);
	baseSprite->setTextSize(2);
	baseSprite->drawString("PAUSED", baseSprite->width()/2, 35);
	baseSprite->setTextSize(1);
	baseSprite->drawString("Press A to play", baseSprite->width()/2, 65);
	baseSprite->setFont(&fonts::Font2);
	baseSprite->drawString("Press B to exit", baseSprite->width()/2, 80);
}

void Snake::Snake::enterInitialsSetup()
{
	Playback.stop();
	tempScore = Highscore.get(0).score;
	name = "AAA";
	charCursor = 0;
	previous = "";
	elapsedMillis = millis();
	hiscoreMillis = millis();
	blinkState = 1;
	hiscoreBlink = 0;
	clearButtonCallbacks();
	buttons->setBtnPressCallback(BTN_UP,[](){
		instance->blinkState = 1;
		instance->elapsedMillis = millis();
		instance->name[instance->charCursor]++;
		// A-Z 0-9 :-? !-/ ' '
		if (instance->name[instance->charCursor] == '0') instance->name[instance->charCursor] = ' ';
		if (instance->name[instance->charCursor] == '!') instance->name[instance->charCursor] = 'A';
		if (instance->name[instance->charCursor] == '[') instance->name[instance->charCursor] = '0';
		if (instance->name[instance->charCursor] == '@') instance->name[instance->charCursor] = '!';
	});
	buttons->setButtonHeldRepeatCallback(BTN_UP, 200, [](uint){
		instance->blinkState = 1;
		instance->elapsedMillis = millis();
		instance->name[instance->charCursor]++;
		// A-Z 0-9 :-? !-/ ' '
		if (instance->name[instance->charCursor] == '0') instance->name[instance->charCursor] = ' ';
		if (instance->name[instance->charCursor] == '!') instance->name[instance->charCursor] = 'A';
		if (instance->name[instance->charCursor] == '[') instance->name[instance->charCursor] = '0';
		if (instance->name[instance->charCursor] == '@') instance->name[instance->charCursor] = '!';
	});
	buttons->setButtonHeldRepeatCallback(BTN_DOWN, 200, [](uint){
		instance->blinkState = 1;
		instance->elapsedMillis = millis();
		instance->name[instance->charCursor]--;
		if (instance->name[instance->charCursor] == ' ') instance->name[instance->charCursor] = '?';
		if (instance->name[instance->charCursor] == '/') instance->name[instance->charCursor] = 'Z';
		if (instance->name[instance->charCursor] == 31)  instance->name[instance->charCursor] = '/';
		if (instance->name[instance->charCursor] == '@') instance->name[instance->charCursor] = ' ';
	});
	buttons->setBtnPressCallback(BTN_DOWN, [](){
		instance->blinkState = 1;
		instance->elapsedMillis = millis();
		instance->name[instance->charCursor]--;
		if (instance->name[instance->charCursor] == ' ') instance->name[instance->charCursor] = '?';
		if (instance->name[instance->charCursor] == '/') instance->name[instance->charCursor] = 'Z';
		if (instance->name[instance->charCursor] == 31)  instance->name[instance->charCursor] = '/';
		if (instance->name[instance->charCursor] == '@') instance->name[instance->charCursor] = ' ';
	});
	buttons->setBtnPressCallback(BTN_LEFT, [](){
		if(instance->charCursor > 0){
			instance->charCursor--;
			instance->blinkState = 1;
			instance->elapsedMillis = millis();
		}
	});
	buttons->setBtnPressCallback(BTN_RIGHT, [](){
		if(instance->charCursor < 2){
			instance->charCursor++;
			instance->blinkState = 1;
			instance->elapsedMillis = millis();
		}
	});
	buttons->setBtnPressCallback(BTN_A, [](){
		Serial.println("next letter");
		instance->charCursor++;
		instance->blinkState = 1;
		instance->elapsedMillis = millis();
	});
}
void Snake::Snake::enterInitialsUpdate() {

	if (millis() - elapsedMillis >= 350) //cursor blinking routine
	{
		elapsedMillis = millis();
		blinkState = !blinkState;
	}
	if(millis()-hiscoreMillis >= 1000)
	{
		hiscoreMillis = millis();
		hiscoreBlink = !hiscoreBlink;
	}
	previous = name;

	if (previous != name)
	{
		blinkState = 1;
		elapsedMillis = millis();
	}

	if(charCursor >= 3)
	{
		Score newScore;
		strcpy(newScore.name, name.c_str());
		newScore.score = hScore;
		Highscore.add(newScore);
		gamestatus = "dataDisplay";
	}
}
void Snake::Snake::enterInitialsDraw() {
	baseSprite->clear(TFT_BLACK);
	baseSprite->setFont(&fonts::Font2);
	baseSprite->setTextColor(TFT_WHITE);
	baseSprite->setTextSize(1);
	baseSprite->drawString("ENTER NAME", baseSprite->width()/2, 8);
	baseSprite->setCursor(39, 80);

	if(hiscoreBlink && hScore > tempScore){
		baseSprite->drawString("NEW HIGH!", baseSprite->width() / 2, 80);
	}
	else{
		baseSprite->printf("SCORE: %04d", hScore);
	}

	baseSprite->setCursor(66, 40);
	baseSprite->print(name[0]);
	baseSprite->setCursor(81, 40);
	baseSprite->print(name[1]);
	baseSprite->setCursor(96, 40);
	baseSprite->print(name[2]);
	// display->drawRect(30, 38, 100, 20, TFT_WHITE);
	if(blinkState){
		baseSprite->drawFastHLine(63 + 15*charCursor, 54, 12, TFT_WHITE);
	}
}

void Snake::Snake::dataDisplaySetup()
{
	clearButtonCallbacks();
	buttons->setBtnPressCallback(BTN_UP, [](){
		instance->gamestatus = "eraseData";
	});
	buttons->setBtnPressCallback(BTN_A, [](){
		instance->gamestatus = "title";
	});
	buttons->setBtnPressCallback(BTN_B, [](){
		instance->gamestatus = "title";
	});
}
void Snake::Snake::dataDisplay()
{
	baseSprite->clear(TFT_BLACK);
	baseSprite->setTextSize(1);
	baseSprite->setFont(&fonts::Font2);
	baseSprite->setTextColor(TFT_YELLOW);
	baseSprite->drawString("HIGHSCORES", baseSprite->width() / 2,-2);
	baseSprite->setTextColor(TFT_GREEN);
	baseSprite->setCursor(3, 110);
	for (int i = 1; i < 6;i++)
	{
		baseSprite->setCursor(22, 2 + i * 16);
		if(i <= Highscore.count())
		{
			baseSprite->printf("%d.   %.3s    %04d", i, Highscore.get(i - 1).name, Highscore.get(i - 1).score);
		}
		else
		{
			baseSprite->printf("%d.    ---   ----", i);
		}
	}
	baseSprite->drawString("Press UP to erase", baseSprite->width() / 2, 105);
}

void Snake::Snake::eraseDataSetup()
{
	elapsedMillis = millis();
	blinkState = 1;
	clearButtonCallbacks();
	buttons->setBtnPressCallback(BTN_B, [](){
		instance->gamestatus = "dataDisplay";

	});
	buttons->setBtnPressCallback(BTN_A, [](){
		Highscore.clear();
		instance->gamestatus = "title";

	});
}
void Snake::Snake::eraseDataDraw()
{
	baseSprite->clear(TFT_BLACK);
	baseSprite->setFont(&fonts::Font2);
	baseSprite->setTextColor(TFT_WHITE);
	baseSprite->setTextDatum(textdatum_t::bottom_center);
	baseSprite->drawString("ARE YOU SURE?", baseSprite->width() / 2, 17);
	baseSprite->drawString("This cannot be reverted!", baseSprite->width() / 2, 37);
	//	baseSprite->drawString("", screen.getWidth() / 2, 53);

	baseSprite->drawString("B: Cancel", baseSprite->width() / 2, 105);

	baseSprite->setCursor(35, 81);
	baseSprite->print("A:");

	if (blinkState){
		baseSprite->drawRect(55, 64, 30*2, 9*2, TFT_RED);
		baseSprite->drawRect(55, 64, 30*2, 9*2, TFT_RED);
		baseSprite->setTextColor(TFT_RED);
		baseSprite->setCursor(62, 81);
		baseSprite->print("DELETE");
	}
	else {
		baseSprite->fillRect(55, 64, 30*2, 9*2, TFT_RED);
		baseSprite->setTextColor(TFT_WHITE);
		baseSprite->setCursor(62, 81);
		baseSprite->print("DELETE");
	}
	baseSprite->setTextDatum(textdatum_t::top_center);
}
void Snake::Snake::eraseDataUpdate()
{
	if (millis() - elapsedMillis >= 350) {
		elapsedMillis = millis();
		blinkState = !blinkState;
	}
}
