#include "pch.h"
#include "RLMatchResult2Json.h"
#include "JSON.h"

#include <direct.h>
#include <chrono>
#include <iostream>
#include <fstream>

using giri::json::JSON;

BAKKESMOD_PLUGIN(RLMatchResult2Json, "Match Result to JSON", plugin_version, 0)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void RLMatchResult2Json::onLoad()
{
	_globalCvarManager = cvarManager;

	LOG("RLMatchResult2Json loaded.");

	auto cvar1 = cvarManager->registerCvar("rlmr2json_save_path", "MatchResults", "Location to save your JSON files inside bakkesmod data folder.", true, false, 0, false, 0, true);
	createDirectory(cvar1.getStringValue());

	cvar1.addOnValueChanged([this](std::string cvarName, CVarWrapper newCvar) {
		createDirectory(newCvar.getStringValue());
	});

	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchEnded", [this](std::string eventName) {
			onMatchEnd(eventName);
		});
}

void RLMatchResult2Json::onUnload() {
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchEnded");
}

void RLMatchResult2Json::onMatchEnd(std::string eventName) {
	LOG("Match has ended.");
	JSON matchResult = getMatchResult();
	std::string fileName = writeJson(matchResult);

	std::stringstream message;
	message << "Your match results were successfully stored to " << fileName << ".";

	gameWrapper->Toast("Match results stored.", message.str(), "default", 5.0, ToastType_Info);
}

void RLMatchResult2Json::createDirectory(std::string path) {
	std::error_code ec;
	auto fullPath = gameWrapper->GetDataFolder() / path;
	
	if (std::filesystem::create_directories(fullPath, ec)) {
		LOG("Save directory created: {}", fullPath.string());
	}
}

std::string RLMatchResult2Json::getCurrentUtcDate() {
	const auto now = std::chrono::system_clock::now();
	return std::format("{:%FT%TZ}", now);
}

JSON RLMatchResult2Json::getMatchResult() {
	JSON result;
	result["version"] = 1;
	result["date"] = getCurrentUtcDate();

	ServerWrapper server = gameWrapper->GetOnlineGame();

	if (!server) {
		LOG("ServerWrapper is null. Match results cannot be stored.");
		result["error"] = "SERVERWRAPPER_NULL";
		return result;
	}

	result["match"] = giri::json::Object();

	result["match"]["gameMode"] = server.GetPlaylist().GetTitle().ToString();
	result["match"]["isRanked"] = server.GetPlaylist().GetbRanked();
	result["match"]["isTournament"] = server.GetPlaylist().IsTournamentMatch() ? 1 : 0;
	result["match"]["isOvertime"] = server.GetbOverTime();

	result["match"]["gameTime"] = server.GetGameTime();
	result["match"]["overtimeTimePlayed"] = server.GetOvertimeTimePlayed();
	result["match"]["isClubMatch"] = server.GetbClubMatch();

	PriWrapper localPlayer = server.GetLocalPrimaryPlayer().GetPRI();
	ArrayWrapper<TeamWrapper> teams = server.GetTeams();
	auto playerTeamNum = localPlayer.GetTeamNum();
	auto otherTeamNum = 1 - playerTeamNum;

	result["teams"] = giri::json::Array(getTeamInfo(teams.Get(playerTeamNum)), getTeamInfo(teams.Get(otherTeamNum)));

	bool isWin = false;
	if (result["teams"][0]["hasForfeit"].ToInt() != 0) {
		result["match"]["isForfeit"] = 1;
		result["match"]["forfeitTeamsIndex"] = 0;
	}
	else if (result["teams"][1]["hasForfeit"].ToInt() != 0) {
		result["match"]["isForfeit"] = 1;
		result["match"]["forfeitTeamsIndex"] = 1;
		isWin = true;
	}
	else {
		result["match"]["isForfeit"] = 0;
		isWin = result["teams"][0]["score"].ToInt() > result["teams"][1]["score"].ToInt();
	}
	result["match"]["result"] = isWin ? "WIN" : "LOSS";

	result["players"] = getPlayersInfo(playerTeamNum, server.GetPRIs());

	LOG("Match results JSON created.");
	return result;
}

JSON RLMatchResult2Json::getTeamInfo(TeamWrapper team) {
	auto teamInfo = giri::json::Object();
	teamInfo["teamNum"] = team.GetTeamNum();
	teamInfo["teamName"] = team.GetSanitizedTeamName().ToString();

	teamInfo["score"] = team.GetScore();
	teamInfo["hasForfeit"] = team.GetbForfeit();

	auto primaryColor = team.GetPrimaryColor();
	auto secondaryColor = team.GetSecondaryColor();

	teamInfo["colors"] = {
		"primary", { "r", primaryColor.R, "g", primaryColor.G, "b", primaryColor.B, "a", primaryColor.A },
		"secondary", { "r", secondaryColor.R, "g", secondaryColor.G, "b", secondaryColor.B, "a", secondaryColor.A }
	};

	return teamInfo;
}

JSON RLMatchResult2Json::getPlayersInfo(int playerTeamNum, ArrayWrapper<PriWrapper> players) {

	auto playersInfo = giri::json::Array(giri::json::Array(), giri::json::Array());

	for (int i = 0; i < players.Count(); i++) {
		PriWrapper player = players.Get(i);

		int teamNum = player.GetTeamNum();

		auto playerInfo = giri::json::Object();

		playerInfo["name"] = player.GetPlayerName().ToString();
		playerInfo["playerId"] = player.GetPlayerID();
		playerInfo["teamNum"] = teamNum;
		playerInfo["score"] = player.GetMatchScore();
		playerInfo["goals"] = player.GetMatchGoals();
		playerInfo["assists"] = player.GetMatchAssists();
		playerInfo["saves"] = player.GetMatchSaves();
		playerInfo["shots"] = player.GetMatchShots();
		//playerInfo["damage"] = player.GetMatchBreakoutDamage();
		//playerInfo["demolishes"] = player.GetMatchDemolishes();
		playerInfo["isMvp"] = player.GetbMatchMVP();
		playerInfo["mmr"] = getPlayerMMR(player);

		playersInfo[teamNum == playerTeamNum ? 0 : 1].append(playerInfo);
	}

	return playersInfo;
}

float RLMatchResult2Json::getPlayerMMR(PriWrapper player) {
	MMRWrapper mmr = gameWrapper->GetMMRWrapper();
	int playlist = mmr.GetCurrentPlaylist();
	auto uniqueIDWrapper = player.GetUniqueIdWrapper();
	return mmr.GetPlayerMMR(uniqueIDWrapper, playlist);
}

std::string RLMatchResult2Json::writeJson(JSON obj) {
	std::string baseFolder = cvarManager->getCvar("rlmr2json_save_path").getStringValue();
	std::stringstream fileName;
	fileName << std::time(nullptr) << ".json";

	auto fullPath = gameWrapper->GetDataFolder() / baseFolder / fileName.str();

	std::ofstream fh;
	fh.open(fullPath);
	fh << obj << std::endl;
	fh.close();

	LOG("Match results JSON stored to {}", fullPath.string());

	return fileName.str();
}


