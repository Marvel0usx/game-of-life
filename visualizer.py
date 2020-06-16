from utils import *
from os import system
from tkinter import *
from typing import Tuple
from tkinter.filedialog import asksaveasfilename, askopenfilename
from tkinter.messagebox import showerror
import sys
import pickle

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
        self.prev_x = None
        self.prev_y = None
        self.__initialize_tk()
        self.__initialize_app()
        self.cells = set()

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
        self.__build_menubar()
        self.__build_main_window()

    def __build_menubar(self) -> None:
        menubar = Menu(master=self._root)
        file_menu = Menu(master=menubar, tearoff=0)
        edit_menu = Menu(master=menubar, tearoff=0)
        # TODO change dummy methods
        file_menu.add_command(label="Load File", command=self._load_file)
        file_menu.add_command(label="Save As", command=self._save_as)
        file_menu.add_command(label="Close Current", command=self._close_map)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self._root.quit)
        edit_menu.add_command(label="Clear All", command=lambda: self._canvas.delete(ALL))
        menubar.add_cascade(label="File", menu=file_menu)
        menubar.add_cascade(label="Edit", menu=edit_menu)
        menubar.add_command(label="About", command=lambda: system(
            "start www.github.com/Marvel0udx/game-of-life"))
        self._root.config(menu=menubar)

    def __build_main_window(self) -> None:
        self.__build_canvas()
        self.__build_control_area()

    def __build_canvas(self) -> None:
        self._canvas = Canvas(master=self._root, width=800, height=560, 
                              relief=GROOVE, bg="gray95")
        self._canvas.pack(side=TOP)
        self._canvas.bind("<B1-Motion>", lambda event: self._paint(event, 1))
        self._canvas.bind('<ButtonRelease-1>', self._reset_cvs)
        self._canvas.bind("<B3-Motion>", lambda event: self._paint(event, 0))
        self._canvas.bind("<MouseWheel>", self._zoom)

    def __build_control_area(self) -> None:
        self._b_start = Button(master=self._root, text="START", width=28,
                               state=ACTIVE, command=self._start_process)
        self._b_pause = Button(master=self._root, text="PAUSE", width=28,
                               state=DISABLED, command=self._pause_process)
        self._b_stop = Button(master=self._root, text="STOP", state=DISABLED,
                              command=self._stop_process, width=28)
        self.slider = Scale(self._root, from_=1, to=30, orient=HORIZONTAL)
        self.slider.set(1)

        self._b_start.pack(side=LEFT, fill=Y, pady=1, padx=1)
        self._b_pause.pack(side=LEFT, fill=Y, pady=1, padx=1)
        self._b_stop.pack(side=LEFT, fill=Y, pady=1, padx=1)
        self.slider.pack(side=RIGHT, fill=BOTH, expand=True, padx=1)

    @property
    def is_running(self):
        return self._is_running.get()

    def _start_process(self) -> None:
        if self.is_running:
            self._b_pause.config(state=ACTIVE)
        else:
            self._is_running.set(value=True)
            self._b_start.config(state=DISABLED)
            self._b_pause.config(state=ACTIVE)
            self._b_stop.config(state=ACTIVE)
    
    def _pause_process(self) -> None:
        if self.is_running:
            self._b_pause.config(state=DISABLED)
            self._b_start.config(state=ACTIVE)
    
    def _stop_process(self) -> None:
        if self.is_running:
            self._b_start.config(state=ACTIVE)
            self._b_stop.config(state=DISABLED)
            self._b_pause.config(state=DISABLED)
            self._is_running.set(value=False)

    def _paint(self, new, flag): 
        if self.prev_x and self.prev_y:
            if self.prev_x != new.x and self.prev_y == new.y:
                self.cells.add((new.x, self.prev_y))
            if self.prev_y != new.y and self.prev_x == new.x:
                self.cells.add((self.prev_x, new.y))
            if self.prev_x != new.x and self.prev_y != new.y:
                self.cells.add((new.x, new.y))
            self._canvas.create_line(self.prev_x, self.prev_y, new.x, new.y, width=self.slider.get(), tag="ddd", fill=("gray95", "black")[flag], capstyle=ROUND, smooth=True)
        self.prev_x = new.x
        self.prev_y = new.y

    def _zoom(self, event):
        factor = 1.001 ** event.delta
        self._canvas.scale(ALL, event.x, event.y, factor, factor)
        
    def _reset_cvs(self, e):
        self.prev_x = None
        self.prev_y = None
        
    def _save_as(self):
        extensions = [("Pickled Files", "*.dat"),
                      ("CSV Files", "*.csv")]
        path = asksaveasfilename(initialdir=r"/", title="Filename", filetypes=extensions, defaultextension=".dat")
        if not path:
            return False
        try:
            with open(path, "wb") as handler:
                pickle.dump(self.cells, handler)
        except Exception as e:
            showerror(f"Could not save {str(path)}.")

    def _load_file(self):
        extensions = [("Pickled Files", "*.dat"),
                      ("CSV Files", "*.csv")]
        path = askopenfilename()
        if not path:
            return False
        try:
            with open(path, "rb") as handler:
                self.cells = pickle.load(handler)
            for pt in self.cells:
                self._canvas.create_line(pt[0], pt[1], pt[0]+1, pt[1]+1, fill="black", width=self.slider.get())
        except Exception as e:
            showerror(f"Could not open {str(path)}.")

    def _close_map(self):
        self._stop_process()
        self._reset_cvs(None)
        self.cells = set()
        self._canvas.delete(ALL)


if __name__ == "__main__":
    root = Tk()
    vis = GameOfLifeVisualizer(root)
    root.mainloop()
