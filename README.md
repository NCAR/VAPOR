[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.10725772.svg)](https://doi.org/10.5281/zenodo.10725772)
[![CircleCI](https://circleci.com/gh/NCAR/VAPOR.svg?style=svg)](https://circleci.com/gh/NCAR/VAPOR) 

## Vapor:

**VAPOR** is the **V**isualization and **A**nalysis **P**latform for **O**cean, Atmosphere, and Solar **R**esearchers.  VAPOR provides an interactive 3D visualization environment that can also produce animations and still frame images.  VAPOR runs on most UNIX and Windows systems equipped with modern 3D graphics cards.

The VAPOR Data Collection (**VDC**) data model allows users progressively access the fidelity of their data, allowing for the visualization of terascale data sets on commodity hardware.  VAPOR can also directly import data formats including WRF, MOM, POP, ROMS, and some GRIB and NetCDF files.

Users can perform ad-hoc analysis with VAPOR's interactive Python interpreter; which allows for the creation, modification, and visualization of new variables based on input model data.

VAPOR is a product of the **NSF National Center for Atmospheric Research's Computational and Information Systems Lab**. Support for VAPOR is provided by the U.S. **National Science Foundation** (grants # 03-25934 and 09-06379, ACI-14-40412), and by the **Korea Institute of Science and Technology Information**

Project homepage and binary releases can be found at [https://www.vapor.ucar.edu/](https://www.vapor.ucar.edu/)

## Citation
If VAPOR benefits your research, please kindly cite [this publication](https://www.mdpi.com/2073-4433/10/9/488):
```
@Article{atmos10090488,
AUTHOR = {Li, Shaomeng and Jaroszynski, Stanislaw and Pearse, Scott and Orf, Leigh and Clyne, John},
TITLE = {VAPOR: A Visualization Package Tailored to Analyze Simulation Data in Earth System Science},
JOURNAL = {Atmosphere},
VOLUME = {10},
YEAR = {2019},
NUMBER = {9},
ARTICLE-NUMBER = {488},
URL = {https://www.mdpi.com/2073-4433/10/9/488},
ISSN = {2073-4433},
ABSTRACT = {Visualization is an essential tool for analysis of data and communication of findings in the sciences, and the Earth System Sciences (ESS) are no exception. However, within ESS, specialized visualization requirements and data models, particularly for those data arising from numerical models, often make general purpose visualization packages difficult, if not impossible, to use effectively. This paper presents VAPOR: a domain-specific visualization package that targets the specialized needs of ESS modelers, particularly those working in research settings where highly-interactive exploratory visualization is beneficial. We specifically describe VAPOR&rsquo;s ability to handle ESS simulation data from a wide variety of numerical models, as well as a multi-resolution representation that enables interactive visualization on very large data while using only commodity computing resources. We also describe VAPOR&rsquo;s visualization capabilities, paying particular attention to features for geo-referenced data and advanced rendering algorithms suitable for time-varying, 3D data. Finally, we illustrate VAPOR&rsquo;s utility in the study of a numerically- simulated tornado. Our results demonstrate both ease-of-use and the rich capabilities of VAPOR in such a use case.},
DOI = {10.3390/atmos10090488}
}
```

## Project Members:

- Nihanth Cherukuru
- John Clyne
- Scott Pearse
- Samuel Li
- Stanislaw Jaroszynski
- Kenny Gruchalla
- Niklas Roeber
- Pamela Gillman

![Vapor Banner](share/images/vapor_banner.png)
