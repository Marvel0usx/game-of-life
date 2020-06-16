from visualizer import GameOfLifeVisualizer, CvsView
from tkinter import *
from tkinter.filedialog import asksaveasfilename, askopenfilename
from tkinter.messagebox import showerror
import pickle


class Controller:
    """Bridge between View and the Map Model. Thread dispatcher.

    === Private Attributes ===
    _root : tkinter root
    _view : window view
    _canvas : canvas view
    _mao : map access object

    === Attributes ===
    is_running : running state of visualizer
    """

    def __init__(self, root) -> None:
        self._root = root
        self.is_running = False
        self._view = GameOfLifeVisualizer(root)
        self._canvas = CvsView(master=self._view.cvs_fr, width=800, height=560,
                               relief=GROOVE, bg="gray95")
        self._canvas.pack(fill=BOTH, expand=TRUE)
        self._bind_all()

    def _bind_all(self) -> None:
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

    def _paint(self, new, flag) -> None:
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

    def _zoom(self, event) -> None:
        factor = 1.001 ** event.delta
        self._canvas.scale(ALL, event.x, event.y, factor, factor)

    def _reset_cvs(self, e) -> None:
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

    def _save_as(self) -> None:
        extensions = [("Pickled Files", "*.dat"),
                      ("CSV Files", "*.csv")]
        path = asksaveasfilename(
            initialdir=r"/", title="Filename", filetypes=extensions, defaultextension=".dat")
        if not path:
            return
        try:
            with open(path, "wb") as handler:
                pickle.dump(self._canvas.cells, handler)
        except Exception as e:
            showerror(f"Could not save {str(path)}.")

    def _load_file(self) -> None:
        extensions = [("Pickled Files", "*.dat"),
                      ("CSV Files", "*.csv")]
        path = askopenfilename()
        if not path:
            return
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
