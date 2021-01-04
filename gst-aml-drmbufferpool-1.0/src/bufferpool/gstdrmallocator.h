/*
 * gstdrmallocator.h
 *
 *  Created on: 2020年8月28日
 *      Author: tao
 */

#ifndef _GST_DRMALLOCATOR_H_
#define _GST_DRMALLOCATOR_H_

#include <stdint.h>
#include <gst/gst.h>
#include <gst/allocators/allocators.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <fcntl.h>

G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GST_TYPE_DRM_ALLOCATOR \
  (gst_drm_allocator_get_type())
#define GST_DRM_ALLOCATOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_DRM_ALLOCATOR,GstDRMAllocator))
#define GST_DRM_ALLOCATOR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_DRM_ALLOCATOR,GstDRMAllocatorClass))
#define GST_IS_DRM_ALLOCATOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_DRM_ALLOCATOR))
#define GST_IS_DRM_ALLOCATOR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_DRM_ALLOCATOR))

typedef struct _GstDRMAllocator      GstDRMAllocator;
typedef struct _GstDRMAllocatorClass GstDRMAllocatorClass;

#define GST_ALLOCATOR_DRM "drm"

enum
{
    GST_MEMORY_FLAG_FMT_AFBC        = GST_MEMORY_FLAG_LAST << 0,
    GST_MEMORY_FLAG_SECURE          = GST_MEMORY_FLAG_LAST << 1,
    GST_MEMORY_FLAG_VIDEO_PLANE     = GST_MEMORY_FLAG_LAST << 2
};

struct _GstDRMAllocator
{
    GstDmaBufAllocator              parent;
    int                             dev_fd;
};

struct _GstDRMAllocatorClass
{
    GstDmaBufAllocatorClass parent_class;
};

GType           gst_drm_allocator_get_type (void);
GstAllocator *  gst_drm_allocator_get (void);
GstAllocator *  gst_drm_allocator_new (void);
gboolean        gst_is_drm_memory (GstMemory *mem);

G_END_DECLS

#endif /* _GST_DRMALLOCATOR_H_ */
