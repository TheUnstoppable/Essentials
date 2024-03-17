# Content Management System
This document details Essentials' Content Management System (CMS) and demonstrates the custom file format which is used to create visuals.
<br /><br />



# What is this?
CMS allows server owners to create chat and menu/popup dialog content that can be activated using full or partial chat phrases/commands, key hooks or when the player joins.
<br /><br />



# How to use?
Essentials CMS uses a custom text-based format named **ECMSL** (stands for Essentials CMS Language) with `.cms` file extension, and allows creation of Chat content, Menu content and Popup content. Essentials ships with various CMS content for demonstration purposes.  
Each `.cms` file should start with a header that defines what the content is (Chat, Menu, Popup) and then following lines after the header are content and trigger types.
<br /><br />

## Placeholders
Each text/data field in CMS supports the following placeholders.
- `{PLAYER}`: Player Name
- `{TITLE}`: Server Title
- `{MAP}`: Current Map
<br /><br />

## Chat Content
This content type allows displaying host messages, color messages, team-color messages, HUD messages and admin messages.
<br /><br />

## Menu Dialog Content
This content type allows displaying text and image content within the full screen menu.
<br /><br />

## Popup Dialog Content
This content type allows displaying text and image content within a popup window.
<br /><br />



# Format
As it was said before, ECMSL is a custom and simple text-based language.  
Due to the nature of the parser, a new line (blank line) is required at the end of file to indicate that the script is over. Blank lines within the code will be evaluated as end of file and rest of the file won't be read.
Before moving on into details, here's an example snippet that prints "Hello World!" as a host message when `!test` is written into the chat.
```
CMSChat ExampleChatContent
Trigger ChatPhraseExact !test
HostMessage Hello World!

```

## Types
Here are the types that are recognized by ECMSL.
<br /><br />

### Number
An unsigned (non-negative) integer value.
<br /><br />

### Text
A string value.
<br /><br />

### Color
Either `DEFAULT` (which is `#FFE600`) or a 6-digit hexadecimal color code value prefixed with a hash (`#`).
<br /><br />

### Boolean
Either `Yes` (which means `true`) or `No` (which means `false`).
<br /><br />

### TextOrientation
Used for determining the position of the text within it's container. Takes either `Left`, `Center` or `Right`.
<br /><br />

### DialogOrientation
Used for determining the position of the popup dialog. Takes the combination of the vertical and horizontal orientations.  
Vertical orientations are: `Top`, `Middle` and `Bottom`.  
Horizontal orientations are: `Left`, `Center` and `Right`.  
`None` is used alone, and disables orientation.  
For example, `MiddleCenter` and `BottomLeft` are valid usages, first one places the dialog to center of the screen and second one places the dialog to the bottom left corner of the screen. However, `CenterMiddle` and `LeftBottom` are not valid and will generate an error.
<br /><br />

### Vector2
Takes two Number values in the format of `X Y`. For example: `15 8`.
<br /><br />


## Header
Header can be one of these 3 values: `CMSChat`, `CMSMenuDialog` and `CMSPopupDialog`.
<br /><br />

### CMSChat
**Parameters**:
- **Name** (Text): Name of this definition.

**Description**: The header to define a chat content.  
**Example**: `CMSChat ExampleChatContent`
<br /><br />

### CMSMenuDialog
**Parameters**:
- **Name** (Text): Name of this definition.
- **Close on Esc** (Boolean): Whether or not to close this dialog when Esc key is hit.

**Description**: The header to define a menu dialog content.  
**Example**: `CMSMenuDialog ExampleMenuDialogContent No`
<br /><br />

### CMSPopupDialog
**Parameters**:
- **Name** (Text): Name of this definition.
- **Position** (Vector2): Position of this dialog on the screen. (Ignored if **Orientation** is not set to `None`)
- **Size** (Vector2): Size of the dialog.
- **Orientation** (DialogOrientation): Orientation of the dialog on the screen.
- **Close on Esc** (Boolean): Whether or not to close this dialog when Esc key is hit.
- **Title** (Text): Title of the dialog. Can be empty.

