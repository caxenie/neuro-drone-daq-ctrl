We have setup 4 quadrotor platforms for experiments. Each platform is based on the PX4 Pixhawk Autopilot.

The system is modular and composed of:

FMU autopilot module equipped with:
* inertial unit (3D Acc and Gyro)
* magnetometer
* barometer
* IO adapter board (e.g. interface with motors and FMU)
* Optic flow sensor board

The software infrastructure for data acquisition, synchronization with 3D tracker system and logging was developed at NST.

The developed software ecosystem contains:
* Low-level library: tflog-libs
* Main interface and control application: tflog
* Complete documentation and external software: tflog-doc

DEVEL TEAM: 
Florian Bergner, Cristian Axenie
florian.bergner@tum.de, cristian.axenie@tum.de

https://wiki.lsr.ei.tum.de/nst/documentation/flyingdrones
