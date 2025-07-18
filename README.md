# LazyGenericRichTextEnhancedInput Plugin

A dynamic Unreal Engine plugin that automatically displays input key icons in Rich Text widgets based on the player's current input device and Enhanced Input bindings.

## Features

- 🎮 **Dynamic Input Detection**: Automatically switches between keyboard/mouse and gamepad icons
- 🔄 **Real-time Updates**: Icons update instantly when players switch input devices
- 🎨 **Customizable Display**: Flexible sizing, stretching, and styling options
- 🖥️ **Multi-Device Support**: Support for different gamepad types with device-specific icon sets
- 🎯 **Enhanced Input Integration**: Works seamlessly with Unreal's Enhanced Input system
- 📝 **Rich Text Integration**: Easy-to-use markup syntax for designers and developers

## Installation

1. Download or clone this plugin into your project's `Plugins` folder
2. Regenerate project files
3. Build your project
4. Enable the plugin in the Plugin Manager

## Quick Start

### 1. Setup Data Tables

Create data tables based on the `FKeyImageRow` structure:

**For Keyboard/Mouse icons:**
```cpp
// Create a Data Table with FKeyImageRow as the base struct
// Add rows for each key you want to display (e.g., W, A, S, D, Space, etc.)
```

**For Gamepad icons:**
```cpp
// Create separate data tables for different gamepad types
// Use hardware device identifiers as keys (e.g., "XboxController", "PS5Controller")
```

### 2. Configure the Decorator

1. Create a Blueprint class inheriting from `ULazyRichTextEnhancedInputDecorator`
2. Set up the following properties:
   - **Keyboard Mouse Images**: Assign your keyboard/mouse data table
   - **Gamepad Images**: Map device names to their respective data tables
   - **Action Map**: Map action names to your Input Action assets

### 3. Add to Rich Text Block

1. In your Rich Text Block widget, add your decorator to the **Decorator Classes** array
2. Use the markup syntax in your text:

```xml
Press <input action="Jump"/> to jump!
Move with <input action="Move" width="32" height="32"/>
```

## Markup Syntax

### Basic Usage
```xml
<input action="ActionName"/>
```

### With Custom Sizing
```xml
<input action="ActionName" width="24" height="24"/>
<input action="ActionName" width="desired" height="desired"/>
```

### With Stretch Options
```xml
<input action="ActionName" stretch="ScaleToFit"/>
<input action="ActionName" stretch="ScaleToFill"/>
<input action="ActionName" stretch="UserSpecified"/>
```

## Configuration Examples

### Blueprint Setup
```cpp
// In your decorator Blueprint class
ActionMap:
- "Jump" -> IA_Jump
- "Move" -> IA_Move  
- "Interact" -> IA_Interact

KeyboardMouseImages: DT_KeyboardIcons
GamepadImages:
- "XboxController" -> DT_XboxIcons
- "PS5Controller" -> DT_PS5Icons
```

### Data Table Structure
```cpp
// FKeyImageRow structure
Row Name: W
Key: W
Image: [Your W key icon texture]

Row Name: Gamepad_FaceButton_Bottom
Key: Gamepad Face Button Bottom
Image: [Your A/X button icon texture]
```

## API Reference

### ULazyRichTextEnhancedInputDecorator

#### Properties
- `KeyboardMouseImages` - Data table for keyboard and mouse icons
- `GamepadImages` - Map of device types to their icon data tables  
- `ActionMap` - Map of action names to Input Action assets
- `EditorBrush` - Preview brush for editor display

#### Methods
- `FindImageBrush(const FKey& Key, const APlayerController* PlayerController)` - Find appropriate icon for a key
- `CreateDecorator(URichTextBlock* InOwner)` - Create the text decorator instance

### FKeyImageRow

#### Properties
- `Key` - The input key this row represents
- `Image` - The slate brush/icon for this key

## Examples

### Basic Movement Instructions
```xml
Use <input action="Move"/> to move around the world.
```

### Combat Tutorial
```xml
Press <input action="Attack" width="24"/> to attack enemies.
Hold <input action="Block" height="20"/> to defend yourself.
```

### Menu Navigation
```xml
Navigate menus with <input action="Navigate"/> and confirm with <input action="Confirm"/>.
```

## Known Limitations

- Only displays one key per action (first valid binding found)
- No support for modifier key combinations (Ctrl+S, Alt+Tab, etc.)
- Requires manual setup of data tables and action mappings
- Limited animation support for device switching

## License

Copyright (C) 2025 Job Omondiale - All Rights Reserved

## Support

For issues, feature requests, or contributions, please use the plugin's GitHub repository.

This plugin is licensed under the BSD-3-Clause license found in the
LICENSE file in the root directory of this source tree.
