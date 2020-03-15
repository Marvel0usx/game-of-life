import copy
from typing import List

# def load_file_to_map():
    # board = [[0 for _ in range(col)] for _ in range(row)]

def save_process_to_file():
    pass

def trigger_process():
    pass

def pause_process():
    pass

def iterator_setup():
    pass

class Map:
    def set_row(self, row: int):
        self._row = row
    
    def set_col(self, col: int):
        self._col = col

    def init_map(self, path: List[int]):
        self._map = path

    def get_iter(self, goal):
        return MapItor(self._map, self._row, self._col, goal)

def updated(map, a, b):
    map[a] += map[b]
    return map

class MapItor:
    def __init__(self, map, row, col, goal):
        self._map = copy.deepcopy(map)
        self._row = row
        self._col = col
        self._curr = 0
        self._goal = goal

    def __iter__(self):
        return self
    
    def __next__(self):
        if self._goal == -1:
            return updated(self._map, self._row, self._col)
        elif self._curr >= self._goal:
            raise StopIteration
        elif self._curr == 0:
            self._curr += 1
            return self._map
        else:
            self._curr += 1
            return updated(self._map, self._row, self._col)

class testItor:
    def __init__(self, num):
        self.limit = num
        self.curr = 0

    def __iter__(self):
        return self
    
    def __next__(self):
        tmp = self.curr
        if self.curr < self.limit:
            self.curr += 1
        else:
            raise StopIteration
        return tmp

if __name__ == "__main__":
    # my_map = Map()
    # my_map.set_col(0)
    # my_map.set_row(1)
    # my_map.init_map([1,2])
    # for i in my_map.get_iter(100):
    #     print(i, end='\n')
    t = testItor(100)
    for i in t:
        print(i)
    print(t.__next__()
    )