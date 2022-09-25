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

#include "fp-device.h"
#include "fp-image-device.h"
#include "fp-image.h"
#include "fpi-assembling.h"
#include "fpi-context.h"
#include "fpi-image-device.h"
#include "fpi-image.h"
#include "fpi-ssm.h"
#include "glibconfig.h"
#include "gusb/gusb-device.h"
#include <stdio.h>
#include <stdlib.h>
#define FP_COMPONENT "goodixtls53xd"

#include <glib.h>
#include <string.h>

#include "drivers_api.h"
#include "goodix.h"
#include "goodix_proto.h"
#include "goodix53xd.h"

#include <math.h>

#define GOODIX53XD_WIDTH 64
#define GOODIX53XD_HEIGHT 80
#define GOODIX53XD_SCAN_WIDTH 64
#define GOODIX53XD_FRAME_SIZE (GOODIX53XD_WIDTH * GOODIX53XD_HEIGHT)
// For every 4 pixels there are 6 bytes and there are 8 extra start bytes and 5
// extra end
#define GOODIX53XD_RAW_FRAME_SIZE                                               \
    (GOODIX53XD_HEIGHT * GOODIX53XD_SCAN_WIDTH) / 4 * 6
#define GOODIX53XD_CAP_FRAMES 1 // Number of frames we capture per swipe

typedef unsigned short Goodix53xdPix;

struct _FpiDeviceGoodixTls53XD {
  FpiDeviceGoodixTls parent;

  guint8* otp;

  GSList* frames;

  Goodix53xdPix empty_img[GOODIX53XD_FRAME_SIZE];
};

G_DECLARE_FINAL_TYPE(FpiDeviceGoodixTls53XD, fpi_device_goodixtls53xd, FPI,
                     DEVICE_GOODIXTLS53XD, FpiDeviceGoodixTls);

G_DEFINE_TYPE(FpiDeviceGoodixTls53XD, fpi_device_goodixtls53xd,
              FPI_TYPE_DEVICE_GOODIXTLS);

// ---- ACTIVE SECTION START ----

/**
 * @brief Checks nothing and moves the state machine to the next state
 * 
 * @param dev 
 * @param user_data 
 * @param error 
 */
static void
check_none(FpDevice *dev, gpointer user_data, GError *error) {
  if (error) {
    fpi_ssm_mark_failed(user_data, error);
    return;
  }

  fpi_ssm_next_state(user_data);
}
/**
 * @brief Checks that firmware name is expected and advances to next state
 * 
 * @param dev 
 * @param firmware 
 * @param user_data 
 * @param error 
 */
static void
check_firmware_version(FpDevice *dev, gchar *firmware,
                                   gpointer user_data, GError *error) {
  if (error) {
    fpi_ssm_mark_failed(user_data, error);
    return;
  }

  fp_dbg("Device firmware: \"%s\"", firmware);

  if (strcmp(firmware, GOODIX_53XD_FIRMWARE_VERSION)) {
    g_set_error(&error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                "Invalid device firmware: \"%s\"", firmware);
    fpi_ssm_mark_failed(user_data, error);
    return;
  }

  fpi_ssm_next_state(user_data);
}
/**
 * @brief Checks that device was reset properly and advances to next state
 * 
 * @param dev 
 * @param success 
 * @param number 
 * @param user_data 
 * @param error 
 */
static void
check_reset(FpDevice *dev, gboolean success, guint16 number,
                        gpointer user_data, GError *error) {
  if (error) {
    fpi_ssm_mark_failed(user_data, error);
    return;
  }

  if (!success) {
    g_set_error(&error, G_IO_ERROR, G_IO_ERROR_FAILED,
                "Failed to reset device");
    fpi_ssm_mark_failed(user_data, error);
    return;
  }

  fp_dbg("Device reset number: %d", number);

  if (number != GOODIX_53XD_RESET_NUMBER) {
    g_set_error(&error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                "Invalid device reset number: %d", number);
    fpi_ssm_mark_failed(user_data, error);
    return;
  }

  fpi_ssm_next_state(user_data);
}
/**
 * @brief Check if preshared key is as expected then advances to next state
 * 
 * @param dev 
 * @param success 
 * @param flags 
 * @param psk 
 * @param length 
 * @param user_data 
 * @param error 
 */
