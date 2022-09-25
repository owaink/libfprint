// Goodix Tls driver for libfprint

// Copyright (C) 2021 Alexander Meiler <alex.meiler@protonmail.com>
// Copyright (C) 2021 Matthieu CHARETTE <matthieu.charette@gmail.com>
// Copyright (C) 2021 Michael Teuscher <michael.teuscher@pm.me>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#pragma once

#define GOODIX_53XD_INTERFACE (0)
#define GOODIX_53XD_EP_IN (0x83 | FPI_USB_ENDPOINT_IN)
#define GOODIX_53XD_EP_OUT (0x1 | FPI_USB_ENDPOINT_OUT)

#define GOODIX_53XD_FIRMWARE_VERSION ("GF5298_GM168SEC_APP_13016")

#define GOODIX_53XD_PSK_FLAGS (0xbb020001)

#define GOODIX_53XD_RESET_NUMBER (2048)

/*
    The preshared key that is used for TLS communication. This should NOT be
    hardcoded, we need to address this before shipping the driver
    TODO: Do not let this go into prod for fuck's sake

    Preshared key is 32 bytes in length
*/
const guint8
goodix_53xd_psk_0[] = {
 0x66, 0x68, 0x7a, 0xad,  0xf8, 0x62, 0xbd, 0x77,
 0x6c, 0x8f, 0xc1, 0x8b,  0x8e, 0x9f, 0x8e, 0x20,
 0x08, 0x97, 0x14, 0x85,  0x6e, 0xe2, 0x33, 0xb3,
 0x90, 0x2a, 0x59, 0x1d,  0x0d, 0x5f, 0x29, 0x25
};

/*
    This is a binary blob that the proprietary driver calls the config. We have
    no idea what it means, but here it is!

    256 bytes in length
*/
const guint8 
goodix_53xd_config[] = {
 // 0-31
 0x70, 0x11, 0x60, 0x71,  0x2c, 0x9d, 0x2c, 0xc9,
 0x1c, 0xe5, 0x18, 0xfd,  0x00, 0xfd, 0x00, 0xfd,
 0x03, 0xba, 0x00, 0x01,  0x80, 0xca, 0x00, 0x08,
 0x00, 0x84, 0x00, 0xbe,  0xc3, 0x86, 0x00, 0xb1,
 // 32-63
 0xb6, 0x88, 0x00, 0xba,  0xba, 0x8a, 0x00, 0xb3,
 0xb3, 0x8c, 0x00, 0xbc,  0xbc, 0x8e, 0x00, 0xb1,
 0xb1, 0x90, 0x00, 0xbb,  0xbb, 0x92, 0x00, 0xb1,
 0xb1, 0x94, 0x00, 0x00,  0x00, 0x96, 0x00, 0x00,
 // 64-95
 0x00, 0x98, 0x00, 0x00,  0x00, 0x9a, 0x00, 0x00,
 0x00, 0xd2, 0x00, 0x00,  0x00, 0xd4, 0x00, 0x00,
 0x00, 0xd6, 0x00, 0x00,  0x00, 0xd8, 0x00, 0x00,
 0x00, 0x50, 0x00, 0x01,  0x05, 0xd0, 0x00, 0x00,
 // 96-127
 0x00, 0x70, 0x00, 0x00,  0x00, 0x72, 0x00, 0x78,
 0x56, 0x74, 0x00, 0x34,  0x12, 0x20, 0x00, 0x10,
 0x40, 0x2a, 0x01, 0x02,  0x04, 0x22, 0x00, 0x01,
 0x20, 0x24, 0x00, 0x32,  0x00, 0x80, 0x00, 0x01,
 // 128-159
 0x00, 0x5c, 0x00, 0x01,  0x01, 0x56, 0x00, 0x24,
 0x20, 0x58, 0x00, 0x01,  0x02, 0x32, 0x00, 0x04,
 0x02, 0x66, 0x00, 0x00,  0x02, 0x7c, 0x00, 0x00,
 0x58, 0x82, 0x00, 0x7f,  0x08, 0x2a, 0x01, 0x82,
 // 160-191
 0x07, 0x22, 0x00, 0x01,  0x20, 0x24, 0x00, 0x14,
 0x00, 0x80, 0x00, 0x01,  0x40, 0x5c, 0x00, 0xe7,
 0x00, 0x56, 0x00, 0x06,  0x14, 0x58, 0x00, 0x04,
 0x02, 0x32, 0x00, 0x0c,  0x02, 0x66, 0x00, 0x00,
 // 192-223
 0x02, 0x7c, 0x00, 0x00,  0x58, 0x82, 0x00, 0x80,
 0x08, 0x2a, 0x01, 0x08,  0x00, 0x5c, 0x00, 0x01,
 0x01, 0x54, 0x00, 0x00,  0x01, 0x62, 0x00, 0x08,
 0x04, 0x64, 0x00, 0x10,  0x00, 0x66, 0x00, 0x00,
 // 224-255
 0x02, 0x7c, 0x00, 0x00,  0x58, 0x2a, 0x01, 0x08,
 0x00, 0x5c, 0x00, 0xdc,  0x00, 0x52, 0x00, 0x08,
 0x00, 0x54, 0x00, 0x00,  0x01, 0x66, 0x00, 0x00,
 0x02, 0x7c, 0x00, 0x00,  0x58, 0x20, 0xc5, 0x1d
};

