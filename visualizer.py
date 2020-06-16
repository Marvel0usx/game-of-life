from tkinter import *
from typing import Optional, Callable, Tuple, Set
from os import system


class GameOfLifeVisualizer():
    """ This class is the GUI interface for the 2D Game-of-live visualizer.

    === Private Attributes ===
    _root : the root of tk
    """
    _root: Tk

    def __init__(self, root=None) -> None:
        self._root = root
        self.__initialize_tk()
        self.__initialize_app()

    def __initialize_tk(self) -> None:
        # Initialize tk options
        self._root.title("Game of Life - 2D Visualizer")
        self._root.resizable(height=False, width=False)
        self.__initialize_color_commons()

    def __initialize_color_commons(self) -> None:
        self._root.option_add('*EntryField.Entry.background', 'white')
        self._root.option_add('*MessageBar.Entry.background', 'gray85')
        self._root.option_add('*Listbox*background', 'white')
        self._root.option_add('*Listbox*foreground', 'black')
        self._root.option_add('*Listbox*selectBackground', 'dark slate blue')
        self._root.option_add('*Listbox*selectForeground', 'white')

    def __initialize_app(self) -> None:
        self.__build_main_window()

    def build_menubar(self, ca: Callable, lf: Callable, sa: Callable) -> None:
        """Function to inject, and bind the functions: clear_all, load_file, and save_all.
        """
        menubar = Menu(master=self._root)
        file_menu = Menu(master=menubar, tearoff=0)
        edit_menu = Menu(master=menubar, tearoff=0)
        edit_menu.add_command(label="Clear All", command=ca)
        # TODO change dummy methods
        file_menu.add_command(
            label="Load File", command=lf, accelerator="(Ctrl+O)")
        file_menu.add_command(label="Save As", command=sa,
                              accelerator="(Ctrl+S)")
        menubar.add_cascade(label="File", menu=file_menu)
        menubar.add_cascade(label="Edit", menu=edit_menu)
        menubar.add_command(label="Exit", command=lambda: self._root.quit())
        menubar.add_command(label="\u22EE",
                            activebackground=menubar.cget("background"), state=DISABLED)
        menubar.add_command(label="About", command=lambda: system(
            "start www.github.com/Marvel0usx/game-of-life"))
        self._root.config(menu=menubar)

    def __build_main_window(self) -> None:
        self.__build_canvas()
        self.__build_control_area()

    def __build_canvas(self) -> None:
        self.cvs_fr = Frame(width=800, height=560)
        self.cvs_fr.pack(side=TOP)

    def __build_control_area(self) -> None:
        self.btn_start = Button(master=self._root, text="START", width=28,
                                state=ACTIVE)
        self.btn_pause = Button(master=self._root, text="PAUSE", width=28,
                                state=DISABLED)
        self.btn_stop = Button(
            master=self._root, text="STOP", state=DISABLED, width=28)
        self.slider = Scale(self._root, from_=1, to=30, orient=HORIZONTAL)
        self.slider.set(1)

        self.btn_start.pack(side=LEFT, fill=Y, pady=1, padx=1)
        self.btn_pause.pack(side=LEFT, fill=Y, pady=1, padx=1)
        self.btn_stop.pack(side=LEFT, fill=Y, pady=1, padx=1)
        self.slider.pack(side=RIGHT, fill=BOTH, expand=True, padx=1)


class CvsView(Canvas):
    """Class extending the Canvas widget supports some extra attributes

    === Attributes ===
    prev_x : x-coordinate
    prev_y : y-coordinate
    cells : set of all points on the canvas
    """
    prev_x : Optional[int]
    prev_y : Optional[int]
    cells : Set[Tuple[int, int]]

    def __init__(self, **kwargs) -> None:
        super().__init__(**kwargs)
        self.prev_x = None
        self.prev_y = None
        self.cells = set()
