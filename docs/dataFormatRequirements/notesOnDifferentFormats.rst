.. _notesOnDifferentFormats:

|

Notes on Different Formats
__________________________

If your data format is not currently supported by Vapor you may be able to convert your data from its native file format to one that *is* supported. Several of Vapor's supported formats may be good candidates for your conversion. The table below shows these formats, along with their supported and unsupported features.

    +--------------------+-----------------------+-----+------+--------+
    |                    |  CF Compliant netCDF  | BOV | VDC  | DCP    |
    +--------------------+-----------------------+-----+------+--------+
    | Regular grids      |  Yes                  | Yes | Yes  | No     | 
    +--------------------+-----------------------+-----+------+--------+
    | Rectilinear grids  |  Yes                  | No  | Yes  | No     |
    +--------------------+-----------------------+-----+------+--------+
    | Curvilinear grids  |  Yes                  | No  | Yes  | No     |
    +--------------------+-----------------------+-----+------+--------+
    | Missing values     |  Yes                  | No  | Yes  | No     |
    +--------------------+-----------------------+-----+------+--------+
    | Raw, Binary data   |  No                   | Yes | Yes  | No     |
    +--------------------+-----------------------+-----+------+--------+
    | Multi-resolution*  |  No                   | No  | Yes  | No     |
    +--------------------+-----------------------+-----+------+--------+
    | Complexity         |  Medium               | Low | High | Medium |
    +--------------------+-----------------------+-----+------+--------+
    | Particle Data      |  No                   | No  | No   | Yes    |
    +--------------------+-----------------------+-----+------+--------+

For example, if your data is organized as a collection of RAW files that are sampled on a `regular grid <https://en.wikipedia.org/wiki/Regular_grid>`_ you may want to use :ref:`Vapor's BOV format <brickOfValues>`. This is the easiest way to import unsupported data because it only involves creating a small, ASCII metadata file that describes your data.

.. note::

    The geophysical modelling community often uses the term "stretched grid" in place of "rectilinear grid".  In Vapor parlance, they are the same.

If your data are sampled on rectilinear (a.k.a. "stretched") or curvilinear grids, or if your grid is regular and stored in a format other than raw-binaries, converting your data to CF compliant NetCDF is probably your best bet. CF NetCDF is widely used in the earth sciences and is highly flexible.  There is a large ecosystem of software tools that are capable reading, writing, and manipulating CF NetCDF files.

If your data are sampled on a high resolution grid, and performance is a concern, you may want to consider VAPOR's multi-resolultion VDC file format. However, this format should only be considered if multi-resolution is required. For small to modest sized data sets, performance may be degraded by using the VDC.

.. note::

    The multi-resolution feature of the VDC Allows you to render large datasets with or without lossy compression.  Viewing compressed data can dramatically increase the interactivity for your visualizations if your computer's resources are constrained by the size of your data.  

    When you're done with exploring compressed data and ready for a final rendering, you can render an image or animation sequence with lossless compression.
