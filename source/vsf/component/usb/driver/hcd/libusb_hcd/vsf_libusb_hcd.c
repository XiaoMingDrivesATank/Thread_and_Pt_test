/*****************************************************************************
 *   Copyright(C)2009-2019 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

/*============================ INCLUDES ======================================*/
#include "component/usb/vsf_usb_cfg.h"

#if VSF_USE_USB_HOST == ENABLED && VSF_USE_USB_HOST_HCD_LIBUSB == ENABLED

#define VSF_USBH_IMPLEMENT_HCD
#define VSF_USBH_IMPLEMENT_HUB
#include "vsf.h"
#include "./vsf_libusb_hcd.h"

#include "libusb.h"

/*============================ MACROS ========================================*/

// TODO: remove to user configuration
#define VSF_LIBUSB_HCD_CFG_DEV_NUM                  1
// CSR8510
#define VSF_LIBUSB_HCD_DEV0_VID                     0x0A12
#define VSF_LIBUSB_HCD_DEV0_PID                     0x0001



#define VSF_EVT_LIBUSB_HCD_BASE                     ((VSF_EVT_USER + 0x100) & ~0xFF)

/*============================ MACROFIED FUNCTIONS ===========================*/

#define VSF_LIBUSB_HCD_DEF_DEV(__N, __BIT)                                      \
            {                                                                   \
                .vid = VSF_LIBUSB_HCD_DEV##__N##_VID,                           \
                .pid = VSF_LIBUSB_HCD_DEV##__N##_PID,                           \
                .addr = -1,                                                     \
            },

/*============================ TYPES =========================================*/

struct vsf_libusb_hcd_dev_t {
    uint16_t vid, pid;
    libusb_device_handle *handle;
    vsf_usbh_dev_t *dev;

    enum usb_device_speed_t speed;
    enum {
        VSF_LIBUSB_HCD_DEV_STATE_DETACHED,
        VSF_LIBUSB_HCD_DEV_STATE_ATTACHED,
    } state;

    int8_t addr;
    union {
        uint8_t value;
        struct {
            uint8_t is_resetting    : 1;
            uint8_t is_attached    : 1;
            uint8_t is_detached     : 1;
        };
    } evt_mask;

    vsf_arch_irq_thread_t irq_thread;
    vsf_arch_irq_request_t irq_request;
};
typedef struct vsf_libusb_hcd_dev_t vsf_libusb_hcd_dev_t;

struct vsf_libusb_hcd_t {
    libusb_context *ctx;

    vsf_libusb_hcd_dev_t devs[VSF_LIBUSB_HCD_CFG_DEV_NUM];
    vsf_usbh_hcd_t *hcd;
    uint32_t new_mask;
    uint8_t cur_dev_idx;
    bool is_hotplug_supported;

    vsf_eda_t *init_eda;
    vsf_arch_irq_thread_t init_thread;
    vsf_teda_t teda;
    vsf_sem_t sem;
    vsf_dlist_t urb_list;
};
typedef struct vsf_libusb_hcd_t vsf_libusb_hcd_t;

struct vsf_libusb_hcd_urb_t {
    vsf_dlist_node_t urb_node;

    enum {
        VSF_LIBUSB_HCD_URB_STATE_IDLE,
        VSF_LIBUSB_HCD_URB_STATE_QUEUED,
        VSF_LIBUSB_HCD_URB_STATE_TO_FREE,
    } state;

    vsf_arch_irq_thread_t irq_thread;
    vsf_arch_irq_request_t irq_request;
};
typedef struct vsf_libusb_hcd_urb_t vsf_libusb_hcd_urb_t;

enum vsf_libusb_hcd_evt_t {
    VSF_EVT_LIBUSB_HCD_ATTACH           = VSF_EVT_LIBUSB_HCD_BASE + 0x100,
    VSF_EVT_LIBUSB_HCD_DETACH           = VSF_EVT_LIBUSB_HCD_BASE + 0x200,
    VSF_EVT_LIBUSB_HCD_READY            = VSF_EVT_LIBUSB_HCD_BASE + 0x300,
};
typedef enum vsf_libusb_hcd_evt_t vsf_libusb_hcd_evt_t;

