#include <iostream>
#include <time.h>
#include <cstdio>
#include "../nlohmann/json.hpp"

using namespace std;
char commands[4][10] = { "left", "right", "up", "down" };
#define _LOG_TOFILE 1

#if _LOG_TOFILE

#include <fstream>
inline string getCurrentDateTime(string s) {
	time_t now = time(0);
	struct tm  tstruct;
	char  buf[80];
	tstruct = *localtime(&now);
	if (s == "now")
		strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
	else if (s == "date")
		strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
	return string(buf);
};
inline void log(string logMsg) {

	string filePath = "log.txt";
	string now = getCurrentDateTime("now");
	ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
	ofs << now << '\t' << logMsg << '\n';
	ofs.close();
}
#elif
inline void log(string logMsg) {
}
#endif



struct player_s {
	std::string mName;
	int mScore;
	int mSpeed;
	std::string mTerritory;
	std::string mLines;
	std::string mBonuses;
	int mPosition[2];
	std::string mDirection;
	int mDirectionId;

};
#define MAX_COUNT_PLAYERS 10
#define MAX_COUNT_BONUSES 200
struct db_s {
	int mThisId;
	player_s mThisPlayer;

	player_s mOtherPlayers[MAX_COUNT_PLAYERS];
	int mPlayersCount;

	int mPrevCommandId;
	int mTick;

	int mGlobal_Speed;
	int mGlobal_Size;
	int mGlobal_Field_size_x;
	int mGlobal_Field_size_y;

	int mBonusesCount;
	int mBonuses[MAX_COUNT_BONUSES][3];//type n = 1,s=2,saw=3 / pos_x / posy
};
db_s db;
void parseStartToDb(nlohmann::json  _jsonTick)
{
	/*
	{"type": "start_game"
	, "params": {"x_cells_count": 31, "y_cells_count": 31
	, "speed": 5
	, "width": 30}}
	*/
	if (strcmp(_jsonTick["type"].get<std::string>().c_str(), "start_game") != 0)
	{
		return;
	}
	

	db.mGlobal_Speed = _jsonTick["params"]["speed"];
	db.mGlobal_Size = _jsonTick["params"]["width"];
	db.mGlobal_Field_size_x = _jsonTick["params"]["x_cells_count"];
	db.mGlobal_Field_size_y = _jsonTick["params"]["y_cells_count"];

}

void 		parsePlayer(player_s & _player,nlohmann::json::iterator  &_json_plCurrentIt)
{

	auto json_plCurrent = _json_plCurrentIt.value();
	_player.mName = _json_plCurrentIt.key();
	_player.mScore = json_plCurrent["score"];
	if (json_plCurrent["direction"].is_null())
	{
		_player.mDirection = "";
	}
	else
	{
		_player.mDirection = json_plCurrent["direction"].get<std::string>().c_str();
	}

	_player.mTerritory = json_plCurrent["territory"].dump();
	_player.mLines = json_plCurrent["lines"].dump();
	_player.mBonuses = json_plCurrent["bonuses"].dump();


	_player.mPosition[0] = json_plCurrent["position"][0];
	_player.mPosition[1] = json_plCurrent["position"][1];

	if (strcmp(_player.mDirection.c_str(), commands[0]) == 0)
	{
		_player.mDirectionId = 0;
	}
	else if (strcmp(_player.mDirection.c_str(), commands[1]) == 0)
	{
		_player.mDirectionId = 1;

	}
	else if (strcmp(_player.mDirection.c_str(), commands[2]) == 0)
	{
		_player.mDirectionId = 2;

	}
	else if (strcmp(_player.mDirection.c_str(), commands[3]) == 0)
	{
		_player.mDirectionId = 3;

	}
	else
	{
		//usualy null at start
		_player.mDirectionId = 0;
	}

}

