from visualizer import GameOfLifeVisualizer, CvsView
from tkinter import *
from tkinter.filedialog import asksaveasfilename, askopenfilename
from tkinter.messagebox import showerror
import pickle
from threading import Thread, Event, RLock
from map import Map
from time import sleep

PADX = 0
PADY = 0

class Controller:
    """Bridge between View and the Map Model. Thread dispatcher.

    === Private Attributes ===
    _root : tkinter root
    _view : window view
    _canvas : canvas view
    _mao : map access object
    _row_start : index of the starting row
    _col_start : index of the starting col

    === Attributes ===
    is_running : running state of visualizer
    is_terminated : if the process is terminated or not
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
        self._initialized = False
        self.last_zoom_factor = 1
        self._view = GameOfLifeVisualizer(root)
        self._canvas = CvsView(master=self._view.cvs_fr, width=800, height=560,
                               relief=GROOVE, bg="black")
        self._canvas.pack(fill=BOTH, expand=TRUE)
        self._bind_all()
        self._update_t = Thread(target=self._update_canvas, daemon=True)
        self._update_t.start()

    def _bind_all(self) -> None:
        self._view.build_menubar(
            self.clear_all,
            self._load_file,
            self._save_as)
        self._view.btn_start.config(command=self._start_process)
        self._view.btn_pause.config(command=self._pause_process)
        self._view.btn_stop.config(comman=self._stop_process)
        self._canvas.bind("<B1-Motion>", lambda event: self._paint(event, 1))
        self._canvas.bind('<ButtonRelease-1>', self._reset_cvs)
        self._canvas.bind("<B3-Motion>", lambda event: self._paint(event, 0))
        self._root.bind("<MouseWheel>", self._zoom)
        self._root.bind("<Control-s>", lambda e: self._save_as())
        self._root.bind("<Control-o>", lambda e: self._load_file())

    def clear_all(self):
        """Method to clear canvas and iterator"""
        self.is_terminated.set()
        self.is_running.clear()
        self._canvas.delete(ALL)
        self._unset_all()
        self._view.btn_start.config(state=ACTIVE)
        self._view.btn_pause.config(state=DISABLED)
        self._view.btn_stop.config(state=DISABLED)

    def _paint(self, new, flag) -> None:
        if self._canvas.prev_x and self._canvas.prev_y:
            if self._canvas.prev_x != new.x and self._canvas.prev_y == new.y:
                self._canvas.cells.add((new.x, self._canvas.prev_y))
            if self._canvas.prev_y != new.y and self._canvas.prev_x == new.x:
                self._canvas.cells.add((self._canvas.prev_x, new.y))
            if self._canvas.prev_x != new.x and self._canvas.prev_y != new.y:
                self._canvas.cells.add((new.x, new.y))
            self._canvas.create_line(self._canvas.prev_x, self._canvas.prev_y, new.x, new.y, width=2, fill=("black", "white")[flag])
        self._canvas.prev_x = new.x
        self._canvas.prev_y = new.y

    def _zoom(self, event) -> None:
        if event.delta > 0:
            if self.last_zoom_factor + 0.1 < 34:
                self.last_zoom_factor += 0.1
        else:
            if self.last_zoom_factor - 0.1 >= 0.1:
                self.last_zoom_factor -= 0.1
        self.last_zoom_x = event.x
        self.last_zoom_y = event.y
        if not self.is_running.is_set():
            self._canvas.scale(ALL, event.x, event.y, self.last_zoom_factor, self.last_zoom_factor)

    def _reset_cvs(self, e) -> None:
        self._canvas.prev_x = None
        self._canvas.prev_y = None

    def _start_process(self) -> None:
        if not self._canvas.cells:
            return
        self._canvas.unbind("<B1-Motion>")
        self._canvas.unbind("<B3-Motion>")
        self._view.btn_pause.config(state=ACTIVE)
        self._view.btn_stop.config(state=ACTIVE)
        self._view.btn_start.config(state=DISABLED)
        if self._initialized:
            self.is_running.set()
        else:
            self._initialized = True
            self._set_all()
            self.is_running.set()

    def _update_canvas(self):
        self.is_running.wait()
        for new_map in self._map_iter:
            self.is_running.wait()
            if self.is_terminated.is_set():
                return
            self._canvas.delete(ALL)
            self._cvs_draw_cells(new_map, True)

            sleep(1 / self._view.slider.get())

    def _cvs_draw_cells(self, cells, paddings):
        if not self.is_running.is_set():
            return
        if self.is_terminated.is_set():
            return
        self._canvas.cells.clear()
        for pt in cells:
            x = pt[0] + self._col_start
            y = pt[1] + self._row_start
            if paddings:
                self._canvas.cells.add((x + PADX, y + PADY))
                with self._cvs_lock:
                    this = self._canvas.create_rectangle(
                        x + PADX, y + PADY, x+1+PADX, y+1+PADY, fill="white",
                            width=1)
            else:
                self._canvas.cells.add((x, y))
                with self._cvs_lock:
                    this = self._canvas.create_rectangle(
                        x, y, x+1, y+1, fill="white", width=1)
            if (self.last_zoom_factor != 1):
                self._canvas.scale(this, self.last_zoom_x, self.last_zoom_y, 
                    self.last_zoom_factor, self.last_zoom_factor)

    def _unset_all(self):
        self._mao = None
        self._map_iter = None
        self._row_start = None
        self._col_start = None

    def _set_all(self):
        if not self._canvas.cells:
            return
        row_max, col_max = max(self._canvas.cells, key=lambda pt: pt[0])[0], \
                max(self._canvas.cells, key=lambda pt: pt[1])[1]
        row_min, col_min = min(self._canvas.cells, key=lambda pt: pt[0])[0], \
                min(self._canvas.cells, key=lambda pt: pt[1])[1]
        self._row_start = row_min - PADX
        self._col_start = col_min - PADY
        # set canvas dimensions with paddings
        height = 2 * PADY + row_max - row_min + 1
        width = 2 * PADX + col_max - col_min + 1
        self._mao = Map(width, height)
        self._mao.set_map(self._cvs2map(width, height))
        self._map_iter = self._mao.get_iter(-1)

    def _cvs2map(self, width, height):
        map = [0 for _ in range(width * height)]
        for pt in self._canvas.cells:
            idx = (pt[0] - self._row_start) * width + (pt[1] - self._col_start)
            map[idx] = 1
        return map

    def _pause_process(self) -> None:
        if self.is_running.is_set():
            self._view.btn_pause.config(state=DISABLED)
            self._view.btn_start.config(state=ACTIVE)
            self.is_running.clear()

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
                    self._cvs_draw_cells(cells, False)
        except Exception as e:
            showerror(f"Could not open {str(path)}.")


if __name__ == "__main__":
    root = Tk()
    Controller(root)
    root.mainloop()