/*============================ PROTOTYPES ====================================*/

static vsf_err_t vsf_libusb_hcd_init_evthandler(vsf_eda_t *eda, vsf_evt_t evt, vsf_usbh_hcd_t *hcd);
static vsf_err_t vsf_libusb_hcd_fini(vsf_usbh_hcd_t *hcd);
static vsf_err_t vsf_libusb_hcd_suspend(vsf_usbh_hcd_t *hcd);
static vsf_err_t vsf_libusb_hcd_resume(vsf_usbh_hcd_t *hcd);
static vsf_err_t vsf_libusb_hcd_alloc_device(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_dev_t *dev);
static void vsf_libusb_hcd_free_device(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_dev_t *dev);
static vsf_usbh_hcd_urb_t * vsf_libusb_hcd_alloc_urb(vsf_usbh_hcd_t *hcd);
static void vsf_libusb_hcd_free_urb(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_urb_t *urb);
static vsf_err_t vsf_libusb_hcd_submit_urb(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_urb_t *urb);
static vsf_err_t vsf_libusb_hcd_relink_urb(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_urb_t *urb);
static vsf_err_t vsf_libusb_hcd_reset_dev(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_dev_t *dev);
static bool vsf_libusb_hcd_is_dev_reset(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_dev_t *dev);

static void __vsf_libusb_hcd_dev_thread(void *arg);

/*============================ GLOBAL VARIABLES ==============================*/

const vsf_usbh_hcd_drv_t vsf_libusb_hcd_drv = {
    .init_evthandler    = vsf_libusb_hcd_init_evthandler,
    .fini               = vsf_libusb_hcd_fini,
    .suspend            = vsf_libusb_hcd_suspend,
    .resume             = vsf_libusb_hcd_resume,
    .alloc_device       = vsf_libusb_hcd_alloc_device,
    .free_device        = vsf_libusb_hcd_free_device,
    .alloc_urb          = vsf_libusb_hcd_alloc_urb,
    .free_urb           = vsf_libusb_hcd_free_urb,
    .submit_urb         = vsf_libusb_hcd_submit_urb,
    .relink_urb         = vsf_libusb_hcd_relink_urb,
    .reset_dev          = vsf_libusb_hcd_reset_dev,
    .is_dev_reset       = vsf_libusb_hcd_is_dev_reset,
};

/*============================ LOCAL VARIABLES ===============================*/

static vsf_libusb_hcd_t __vsf_libusb_hcd = {
    .devs ={
        MREPEAT(VSF_LIBUSB_HCD_CFG_DEV_NUM, VSF_LIBUSB_HCD_DEF_DEV, NULL)
    },
};

/*============================ IMPLEMENTATION ================================*/

static void __vsf_libusb_hcd_on_left(vsf_libusb_hcd_dev_t *libusb_dev)
{
    libusb_dev->handle = NULL;
    libusb_dev->evt_mask.is_detached = true;
    __vsf_arch_irq_request_send(&libusb_dev->irq_request);
}

static void __vsf_libusb_hcd_on_arrived(vsf_libusb_hcd_dev_t *libusb_dev)
{
    libusb_device *device = libusb_get_device(libusb_dev->handle);
    switch (libusb_get_device_speed(device)) {
        case LIBUSB_SPEED_UNKNOWN:
            libusb_dev->speed = USB_SPEED_UNKNOWN;
            break;
        case LIBUSB_SPEED_LOW:
            libusb_dev->speed = USB_SPEED_LOW;
            break;
        case LIBUSB_SPEED_FULL:
            libusb_dev->speed = USB_SPEED_FULL;
            break;
        case LIBUSB_SPEED_HIGH:
            libusb_dev->speed = USB_SPEED_HIGH;
            break;
        case LIBUSB_SPEED_SUPER_PLUS:
            // not supported, use USB_SPEED_SUPER
        case LIBUSB_SPEED_SUPER:
            libusb_dev->speed = USB_SPEED_SUPER;
            break;
        }
        libusb_dev->evt_mask.is_attached = true;
        __vsf_arch_irq_request_send(&libusb_dev->irq_request);
}