void parseTickToDb(nlohmann::json  _jsonTick)
{
	/*
	{"type": "start_game"
	, "params": {"x_cells_count": 31, "y_cells_count": 31
	, "speed": 5
	, "width": 30}}

	{"type": "tick"
	, "params" : {"players": {"1": {"score": 4 
									,"direction" : "up"
									, "territory" : [[255, 285], [285, 285], [315, 315], [225, 285], [285, 255], [285, 315], [225, 255], [255, 315], [195, 255], [195, 285], [315, 255], [255, 255], [315, 285]]
									, "lines" : []
									, "position" : [285, 315]
									, "bonuses" : []}
								, "2" : {"score": 0
									, "direction" : "up"
									, "territory" : [[255, 555], [285, 615], [255, 615], [255, 585], [285, 585], [285, 555], [315, 615], [315, 555], [315, 585]]
									, "lines" : [[255, 525], [255, 495], [255, 465], [285, 465], [315, 465], [315, 495], [315, 525]]
									, "position" : [315, 525]
									, "bonuses" : []}
								, "i" : {"score": 0
									, "direction" : "left"
									, "territory" : [[585, 315], [615, 315], [555, 255], [555, 285], [585, 285], [555, 315], [615, 255], [585, 255], [615, 285]]
									, "lines" : [[615, 225], [645, 225], [675, 225], [705, 225], [705, 195], [675, 195], [645, 195]]
									, "position" : [645, 195]
									, "bonuses" : []}}
				, "bonuses" : []
				, "tick_num" : 55}}
	
	*/



	db.mTick = _jsonTick["params"]["tick_num"];

	//bonuses

	int iBonusIdx = 0;
	for (auto& element :  _jsonTick["params"]["bonuses"]) {
		const char* pBonusName = element["type"].get<std::string>().c_str();
		int bonuseCodeId = 0;
		if (strcmp(pBonusName, "n") == 0)
		{
			bonuseCodeId = 1;

		}
		else if (strcmp(pBonusName, "s") == 0)
		{
			bonuseCodeId = 2;

		}
		else if(strcmp(pBonusName, "saw") == 0)
		{
			bonuseCodeId = 3;

		}
		else
		{
			bonuseCodeId = 0;

		}
		db.mBonuses[iBonusIdx][0] = bonuseCodeId;
		db.mBonuses[iBonusIdx][1] = element["position"][0];
		db.mBonuses[iBonusIdx][2] = element["position"][1];

		iBonusIdx++;
		if (iBonusIdx >= MAX_COUNT_BONUSES - 1)
			break;
	}
	db.mBonusesCount = iBonusIdx;



	//players
	
	auto json_plThisIt = _jsonTick["params"]["players"].find("i");
	if (json_plThisIt != _jsonTick["params"]["players"].end())
	{
		parsePlayer(db.mThisPlayer, json_plThisIt);
	}
	



	//parse i
	int iMaxPlayersId = MAX_COUNT_PLAYERS;
	int iPlayerIndx = 0;
	for (int i = 1; i <= iMaxPlayersId; i++)
	{
		char cPlayerId[100];
		itoa(i, cPlayerId, 10);
		auto json_plCurrentIt = _jsonTick["params"]["players"].find(cPlayerId);
		if (json_plCurrentIt == _jsonTick["params"]["players"].end())
		{
			//not exist
			continue;
		}
		parsePlayer(db.mOtherPlayers[iPlayerIndx], json_plCurrentIt);
		//load
		

		iPlayerIndx++;
	}

	db.mPlayersCount = iPlayerIndx;

	//_jsonTick["params"]["bonuses"];
	//_jsonTick["params"]["players"];
	
}
void dumpStart()
{
	char cBuff[1024];
	sprintf(cBuff, "speed = %d \n\t size = %d \n\t filed_size_X = %d\n\t filed_size_Y = %d"
		, db.mGlobal_Speed, db.mGlobal_Size
		, db.mGlobal_Field_size_x, db.mGlobal_Field_size_y
	);
	log(cBuff);
}
void dumpTick()
{
	char cBuff[1024];
	sprintf(cBuff, "tick = %d players = %d "
		, db.mTick, db.mPlayersCount
	);
	log(cBuff);
	sprintf(cBuff, "name = %s score = %d "
		, db.mThisPlayer.mName.c_str(), db.mThisPlayer.mScore);
	log(cBuff);


	for(int i =0;i<db.mPlayersCount;i++)
	{
		sprintf(cBuff, "name = %s score = %d "
			, db.mOtherPlayers[i].mName.c_str(), db.mOtherPlayers[i].mScore);
			log(cBuff);
	}
}
int main() {
	
    srand(time(NULL));
    string input_string, input_type;
	
    while (true) {
		log("start input:");
        getline(cin, input_string);
		log(input_string);
		nlohmann::json j;
		j = nlohmann::json::parse(input_string);
		
		
		
		if (strcmp(j["type"].get<std::string>().c_str(), "tick") == 0)
		{
			parseTickToDb(j);
			dumpTick();
		}
		else if (strcmp(j["type"].get<std::string>().c_str(), "start_game") == 0)
		{
			parseStartToDb(j);
			dumpStart();
		}
		else
		{
			//unexpected
			return 0;
		}


		
		


		auto index = rand() % 4;
        nlohmann::json command;
        command["command"] = commands[index];
        cout << command.dump() << endl;
		log("result :");
		log(command.dump());
		log("---------------");
	}

    return 0;
}