#include "Highscore.h"

#include <FS.h>
#include <SPIFFS.h>
#include <ByteBoi.h>

Snake::HighscoreImpl Snake::Highscore;

void Snake::HighscoreImpl::begin(){
	if(!SPIFFS.begin()){
		Serial.println("SPIFFS begin error");
	}

	bool firstTime = !SPIFFS.exists(HS_FILENAME);
	if(firstTime){
		data.count = 0;
		save();
	}else{
		load();
	}
}

void Snake::HighscoreImpl::add(const Score& score){
	if(data.count == HS_MAX_SCORES && score.score < data.scores[data.count-1].score) return; // HS full, new score is lower than last in HS

	uint8_t i;
	for(i = 0; i < data.count; i++){
		if(score.score <= data.scores[i].score) continue; // if score is lower or equal, continue

		// otherwise, move others down
		uint8_t first = i;
		uint8_t count = data.count - i;
		if(data.count == HS_MAX_SCORES) count--; // if HS is full, we're moving one score less, so that we don't write over space that isn't part of the HS

		// move
		// memcpy(&data.scores[first+1], &data.scores[first], count * sizeof(Score));
		for(int j = first + count; j > first; j--){
			memcpy(&data.scores[j], &data.scores[j-1], sizeof(Score));
		}
		Serial.println();

		break;
	}

	if(i == HS_MAX_SCORES) return; // all scores are higher than the new one

	// write the new score
	memcpy(&data.scores[i], &score, sizeof(Score));
	data.count = min(HS_MAX_SCORES, data.count+1);
	save();
}

void Snake::HighscoreImpl::clear(){
	data.count = 0;
	save();
}

const Snake::Score& Snake::HighscoreImpl::get(uint8_t i){
	return data.scores[i];
}

uint8_t Snake::HighscoreImpl::count(){
	return data.count;
}

void Snake::HighscoreImpl::save(){
	File file = ByteBoi.openData(HS_FILENAME, "w");
	file.write((byte*) &data, sizeof(Data));
	file.close();
}

void Snake::HighscoreImpl::load(){
	File file = ByteBoi.openData(HS_FILENAME, "r");
	file.readBytes((char*) &data , sizeof(Data));
	file.close();
}