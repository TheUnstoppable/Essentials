# Commands
Here you can find a list of commands that Essentials adds in server.

## Syntax
Parameters in angle brackets `< >` are required parameters and has to be specified for the command to execute. Parameters in square brackets `[ ]` are optional parameters and doesn't have to be specified.  
If an optional parameter comes before required parameter, command understands that an optional parameter is not specified and will treat the command execution in that way.
<br />

### !fly
- **Alias(es)**: !fly
- **Usage**: !fly \[target\]  

Toggles fly mode of command executor, or `target`.
<br /><br />


### !spectate
- **Alias(es)**: !spectate, !specmode, !spec
- **Usage**: !spectate \[target\]  

Toggles spectate mode of command executor, or `target`.
<br /><br />


### !spawnobj
- **Alias(es)**: !spawnobj, !spawn, !spobj, !sp
- **Usage**: !spawnobj \[target\] \<preset\>

Spawns specified object `preset` at command executor's or `target`'s location. Presets to block can be configured from configuration.
<br /><br />


### !changechar
- **Alias(es)**: !changechar, !cchar, !cc
- **Usage**: !changechar \[target\] \<preset\>

Changes command executor's or `target`'s object to specified soldier `preset`. Presets to block can be configured from configuration.
<br /><br />


### !giveweap
- **Alias(es)**: !giveweap, !gweap, !gw
- **Usage**: !giveweap \[target\] \<preset\>

Gives command executor's or `target`'s player or vehicle object specified weapon `preset`. Presets to block can be configured from configuration.
<br /><br />


### !printfds
- **Alias(es)**: !printfds, !fds
- **Usage**: !printfds \<line\>

Executes `line` in console.
<br /><br />


### !fastspawn
- **Alias(es)**: !fastspawn, !fastsp, !fsp
- **Usage**: !fastspawn \<keyword\>

Spawns specified object at command executor's location, by predefined `keyword` in configuration.
<br /><br />


### !givemoney
- **Alias(es)**: !givemoney, !money, !gm
- **Usage**: !givemoney \[target\] \<amount\>

Gives `amount` of money to command executor, or `target`.
<br /><br />


### !goto
- **Alias(es)**: !goto
- **Usage**: !goto \<target\>

Puts command executor to `target`'s location.
<br /><br />


### !put
- **Alias(es)**: !put
- **Usage**: !put \<player\> \<target\>

Puts `player` to `target`'s location.
<br /><br />


### !putall
- **Alias(es)**: !putall
- **Usage**: !putall \[target\]

Puts every player to command executor's or `target`'s location.
<br /><br />


### !putallline
- **Alias(es)**: !putallline
- **Usage**: !putallline \[target\] \<distance\>

Puts every player in front of command executor or `target` in line, with `distance` between each player.
<br /><br />


### !putallcircle
- **Alias(es)**: !putallcircle
- **Usage**: !putallcircle \[target\] \<radius\>

Puts every player around command executor or `target` in circle, with `radius`.
<br /><br />


### !sethealth
- **Alias(es)**: !sethealth, !shealth, !sh
- **Usage**: !sethealth \[target\] \<amount\>

Sets command executor's or `target`'s player or vehicle object's health to `amount`.
<br /><br />


### !setarmor
- **Alias(es)**: !setarmor, !sarmor, !sa
- **Usage**: !setarmor \[target\] \<amount\>

Sets command executor's or `target`'s player or vehicle object's armor to `amount`.
<br /><br />


### !setmaxhealth
- **Alias(es)**: !setmaxhealth, !smhealth, !smh
- **Usage**: !setmaxhealth \[target\] \<amount\>

Sets command executor's or `target`'s player or vehicle object's maximum health to `amount`.
<br /><br />


### !setmaxarmor
- **Alias(es)**: !setmaxarmor, !smarmor, !sma
- **Usage**: !setmaxarmor \[target\] \<amount\>

Sets command executor's or `target`'s player or vehicle object's maximum armor to `amount`.
<br /><br />


### !setclipammo
- **Alias(es)**: !setclipammo, !scammo, !sca
- **Usage**: !setclipammo \[target\] \<amount\>

Sets command executor's or `target`'s player or vehicle object's current weapon's clip ammo (the one that decreases when weapon is shot) to `amount`.
<br /><br />


### !setinvammo
- **Alias(es)**: !setinvammo, !siammo, !sia
- **Usage**: !setinvammo \[target\] \<amount\>

Sets command executor's or `target`'s player or vehicle object's current weapon's inventory ammo (the one that decreases when weapon is reloaded) to `amount`.
<br /><br />


### !setobjteam
- **Alias(es)**: !setobjteam, !soteam, !st
- **Usage**: !setobjteam \[target\] \<team\>

Changes command executor's or `target`'s object's team to `team`. Not to be confused with *player team*, this command affects **object** team.
<br /><br />


### !attach
- **Alias(es)**: !attach
- **Usage**: !attach \<target\> \<script\> \[parameters\]

Attaches `script` to `target`'s player or vehicle object with optional `parameters`. Scripts to block can be configured from configuration.
<br /><br />


### !detach
- **Alias(es)**: !detach
- **Usage**: !detach \<target\> \<script\>

Detaches `script` from `target`'s player or vehicle object. Scripts to block can be configured from configuration.
<br /><br />


### !changemodel
- **Alias(es)**: !changemodel, !setmodel, !chmodel, !smodel, !sm
- **Usage**: !changemodel \[target\] \<model name\>

Changes command executor's or `target`'s player or vehicle object's model to `model name`. Models to block can be configured from configuration.
<br /><br />


### !getposition
- **Alias(es)**: !getposition, !getpos, !gp
- **Usage**: !getposition \[target\]

Gets location and facing of command executor's or `target`'s player or vehicle object.
<br /><br />


### !setspeed
- **Alias(es)**: !setspeed, !sspeed, !ssp
- **Usage**: !setspeed \[target\] \<speed\>

Changes command executor's or `target`'s player speed to `speed`.
<br /><br />


### !revivebld
- **Alias(es)**: !revivebld, !revive, !revbld, !rb
- **Usage**: !revivebld \<team\> \<preset\> **OR** !revivebld \<team\> \<keyword\>

Revives `team`'s building by `preset` or `keyword`. Keywords are synonyms for buildings and can be configured from configuration.
<br /><br />


### !godmode
- **Alias(es)**: !godmode, !god
- **Usage**: !god \[target\]

Puts command executor or `target` into god mode. God mode preset and it's inventory can be configured from configuration.
<br /><br />


### !screenshot
- **Alias(es)**: !screenshot, !screenie, !sshot, !ss
- **Usage**: !screenshot \<target\>

Takes a screenshot of `target`'s game. This requires `ssurl` console command to be set.
<br /><br />


### !getmodelname
- **Alias(es)**: !getmodelname, !mdlname, !gmn
- **Usage**: !getmodelname \[target\]

Gets model name of command executor's or `target`'s player or vehicle object.
<br /><br />


### !getpresetname
- **Alias(es)**: !getpresetname, !prsname, !gpn
- **Usage**: !getpresetname \[target\]

Gets preset name and ID of command executor's or `target`'s player or vehicle object.
<br /><br />
