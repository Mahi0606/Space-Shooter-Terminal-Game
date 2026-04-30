# Space Shooter (Terminal)

A horizontal space shooter that runs entirely in the terminal: ANSI colors, a retro-style HUD, waves of enemies, bosses, terrain, power-ups, and optional sound effects.

## Requirements

- **Compiler:** GCC (or Clang) with **C11** support  
- **OS:** Unix-like terminal (**macOS** or Linux). The game uses POSIX APIs (`termios`, `select`, etc.) for raw keyboard input and terminal control.  
- **Sound (optional):** On **macOS**, effects use `afplay` and built-in `/System/Library/Sounds` files. On other systems, link still succeeds; if `afplay` is missing, sound calls may no-op or fail at runtime depending on your environment.

## Build and run

```bash
make          # produces ./space_shooter
make run      # build (if needed) and launch
make clean    # remove object files and binary
```

## How to play

From the **main menu**:

| Key | Action        |
|-----|---------------|
| `1` | Start game    |
| `2` | View controls |
| `3` | Quit          |

### In-game controls

| Key | Action |
|-----|--------|
| `W` / **Up** | Move up |
| `S` / **Down** | Move down |
| `A` / **Left** | Move backward (left) |
| `D` / **Right** | Move forward (right) |
| **Space** | Fire |
| `P` | Pause |
| `Q` | Quit to menu |

Arrow keys are supported (mapped like WASD).

### Objective and difficulty

- Destroy enemies for **score**. Difficulty ramps with **level** (faster spawns, tougher pacing).  
- **Levels** advance as your score crosses thresholds (every 200 points by default).  
- **Bosses** appear on a fixed level interval of 5.  
- **Game over** if your ship reaches **0 HP** or too many enemies **escape** off the left edge (see in-game controls screen for the exact escape limit).

### Power-ups (drops when you destroy enemies)

| Pickup | Effect |
|--------|--------|
| `+` | Restore **20 HP** |
| `F` | **Rapid fire** for 10s time |
| `O` | **Shield** — absorbs 5 hits |
| `D` | **Double shot** for 10s time |

### Tips

- Use a **large terminal** window for the best layout; the game can react to **resize** while in menus.  
- If the display looks wrong, try a terminal with good ANSI color support.

## Project layout

| Area | Role |
|------|------|
| `game.c` / `game.h` | Core loop, entities, collisions, modes |
| `screen.c` / `screen.h` | Buffer, colors, drawing, terminal size |
| `hud.c` / `hud.h` | Top/bottom HUD and notifications |
| `boost.c` / `boost.h` | Power-up spawn, update, pickup |
| `sound.c` / `sound.h` | Non-blocking SFX (macOS `afplay`) |
| `keyboard.c` | Raw, non-blocking input |
| `memory.c`, `string.c`, `math.c` | Small helpers used across the game |
