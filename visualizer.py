from tkinter import *
from tkinter.filedialog import asksaveasfilename, askopenfilename
from tkinter.messagebox import showerror
from typing import Tuple
from os import system
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
    _root: Tk
    _col: int
    _row: int
    _map: Tuple[Tuple[int]]
    _is_running: BooleanVar

    def __init__(self, root=None) -> None:
        self._root = root
        self._col = StringVar()
        self._row = StringVar()
        self._map = None
        self.prev_x = None
        self.prev_y = None
        self.__initialize_tk()
        self.__initialize_app()
        self.cells = set()
        self.is_running = False

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

    def build_menubar(self, ca, lf, sa) -> None:
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
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.prev_x = None
        self.prev_y = None
        self.cells = set()


class Controller:
    def __init__(self, root):
        self._root = root
        self.is_running = False
        self._view = GameOfLifeVisualizer(root)
        self._canvas = CvsView(master=self._view.cvs_fr, width=800, height=560,
                               relief=GROOVE, bg="gray95")
        self._canvas.pack(fill=BOTH, expand=TRUE)
        self._bind_all()

    def _bind_all(self):
        self._view.build_menubar(
            lambda: self._canvas.delete(ALL),
            self._load_file,
            self._save_as)
        self._view.btn_start.config(command=self._start_process)
        self._view.btn_pause.config(command=self._pause_process)
        self._view.btn_stop.config(comman=self._stop_process)
        self._canvas.bind("<B1-Motion>", lambda event: self._paint(event, 1))
        self._canvas.bind('<ButtonRelease-1>', self._reset_cvs)
        self._canvas.bind("<B3-Motion>", lambda event: self._paint(event, 0))
        self._canvas.bind("<MouseWheel>", self._zoom)
        self._root.bind("<Control-s>", lambda e: self._save_as())
        self._root.bind("<Control-o>", lambda e: self._load_file())

    def _paint(self, new, flag):
        if self._canvas.prev_x and self._canvas.prev_y:
            if self._canvas.prev_x != new.x and self._canvas.prev_y == new.y:
                self._canvas.cells.add((new.x, self._canvas.prev_y))
            if self._canvas.prev_y != new.y and self._canvas.prev_x == new.x:
                self._canvas.cells.add((self._canvas.prev_x, new.y))
            if self._canvas.prev_x != new.x and self._canvas.prev_y != new.y:
                self._canvas.cells.add((new.x, new.y))
            self._canvas.create_line(self._canvas.prev_x, self._canvas.prev_y, new.x, new.y, width=self._view.slider.get(
            ), fill=("gray95", "black")[flag], capstyle=ROUND, smooth=True)
        self._canvas.prev_x = new.x
        self._canvas.prev_y = new.y

    def _zoom(self, event):
        factor = 1.001 ** event.delta
        self._canvas.scale(ALL, event.x, event.y, factor, factor)

    def _reset_cvs(self, e):
        self._canvas.prev_x = None
        self._canvas.prev_y = None

    def _start_process(self) -> None:
        if self.is_running:
            self._view.btn_pause.config(state=ACTIVE)
        else:
            self.is_running = True
            self._view.btn_start.config(state=DISABLED)
            self._view.btn_pause.config(state=ACTIVE)
            self._view.btn_stop.config(state=ACTIVE)

    def _pause_process(self) -> None:
        if self.is_running:
            self._view.btn_pause.config(state=DISABLED)
            self._view.btn_start.config(state=ACTIVE)

    def _stop_process(self) -> None:
        if self.is_running:
            self._view.btn_start.config(state=ACTIVE)
            self._view.btn_stop.config(state=DISABLED)
            self._view.btn_pause.config(state=DISABLED)
            self.is_running = False

    def _save_as(self):
        extensions = [("Pickled Files", "*.dat"),
                      ("CSV Files", "*.csv")]
        path = asksaveasfilename(
            initialdir=r"/", title="Filename", filetypes=extensions, defaultextension=".dat")
        if not path:
            return False
        try:
            with open(path, "wb") as handler:
                pickle.dump(self._canvas.cells, handler)
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
                cells = pickle.load(handler)
                # do not replace if error ocurrs
                self._canvas.cells = cells
            for pt in self._canvas.cells:
                self._canvas.create_line(
                    pt[0], pt[1], pt[0]+1, pt[1]+1, fill="black", width=self._view.slider.get())
        except Exception as e:
            showerror(f"Could not open {str(path)}.")


if __name__ == "__main__":
    root = Tk()
    Controller(root)
    root.mainloop()
