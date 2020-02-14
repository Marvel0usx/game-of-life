from distutils.core import setup, Extension
setup(name="map", version="0.2",
      ext_modules=[Extension("map", ["map.c"])])
