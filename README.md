# RLMatchResult2Json

## Description

Plugin for BakkesMod that stores match results of the game Rocket League to a JSON file.

BakkesMod is a mod for video game Rocket League which allows to load plugins to access data from the game during run time and perform various actions. This plugin stores the results of a match at the end of the match to a JSON file.

It stores the following data for a match, if available:
* Date
* Game mode
* Is Ranked match?
* Is Tournament match?
* Was an overtime played?
* Game time
* Overtime length
* Was it a match between clubs?
* Was there a forfeit?
  * If yes, which team did forfeit?
* Is it a win of the perspective of the player that created the JSON file?
* For each team:
  * Team index ID
  * Team name
  * Score (i.e. number of goals)
  * Has forfeit?
  * Primary color
  * Secondary color
* For each player of the match:
  * Team index ID
  * Player Name
  * Player ID
  * Score (i.e. Points)
  * Number of goals
  * Number of assists
  * Number of saves
  * Number of shots
  * Is MVP of the match?

The JSON files are stored to the data file of the BakkesMod AppData folder (by default C:\Users\<user>\AppData\Roaming\bakkesmod\bakkesmod\data\MatchResults).

## Building

The plugin was implemented and compiled in Visual Studio 2022.

After the compilation is completed successfully, simply copy the RLMatchResult2Json.dll file from the /plugins folder to the plugins folder of your BakkesMod AppData folder (by default C:\Users\<user>\AppData\Roaming\bakkesmod\bakkesmod\plugins).