static int LIBUSB_CALL __vsf_libusb_hcd_hotplug_cb(libusb_context *ctx, libusb_device *device,
    libusb_hotplug_event event, void *user_data)
{
    struct libusb_device_descriptor desc_device;
    vsf_libusb_hcd_dev_t *libusb_dev = NULL;

    if (LIBUSB_SUCCESS == libusb_get_device_descriptor(device, &desc_device)) {
        for (int i = 0; i < dimof(__vsf_libusb_hcd.devs); i++) {
            if (    (__vsf_libusb_hcd.devs[i].vid == desc_device.idVendor)
                &&  (__vsf_libusb_hcd.devs[i].pid == desc_device.idProduct)) {
                libusb_dev = &__vsf_libusb_hcd.devs[i];
                break;
            }
        }

        if (libusb_dev != NULL) {
            switch (event) {
            case LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED:
                if (NULL == libusb_dev->handle) {
                    if (LIBUSB_SUCCESS == libusb_open(device, &libusb_dev->handle)) {
                        __vsf_libusb_hcd_on_arrived(libusb_dev);
                    } else {
                        libusb_dev->handle = NULL;
                    }
                }
                break;
            case LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT:
                if (libusb_dev->handle != NULL) {
                    __vsf_libusb_hcd_on_left(libusb_dev);
                }
                break;
            }
        }
    }
    return 0;
}

// return 0 on success, non-0 otherwise
static int __vsf_libusb_hcd_init(void)
{
    vsf_libusb_hcd_param_t *param = __vsf_libusb_hcd.hcd->param;
    vsf_libusb_hcd_dev_t *libusb_dev;
    int err;

    err = libusb_init(&__vsf_libusb_hcd.ctx);
    if (err < 0) {
        vsf_trace(VSF_TRACE_ERROR,
            "fail to init libusb, errcode is %d\r\n", err);
        return err;
    }

    __vsf_libusb_hcd.is_hotplug_supported = libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG);

    for (int i = 0; i < dimof(__vsf_libusb_hcd.devs); i++) {
        libusb_dev = &__vsf_libusb_hcd.devs[i];
        libusb_dev->state = VSF_LIBUSB_HCD_DEV_STATE_DETACHED;
        libusb_dev->evt_mask.value = 0;
        libusb_dev->handle = NULL;
        libusb_dev->addr = -1;

        __vsf_arch_irq_request_init(&libusb_dev->irq_request);
        libusb_dev->irq_thread.name = "libusb_hcd_dev";
        __vsf_arch_irq_init(&libusb_dev->irq_thread, __vsf_libusb_hcd_dev_thread, param->priority, true);

        if (__vsf_libusb_hcd.is_hotplug_supported) {
            libusb_hotplug_register_callback(__vsf_libusb_hcd.ctx,
                LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                LIBUSB_HOTPLUG_ENUMERATE, libusb_dev->vid, libusb_dev->pid, LIBUSB_HOTPLUG_MATCH_ANY,
                __vsf_libusb_hcd_hotplug_cb, NULL, NULL);
        }
    }
    return 0;
}

// TODO: call libusb_claim_interface for non-control transfer
static int __vsf_libusb_hcd_submit_urb(vsf_usbh_hcd_urb_t *urb)
{
    vsf_usbh_hcd_dev_t *dev = urb->dev_hcd;
    vsf_libusb_hcd_dev_t *libusb_dev = dev->dev_priv;
    vsf_usbh_eppipe_t pipe = urb->pipe;

    if (NULL == libusb_dev->handle) {
        return LIBUSB_ERROR_NO_DEVICE;
    }

    switch (pipe.type) {
    case USB_ENDPOINT_XFER_CONTROL:
        if (pipe.endpoint != 0) {
            return LIBUSB_ERROR_INVALID_PARAM;
        }

        {
            struct usb_ctrlrequest_t *setup = &urb->setup_packet;

            return libusb_control_transfer(libusb_dev->handle, setup->bRequestType,
                        setup->bRequest, setup->wValue, setup->wIndex, urb->buffer,
                        setup->wLength, urb->timeout);
        }
    case USB_ENDPOINT_XFER_ISOC:
        // TODO: add support to iso transfer
        return LIBUSB_ERROR_NOT_SUPPORTED;
    case USB_ENDPOINT_XFER_BULK:
        {
            int actual_length;
            int err = libusb_bulk_transfer(libusb_dev->handle,
                        (pipe.dir_in1out0 ? 0x80 : 0x00) | pipe.endpoint,
                        urb->buffer, urb->transfer_length, &actual_length, urb->timeout);
            if (err < 0) {
                return err;
            } else {
                return actual_length;
            }
        }
    case USB_ENDPOINT_XFER_INT:
        {
            int actual_length;
            int err = libusb_interrupt_transfer(libusb_dev->handle,
                        (pipe.dir_in1out0 ? 0x80 : 0x00) | pipe.endpoint,
                        urb->buffer, urb->transfer_length, &actual_length, urb->timeout);
            if (err < 0) {
                return err;
            } else {
                return actual_length;
            }
        }
        break;
    }
    return LIBUSB_ERROR_INVALID_PARAM;
}

