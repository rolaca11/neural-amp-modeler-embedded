#!/bin/bash -
# Flash programming script for STM32N6 NUCLEO board
# Programs FSBL, Secure, and Non-Secure signed binaries to external flash via SWD

# ============================================================ Configuration ============================================================

# STM32CubeProgrammer CLI path
stm32programmercli="/opt/st/stm32cubeclt_1.21.0/STM32CubeProgrammer/bin/STM32_Programmer_CLI"

# External flash loader for NUCLEO-N657X0-Q board (MX25UM51245G on XSPI2)
stm32ExtLoaderFlash="/home/ropi/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/ExternalLoader/MX25UM51245G_STM32N6570-NUCLEO.stldr"

# Base address of external flash
fsbl_address=0x70000000
s_address=0x70100000
ns_address=0x70200000

# Binary paths (signed binaries produced by the build)
fsbl_bin="FSBL/build/stm32n6-test_FSBL.signed.bin"
appli_s_bin="AppliSecure/build/stm32n6-test_AppliSecure.signed.bin"
appli_ns_bin="AppliNonSecure/build/stm32n6-test_AppliNonSecure.signed.bin"

# CubeProgrammer connection
connect_reset="-c port=SWD ap=1"

# ============================================================ Functions ============================================================

error()
{
    echo "       Error when trying to $action"
    echo "       Flash programming aborted"
    echo "Error when trying to $action"
    echo "Flash programming aborted"
    echo
    exit 1
}

# ============================================================ Pre-flight checks ============================================================

# Ensure we run from the project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR" || exit 1

# Verify binaries exist
for bin in "$fsbl_bin" "$appli_s_bin" "$appli_ns_bin"; do
    if [ ! -f "$bin" ]; then
        echo "ERROR: Binary not found: $bin"
        echo "Please build the project first."
        exit 1
    fi
done

echo "===== STM32N6 Flash Programming ====="
echo "FSBL:           $fsbl_bin"
echo "AppliSecure:    $appli_s_bin"
echo "AppliNonSecure: $appli_ns_bin"
echo "======================================"
echo

# ============================================================ Download images ============================================================

action="Reset the target"
echo "$action"
"$stm32programmercli" $connect_reset
if [ $? -ne 0 ]; then
    error
fi
echo "Reset done"

action="Write FSBL"
echo "$action"
"$stm32programmercli" $connect_reset -el "$stm32ExtLoaderFlash" -d "$fsbl_bin" $fsbl_address -v
if [ $? -ne 0 ]; then
    error
fi
echo "FSBL written"

action="Write AppliSecure"
echo "$action"
"$stm32programmercli" $connect_reset -el "$stm32ExtLoaderFlash" -d "$appli_s_bin" $s_address -v
if [ $? -ne 0 ]; then
    error
fi
echo "AppliSecure written"

action="Write AppliNonSecure"
echo "$action"
"$stm32programmercli" $connect_reset -el "$stm32ExtLoaderFlash" -d "$appli_ns_bin" $ns_address -v
if [ $? -ne 0 ]; then
    error
fi
echo "AppliNonSecure written"

echo
echo "Flash programming success"
exit 0