**Description**: The header to define a popup dialog content.  
**Example**: `CMSPopupDialog 0 0 300 200 MiddleCenter Yes Example Dialog`
<br /><br />


## Content
Below are the tags to define the content elements.
<br /><br />

### DialogText
**Applicable Types:** `CMSMenuDialog`, `CMSPopupDialog`  
**Parameters**:
- **Position** (Vector2): Position of this element on the screen.
- **Size** (Vector2): Size of the element.
- **Color** (Color): Color of the text.
- **Orientation** (TextOrientation): Orientation of the element within it's container.
- **Data** (Text): The text to display on the element.

**Description**: Displays a body text on dialog content.  
**Example**: `DialogText 4 36 200 8 DEFAULT Left Welcome to the server guide, {PLAYER}!`
<br /><br />

### DialogTitle
**Applicable Types:** `CMSMenuDialog`, `CMSPopupDialog`  
**Parameters**:
- **Position** (Vector2): Position of this element on the screen.
- **Size** (Vector2): Size of the element.
- **Color** (Color): Color of the text.
- **Orientation** (TextOrientation): Orientation of the element within it's container.
- **Data** (Text): The text to display on the element.

**Description**: Displays a title text on dialog content.  
**Example**: `DialogTitle 4 4 400 30 DEFAULT Center Server Rules`
<br /><br />

### DialogImage
**Applicable Types:** `CMSMenuDialog`, `CMSPopupDialog`  
**Parameters**:
- **Position** (Vector2): Position of this element on the screen.
- **Size** (Vector2): Size of the element.
- **Data** (Text): Full name of the texture file to display.

**Description**: Displays a texture on the screen.  
**Example**: `DialogImage 5 65 50 50 serverlogo.dds`
<br /><br />

### CloseBorderedButton
**Applicable Types:** `CMSMenuDialog`, `CMSPopupDialog`  
**Parameters**:
- **Position** (Vector2): Position of this element on the screen.
- **Size** (Vector2): Size of the element.
- **Data** (Text): The text to display on the element.

**Description**: Displays a bordered button that closes the dialog when pressed.  
**Example**: `CloseBorderedButton 350 283 45 12 Close`
<br /><br />

### CloseTitleButton
**Applicable Types:** `CMSMenuDialog`, `CMSPopupDialog`  
**Parameters**:
- **Position** (Vector2): Position of this element on the screen.
- **Size** (Vector2): Size of the element.
- **Data** (Text): The text to display on the element.

**Description**: Displays a title button that closes the dialog when pressed.  
**Example**: `CloseTitleButton 5 270 45 25 Back`
<br /><br />

### CloseImageButton
**Applicable Types:** `CMSMenuDialog`, `CMSPopupDialog`  
**Parameters**:
- **Position** (Vector2): Position of this element on the screen.
- **Size** (Vector2): Size of the element.
- **Data** (Text): The texture to display on the element.

**Description**: Displays an image button that closes the dialog when pressed.  
**Example**: `CloseImageButton 390 5 5 5 closebtn.tga`
<br /><br />

### HostMessage
**Applicable Types:** `CMSChat`  
**Parameters**:
- **Data** (Text): The text to display on the chat.

**Description**: Displays a text on the chat as a host message.  
**Example**: `HostMessage You are currently playing in {MAP}.`
<br /><br />

### ColorMessage
**Applicable Types:** `CMSChat`  
**Parameters**:
- **Color** (Color): Color of the text.
- **Data** (Text): The text to display on the chat.

**Description**: Displays a raw text on the chat with a color.  
**Example**: `ColorMessage #9999FF Welcome to {TITLE}, {PLAYER}!`
<br /><br />

### TeamColorMessage
**Applicable Types:** `CMSChat`  
**Parameters**:
- **Data** (Text): The text to display on the chat.

