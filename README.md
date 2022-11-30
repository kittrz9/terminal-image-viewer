# Terminal image viewer
![](demo.gif)<br>
A program to view images in the terminal using ANSI escape codes<br>
Definitely not practical, done mostly because I thought it would be funny<br>
<br>
Uses [stbi_image.h](https://github.com/nothings/stb/blob/master/stb_image.h) to load images<br>
Probably doesn't work that well with terminals that dont handle ansi escape codes well but whatever. This was tested with [kitty](https://sw.kovidgoyal.net/kitty/) and xterm so I know those work lmao<br>
<br>
Eight color mode and sixteen color mode both compare against the colors I have for my terminal's color scheme (the default one that comes with kitty) so if you want to have it work with yours you'd probably need to edit the look up table with your color values (although if it follows the normal color scheme it should still work fine)<br>
