# Qt Creator utilities

Different useful/useless utilities.

## Clang tools

Allows to automatically run clang tools (tidy/modularize) on specific IDE events
(file changed, file added, project switched, ...).
Also parses tools' output and shows it in Tasks pane.
![Preview](util/clangtools.png?raw=true)

## OCLint

Allows to automatically run oclint static analyzer on specific IDE events
(file changed, file added, project switched, ...).
Also parses its output and shows it in Tasks pane.

## Docked output pane

Makes additional application output pane that contains dock widgets instead of tab widgets.
So now it is possible to see several apps' output an once.

![Preview](util/dockoutput.png?raw=true)

## Organize includes

Removes unused, adds absent, resolves misplaced and sorts includes in current document.

## Discover code

Visualizes code structure/relations in uml notation.

![Preview](util/codediscover.png?raw=true)

## Continuous integration

Adds pane with continuous integration status. Shows repository and build information.
Currently supports [drone.io](https://drone.io/).

![Preview](util/ci.png?raw=true)
