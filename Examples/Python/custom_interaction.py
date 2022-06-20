"""Custom Interaction

This script provides an example of a custom interaction style that
utilizes some of the looking glass functions, including moving the
focal plane, turning on/off clipping, and moving the clipping planes.

It is most helpful for being able to interactively determine where the
focal plane and clipping planes should be located to produce the desired
image.

See the message that is displayed at start-up for more instructions.
"""
from functools import partial

import numpy as np

import vtk

from vtk import vtkRenderingLookingGlass


# A custom interactor for key press events
class LookingGlassInteractorStyle(vtk.vtkInteractorStyleTrackballCamera):
    def __init__(self, renWin):
        self.renWin = renWin
        self.AddObserver('KeyPressEvent', self.key_press_event)
        self.movement_function = None
        self.movement_factor = None
        self.default_movement_factor = 0.01

        self.print_start_message()

    def print_start_message(self):
        msg = """
        To use the LookingGlassInteractorStyle, first use the mouse to
        left-click on the render window display, and then use the following
        key presses:

        p: toggle focal plane movement
        c: toggle on clipping planes (both near and far)
        n: toggle near clipping plane movement
        f: toggle far clipping plane movement

        Up: move active plane forward
        Down: move active plane backward
        Left: decrease movement sensitivity (for current plane)
        Right: increase movement sensitivity (for current plane)
        """
        print(msg)

    @property
    def interactor(self):
        return self.renWin.GetInteractor()

    @property
    def renderer(self):
        return self.renWin.GetRenderers().GetFirstRenderer()

    def render(self):
        self.renWin.Render()

    def key_press_event(self, obj, event):
        key = self.interactor.GetKeySym()

        toggle_move_func = self.toggle_movement_function
        callbacks = {
            'c': self.toggle_clipping,
            'f': partial(toggle_move_func, func=self.move_far_clipping),
            'n': partial(toggle_move_func, func=self.move_near_clipping),
            'p': partial(toggle_move_func, func=self.move_focal_plane),
            'Up': self.on_up_pressed,
            'Down': self.on_down_pressed,
            'Left': self.on_left_pressed,
            'Right': self.on_right_pressed,
        }

        if key in callbacks:
            callbacks[key]()

    @property
    def use_clipping_limits(self):
        return self.renWin.GetUseClippingLimits()

    @use_clipping_limits.setter
    def use_clipping_limits(self, b):
        self.renWin.SetUseClippingLimits(b)

    def toggle_clipping(self):
        value = not self.use_clipping_limits
        print(f'Toggling clipping to {value}')
        self.use_clipping_limits = value
        self.render()

    def toggle_movement_function(self, func):
        factor = self.default_movement_factor

        # Python versions earlier than 3.8 do not allow us to check if
        # a function is the same as another function. So compare the names
        # instead.
        prev_func = self.movement_function
        if prev_func is not None and prev_func.__name__ == func.__name__:
            # Toggle it off
            func = None
            factor = None
            print('Toggling off movement function')
        else:
            print(f'Toggling movement function to {func.__name__}')
            print('Press up/down to modify, and left/right for sensitivity')

        self.movement_function = func
        self.movement_factor = factor

    def on_up_pressed(self):
        if self.movement_function is None:
            # Do nothing
            return

        # Move by the movement factor
        self.movement_function(self.movement_factor)

    def on_down_pressed(self):
        if self.movement_function is None:
            # Do nothing
            return

        # Move by the negative movement factor
        self.movement_function(-self.movement_factor)

    def on_left_pressed(self):
        if self.movement_factor is None:
            # Do nothing
            return

        # Decrease movement factor
        self.movement_factor *= 0.9

        # Round so we don't print floating point errors
        rounded = round(self.movement_factor, 15)
        print(f'Movement factor decreased to {rounded}')

    def on_right_pressed(self):
        if self.movement_factor is None:
            # Do nothing
            return

        # Increase movement factor
        self.movement_factor *= 1.1

        # Round so we don't print floating point errors
        rounded = round(self.movement_factor, 15)
        print(f'Movement factor increased to {rounded}')

    def move_far_clipping(self, factor):
        if not self.use_clipping_limits:
            print('Warning: clipping limits are off. Toggle them on with "c"')

        print(f'Moving far clipping plane by {factor}')
        value = self.renWin.GetFarClippingLimit()
        self.renWin.SetFarClippingLimit(value * factor + value)
        self.render()

    def move_near_clipping(self, factor):
        if not self.use_clipping_limits:
            print('Warning: clipping limits are off. Toggle them on with "c"')

        print(f'Moving near clipping plane by {factor}')
        value = self.renWin.GetNearClippingLimit()
        self.renWin.SetNearClippingLimit(value * factor + value)
        self.render()

    def move_focal_plane(self, factor):
        print(f'Moving focal plane by {factor * 100}%')
        camera = self.renderer.GetActiveCamera()
        fp = np.array(camera.GetFocalPoint())
        pos = np.array(camera.GetPosition())

        distance = np.linalg.norm(fp - pos)
        direction = (fp - pos) / distance

        distance = distance * factor + distance

        new_fp = pos + direction * distance
        camera.SetFocalPoint(new_fp)
        self.render()

# Declare a VTK rendering process
ren = vtk.vtkRenderer()

# Create the looking glass render window
renWin = vtkRenderingLookingGlass.vtkLookingGlassInterface.CreateLookingGlassRenderWindow()

# Add the rendering process to the window
renWin.AddRenderer(ren)

# Create and modify the interactor. Use our custom interactor style.
iren = vtk.vtkRenderWindowInteractor()
style = LookingGlassInteractorStyle(renWin)
iren.SetInteractorStyle(style)
iren.SetRenderWindow(renWin)

# The mouse controls the position of the camera

# Add some text to the display
text = vtk.vtkVectorText()
text.SetText('Hello VTK!')
textMapper = vtk.vtkPolyDataMapper()
textMapper.SetInputConnection(text.GetOutputPort())
textActor = vtk.vtkActor()
textActor.SetMapper(textMapper)
ren.AddActor(textActor)

# Position a cone above the text
cone = vtk.vtkConeSource()
cone.SetRadius(2)
cone.SetHeight(4)
cone.SetCenter(4,4,2)
cone.SetDirection(0,0,1)
coneMapper = vtk.vtkPolyDataMapper()
coneMapper.SetInputConnection(cone.GetOutputPort())
coneActor = vtk.vtkActor()
coneActor.SetMapper(coneMapper)
ren.AddActor(coneActor)

# Initialize the window
renWin.Initialize()
ren.ResetCamera()
ren.GetActiveCamera().SetViewAngle(30)

# The mouse controls the camera until 'q' is pressed to exit
iren.Start()
