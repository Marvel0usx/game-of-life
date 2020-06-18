from visualizer import GameOfLifeVisualizer, CvsView
from tkinter import *
from tkinter.filedialog import asksaveasfilename, askopenfilename
from tkinter.messagebox import showerror
import pickle
from threading import Thread, Event, RLock
from map import Map


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
        self.is_running = Event()
        self.is_terminated = Event()
        self.is_running.clear()
        self.is_terminated.clear()
        self._cvs_lock = RLock()
        self._mao = None
        self._map_iter = None
        self._update_t = None
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
        with self._cvs_lock:
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
        with self._cvs_lock:
            self._canvas.scale(ALL, event.x, event.y, factor, factor)

    def _reset_cvs(self, e) -> None:
        with self._cvs_lock:
            self._canvas.prev_x = None
            self._canvas.prev_y = None

    def _start_process(self) -> None:
        if self.is_running.is_set():
            self._view.btn_pause.config(state=ACTIVE)
        else:
            self.is_running.set()
            self._view.btn_start.config(state=DISABLED)
            self._view.btn_pause.config(state=ACTIVE)
            self._view.btn_stop.config(state=ACTIVE)

            self._set_all()
            self._update_t = Thread(target=self._update_canvas, daemon=True)
            self._update_t.start()

    def _update_canvas(self):
        for new_map in self._map_iter:
            if self.is_terminated.is_set():
                return
            self.is_running.wait()
            with self._cvs_lock:
                self._canvas.delete(ALL)
                self._cvs_draw_cells(new_map)

    def _cvs_draw_cells(self, cells):
        self._canvas.cells = cells
        for pt in self._canvas.cells:
            self._canvas.create_line(
                pt[0], pt[1], pt[0]+1, pt[1]+1, fill="black", width=self._view.slider.get())

    def _unset_all(self):
        self._mao = None
        self._map_iter = None

    def _set_all(self):
        row, col = max(self._canvas.cells, key=lambda pt: pt[0]), \
               max(self._canvas.cells, key=lambda pt: pt[1])
        init_list = self._cvs2map(row, col)
        self._mao = Map(row, col)
        self._mao.set_map(init_list)
        self._map_iter = self._mao.get_iter(-1)

    def _cvs2map(self, row, col):
        map = [0 for _ in range(row * col)]
        for pt in self._canvas.cells:
            map[pt[0] * col + pt[1]] = 1
        return map

    def _pause_process(self) -> None:
        if self.is_running.is_set():
            self.is_running.clear()
            self._view.btn_pause.config(state=DISABLED)
            self._view.btn_start.config(state=ACTIVE)

    def _stop_process(self) -> None:
        if self.is_running.is_set():
            self.is_running.clear()
            self.is_terminated.set()
            self._unset_all()

            self._view.btn_start.config(state=ACTIVE)
            self._view.btn_stop.config(state=DISABLED)
            self._view.btn_pause.config(state=DISABLED)

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
                with self._cvs_lock:
                    self._canvas.delete(ALL)
                    self._cvs_draw_cells(cells)
        except Exception as e:
            showerror(f"Could not open {str(path)}.")


if __name__ == "__main__":
    root = Tk()
    Controller(root)
    root.mainloop()