static void __vsf_libusb_hcd_dev_thread(void *arg)
{
    vsf_arch_irq_thread_t *irq_thread = arg;
    vsf_libusb_hcd_dev_t *libusb_dev = container_of(irq_thread, vsf_libusb_hcd_dev_t, irq_thread);
    vsf_arch_irq_request_t *irq_request = &libusb_dev->irq_request;
    int idx = libusb_dev - &__vsf_libusb_hcd.devs[0];

    __vsf_arch_irq_set_background(irq_thread);
    while (1) {
        __vsf_arch_irq_request_pend(irq_request);

        if (libusb_dev->evt_mask.is_attached) {
            libusb_dev->evt_mask.is_attached = false;
            __vsf_arch_irq_start(irq_thread);
                vsf_eda_post_evt(&__vsf_libusb_hcd.teda.use_as__vsf_eda_t, VSF_EVT_LIBUSB_HCD_ATTACH | idx);
            __vsf_arch_irq_end(irq_thread, false);
        }
        if (libusb_dev->evt_mask.is_detached) {
            libusb_dev->evt_mask.is_detached = false;
            __vsf_arch_irq_start(irq_thread);
                vsf_eda_post_evt(&__vsf_libusb_hcd.teda.use_as__vsf_eda_t, VSF_EVT_LIBUSB_HCD_DETACH | idx);
            __vsf_arch_irq_end(irq_thread, false);
        }
        if (libusb_dev->evt_mask.is_resetting) {
            libusb_reset_device(libusb_dev->handle);
            libusb_dev->evt_mask.is_resetting = false;
        }
    }
}

static void __vsf_libusb_hcd_urb_thread(void *arg)
{
    vsf_arch_irq_thread_t *irq_thread = arg;
    vsf_libusb_hcd_urb_t *libusb_urb = container_of(irq_thread, vsf_libusb_hcd_urb_t, irq_thread);
    vsf_usbh_hcd_urb_t *urb = container_of(libusb_urb, vsf_usbh_hcd_urb_t, priv);
    vsf_arch_irq_request_t *irq_request = &libusb_urb->irq_request;
    bool is_to_free;
    int actual_length;

    __vsf_arch_irq_set_background(irq_thread);
    while (1) {
        __vsf_arch_irq_request_pend(irq_request);

            is_to_free = VSF_LIBUSB_HCD_URB_STATE_TO_FREE == libusb_urb->state;
            if (!is_to_free) {
                vsf_usbh_hcd_dev_t *dev = urb->dev_hcd;
                vsf_libusb_hcd_dev_t *libusb_dev = dev->dev_priv;

                actual_length = __vsf_libusb_hcd_submit_urb(urb);
                if (actual_length < 0) {
                    urb->status = actual_length;
                } else {
                    urb->status = URB_OK;
                    urb->actual_length = actual_length;

                    if (USB_ENDPOINT_XFER_CONTROL == urb->pipe.type) {
                        struct usb_ctrlrequest_t *setup = &urb->setup_packet;

                        // set configuration is handled here
                        if (    ((USB_RECIP_DEVICE | USB_DIR_OUT) == setup->bRequestType)
                            &&  (USB_REQ_SET_CONFIGURATION == setup->bRequest)) {

                            int config = setup->wValue;
                            struct libusb_config_descriptor *config_desc;

                            if (LIBUSB_SUCCESS == libusb_get_config_descriptor_by_value(
                                        libusb_get_device(libusb_dev->handle), config, &config_desc)) {
                                for (uint8_t i = 0; i < config_desc->bNumInterfaces; i++) {
                                    libusb_claim_interface(libusb_dev->handle, i);
                                }
                                libusb_free_config_descriptor(config_desc);
                            }
                        }
                    }
                }
                if (urb->status == LIBUSB_ERROR_IO) {
                    __vsf_libusb_hcd_on_left(libusb_dev);
                }
            }

        __vsf_arch_irq_start(irq_thread);
            vsf_eda_post_msg(&__vsf_libusb_hcd.teda.use_as__vsf_eda_t, urb);
        __vsf_arch_irq_end(irq_thread, false);

        if (is_to_free) {
            __vsf_arch_irq_fini(irq_thread);
            __vsf_arch_irq_request_fini(irq_request);
            return;
        }
    }
}

