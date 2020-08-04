# game-of-life
A visualization of 2D Conway's Game of Life based on CPython module and threaded GUI by tkinter.

### The Game

The Game of Life is not a typical computer game. It is a 'cellular automaton', and was invented by Cambridge mathematician John Conway.

This game became widely known when it was mentioned in an article published by Scientific American in 1970. It consists of a collection of cells which, based on a few mathematical rules, can live, die or multiply. Depending on the initial conditions, the cells form various patterns throughout the course of the game.

### The Rules

- **For a space that is 'populated':**

  Each cell with one or no neighbors dies, as if by solitude.

  Each cell with four or more neighbors dies, as if by overpopulation.

  Each cell with two or three neighbors survives.

- **For a space that is 'empty' or 'unpopulated'**

  Each cell with three neighbors becomes populated.

### The Controls

Draw the figure that you want on the canvas. The left button on your mouse is drawing, and the right is erasing. After drawing the figure, click on start to begin the simulation. You can use scroll to zoom in and out, and use the widget at the bottom-right to adjust the speed of each generation.

### Initiative

Try to write Python module in C, and study the interpreter of Python (2.7.8).

### Screenshots

![](https://haoyan.org/assets/images/gof&#32;(2).png =200px)

![](https://haoyan.org/assets/images/gof&#32;(1).png =200px)

