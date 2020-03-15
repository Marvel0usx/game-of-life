from Map import *

class Singleton(type):
    """
    """
    __created__ = None
    def __call__(cls, *args, **kwargs):
        """ Limit the number of instances at instantialization
        """
        if not Singleton.__created__:
            Singleton.__created__ = super(
                Singleton, cls).__call__(*args, **kwargs)
            return Singleton.__created__
        else:
            return Singleton.__created__

class MapManager(object, metaclass=Singleton):
    """ TODO: add doc
    """
    #TODO: Finish these methods
    def new_map(self, row: int, col: int, goal: int) -> bool:
        try:
            self.m = Map.map()
            return True
        except Exception:
            return False
    
    def init_map(self, init_list: list) -> bool:
        try:
            self.m.set_map(list)
            return True
        except Exception:
            return False

    def get_map_gen(self) -> MapGen:
        pass

    def serialize(self) -> bool:
        pass

    