static void
check_preset_psk_read(FpDevice *dev, gboolean success,
                                  guint32 flags, guint8 *psk, guint16 length,
                                  gpointer user_data, GError *error) {
  g_autofree gchar *psk_str = data_to_str(psk, length);

  if (error) {
    fpi_ssm_mark_failed(user_data, error);
    return;
  }

  if (!success) {
    g_set_error(&error, G_IO_ERROR, G_IO_ERROR_FAILED,
                "Failed to read PSK from device");
    fpi_ssm_mark_failed(user_data, error);
    return;
  }

  fp_dbg("Device PSK: 0x%s", psk_str);
  fp_dbg("Device PSK flags: 0x%08x", flags);

  if (flags != GOODIX_53XD_PSK_FLAGS) {
    g_set_error(&error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                "Invalid device PSK flags: 0x%08x", flags);
    fpi_ssm_mark_failed(user_data, error);
    return;
  }

  if (length != sizeof(goodix_53xd_psk_0)) {
    g_set_error(&error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                "Invalid device PSK: 0x%s", psk_str);
    fpi_ssm_mark_failed(user_data, error);
    return;
  }

  if (memcmp(psk, goodix_53xd_psk_0, sizeof(goodix_53xd_psk_0))) {
    g_set_error(&error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                "Invalid device PSK: 0x%s", psk_str);
    fpi_ssm_mark_failed(user_data, error);
    return;
  }

  fpi_ssm_next_state(user_data);
}
/**
 * @brief Checks for error and advances to next state
 * 
 * @param dev 
 * @param user_data 
 * @param err 
 */
static void
check_idle(FpDevice* dev, gpointer user_data, GError* err)
{

    if (err) {
        fpi_ssm_mark_failed(user_data, err);
        return;
    }
    fpi_ssm_next_state(user_data);
}

static void
check_config_upload(FpDevice* dev, gboolean success,
                                gpointer user_data, GError* error)
{
    if (error) {
        fpi_ssm_mark_failed(user_data, error);
    }
    else if (!success) {
        fpi_ssm_mark_failed(user_data,
                            g_error_new(FP_DEVICE_ERROR, FP_DEVICE_ERROR_PROTO,
                                        "failed to upload mcu config"));
    }
    else {
        fpi_ssm_next_state(user_data);
    }
}

static void
read_otp_callback(FpDevice* dev, guint8* data, guint16 len,
                              gpointer ssm, GError* err)
{
    if (err) {
        fpi_ssm_mark_failed(ssm, err);
        return;
    }
    if (len < 64) {
        fpi_ssm_mark_failed(ssm, g_error_new(FP_DEVICE_ERROR,
                                             FP_DEVICE_ERROR_DATA_INVALID,
                                             "OTP is invalid (len: %d)", 64));
        return;
    }
    FpiDeviceGoodixTls53XD* self = FPI_DEVICE_GOODIXTLS53XD(dev);
    self->otp = malloc(64);
    memcpy(self->otp, data, len);
    fpi_ssm_next_state(ssm);
}

/**
 * @brief Runs functions depending on current state of SSM
 * @details This is the main part of the driver. This function runs every time
 *  the state machine moves to the next state. This function looks at the 
 *  current state of the ssm responds accordingly
 * 
 * @param ssm 
 * @param dev 
 */
static void
activate_run_state(FpiSsm* ssm, FpDevice* dev)
{

    switch (fpi_ssm_get_cur_state(ssm)) {
    case ACTIVATE_READ_AND_NOP:
        // Nop seems to clear the previous command buffer. But we are
        // unable to do so.
        goodix_start_read_loop(dev);
        goodix_send_nop(dev, check_none, ssm);
        break;

    case ACTIVATE_ENABLE_CHIP:
        goodix_send_enable_chip(dev, TRUE, check_none, ssm);
        break;

    case ACTIVATE_NOP:
        goodix_send_nop(dev, check_none, ssm);
        break;

    case ACTIVATE_CHECK_FW_VER:
        goodix_send_firmware_version(dev, check_firmware_version, ssm);
        break;

    case ACTIVATE_CHECK_PSK:
        goodix_send_preset_psk_read(dev, GOODIX_53XD_PSK_FLAGS, 32,
                                    check_preset_psk_read, ssm);
        break;

    case ACTIVATE_RESET:
        goodix_send_reset(dev, TRUE, 20, check_reset, ssm);
        break;

    case ACTIVATE_OTP:
        goodix_send_read_otp(dev, read_otp_callback, ssm);
        break;

    case ACTIVATE_SET_MCU_IDLE:
        goodix_send_mcu_switch_to_idle_mode(dev, 20, check_idle, ssm);
        break;

    case ACTIVATE_SET_MCU_CONFIG:
        goodix_send_upload_config_mcu(dev, goodix_53xd_config,
                                        sizeof(goodix_53xd_config), NULL,
                                        check_config_upload, ssm);
        break;
    default: // TODO What happens if we have a bad state?
    }
}
/**
 * @brief Checks for error, then marks device activation as complete if no error
 * @details Called when finishing device activation, either successful or not
 * 
 * @param dev 
 * @param user_data 
 * @param error 
 */
