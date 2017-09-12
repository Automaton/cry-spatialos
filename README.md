# CrySpatialOS ("crysos")

SpatialOS 11 integration for CRYENGINE 5.4

## Directory Structure

### sdk
`sdk` contains the C++ files to be included in your CRYENGINE project to integrate with SpatialOS.

### tools
`tools` contains the project `ComponentGenerator`, which is used to generate CRYENGINE Entity Components to mirror your SpatialOS components for use within CRYENGINE from C++ and/or Schematyc.

### tps-example
This is a simple third-person shooter game template produced using the integration.

## Building The Example Project

1. Ensure CryENGINE 5.4 or later and `spatial` are installed
2. Run `spatial worker build` from the `tps-example` directory (which will build the `game-worker`)
3. Generate the solution for `tps-client` by right-clicking `Game.cryproject`
4. Build the solution in Visual Studio to provide the `tps-client` worker. (See below section for more information)
5. Run `spatial local launch` from the `tps-example` directory
6. Run `GameWorker.exe 0` from the appropriate output directory (we arbitrarily give it worker ID 0)
7. Run any number of clients by right-clicking `Game.cryproject` and selecting `Run Game` (or the exe generated from the `Package Build` option)

### Building tps-client
At the moment it is currently required to use the generated code provided in the git repository as it has been modified to include default constructors. 
Building the solution will overwrite these files (on first build) so it is necessary to checkout these files (`tps-client/Code/Generated`) once they have been overwritten.
