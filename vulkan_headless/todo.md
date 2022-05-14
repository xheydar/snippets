
# DATE : 20220405

## List of todos

    - [x] Add some kind of lighting to the scene

# DATE : 20220403

## Current status of the project

    - We calculate a graphical overlay for the entire image and put it over the image captured by 
      the webcam.
    - Currenly for a 1080p image the overlay time ( On M1 Pro ) is around 0.9 miliseconds
    - Rendering time for the overlay is around 7 to 8 miliseconds

## Here is a list of todo things for this project
### What needs to be done for other people to be able to develope on this project?
### Sorted based on priority

    - [x] Vertex Clean up : We should remove the already existing vertices and properly referece 
          them with indices

          ** The cleanup was properly done

    - [x] Texture : There texture I'm loading is crude and horrible. Is it possible to add texture 
          to faces using shaders?

          ** Now I can load proper textures on the 3D objects

    - [x] Shaders : Currently I'm loading only one shader for the whole project. Is there a way 
          to load different shaders for different models?

          ** Still loading only one shader for the project. I load different information for different
          objects into the shader. This should be enough for now.