static void __vsf_libusb_hcd_init_thread(void *arg)
{
    vsf_arch_irq_thread_t *irq_thread = arg;

    __vsf_arch_irq_set_background(irq_thread);
        __vsf_libusb_hcd_init();
    __vsf_arch_irq_start(irq_thread);
        vsf_eda_post_evt(__vsf_libusb_hcd.init_eda, VSF_EVT_LIBUSB_HCD_READY);
    __vsf_arch_irq_end(irq_thread, false);

    while (!__vsf_libusb_hcd.is_hotplug_supported) {
        vsf_libusb_hcd_dev_t *libusb_dev = &__vsf_libusb_hcd.devs[0];
        for (int i = 0; i < dimof(__vsf_libusb_hcd.devs); i++, libusb_dev++) {
            if (NULL == libusb_dev->handle) {
                libusb_dev->handle = libusb_open_device_with_vid_pid(
                    __vsf_libusb_hcd.ctx, libusb_dev->vid, libusb_dev->pid);
                if (libusb_dev->handle != NULL) {
                    __vsf_libusb_hcd_on_arrived(libusb_dev);
                }
            }
        }
        Sleep(100);
    }
    __vsf_arch_irq_fini(irq_thread);
}






static void vsf_libusb_hcd_free_urb_do(vsf_usbh_hcd_urb_t *urb)
{
    VSF_USBH_FREE(urb);
}

