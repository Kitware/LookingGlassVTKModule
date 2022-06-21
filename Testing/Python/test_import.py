def test_basic_import():
    from vtk import vtkRenderingLookingGlass

    # Make sure it has one of the attributes we are expecting
    assert hasattr(vtkRenderingLookingGlass, 'vtkLookingGlassInterface')
