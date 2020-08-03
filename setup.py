from distutils.core import setup, Extension

setup(name="map", version="0.3",
      description="Map for the Game-of-life Visualizer",
      author="Marvel0usx",
      url="https://github.com/Marvel0usx/game-of-life",
      ext_modules=[Extension("map", sources=["map.c", "life_helpers.c"])])
