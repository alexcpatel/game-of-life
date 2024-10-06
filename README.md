# Conway's Game of Life

![Example GIF](example.gif)

This is an implementation of Conway's Game of Life using C++ and SFML. The Game of Life is a cellular automaton simulation devised by mathematician John Conway.

## Controls

- Left-click: Draw/erase cells
- Space: Play/Pause simulation
- Right arrow: Step forward
- Left arrow: Step backward
- R: Randomize grid
- C: Clear grid
- S: Toggle simulation speed

## Demo

![Demo GIF](demo.gif)

## Building and Running

- SFML must be installed.
- Example VS Code build task (tested on Apple M2):
```
{
    "type": "cppbuild",
    "label": "C/C++: clang++ build active file",
    "command": "/usr/bin/clang++",
    "args": [
        "-fcolor-diagnostics",
        "-fansi-escape-codes",
        "-g",
        "${file}",
        "-I",
        "/opt/homebrew/Cellar/sfml/2.6.1/include",
        "-o",
        "${fileDirname}/${fileBasenameNoExtension}",
        "-L",
        "/opt/homebrew/Cellar/sfml/2.6.1/lib",
        "-lsfml-graphics",
        "-lsfml-window",
        "-lsfml-system"
    ],
    "options": {
        "cwd": "${fileDirname}"
    },
    "problemMatcher": [
        "$gcc"
    ],
    "group": "build",
    "detail": "Build game."
}
```
- Run the executable.

## Dependencies

- SFML 2.5 or higher
- C++11 or higher
