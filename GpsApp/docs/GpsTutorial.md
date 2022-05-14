# F' GPS Tutorial

## Table of Contents

This tutororial is intended to help you extend your F' development capabilties from the [MathComponent](https://github.com/nasa/fprime/tree/devel/docs/Tutorials/MathComponent) and [CrossCompilation](https://github.com/nasa/fprime/tree/devel/docs/Tutorials/CrossCompilation) and serve as some background for the [RPI demo](https://github.com/nasa/fprime/tree/devel/RPI). Specifically this tutorial will employ some of the provided F' components to use the data bus on an embedded target in order to connect another device to the target in order to start building a complete embedded system. In this case we will use a GPS device, connected over UART, to a Raspberry Pi running F' on a Linux-based OS.

Before diving in we should point out what this tutorial *is not* so that you understand the other tasks that you will need to work as you develop your own embedded system using the F' architecture and framework. This tutorial is not:
 - A GPS or GNSS tutorial; we will not cover how to use a GNSS/GPS device as part of your system. In particular we will not develop navigation or timing functions from the GPS device
 - A systems architecture tutorial; we will not cover how to develop a set of requirements on what your system will need to do, allocate those functions to components, and then laying out those components into a topology with ports and types. 
 - A coding style or software system development guide. This tutorial has been developed by the community and as such does not follow the JPL styles used in the JPL-developed tutorials. This tutorial will also not address development techniques like configuration management, file naming/location, or unit test practices.

We call out the above because these are tasks you really should be doing with your team as you develop your system. At best we'll include notes along the lines of the above when we get to things like "name this per your sytle guide."

**Prerequisites:** This tutorial assumes the you have exctuted and understood the: 
 - [Getting Started Tutorial](../GettingStarted/Tutorial.md)
 - [MathComponent Tutorial](../MathComponent/Tutorial.md)
 - [CrossCompilation Tutorial](https://github.com/nasa/fprime/tree/devel/docs/Tutorials/CrossCompilation).

As such, this tutorial builds on the prerequisites in those tutorials. This tutorial will make extensive use of the [FPP Users Guide](https://fprime-community.github.io/fpp/fpp-users-guide.html) as well, so please read through that and refer back to it as we go.

We have written this guide making use of a [Raspberry Pi 4](https://www.raspberrypi.com/products/raspberry-pi-4-model-b/) as the embedded target and the [Adafruit Ultimate GPS FeatherWing](https://www.adafruit.com/product/3133) as the connected device. Due to this being written during COVID times, we have not been able to procure let alone test alternate hardware sets. We do want to point out that both Raspberry Pi and Adafruit teams provide extensive documentation and support, so please consider supporting them as you work to learn embedded systems.

**F´ Version:** This tutorial is designed to work with release `v3.0.0`.

Working on this tutorial will modify some files under version control in the
F Prime git repository.
Therefore it is a good idea to do this work on a new branch.
For example:

```bash
git checkout -b GpsApp v3.0.0
```

If you wish, you can save your work by committing to this branch.

<a name="Gps-Component"></name>
<a name="The-Gps-Component"></a>
## The Gps Component

We will jump directly to creating the `Gps` component, since this component will only make use of existing ports and types. The steps are:

-  Construct the FPP model
-  Add the model to the project
-  Build the stub implementation
 - Complete the implementation

can build and test the `MathSender` component.
There are five steps:
