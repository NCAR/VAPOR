.. _raytracingParameters:

Raytracing Parameters
`````````````````````

    *Rendering Algorithm* - Defaults to either Curvilinear or Regular, depending on the topology of the current variable's grid.  If using an older graphics card, the Isosurface Renderer may default to Rectilinear due to the increased complexity of the Curvilinear raytracer.

    *Sampling Rate Multiplier* - The volume renderer takes samples along the projected camera rays to determine the volume density at a point and the multiplier increases the number of samples which can give a more accurate visualization in certain cases.