static void vsf_libusb_hcd_evthandler(vsf_eda_t *eda, vsf_evt_t evt)
{
    vsf_libusb_hcd_t *libusb = container_of(eda, vsf_libusb_hcd_t, teda);

    switch (evt) {
    case VSF_EVT_INIT:
        vsf_dlist_init(&__vsf_libusb_hcd.urb_list);
        vsf_eda_sem_init(&__vsf_libusb_hcd.sem, 0);
        vsf_teda_set_timer_ms(100);

    wait_next_urb:
        if (vsf_eda_sem_pend(&__vsf_libusb_hcd.sem, -1)) {
            break;
        }
        // fall through
    case VSF_EVT_SYNC:
        {
            vsf_libusb_hcd_urb_t *libusb_urb;
            vsf_protect_t orig = vsf_protect_sched();
                vsf_dlist_remove_head(vsf_libusb_hcd_urb_t, urb_node,
                        &__vsf_libusb_hcd.urb_list, libusb_urb);
            vsf_unprotect_sched(orig);

            if (libusb_urb != NULL) {
                vsf_usbh_hcd_urb_t *urb = container_of(libusb_urb, vsf_usbh_hcd_urb_t, priv);

                if (VSF_LIBUSB_HCD_URB_STATE_TO_FREE == libusb_urb->state) {
                    vsf_libusb_hcd_free_urb_do(urb);
                } else {
                    if (USB_ENDPOINT_XFER_CONTROL == urb->pipe.type) {
                        struct usb_ctrlrequest_t *setup = &urb->setup_packet;

                        // set address is handled here
                        if (    ((USB_RECIP_DEVICE | USB_DIR_OUT) == setup->bRequestType)
                            &&  (USB_REQ_SET_ADDRESS == setup->bRequest)) {

                            vsf_usbh_hcd_dev_t *dev = urb->dev_hcd;
                            vsf_libusb_hcd_dev_t *libusb_dev = dev->dev_priv;

                            VSF_USB_ASSERT(0 == libusb_dev->addr);
                            libusb_dev->addr = setup->wValue;
                            urb->status = URB_OK;
                            urb->actual_length = 0;
                            vsf_eda_post_msg(&__vsf_libusb_hcd.teda.use_as__vsf_eda_t, urb);
                            goto wait_next_urb;
                        }
                    }

                    // irq_thread will process the urb
                    __vsf_arch_irq_request_send(&libusb_urb->irq_request);
                }
            }
            goto wait_next_urb;
        }
        break;
    case VSF_EVT_TIMER:
        if (__vsf_libusb_hcd.new_mask != 0) {
            vsf_usbh_t *usbh = (vsf_usbh_t *)libusb->hcd;
            if (NULL == usbh->dev_new) {
                int idx = ffz(~__vsf_libusb_hcd.new_mask);
                VSF_USB_ASSERT(idx < dimof(__vsf_libusb_hcd.devs));
                vsf_libusb_hcd_dev_t *libusb_dev = &__vsf_libusb_hcd.devs[idx];
                __vsf_libusb_hcd.cur_dev_idx = idx;
                __vsf_libusb_hcd.new_mask &= ~(1 << idx);
                libusb_dev->addr = 0;
                libusb_dev->dev = vsf_usbh_new_device((vsf_usbh_t *)libusb->hcd, libusb_dev->speed, NULL, 0);
            }
        }
        vsf_teda_set_timer_ms(100);
        break;
    case VSF_EVT_MESSAGE:
        {
            vsf_usbh_hcd_urb_t *urb = vsf_eda_get_cur_msg();
            vsf_libusb_hcd_urb_t *libusb_urb = (vsf_libusb_hcd_urb_t *)urb->priv;

            if (VSF_LIBUSB_HCD_URB_STATE_TO_FREE == libusb_urb->state) {
                vsf_libusb_hcd_free_urb_do(urb);
            } else {
                vsf_eda_post_msg(urb->eda_caller, urb);
            }
        }
        break;
    default:
        {
            int idx = evt & 0xFF;
            VSF_USB_ASSERT(idx < dimof(__vsf_libusb_hcd.devs));
            vsf_libusb_hcd_dev_t *libusb_dev = &__vsf_libusb_hcd.devs[idx];

            switch (evt & ~0xFF) {
            case VSF_EVT_LIBUSB_HCD_ATTACH:
                __vsf_libusb_hcd.new_mask |= 1 << idx;
                break;
            case VSF_EVT_LIBUSB_HCD_DETACH:
                if (libusb_dev->state != VSF_LIBUSB_HCD_DEV_STATE_DETACHED) {
                    libusb_dev->state = VSF_LIBUSB_HCD_DEV_STATE_DETACHED;
                    vsf_usbh_disconnect_device((vsf_usbh_t *)libusb->hcd, libusb_dev->dev);
                }
                break;
            }
        }   
    }
}

static vsf_err_t vsf_libusb_hcd_init_evthandler(vsf_eda_t *eda, vsf_evt_t evt, vsf_usbh_hcd_t *hcd)
{
    vsf_libusb_hcd_param_t *param = hcd->param;

    switch (evt) {
    case VSF_EVT_INIT:
        __vsf_libusb_hcd.hcd = hcd;
        __vsf_libusb_hcd.new_mask = 0;
        __vsf_libusb_hcd.cur_dev_idx = 0;

        vsf_eda_set_evthandler(&__vsf_libusb_hcd.teda.use_as__vsf_eda_t, vsf_libusb_hcd_evthandler);
        vsf_teda_init(&__vsf_libusb_hcd.teda, vsf_prio_inherit, false);

        __vsf_libusb_hcd.init_eda = eda;
        __vsf_libusb_hcd.init_thread.name = "libusb_hcd_init";
        __vsf_arch_irq_init(&__vsf_libusb_hcd.init_thread, __vsf_libusb_hcd_init_thread, param->priority, true);
        break;
    case VSF_EVT_LIBUSB_HCD_READY:
        return VSF_ERR_NONE;
    }
    return VSF_ERR_NOT_READY;
}

