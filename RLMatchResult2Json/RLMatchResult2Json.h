#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "JSON.h"

#include "version.h"

constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

using giri::json::JSON;

class RLMatchResult2Json: public BakkesMod::Plugin::BakkesModPlugin
{

private:
	void createDirectory(std::string path);
	JSON getMatchResult();
	std::string createOutputFileName(JSON obj);
	void writeJson(std::string fileName, JSON obj);
	std::string getCurrentUtcDate();
	JSON getTeamInfo(TeamWrapper team);
	JSON getPlayersInfo(int playerTeamNum, ArrayWrapper<PriWrapper> players);
	float getPlayerMMR(PriWrapper player);

public:
	void onLoad() override;
	void onUnload() override;
	void onMatchEnd(std::string eventName);

};
