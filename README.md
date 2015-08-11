[Demo for the impatient](http://portfolio.cooper.stevenson.name:3000/vol_1_iss_1/games/seigen_kitani/56). Click on each of the diagram thumbnails to see them in the main viewing area.

The goal of this project is to [facilitate communications](http://portfolio.cooper.stevenson.name:3000/vol_1_iss_1/editorial/#editorial) between Go experts and readers. To that end I created and modified a series of scripts to create [relationship (heatmap), territory, and markup diagrams](http://portfolio.cooper.stevenson.name:3000/vol_1_iss_1/games/seigen_kitani/56) for each move in the game.

# Dependencies

* Oakfoam (see the config file in oakfoam/config.gtp)
* Python
* Python sgflib

# Basic workflow

> These steps (the first few, anyway) are codified in gen_mv/mv.py and (I believe) sgf/sgfmv/sgfmv.py. The following should be used only as a guide

1. Run sgf/sgf_check/sgfc on your sgf (with the 'strict' switch I believe) to clean and sanitize.
1. Run the sgf2gtp (sgfmv?) to convert the sgf to a gtp file the Oakfoam go engine can understand
1. Parse the Oakfoam log file for territory, criticality, SGF information markup, etc. for use as input to the SVG diagrams
1. Run sgf2svg.py, sgf2svg_color.py, and sgf2svg_territory.py for your markup, criticality, and territory diagrams respectively
