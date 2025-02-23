# Bombratter Level Format

The level files of Bombratter will be .pngs which look quite strange because they don't have any colors on them, they are mostly red since that's the main channel
that stores information about the level.

# Red Channel
---

The red channel is used for pallette information. A pixel with a red value of 197 will get a different color in the level pallette.

# Blue Channel
---

The blue channel is used to store depth information, how near a pixel is to the camera.

# Green Channel
---

The green channel is not used to store anything.

# Pallettes
---

A pallette is a 16x16 file which maps out how the level will be colored.

The first row of a pallette is the 'other' tiles.
The second row is the terrain, from dark to light to make a variety of shades.
The third row is a single pixel of blackness.

The pallette layout for a level's red channel looks like this:

0 - Black 
1 - 1st terrain shade
5 - 2nd terrain shade
10 - 3rd terrain shade
15 - 4th terrain shade
20 - 5th terrain shade
25 - 6th terrain shade
30 - 7th terrain shade
35 - 8th terrain shade
40 - 9th terrain shade
45 - 10th terrain shade
50 - 11th terrain shade
55 - 12th terrain shade
60 - 13th terrain shade
65 - 14th terrain shade
70 - 15th terrain shade
75 - 16th terrain shade
255 - Skybox
200 - Plants
