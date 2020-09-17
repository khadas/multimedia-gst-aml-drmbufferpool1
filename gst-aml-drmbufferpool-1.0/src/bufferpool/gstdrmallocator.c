/*
 * gstdrmallocator.c
 *
 *  Created on: 2020年8月28日
 *      Author: tao
 */


#include "gstdrmallocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <meson_drm.h>

GST_DEBUG_CATEGORY_STATIC (gst_drm_allocator_debug);
#define GST_CAT_DEFAULT gst_drm_allocator_debug

#define gst_drm_allocator_parent_class parent_class
G_DEFINE_TYPE (GstDRMAllocator, gst_drm_allocator, GST_TYPE_DMABUF_ALLOCATOR);

GstMemory *
gst_drm_mem_alloc (GstAllocator * allocator, gsize size,
    GstAllocationParams * params)
{
    GstDRMAllocator *self = GST_DRM_ALLOCATOR (allocator);
    GstMemory *mem = NULL;
    struct drm_mode_create_dumb creq;
    struct drm_mode_destroy_dumb dreq;
    int fd;
    int ret;

    g_return_val_if_fail (GST_IS_DRM_ALLOCATOR (allocator), NULL);
    g_return_val_if_fail(self->dev_fd >=0 , NULL);

    memset(&creq, 0, sizeof(struct drm_mode_create_dumb));
    creq.width = size;
    creq.height = 1;
    creq.bpp = 8;
    if (params->flags & GST_MEMORY_FLAG_FMT_AFBC)
        creq.flags = MESON_USE_VIDEO_AFBC;
    else
        creq.flags = MESON_USE_VIDEO_PLANE;

    if (params->flags & GST_MEMORY_FLAG_SECURE)
        creq.flags |= MESON_USE_PROTECTED;

    ret = drmIoctl(self->dev_fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
    if (ret < 0) {
        GST_ERROR_OBJECT(self, "Create DRM dumb buffer failed");
        return NULL;
    }

    drmPrimeHandleToFD (self->dev_fd, creq.handle, DRM_CLOEXEC | O_RDWR, &fd);

    if (fd < 0) {
        GST_ERROR_OBJECT(self, "Invalid fd returned: %d", fd);
        goto error_get;
    }

    mem = gst_fd_allocator_alloc(allocator, fd, size, GST_FD_MEMORY_FLAG_NONE);
    if (!mem) {
        GST_ERROR("GstDmaBufMemory failed");
        goto error_alloc;
    }

    GST_LOG("alloc dma %d", fd);

    return mem;

error_alloc:
    close(fd);
error_get:
    memset(&dreq, 0, sizeof(struct drm_mode_destroy_dumb));
    dreq.handle = creq.handle;
    drmIoctl(self->dev_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    return NULL;
}

void
gst_drm_mem_free(GstAllocator *allocator, GstMemory *memory)
{
    GstDRMAllocator *self = GST_DRM_ALLOCATOR (allocator);
    uint32_t handle = 0;
    int fd;

    g_return_if_fail (GST_IS_ALLOCATOR (allocator));
    g_return_if_fail (memory != NULL);
    g_return_if_fail (gst_is_drm_memory (memory));
    g_return_if_fail(self->dev_fd >= 0);

    fd = gst_fd_memory_get_fd(memory);

    GST_LOG("free dma %d", fd);
    drmPrimeFDToHandle(self->dev_fd, fd, &handle);

    GST_ALLOCATOR_CLASS (parent_class)->free(allocator, memory);
    if (handle) {
        struct drm_mode_destroy_dumb dreq;
        memset(&dreq, 0, sizeof(struct drm_mode_destroy_dumb));
        dreq.handle = handle;
        drmIoctl(self->dev_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    }
}

void gst_drm_allocator_finalize(GObject *object)
{
    GstDRMAllocator *self = GST_DRM_ALLOCATOR(object);
    close(self->dev_fd);
    self->dev_fd = -1;
    G_OBJECT_CLASS(parent_class)->finalize(object);
}


static void
gst_drm_allocator_class_init (GstDRMAllocatorClass * klass)
{
    GObjectClass *gobject_class = (GObjectClass *) klass;
    GstAllocatorClass *alloc_class = (GstAllocatorClass *) klass;

    gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_drm_allocator_finalize);
    alloc_class->alloc = GST_DEBUG_FUNCPTR (gst_drm_mem_alloc);
    alloc_class->free = GST_DEBUG_FUNCPTR (gst_drm_mem_free);

    GST_DEBUG_CATEGORY_INIT(gst_drm_allocator_debug, "drmallocator", 0, "DRM Allocator");
}


static void
gst_drm_allocator_init (GstDRMAllocator * self)
{
    GstAllocator *allocator = GST_ALLOCATOR_CAST(self);
    self->dev_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (self->dev_fd < 0) {
        GST_ERROR_OBJECT(self, "Failed to open DRM device");
    } else {
        drmDropMaster(self->dev_fd);
    }

    allocator->mem_type = GST_ALLOCATOR_DRM;

    GST_OBJECT_FLAG_UNSET(self, GST_ALLOCATOR_FLAG_CUSTOM_ALLOC);
}

void
gst_drm_allocator_register (void)
{
  gst_allocator_register (GST_ALLOCATOR_DRM,
      g_object_new (GST_TYPE_DRM_ALLOCATOR, NULL));
}

GstAllocator *
gst_drm_allocator_get (void)
{
  GstAllocator *alloc;
  alloc = gst_allocator_find (GST_ALLOCATOR_DRM);
  if (!alloc) {
    gst_drm_allocator_register();
    alloc = gst_allocator_find (GST_ALLOCATOR_DRM);
  }
  return alloc;
}

gboolean
gst_is_drm_memory (GstMemory *mem)
{
    return gst_memory_is_type(mem, GST_ALLOCATOR_DRM);
}

