# Essentials
[![Jenkins Build](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fci.unstoppable.work%2Fjob%2FDragonade%2520Plugins%2Fjob%2FAreaSaver%2F)](https://ci.unstoppable.work/job/Dragonade%20Plugins/job/Essentials/)
[![Discord](https://img.shields.io/discord/647431164138749966?label=support)](https://discord.gg/KjeQ7xv)

## Overview
Essentials is a package of must-have features for all C&C Renegade servers running Dragonade. Essentials aims to extend features and API of Dragonade without modifying it's code.
Essentials comes with useful, extremely configurable and easy to use commands, features and APIs for developers.

## Features

### Holograms API
Essentials provides a simple API to create text holograms in maps at custom locations and handles object events for developers. Holograms spawns an invisible soldier and uses it's player name display to create holographic texts in the map.

### Connection Messages
Essentials can send host messages into the game when someone requests to join server or when loses connection. Essentials also sends extended information about connection requests to Log Server, so bots and loggers can get detailed information about player before they join.

### Anti-Chat Flood
This feature checks clients for flooding in the same way as radio message spam check.

### Bad Word Filter
This feature checks messages sent by players for bad words, then warns players if their messages contained a bad word and optionally kicks after configurable amount of warnings.

### Crate Injector
This feature takes control of Dragonade's crate spawning logic and spawns configurable power-ups at crate spawn locations. These power-ups can do Dragonade's default crate logic when they are picked up, or can do it's original power-up behavior. This feature is useful if you are looking to spawn special power-ups at crate spawn locations or want to randomize crate models.

### Grunt Effect
This feature plays grunt sound with small camera shake as seen in Single Player.

### Map Holograms
This feature utilizes Essentials' Holograms API and creates holograms in maps defined by their map names in configuration.

### Jukebox
Essentials comes with a very advanced Jukebox feature, allowing players to create their own playlists with loop modes and shuffling.

### Custom Power-Up Spawners
This feature creates custom power-up spawners per map or globally with customizable spawn delay and presets.

### Auto Announce
This feature announces messages defined in the configuration.

### Authentication
Displays a dialog to registered players during connection request. Prompts for password, nick change or allows cancellation of authorization and kicks the player.

### Custom Tag
Displays custom tags under player's name tags, and saves into a database.

### Voting
Allows voting for more time, ending the current map, restarting the current map, changing the next map, kicking a player and disabling building repairs.  
Includes a "Forced Polls" option that displays an interactive dialog and enforces everyone to vote.

### Content Management System
This feature lets server owners create custom dialog and chat message content using a text-based custom markup language, which can be activated by an API, player join event or chat phrases/commands.

## Game Modes

### Sniping
This game mode is designed to provide a true sniping server experience. Besides of Dragonade's default configuration-based game mode, this game mode provides warning/kill zones, money-per-kill system, blockers, custom player spawns, custom purchase terminals and custom pricing.

### Free For All (FFA)
In this game mode, players try to get as much kills as possible. This is also known as Deathmatch game mode, where everyone can kill each other. This game mode forces all players into a team with custom player models. Has built in spawn system that server owners can define custom spawn points from configuration and completely disable spawn points in the map. Teaming (also known as Team Deathmatch/TDM) with this game mode is not possible. By default, Essentials comes with a configuration that defines all mission maps except for 1st mission (M13.mix) to be 100% compatible for FFA with power-up spawners using *Custom Power-Up Spawners* game feature, custom spawn points and blockers. You can find the default configuration of this game mode [here](Essentials/da_ffa.ini).

## Support
You can find a list of commands and their descriptions at [here](docs/commands.md).  
You can view API documentation at [here](docs/api/index.md).  
Configuration is self-explanatory, comments should help you understand. You can read the default configuration [here](Essentials/Essentials.ini).  
<br />
If you need further help or want to suggest a feature/improvement, you will get the fastest response by joining in Discord server at https://discord.gg/KjeQ7xv, but be sure to read `#important` channel first.
