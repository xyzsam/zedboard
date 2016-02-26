Zynq Zedboard Applications
==========================

This repository contains source code and scripts for a variety of applications
that can be run on the Zynq Zedboard, both on bare-metal and on Linux. Each
application contains an accelerator that was created using Vivado HLS. User
software demonstrates how to communicate with these custom peripherals.

Prerequistes
------------
You will need to have familarity with using the Vivado Design Suite, HLS, and
the Zedboard platform. I will not provide step-by-step instructions on how to
bring up a system on the Zedboard - there are many excellent tutorials online
for this purpose. This source code is designed for people who understand the
basics of how Zynq development works and wants to know how to do something
specific (such as DMA, interrupts, and interfacing with HLS).

Tools:
- Vivado Design Suite 2015.1
- Vivado HLS 2015.1
- Xilinx SDK 2015.1
- ARM bare-metal and Linux cross-compiler toolchain. I recommend the following two from Linaro:
  * `gcc-linaro-arm-linux-gnueabihf-4.8-2014.04_linux`
  * `cc-linaro-arm-none-eabi-4.8-2014.04_linux`

General instructions
--------------------

To get started, you can just directly import the baremetal, bsp, and hw_platform
directories as separate projects into a new SDK workspace. You will need to
set the appropriate references (baremetal references bsp, which references
hw_platform).

To build from scratch:

1. Build the custom accelerator IP and add the IP to your user IP repository.

    cd project/hls
    make hls
    cd hls_dir/impl/ip
    cp xilinx_hls_accelerator_name_1_0.zip /path/to/ip/repo
    cd /path/to/ip/repo
    unzip xilinx_hls_accelerator_name_1_0.zip -d xilinx_hls_accelerator_name_1_0

2. Import the system block diagram (`project/tcl/system.tcl`) into Vivado. If
   Vivado does not find the custom peripheral, go into IP Catalog and refresh
   user repositories. You may have to manually add the user repo first before
   importing the block diagram.

3. Synthesize, implement, and generate the bitstream.

4. Export the hardware to the SDK.

5. Import the baremetal code as a new application project in SDK. Let SDK generate
   the BSP for you. If the BSP does not include the custom peripheral drivers, you
   will need to add a new software repository in SDK and put the driver there
   manually, then update the driver selections in BSP.


Note about baremetal code
-------------------------
The baremetal code generally must be compiled within Xilinx SDK. It is
possible, but quite difficult, to compile them in an external environment.
Although Makefiles are provided for these baremetal targets, do not expect them
to link successfully. Instead, create a new Xilinx SDK project, import the source
code into the project, and let the SDK take care of the builds for you.
