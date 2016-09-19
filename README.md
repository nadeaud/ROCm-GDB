<<<<<<< HEAD
# ROCm-GDB
The ROCm-GDB repository includes the source code for ROCm-GDB. ROCm-GDB is a modified version of GDB 7.8 that supports debugging GPU kernels on Radeon Open Compute platforms (ROCm).

# Package Contents
The ROCm-GDB repository includes
* A modified version of gdb-7.8 to support GPU debugging. Note the main ROCm specific files are located in *gdb-7.8/gdb* with the *hsail-** prefix.
* The ROCm debug facilities library located in *amd/HwDbgFacilities/*. This library provides symbol processing for GPU kernels.

# Build Steps
1. Clone the ROCm-GDB repository
  * `git clone https://github.com/RadeonOpenCompute/ROCm-GDB.git`
2. The gdb build has been modified with new files and configure settings to enable GPU debugging. The scripts below should be run to compile gdb.
The *run_configure_rocm.sh* script calls the GNU autotools configure with additional parameters.
  * `./run_configure_rocm.sh debug`
3. The `run_configure_rocm.sh` script also generates the *run_make_rocm.sh* which sets environment variables for the *Make* step
  * `./run_make_rocm.sh`

# Running ROCm-GDB
The `run_make_rocm.sh` script builds the gdb executable.

To run the ROCm debugger, you'd also need to get the [ROCm GPU Debug SDK](https://github.com/RadeonOpenCompute/ROCm-GPUDebugSDK).

Before running the rocm debugger, the *LD_LIBRARY_PATH* should include paths to
* The ROCm GPU Debug Agent library built in the ROCm GPU Debug SDK (located in *gpudebugsdk/lib/x86_64*)
* The ROCm GPU Debugging library binary shippped with the ROCm GPU Debug SDK (located in *gpudebugsdk/lib/x86_64*)
* Before running ROCm-GDB, please update your .gdbinit file  with text in *gpudebugsdk/src/HSADebugAgent/gdbinit*. The rocmConfigure function in the ~/.gdbinit sets up gdb internals for supporting GPU kernel debug.
* The gdb executable should be run from within the *rocm-gdb-local* script. The ROCm runtime requires certain environment variables to enable kernel debugging and this is set up by the *rocm-gdb-local* script.
```
./rocm-gdb-local < sample application>
```
* [A brief tutorial on how to debug GPU applications using ROCm-GDB](https://github.com/RadeonOpenCompute/ROCm-Debugger/blob/master/TUTORIAL.md)
=======
# ROCm-GPUDebugSDK
The ROCm-GPUDebugSDK repository provides the components required to build a GPU kernel debugger for Radeon Open Compute platforms (ROCm).
The ROCm GPU Debug SDK components are used by ROCm-GDB and CodeXL debugger to support debugging GPU kernels on ROCm.

# Package Contents
The ROCm GPU Debug SDK includes the source code and libraries briefly listed below
* Source code 
  * HSA Debug Agent: The HSA Debug Agent is a library injected into an HSA application by the ROCR-Runtime. The source code for the Agent is provided in *src/HSADebugAgent*.
  * Debug Facilities: The Debug Facilities is a utility library to perform symbol processing for ROCm code object.  The header file *FacilitiesInterface.h* is in the *include* folder while the source code is provided in *src/HwDbgFacilities*.
  * Matrix multiplication example: A sample HSA application that runs a matrix multiplication kernel.
* Header files and libraries
  * libAMDGPUDebugHSA-x64: This library provides the low level hardware control required to enable debugging a kernel executing on ROCm. The functionality of this library is exposed by the header file *AMDGPUDebug.h*  in *include/*. The HSA Debug Agent library uses this interface
  * libelf: A libelf library compatible with the ROCm and its corresponding header files. The HSA Debug Agent library uses this libelf.
	
# Build Steps
1. Install ROCm using the instruction [here](https://github.com/RadeonOpenCompute/ROCm#installing-from-amd-rocm-repositories)
2. Clone the Debug SDK repository
  * `git clone https://github.com/RadeonOpenCompute/ROCm-GPUDebugSDK.git`
3. Build the AMD HSA Debug Agent Library and the Matrix multiplication examples by calling *make* in the *src/HSADebugAgent* and the *samples/MatrixMultiplication* directories respectively
  * `cd src/HSADebugAgent`
  * `make`
    * Note that *matrixMul_kernel.hsail* is included for reference only. This sample will load the pre-built hsa binary (*matrixMul_kernel.brig*) to run the kernel.
  * `cd samples/MatrixMultiplication`
  * `make`
4. Build the ROCm-GDB debugger [as shown in the GDB repository](https://github.com/RadeonOpenCompute/ROCm-GDB).
>>>>>>> 0df5734a67968e4408983049cf078b749f82d88f