static vsf_err_t vsf_libusb_hcd_fini(vsf_usbh_hcd_t *hcd)
{
    return VSF_ERR_NONE;
}

static vsf_err_t vsf_libusb_hcd_suspend(vsf_usbh_hcd_t *hcd)
{
    return VSF_ERR_NONE;
}

static vsf_err_t vsf_libusb_hcd_resume(vsf_usbh_hcd_t *hcd)
{
    return VSF_ERR_NONE;
}

static vsf_err_t vsf_libusb_hcd_alloc_device(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_dev_t *dev)
{
    VSF_USB_ASSERT(__vsf_libusb_hcd.cur_dev_idx < dimof(__vsf_libusb_hcd.devs));
    dev->dev_priv = &__vsf_libusb_hcd.devs[__vsf_libusb_hcd.cur_dev_idx];
    return VSF_ERR_NONE;
}

static void vsf_libusb_hcd_free_device(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_dev_t *dev)
{
    dev->dev_priv = NULL;
}

static vsf_usbh_hcd_urb_t * vsf_libusb_hcd_alloc_urb(vsf_usbh_hcd_t *hcd)
{
    uint_fast32_t size = sizeof(vsf_usbh_hcd_urb_t) + sizeof(vsf_libusb_hcd_urb_t);
    vsf_usbh_hcd_urb_t *urb = VSF_USBH_MALLOC(size);

    if (urb != NULL) {
        memset(urb, 0, size);

        vsf_libusb_hcd_urb_t *libusb_urb = (vsf_libusb_hcd_urb_t *)urb->priv;
        vsf_libusb_hcd_param_t *param = __vsf_libusb_hcd.hcd->param;
        __vsf_arch_irq_request_init(&libusb_urb->irq_request);
        libusb_urb->irq_thread.name = "libusb_hcd_urb";
        __vsf_arch_irq_init(&libusb_urb->irq_thread, __vsf_libusb_hcd_urb_thread, param->priority, true);
    }
    return urb;
}

static void vsf_libusb_hcd_free_urb(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_urb_t *urb)
{
    vsf_libusb_hcd_urb_t *libusb_urb = (vsf_libusb_hcd_urb_t *)urb->priv;
    libusb_urb->state = VSF_LIBUSB_HCD_URB_STATE_TO_FREE;
    __vsf_arch_irq_request_send(&libusb_urb->irq_request);
}

static vsf_err_t vsf_libusb_hcd_submit_urb(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_urb_t *urb)
{
    vsf_libusb_hcd_urb_t *libusb_urb = (vsf_libusb_hcd_urb_t *)urb->priv;
    vsf_dlist_init_node(vsf_libusb_hcd_urb_t, urb_node, libusb_urb);
    vsf_protect_t orig = vsf_protect_sched();
        vsf_dlist_add_to_tail(vsf_libusb_hcd_urb_t, urb_node, &__vsf_libusb_hcd.urb_list, libusb_urb);
        libusb_urb->state = VSF_LIBUSB_HCD_URB_STATE_QUEUED;
    vsf_unprotect_sched(orig);
    vsf_eda_sem_post(&__vsf_libusb_hcd.sem);
    return VSF_ERR_NONE;
}

static vsf_err_t vsf_libusb_hcd_relink_urb(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_urb_t *urb)
{
    return vsf_libusb_hcd_submit_urb(hcd, urb);
}

static vsf_err_t vsf_libusb_hcd_reset_dev(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_dev_t *dev)
{
    vsf_libusb_hcd_dev_t *libusb_dev = (vsf_libusb_hcd_dev_t *)dev->dev_priv;
    libusb_dev->evt_mask.is_resetting = true;
    __vsf_arch_irq_request_send(&libusb_dev->irq_request);
    return VSF_ERR_NONE;
}

static bool vsf_libusb_hcd_is_dev_reset(vsf_usbh_hcd_t *hcd, vsf_usbh_hcd_dev_t *dev)
{
    vsf_libusb_hcd_dev_t *libusb_dev = (vsf_libusb_hcd_dev_t *)dev->dev_priv;
    return libusb_dev->evt_mask.is_resetting;
}

#endif
