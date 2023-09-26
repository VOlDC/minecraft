# test_repo
```diff

simple minecraft project i made within 1 week (not true)

-- LIBRARIES --

anyway freetype, glm, glfw, glad and other stuff do have to be downloaded from:

freetype:
+ - download freetype from freetype.org
+ - compile it using cmake-gui
 

- - for the ones who need help doing so, heres a quick and short tutorial

 - first of all, after u downloaded cmake-gui (you might already have it) and freetype,
 - u want to write in the "Where is the source code: " line this directory: "PathTo_ft2132/ft2132/freetype-2.13.2", and ofcourse the PathTo_ft2132 should be the directory to ur freetype folder, for examples mines in this directory: C:\Users\my_account\Downloads\ft2132\freetype-2.13.2
 - then u want to write in the "Where to build the binaries: " line, basically whereever u want to build the binaries, which include all the essential stuff
 
 - after u complete those 2 steps, click configure, then click generate, and then ur project is done, but the .lib file which is essential isnt there, so for that u have to go inside the folder u builded the binaries, and then go inside the freetype.sln folder and then right click ALL_BUILD and then build it (it cant be builded as Debug or Release, or like any mode u want, i chose Release and x64)

+ and in order to add freetype to ur project, u first of all have to add this into ur include directories: PathTo_ft2132\ft2132\freetype-2.13.2\include

+ then add into library directories: freetype.lib

glm:
+ - github repo: https://github.com/g-truc/glm
+ - site: https://glm.g-truc.net/0.9.9/

glfw:
+ - site: glfw.org
! - how to add it into visual studio:

! -- INCLUDE DIRECTORY

! - first of all add its include directory, which should be this: PathTo_GLFW\glfw-3.3.8.bin.WIN64\glfw-3.3.8.bin.WIN64\include

! -- LIBRARY INCLUDING --

! - then u want to add its library directory which should be this PathTo_GLFW\glfw-3.3.8.bin.WIN64\glfw-3.3.8.bin.WIN64\Your_Version_Of_VisualStudio, the Your_Version_Of_VisualStudio, is pretty self explanatory, for example if i use visual studio 2022, im gonna use lib-vc2022

! -- ADDITIONAL DEPENDENCIES --

! - now it comes to additional dependencies (.lib files): and theese are all the .lib files:

! - glfw3.lib
! - opengl32.lib
! - user32.lib
! - gdi32.lib
! - shell32.lib

glad:
 - site i took it from: https://glad.dav1d.de/
 - make sure in the api section, gl is chosen as Version 3.3
 - also under Profile section it must be Core

+ virus total check: https://www.virustotal.com/gui/home/upload

! PerlinNoise.hpp taken from: https://github.com/Reputeless/PerlinNoise

other important stuff are taken from: learnopengl.com, which is probally the best way, i have learned some opengl, and it also includes some other nice stuff