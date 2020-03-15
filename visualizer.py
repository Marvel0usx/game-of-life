from utils import *
from os import system
from tkinter import *
from typing import Tuple
import sys

class GameOfLifeVisualizer():
    """ This class is the GUI interface for the 2D Game-of-live visualizer.

    === Private Attributes ===
    _root : the root of tk
    _col  : number of columns on the map
    _row  : number of rows on the map
    _map  : the map
    _is_running : state of the game
    """
    _root : Tk
    _col  : int
    _row  : int
    _map  : Tuple[Tuple[int]]
    _is_running : BooleanVar

    def __init__(self, root=None) -> None:
        self._root = root
        self._col = StringVar()
        self._row = StringVar()
        self._map = None
        self._is_running = BooleanVar(value=False)

        self.__initialize_tk()
        self.__initialize_app()

    def __initialize_tk(self) -> None:
        # Initialize tk options
        self._root.geometry("800x600")
        self._root.title("Game of Life - 2D Visualizer")
        self._root.resizable(height=False, width=False)
        self.__initialize_color_commons()

    def __initialize_color_commons(self) -> None:
        self._root.option_add('*background', 'gray')
        self._root.option_add('*foreground', 'black')
        self._root.option_add('*EntryField.Entry.background', 'white')
        self._root.option_add('*MessageBar.Entry.background', 'gray85')
        self._root.option_add('*Listbox*background', 'white')
        self._root.option_add('*Listbox*foreground', 'black')
        self._root.option_add('*Listbox*selectBackground', 'dark slate blue')
        self._root.option_add('*Listbox*selectForeground', 'white')

    def __initialize_app(self) -> None:
        self.__build_menubar()
        self.__build_main_window()

    def __build_menubar(self) -> None:
        menubar = Menu(master=self._root)
        file_menu = Menu(master=menubar, tearoff=0)
        # TODO change dummy methods
        file_menu.add_command(label="Load File", command=self.dummy)
        file_menu.add_command(label="Save Process", command=self.dummy)
        file_menu.add_command(label="Close Current", command=self.dummy)
        menubar.add_cascade(label="File", menu=file_menu)
        menubar.add_command(label="About", command=lambda: system(
            "start www.github.com/Marvel0udx/game-of-life"))
        self._root.config(menu=menubar)

    def __build_main_window(self) -> None:
        self.__build_canvas()
        self.__build_control_area()

    def __build_canvas(self) -> None:
        self._canvas = Canvas(master=self._root, width=800, height=560, 
                              relief=GROOVE)
        self._canvas.pack(side=BOTTOM, fill=X)

    def __build_control_area(self) -> None:
        self._b_set = Button(master=self._root, text="SET", state=ACTIVE,
                             command=self._set_vars, width=28)
        self._b_start = Button(master=self._root, text="START", width=28,
                               state=DISABLED, command=self._start_process)
        self._b_pause = Button(master=self._root, text="PAUSE", width=28,
                               state=DISABLED, command=self._pause_process)
        self._b_stop = Button(master=self._root, text="STOP", state=DISABLED,
                              command=self._stop_process, width=28)

        self._b_set.pack(side=LEFT, fill=Y, pady=1, padx=1)
        self._b_start.pack(side=LEFT, fill=Y, pady=1, padx=1)
        self._b_pause.pack(side=LEFT, fill=Y, pady=1, padx=1)
        self._b_stop.pack(side=LEFT, fill=Y, pady=1, padx=1)
    

    def dummy(self) -> None:
        pass

    def _set_vars(self) -> None:
        popup = Toplevel(self._canvas, height=50, width=200, pady=15)
        popup.title("Set Value")

        Label(popup, text="Number of Rows", fg="gray98").grid(row=1, column=1, padx=2)
        Label(popup, text="Number of Rows", fg="gray98").grid(row=2, column=1, padx=2)
        Entry(popup, textvariable=self._row, width=25, fg="gray95").grid(
                        column=2, row=1, padx=2, pady=10, sticky=W, columnspan=2)
        Entry(popup, textvariable=self._col, fg="gray95", width=15).grid(
                        column=2, row=2, padx=2, pady=10, sticky=W, columnspan=2)
        popup.transient(self._canvas)
        self._b_start.config(state=ACTIVE)

    def _start_process(self) -> None:
        self._b_start.config(state=DISABLED)
        if self._is_running.get():
            self._b_pause.config(state=ACTIVE)
        else:
            self._is_running.set(value=True)
            self._b_set.config(state=DISABLED)
            self._b_pause.config(state=ACTIVE)
            self._b_stop.config(state=ACTIVE)
    
    def _pause_process(self) -> None:
        self._b_pause.config(state=DISABLED)
        self._b_start.config(state=ACTIVE)
    
    def _stop_process(self) -> None:
        self._b_set.config(state=ACTIVE)
        self._b_start.config(state=DISABLED)
        self._b_stop.config(state=DISABLED)
        self._b_pause.config(state=DISABLED)
        self._is_running.set(value=False)

if __name__ == "__main__":
    root = Tk()
    vis = GameOfLifeVisualizer(root)
    root.mainloop()
