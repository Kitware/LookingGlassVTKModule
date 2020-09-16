vtk_add_test_cxx(vtkRenderingLookingGlassCxxTests tests
  TestLookingGlassPass.cxx,NO_VALID
  TestDragon.cxx,NO_VALID
  )

vtk_test_cxx_executable(vtkRenderingLookingGlassCxxTests tests RENDERING_FACTORY)