static void
tls_activation_complete(FpDevice* dev, gpointer user_data,
                                    GError* error)
{
    if (error) {
        fp_err("failed to complete tls activation: %s", error->message);
        return;
    }
    FpImageDevice* image_dev = FP_IMAGE_DEVICE(dev);

    fpi_image_device_activate_complete(image_dev, error);
}
/**
 * @brief Checks 
 * 
 * @param ssm 
 * @param dev 
 * @param error 
 */
static void
activate_complete(FpiSsm* ssm, FpDevice* dev, GError* error)
{
    G_DEBUG_HERE();
    if (!error)
        goodix_tls(dev, tls_activation_complete, NULL);
    else {
        fp_err("failed during activation: %s (code: %d)", error->message,
               error->code);
        fpi_image_device_activate_complete(FP_IMAGE_DEVICE(dev), error);
    }
}

// ---- ACTIVE SECTION END ----

// -----------------------------------------------------------------------------

// ---- SCAN SECTION START ----

static void
check_none_cmd(FpDevice* dev, guint8* data, guint16 len,
                           gpointer ssm, GError* err)
{
    if (err) {
        fpi_ssm_mark_failed(ssm, err);
        return;
    }
    fpi_ssm_next_state(ssm);
}

static unsigned char
get_pix(struct fpi_frame_asmbl_ctx* ctx,
                             struct fpi_frame* frame, unsigned int x,
                             unsigned int y)
{
    return frame->data[x + y * GOODIX53XD_WIDTH];
}

static void
decode_frame(Goodix53xdPix frame[GOODIX53XD_FRAME_SIZE],
                         const guint8* raw_frame)
{
    Goodix53xdPix uncropped[GOODIX53XD_SCAN_WIDTH * GOODIX53XD_HEIGHT];
    Goodix53xdPix* pix = uncropped;
    for (int i = 0; i < GOODIX53XD_RAW_FRAME_SIZE; i += 6) {
        const guint8* chunk = raw_frame + i;
        *pix++ = ((chunk[0] & 0xf) << 8) + chunk[1];
        *pix++ = (chunk[3] << 4) + (chunk[0] >> 4);
        *pix++ = ((chunk[5] & 0xf) << 8) + chunk[2];
        *pix++ = (chunk[4] << 4) + (chunk[5] >> 4);
    }

    for (int y = 0; y != GOODIX53XD_HEIGHT; ++y) {
        for (int x = 0; x != GOODIX53XD_WIDTH; ++x) {
            const int idx = x + y * GOODIX53XD_SCAN_WIDTH;
            frame[x + y * GOODIX53XD_WIDTH] = uncropped[idx];
        }
    }
}

/**
 * @brief Squashes the 12 bit pixels of a raw frame into the 4 bit pixels used
 * by libfprint.
 * @details Borrowed from the elan driver. We reduce frames to
 * within the max and min.
 *
 * @param frame
 * @param squashed
 */
static void
squash_frame_linear(Goodix53xdPix* frame, guint8* squashed)
{
    Goodix53xdPix min = 0xffff;
    Goodix53xdPix max = 0;

    for (int i = 0; i != GOODIX53XD_FRAME_SIZE; ++i) {
        const Goodix53xdPix pix = frame[i];
        if (pix < min) {
            min = pix;
        }
        if (pix > max) {
            max = pix;
        }
    }

    for (int i = 0; i != GOODIX53XD_FRAME_SIZE; ++i) {
        const Goodix53xdPix pix = frame[i];
        if (pix - min == 0 || max - min == 0) {
            squashed[i] = 0;
        }
        else {
            squashed[i] = (pix - min) * 0xff / (max - min);
        }
    }
}

/**
 * @brief Subtracts the background from the frame
 *
 * @param frame
 * @param background
 */

static void
process_frame(Goodix53xdPix* raw_frame, frame_processing_info* info)
{
    struct fpi_frame* frame =
        g_malloc(GOODIX53XD_FRAME_SIZE + sizeof(struct fpi_frame));
    //postprocess_frame(raw_frame, info->dev->empty_img);
    squash_frame_linear(raw_frame, frame->data);

    *(info->frames) = g_slist_append(*(info->frames), frame);
}

