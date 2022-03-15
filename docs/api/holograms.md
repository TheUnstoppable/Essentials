# Holograms API
Here you can find documentation for Essentials' Hologram API.

## What is this?
Hologram API creates text holograms by utilizing player name displays of soldiers. Soldiers created by API is configured to be invisible to both players and scripts by changing to a special model. Holograms can have 4 different colors: GDI color (Yellow), Nod color (Red), Neutral color (Gray-ish White) and Renegade (White) color.  
Developers create holograms by giving them a unique name, a display text, color and their position in map. Objects and object events are handled by API, so developers don't have to deal with objects and their events while creating holograms.  
**Holograms persists between levels and developers are responsible of cleaning up the holograms they've created. (Holograms are cleaned up on server shutdown or configuration error automatically)**

## How to use?
Start by including **EssentialsHologram.h** in your source file. Then to create your first hologram, use `EssentialsHologramsManager::Create_Hologram`. This function returns a wrapper for hologram object and takes a unique name, text to display, color of the text and position as parameter.

## Key elements

### `EssentialsHologramDataClass`
This object holds the information about your hologram. You can get it's name and change it's color, text and position. A hologram's name cannot be changed and is read-only.
<br /><br />

### `EssentialsHologramDataClass::Get_Hologram_Name()`
This function returns the name of the hologram.  
Example: `auto hologramName = hologram->Get_Hologram_Name();`
<br /><br />

### `EssentialsHologramDataClass::Get_Hologram_Color()`
This function returns the color of the hologram.  
Example: `auto hologramColor = hologram->Get_Hologram_Color();`
<br /><br />

### `EssentialsHologramDataClass::Get_Hologram_Text()`
This function returns the display text of the hologram.  
Example: `auto hologramText = hologram->Get_Hologram_Text();`
<br /><br />

### `EssentialsHologramDataClass::Get_Position()`
This function returns the position of the hologram.  
Example: `auto hologramPos = hologram->Get_Position();`
<br /><br />

### `EssentialsHologramDataClass::Set_Hologram_Color(HologramColor NewColor)`
This function changes the color of the hologram to `NewColor`, and reinitializes the hologram object.  
Example: `hologram->Set_Hologram_Color(EssentialsHologramColor::COLOR_RENEGADE);`
<br /><br />

### `EssentialsHologramDataClass::Set_Hologram_Text(const wchar_t* NewText)`
This function changes the display text of the hologram to `NewText`, and reinitializes the hologram object.  
Example: `hologram->Set_Hologram_Text(L"Hello World!");`
<br /><br />

### `EssentialsHologramDataClass::Set_Position(const Vector3& NewPosition)`
This function changes the position of the hologram to `NewPosition`, and reinitializes the hologram object.  
Example: `hologram->Set_Position(Commands->Get_Position(exampleObj));`
<br /><br />

### `EssentialsHologramDataClass::Reinitialize_Object()`
This function deletes, recreates and reconfigures the hologram object.  
Example: `hologram->Reinitialize_Object();`
<br /><br />

### `EssentialsHologramsManager`
This static class manages the creation, deletion and storing of holograms.
<br /><br />

### `EssentialsHologramsManager::Create_Hologram(const char* Name, const wchar_t* Text, const HologramColor Color, const Vector3& Position)`
This function creates a new hologram with specified `Name`, specified `Text` and specified `Color` at `Position`. If a hologram with `Name` exists, existing hologram will be deleted before creation. Returns `EssentialsHologramDataClass`, or `nullptr` if API is somehow disabled.  
Example: `EssentialsHologramsManager::Create_Hologram("TestHologram", L"Example Hologram", EssentialsHologramColor::COLOR_GDI, Vector3(0, 0, 4));`
<br /><br />

### `EssentialsHologramsManager::Find_Hologram(const char* Name)`
Finds a hologram by `Name`. Returns `EssentialsHologramDataClass`, or `nullptr` if API is somehow disabled.  
Example: `auto hologram = EssentialsHologramsManager::Find_Hologram("TestHologram");`
<br /><br />

### `EssentialsHologramsManager::Delete_Hologram(EssentialsHologramDataClass* Hologram)`
Deletes a hologram by their data wrapper. Returns `true` if deletion was successful, or `false` if unsuccessful or API is somehow disabled.  
Example: `EssentialsHologramsManager::Delete_Hologram(hologram);`
<br /><br />

### `EssentialsHologramsManager::Delete_Hologram(const char* Name)`
Deletes a hologram by `Name`. Returns `true` if deletion was successful, or `false` if hologram couldn't be found, unsuccessful or API is somehow disabled.  
Example: `EssentialsHologramsManager::Delete_Hologram("TestHologram");`
