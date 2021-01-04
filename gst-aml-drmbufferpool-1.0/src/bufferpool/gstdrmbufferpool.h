/*
 * gstdrmbufferpool.h
 *
 *  Created on: 2020年8月28日
 *      Author: tao
 */

#ifndef _GST_DRM_BUFFERPOOL_H_
#define _GST_DRM_BUFFERPOOL_H_

#include <gst/video/video.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>

G_BEGIN_DECLS

#define GST_TYPE_DRM_BUFFER_POOL \
      (gst_drm_bufferpool_get_type())
#define GST_DRM_BUFFER_POOL(obj) \
      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_DRM_BUFFER_POOL, GstDRMBufferPool))
#define GST_DRM_BUFFER_POOL_CAST(obj) \
      ((GstDRMBufferPool*)(obj))

typedef struct _GstDRMBufferPoolClass GstDRMBufferPoolClass;
typedef struct _GstDRMBufferPool GstDRMBufferPool;

typedef enum {
    GST_DRM_BUFFERPOOL_TYPE_AFBC,
    GST_DRM_BUFFERPOOL_TYPE_VIDEO_PLANE,
    GST_DRM_BUFFERPOOL_TYPE_ION
} GstBufpoolFlags;


struct _GstDRMBufferPool
{
    GstVideoBufferPool          parent;
    GstAllocator                *allocator;
    GstVideoInfo                vinfo;
    GstCaps                     *caps;
    gboolean                    add_videometa;
    GstBufpoolFlags             flag;
    gboolean                    secure;
};

struct _GstDRMBufferPoolClass
{
    GstVideoBufferPoolClass parent_class;
};



GType gst_drm_bufferpool_get_type (void);

GstBufferPool *gst_drm_bufferpool_new (gboolean secure, GstBufpoolFlags flag);

G_END_DECLS



#endif /* _GST_DRM_BUFFERPOOL_H_ */