static void
save_frame(FpiDeviceGoodixTls53XD* self, guint8* raw)
{
    Goodix53xdPix* frame = 
                        malloc(GOODIX53XD_FRAME_SIZE * sizeof(Goodix53xdPix));
    decode_frame(frame, raw);
    self->frames = g_slist_append(self->frames, frame);
}

static void
scan_on_read_img(FpDevice* dev, guint8* data, guint16 len,
                             gpointer ssm, GError* err)
{
    if (err) {
        fpi_ssm_mark_failed(ssm, err);
        return;
    }


    FpiDeviceGoodixTls53XD* self = FPI_DEVICE_GOODIXTLS53XD(dev);
    save_frame(self, data);
    if (g_slist_length(self->frames) <= GOODIX53XD_CAP_FRAMES) {
        fpi_ssm_jump_to_state(ssm, SCAN_STAGE_SWITCH_TO_FDT_MODE);
    }
    else {
        GSList* raw_frames = g_slist_nth(self->frames, 1);

        FpImageDevice* img_dev = FP_IMAGE_DEVICE(dev);
        struct fpi_frame_asmbl_ctx assembly_ctx;
        assembly_ctx.frame_width = GOODIX53XD_WIDTH;
        assembly_ctx.frame_height = GOODIX53XD_HEIGHT;
        assembly_ctx.image_width = GOODIX53XD_WIDTH*3;
        assembly_ctx.get_pixel = get_pix;

        GSList* frames = NULL;
        frame_processing_info pinfo = {.dev = self, .frames = &frames};

        g_slist_foreach(raw_frames, (GFunc) process_frame, &pinfo);
        frames = g_slist_reverse(frames);

        fpi_do_movement_estimation(&assembly_ctx, frames);
        FpImage* img = fpi_assemble_frames(&assembly_ctx, frames);

        g_slist_free_full(frames, g_free);
        g_slist_free_full(self->frames, g_free);
        self->frames = g_slist_alloc();

        fpi_image_device_image_captured(img_dev, img);


        fpi_image_device_report_finger_status(img_dev, FALSE);

        fpi_ssm_next_state(ssm);
    }
}


static void 
goodix_53xd_send_mcu_get_image(FpDevice* dev, guint8* payload, guint16 length,
                            GoodixImageCallback callback, gpointer user_data)
{

  GoodixCallbackInfo *cb_info;

  if (callback)
  {
    cb_info = malloc(sizeof(GoodixCallbackInfo));

    cb_info->callback = G_CALLBACK(callback);
    cb_info->user_data = user_data;

    goodix_send_protocol(dev, GOODIX_CMD_MCU_GET_IMAGE, payload,
                         length, NULL, TRUE, GOODIX_TIMEOUT, TRUE,
                         goodix_receive_default, cb_info);
    return;
  }

  goodix_send_protocol(dev, GOODIX_CMD_MCU_GET_IMAGE, payload,
                       length, NULL, TRUE, GOODIX_TIMEOUT, TRUE,
                       NULL, NULL);
}

static void
goodix_53xd_tls_read_image(FpDevice* dev, guint8* payload,
                                        guint16 length, 
                                        GoodixImageCallback callback,
                                        gpointer user_data)
{
  g_assert(callback);
  GoodixCallbackInfo *cb_info = malloc(sizeof(GoodixCallbackInfo));

  cb_info->callback = G_CALLBACK(callback);
  cb_info->user_data = user_data;

  goodix_53xd_send_mcu_get_image(dev, payload, length, 
                                    goodix_tls_ready_image_handler, cb_info);
}

static void
scan_get_img(FpDevice* dev, FpiSsm* ssm)
{
    FpImageDevice* img_dev = FP_IMAGE_DEVICE(dev);
    FpiDeviceGoodixTls53XD* self = FPI_DEVICE_GOODIXTLS53XD(img_dev);
    guint8 payload[] = {0x41, 0x03, self->otp[26], 0x00,
        self->otp[26] - 6, 0x00, self->otp[45], 0x00, self->otp[45] - 4, 0x00};
    goodix_53xd_tls_read_image(dev, (guint8 *)&payload,
                                sizeof(payload), scan_on_read_img, ssm);
}

