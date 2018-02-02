# Example on using EyeFinderReader

from eyefinder import eyefinderreader

with eyefinderreader.EyeFinderReader() as efr:
    print(efr.read(),
          "\n\n***\n\n",
          efr.read())
