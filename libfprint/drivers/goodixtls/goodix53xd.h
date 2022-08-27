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


const guint8 goodix_53xd_psk_0[] = {
    0x66, 0x68, 0x7a, 0xad, 0xf8, 0x62, 0xbd, 0x77, 0x6c, 0x8f, 0xc1, 
    0x8b, 0x8e, 0x9f, 0x8e, 0x20, 0x08, 0x97, 0x14, 0x85, 0x6e, 0xe2, 
    0x33, 0xb3, 0x90, 0x2a, 0x59, 0x1d, 0x0d, 0x5f, 0x29, 0x25};

/*
const guint8 goodix_53xd_psk_0[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
*/
guint8 goodix_53xd_config[] = {
    0x70, 0x11, 0x60, 0x71, 0x2c, 0x9d, 0x2c, 0xc9, 0x1c, 0xe5, 0x18,
    0xfd, 0x00, 0xfd, 0x00, 0xfd, 0x03, 0xba, 0x00, 0x01, 0x80, 0xca,
    0x00, 0x08, 0x00, 0x84, 0x00, 0xbe, 0xc3, 0x86, 0x00, 0xb1, 0xb6,
    0x88, 0x00, 0xba, 0xba, 0x8a, 0x00, 0xb3, 0xb3, 0x8c, 0x00, 0xbc, 
    0xbc, 0x8e, 0x00, 0xb1, 0xb1, 0x90, 0x00, 0xbb, 0xbb, 0x92, 0x00,
    0xb1, 0xb1, 0x94, 0x00, 0x00, 0x00, 0x96, 0x00, 0x00, 0x00, 0x98,
    0x00, 0x00, 0x00, 0x9a, 0x00, 0x00, 0x00, 0xd2, 0x00, 0x00, 0x00,
    0xd4, 0x00, 0x00, 0x00, 0xd6, 0x00, 0x00, 0x00, 0xd8, 0x00, 0x00,
    0x00, 0x50, 0x00, 0x01, 0x05, 0xd0, 0x00, 0x00, 0x00, 0x70, 0x00,
    0x00, 0x00, 0x72, 0x00, 0x78, 0x56, 0x74, 0x00, 0x34, 0x12, 0x20,
    0x00, 0x10, 0x40, 0x2a, 0x01, 0x02, 0x04, 0x22, 0x00, 0x01, 0x20,
    0x24, 0x00, 0x32, 0x00, 0x80, 0x00, 0x01, 0x00, 0x5c, 0x00, 0x01,
    0x01, 0x56, 0x00, 0x24, 0x20, 0x58, 0x00, 0x01, 0x02, 0x32, 0x00,
    0x04, 0x02, 0x66, 0x00, 0x00, 0x02, 0x7c, 0x00, 0x00, 0x58, 0x82,
    0x00, 0x7f, 0x08, 0x2a, 0x01, 0x82, 0x07, 0x22, 0x00, 0x01, 0x20,
    0x24, 0x00, 0x14, 0x00, 0x80, 0x00, 0x01, 0x40, 0x5c, 0x00, 0xe7,
    0x00, 0x56, 0x00, 0x06, 0x14, 0x58, 0x00, 0x04, 0x02, 0x32, 0x00,
    0x0c, 0x02, 0x66, 0x00, 0x00, 0x02, 0x7c, 0x00, 0x00, 0x58, 0x82,
    0x00, 0x80, 0x08, 0x2a, 0x01, 0x08, 0x00, 0x5c, 0x00, 0x01, 0x01,
    0x54, 0x00, 0x00, 0x01, 0x62, 0x00, 0x08, 0x04, 0x64, 0x00, 0x10,
    0x00, 0x66, 0x00, 0x00, 0x02, 0x7c, 0x00, 0x00, 0x58, 0x2a, 0x01, 
    0x08, 0x00, 0x5c, 0x00, 0xdc, 0x00, 0x52, 0x00, 0x08, 0x00, 0x54, 
    0x00, 0x00, 0x01, 0x66, 0x00, 0x00, 0x02, 0x7c, 0x00, 0x00, 0x58, 
    0x20, 0xc5, 0x1d};

static const FpIdEntry id_table[] = {
    {.vid = 0x27c6, .pid = 0x538d},
    {.vid = 0x27c6, .pid = 0x532d},
    {.vid = 0, .pid = 0, .driver_data = 0},
};


static void write_sensor_complete(FpDevice *dev, gpointer user_data, GError *error) ;
static void receive_fdt_down_ack(FpDevice* dev, guint8* data, guint16 len,
                           gpointer ssm, GError* err);