static void
goodix_send_mcu_switch_to_fdt_down_with_no_reply(FpDevice *dev, 
                                                guint8 *mode,
                                                guint16 length,
                                                GDestroyNotify free_func,
                                                GoodixDefaultCallback callback,
                                                gpointer user_data)
{
  GoodixCallbackInfo *cb_info;

  if (callback)
  {
    cb_info = malloc(sizeof(GoodixCallbackInfo));

    cb_info->callback = G_CALLBACK(callback);
    cb_info->user_data = user_data;

    goodix_send_protocol(dev, GOODIX_CMD_MCU_SWITCH_TO_FDT_DOWN, mode, length,
                         free_func, TRUE, 0, FALSE, goodix_receive_default,
                         cb_info);
    return;
  }

  goodix_send_protocol(dev, GOODIX_CMD_MCU_SWITCH_TO_FDT_DOWN, mode, length,
                       free_func, TRUE, 0, FALSE, NULL, NULL);
}

static void
goodix_send_mcu_switch_to_fdt_mode_no_reply(FpDevice *dev, 
                                            guint8 *mode,
                                            guint16 length,
                                            GDestroyNotify free_func,
                                            GoodixDefaultCallback callback,
                                            gpointer user_data)
{
  GoodixCallbackInfo *cb_info;

  if (callback)
  {
    cb_info = malloc(sizeof(GoodixCallbackInfo));

    cb_info->callback = G_CALLBACK(callback);
    cb_info->user_data = user_data;

    goodix_send_protocol(dev, GOODIX_CMD_MCU_SWITCH_TO_FDT_MODE, mode, length,
                         free_func, TRUE, 0, FALSE, goodix_receive_default,
                         cb_info);
    return;
  }

  goodix_send_protocol(dev, GOODIX_CMD_MCU_SWITCH_TO_FDT_MODE, mode, length,
                       free_func, TRUE, 0, FALSE, NULL, NULL);
}

static void
scan_run_state(FpiSsm* ssm, FpDevice* dev)
{
    FpImageDevice* img_dev = FP_IMAGE_DEVICE(dev);
    FpiDeviceGoodixTls53XD* self = FPI_DEVICE_GOODIXTLS53XD(img_dev);


    switch (fpi_ssm_get_cur_state(ssm)) {

    case SCAN_STAGE_SWITCH_TO_FDT_MODE:
        goodix_send_mcu_switch_to_fdt_mode_no_reply(dev, 
                                        (guint8 *)fdt_switch_state_mode_53xd,
                                        sizeof(fdt_switch_state_mode_53xd), NULL,
                                        check_none_cmd, ssm);
        break;

    case SCAN_STAGE_SWITCH_TO_FDT_DOWN:
        // FDT Down Cali
        fdt_switch_state_down_53xd[2] = self->otp[33];
        fdt_switch_state_down_53xd[4] = self->otp[41];
        fdt_switch_state_down_53xd[6] = self->otp[42];
        fdt_switch_state_down_53xd[8] = self->otp[43];

        // First FDT down must not send a reply
        fdt_switch_state_down_53xd[26] = 0x00;
        goodix_send_mcu_switch_to_fdt_down_with_no_reply(dev, 
                                        (guint8*) fdt_switch_state_down_53xd,
                                        sizeof(fdt_switch_state_down_53xd),
                                        NULL, receive_fdt_down_ack, ssm);
        break;
    case SCAN_STAGE_GET_IMG:
        fpi_image_device_report_finger_status(img_dev, TRUE);
        guint16 payload = {0x05}; 
        goodix_send_write_sensor_register(dev,
                                     556, payload, write_sensor_complete, ssm);
        break;
    }
}

static void
receive_fdt_down_ack(FpDevice* dev, guint8* data, guint16 len,
                           gpointer ssm, GError* err)
{
    if (err) {
        fpi_ssm_mark_failed(ssm, err);
        return;
    }

    // Second FDT down must send a response
    fdt_switch_state_down_53xd[26] = 0x01;
    goodix_send_mcu_switch_to_fdt_down(dev, 
                                    (guint8*) fdt_switch_state_down_53xd,
                                    sizeof(fdt_switch_state_down_53xd), NULL,
                                    check_none_cmd, ssm);
}

static void
write_sensor_complete(FpDevice *dev, 
                                    gpointer user_data, GError *error) 
{
    if (error) {
        fp_err("failed to scan: %s (code: %d)", error->message, error->code);
        return;
    }
    scan_get_img(dev, user_data);
}

static void
scan_complete(FpiSsm* ssm, FpDevice* dev, GError* error)
{
    if (error) {
        fp_err("failed to scan: %s (code: %d)", error->message, error->code);
        return;
    }
    fp_dbg("finished scan");
}