/*
    Another binary blob from the proprietary windows driver, switches to
    "fdt mode". No idea what fdt stands for. Could stand for "Finger Down Touch"
    or "Flattened Device Tree"?
    
    21 bytes in length (weird size but ok)
*/

guint8
fdt_switch_state_mode_53xd[] = {
 0x0d, 0x01, 0x28, 0x01,  0x22, 0x01, 0x28, 0x01,
 0x24, 0x01, 0x91, 0x91,  0x8b, 0x8b, 0x96, 0x96,
 0x91, 0x91, 0x98, 0x98,  0x90, 0x90, 0x92, 0x92,
 0x88, 0x88, 0x00
};

/*
    Another binary blob from the proprietary windows driver, switches to
    "fdt down" or deactivates the fdt? No idea what fdt stands for.
    Could stand for "Finger Down Touch" or "Flattened Device Tree"?
    
    21 bytes in length (weird size but ok)
*/

guint8
fdt_switch_state_down_53xd[] = {
 0x8c, 0x01, 0x28, 0x01,  0x22, 0x01, 0x28, 0x01,
 0x24, 0x01, 0x91, 0x91,  0x8b, 0x8b, 0x96, 0x96,
 0x91, 0x91, 0x98, 0x98,  0x90, 0x90, 0x92, 0x92,
 0x88, 0x88, 0x00
};

/*
    States to go through for scanning an empty image, processed by the ssm
    TODO: Is this necessary?
*/
enum
scan_empty_img_state {
 SCAN_EMPTY_NAV0,
 SCAN_EMPTY_GET_IMG,

 SCAN_EMPTY_NUM,
};

/*
 These are the states the driver iterates through when a device it supports
 is "activated", with the final state just being a marker of how many states
 there are in total

 TODO: Checking the PSK and firmware can probably be moved to device init
*/
enum
activate_states {
 ACTIVATE_READ_AND_NOP,     // First, send a no-operation
                            // command to get a clean slate
 ACTIVATE_ENABLE_CHIP,      // Second, tell the sensor to start
                            // listening for a finger
 ACTIVATE_NOP,              // Third, do nothing...? TODO
 ACTIVATE_CHECK_FW_VER,     // Fourth, check the running firmware
                            // is what we expect
 ACTIVATE_CHECK_PSK,        // Fifth, check that the preshared key
                            // used for TLS communication is as expected
 ACTIVATE_RESET,            // Sixth, no idea TODO
 ACTIVATE_OTP,              // Seventh, establish one time
                            // password for TLS communication
 ACTIVATE_SET_MCU_IDLE,     // Eighth, tell the sensor to idle
 ACTIVATE_SET_MCU_CONFIG,   // Ninth, pass configuration sensor
 ACTIVATE_NUM_STATES,       // Number of states in state machine, currently nine
};

/*
    Stages for establishing one-time password for TLS communication
*/
enum
otp_write_states {
 OTP_WRITE_1,
 OTP_WRITE_2,

 OTP_WRITE_NUM,
};

/*
    Stages to go through to acquire an image from the device, based on 
    windows proprietary driver
*/
enum
SCAN_STAGES {
 SCAN_STAGE_SWITCH_TO_FDT_DOWN, // Set FDT (no idea what it is) to down
 SCAN_STAGE_SWITCH_TO_FDT_MODE, // Then immediately set it to "mode"? 
 SCAN_STAGE_GET_IMG,            // Actually get the image

 SCAN_STAGE_NUM,
};

typedef struct
_frame_processing_info {
 FpiDeviceGoodixTls53XD* dev;
 GSList** frames;

} frame_processing_info;

static void
write_sensor_complete(FpDevice *dev, gpointer user_data, GError *error);
static void
receive_fdt_down_ack(FpDevice* dev, guint8* data, guint16 len, gpointer ssm, 
      GError* err);

/*
    These are the devices that are supported by this driver, identified
    by the vendor id (.vid) and product id (.pid)
*/
static const
FpIdEntry id_table[] = {
 {.vid = 0x27c6, .pid = 0x538d},
 {.vid = 0x27c6, .pid = 0x532d}, // XPS 13 2 in 1 (7390)
 {.vid = 0, .pid = 0, .driver_data = 0},
};