**Description**: Displays a raw text on the chat with the color of player's team.  
**Example**: `TeamColorMessage This message has the color of your team!`
<br /><br />

### HUDMessage
**Applicable Types:** `CMSChat`  
**Parameters**:
- **Color** (Color): Color of the text.
- **Data** (Text): The text to display on the chat.

**Description**: Displays a text in the middle of player's screen with color.  
**Example**: `HUDMessage #FF1111 You have angered TeamWolf!`
<br /><br />

### AdminMessage
**Applicable Types:** `CMSChat`  
**Parameters**:
- **Data** (Text): The text to display on the chat.

**Description**: Displays a message popup in the middle of player's screen.  
**Example**: `AdminMessage Please submit crash dumps to tiberiantechnologies.org/Contact for a resolution for your issue.`
<br /><br />

### Sound
**Applicable Types:** `CMSMenuDialog`, `CMSPopupDialog`, `CMSChat`  
**Parameters**:
- **Data** (Text): The sound to play.

**Description**: Plays a sound.  
**Example**: `Sound amb_airraid.wav`
<br /><br />


## Triggers
Below are the methods to activate CMS definitions.
<br /><br />

### ChatPhraseExact
**Parameters**:
- **Data** (Text): The phrase to look for.

**Description**: Checks if the message sent by the player equals to the trigger value. (Case-sensitive)  
**Example:** `Trigger ChatPhraseExact !help`
<br /><br />

### ChatPhraseMatch
**Parameters**:
- **Data** (Text): The phrase to look for.

**Description**: Checks if the message sent by the player contains trigger value. (Case-insensitive)  
**Example:** `Trigger ChatPhraseMatch lag`
<br /><br />

### KeyHook
**Parameters**:
- **Data** (Text): Name of the key hook.

**Description**: Activates the CMS definition when player triggers a key hook.  
**Example:** `Trigger KeyHook PhoneHelp`
<br /><br />

### PlayerJoin
**Parameters**: _None_  
**Description**: Activates the CMS definition when player joins the game.  
**Example:** `Trigger PlayerJoin`
<br /><br />

## Examples
Here are more examples of ECMSL.

```
CMSChat ExampleChatModule
Trigger ChatPhraseExact !example
Trigger ChatPhraseMatch potato
HostMessage Hello world!
ColorMessage #FF0000 This is a red message!
ColorMessage #00FF00 And this is a green message!
TeamColorMessage This message has the color of your team's color!
HUDMessage #7777FF Essentials is a cool plugin!
HostMessage Player Name = {PLAYER}
HostMessage Server Title = {TITLE}
HostMessage Current Map = {MAP}
Sound amb_airraid.wav

```

```
CMSChat ExampleJoin
Trigger PlayerJoin
ColorMessage #FFFFFF Welcome to the server {PLAYER}! This server is powered by Essentials!

```

```
CMSMenuDialog ExampleMenuDialog Yes
Trigger ChatPhraseExact !menu
DialogTitle 5 5 390 35 DEFAULT Center Example Menu Dialog
DialogText 5 45 390 8 DEFAULT Left This is an example menu dialog provided by Essentials' CMS.
DialogText 5 53 390 8 DEFAULT Left You can close this menu by pressing the Esc key. Or alternatively;
CloseBorderedButton 5 62 100 12 Close Dialog
CloseTitleButton 5 280 50 15 Back

```

```
CMSPopupDialog ExamplePopupDialog 0 0 300 200 MiddleCenter Yes Example Popup Dialog
Trigger ChatPhraseExact !popup
DialogText 4 4 292 8 DEFAULT Left This is an example popup dialog. Pressing Esc closes this popup.
DialogText 4 30 292 8 DEFAULT Left Here is a photo of Westwood's real-life Chameleon:
DialogImage 4 39 75 75 deathtransitioneffect.dds
DialogText 120 70 292 8 DEFAULT Left Clicking this image below also closes this dialog.
CloseImageButton 120 79 75 75 chameleontexture.dds

```