static void
scan_start(FpiDeviceGoodixTls53XD* dev)
{
    fpi_ssm_start(fpi_ssm_new(FP_DEVICE(dev), scan_run_state, SCAN_STAGE_NUM),
                  scan_complete);
}

// ---- SCAN SECTION END ----

// -----------------------------------------------------------------------------

// ---- DEV SECTION START ----

static void
dev_init(FpImageDevice *img_dev) {
  FpDevice *dev = FP_DEVICE(img_dev);
  GError *error = NULL;

  if (goodix_dev_init(dev, &error)) {
    fpi_image_device_open_complete(img_dev, error);
    return;
  }

  fpi_image_device_open_complete(img_dev, NULL);
}

static void
dev_deinit(FpImageDevice *img_dev) {
  FpDevice *dev = FP_DEVICE(img_dev);
  GError *error = NULL;

  if (goodix_dev_deinit(dev, &error)) {
    fpi_image_device_close_complete(img_dev, error);
    return;
  }

  fpi_image_device_close_complete(img_dev, NULL);
}

static void
dev_activate(FpImageDevice *img_dev) {
    FpDevice* dev = FP_DEVICE(img_dev);
    FpiSsm* ssm = fpi_ssm_new(dev, activate_run_state, ACTIVATE_NUM_STATES);
    fpi_ssm_start(ssm, activate_complete);
}

static void
dev_change_state(FpImageDevice* img_dev, FpiImageDeviceState state)
{
    FpiDeviceGoodixTls53XD* self = FPI_DEVICE_GOODIXTLS53XD(img_dev);
    G_DEBUG_HERE();

    if (state == FPI_IMAGE_DEVICE_STATE_AWAIT_FINGER_ON) {
        scan_start(self);
    }
}

static void
goodix53xd_reset_state(FpiDeviceGoodixTls53XD* self) {}

static void
dev_deactivate(FpImageDevice *img_dev) {
    FpDevice* dev = FP_DEVICE(img_dev);
    goodix_reset_state(dev);
    GError* error = NULL;
    goodix_shutdown_tls(dev, &error);
    goodix53xd_reset_state(FPI_DEVICE_GOODIXTLS53XD(img_dev));
    fpi_image_device_deactivate_complete(img_dev, error);
}

// ---- DEV SECTION END ----

static void
fpi_device_goodixtls53xd_init(FpiDeviceGoodixTls53XD* self)
{
    self->frames = g_slist_alloc();
}

/*
    This is the only thing the rest of libfprint cares about. This tells
    libfprint how to interact with the device, that it is a image based device,
    usb based device, which endpoints to use for it, etc
*/
static void
fpi_device_goodixtls53xd_class_init(FpiDeviceGoodixTls53XDClass *class) {
  FpiDeviceGoodixTlsClass *gx_class = FPI_DEVICE_GOODIXTLS_CLASS(class);
  FpDeviceClass *dev_class = FP_DEVICE_CLASS(class);
  FpImageDeviceClass *img_dev_class = FP_IMAGE_DEVICE_CLASS(class);

  gx_class->interface = GOODIX_53XD_INTERFACE;
  gx_class->ep_in = GOODIX_53XD_EP_IN;
  gx_class->ep_out = GOODIX_53XD_EP_OUT;

  dev_class->id = "goodixtls53xd";
  dev_class->full_name = "Goodix TLS Fingerprint Sensor 53XD";
  dev_class->type = FP_DEVICE_TYPE_USB; 
  dev_class->id_table = id_table; // Devices supported by driver

  dev_class->scan_type = FP_SCAN_TYPE_PRESS; // Either scan or swipe

  img_dev_class->bz3_threshold = 24; // Detection threshold
  img_dev_class->img_width = GOODIX53XD_WIDTH; // Only given for constant width
  img_dev_class->img_height = GOODIX53XD_HEIGHT; // Same but height

  img_dev_class->img_open = dev_init;   // Called to claim and open device
  img_dev_class->img_close = dev_deinit;    // Called to close and
                                            // release device
  img_dev_class->activate = dev_activate;   // Called to start finger scanning 
                                            // and/or finger detection
  img_dev_class->deactivate = dev_deactivate;   // Called to stop waiting
                                                //for finger
  img_dev_class->change_state = dev_change_state;   // Called anytime the device
                                                    // changes state, such as
                                                    // going from idle to
                                                    // waiting for finger
 

  fpi_device_class_auto_initialize_features(dev_class);
}
