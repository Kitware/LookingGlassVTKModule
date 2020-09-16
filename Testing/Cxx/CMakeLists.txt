vtk_add_test_cxx(vtkLookingGlassCxxTests tests
  TestLookingGlassPass.cxx,NO_VALID
  TestDragon.cxx,NO_VALID
  )

vtk_test_cxx_executable(vtkLookingGlassCxxTests tests RENDERING_FACTORY)
