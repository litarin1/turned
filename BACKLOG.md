# Global
- Physics

# TODO:
- Consider creating inheritance between Ship and Sprite
#### Rendering
#### Inputs
- dynamic keys mapping
- get rid of GLFW, because it does not respects keyboard remapping, (mainly caps:escape xkb option)

## (My own priority):
- Physics timer (single-threaded for now: just run physics in main loop sometimes)
#### Rendering
999 Try to decrease the input-lag by rendering everything on a framebuffer and then rotating in by mouse movement in a separate no-GPU thread\
