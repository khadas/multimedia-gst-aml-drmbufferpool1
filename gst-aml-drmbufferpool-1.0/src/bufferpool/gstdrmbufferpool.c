/*
 * gstdrmbufferpool.c
 *
 *  Created on: 2020年8月28日
 *      Author: tao
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstdrmbufferpool.h"
#include "gstdrmallocator.h"

#define ALIGN_PAD(x, y)    (((x) + ((y) - 1)) & (~((y) - 1)))


GST_DEBUG_CATEGORY_STATIC (gst_drm_bufferpool_debug);
#define GST_CAT_DEFAULT gst_drm_bufferpool_debug

#define gst_drm_bufferpool_parent_class parent_class
G_DEFINE_TYPE (GstDRMBufferPool, gst_drm_bufferpool, GST_TYPE_VIDEO_BUFFER_POOL);

static const gchar **
gst_drm_bufferpool_get_options (GstBufferPool * pool)
{
    static const gchar *options[] = {
        GST_BUFFER_POOL_OPTION_VIDEO_META,
        GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT,
        NULL};
    return options;
}

static gboolean
gst_drm_bufferpool_set_config(GstBufferPool *pool, GstStructure *config)
{
    GstDRMBufferPool *self = GST_DRM_BUFFER_POOL_CAST(pool);
    GstCaps *caps = NULL;
    GstVideoAlignment align;

     if (!gst_buffer_pool_config_get_params(config, &caps, NULL, NULL, NULL))
        goto wrong_config;
    if (caps == NULL)
        goto no_caps;

    if (!gst_video_info_from_caps(&self->vinfo, caps))
        goto wrong_caps;
    gst_caps_replace(&self->caps, caps);
    gst_buffer_pool_config_get_video_alignment(config, &align);
    gst_video_info_align(&self->vinfo, &align);
    self->add_videometa = gst_buffer_pool_config_has_option (config,
          GST_BUFFER_POOL_OPTION_VIDEO_META);
    if (self->allocator)
        gst_object_unref(self->allocator);
    self->allocator = gst_drm_allocator_get();

    return GST_BUFFER_POOL_CLASS (parent_class)->set_config (pool, config);

/* ERRORS */
wrong_config:
    {
        GST_WARNING_OBJECT(pool, "invalid config");
        return FALSE;
    }
no_caps:
    {
        GST_WARNING_OBJECT(pool, "no caps in config");
        return FALSE;
    }
wrong_caps:
    {
        GST_WARNING_OBJECT(pool,
                "failed getting geometry from caps %" GST_PTR_FORMAT, caps);
        return FALSE;
    }
}

static GstFlowReturn
gst_drm_bufferpool_alloc_buffer(GstBufferPool *pool, GstBuffer **buffer,
        GstBufferPoolAcquireParams *params)
{
    GstDRMBufferPool *self = GST_DRM_BUFFER_POOL_CAST(pool);
    GstBuffer *buf;
    GstVideoFormat format;
    guint i;


    format = GST_VIDEO_INFO_FORMAT(&self->vinfo);
    if (format != GST_VIDEO_FORMAT_NV12
            && format != GST_VIDEO_FORMAT_NV21)
        goto not_support;

    if (!(buf = gst_buffer_new())) {
        goto no_buffer;
    }

    for (i = 0; i < GST_VIDEO_INFO_N_PLANES(&self->vinfo); i++) {
        GstMemory *mem;
        GstAllocationParams params2 = { 0 };
        gsize size;

        switch (self->flag) {
        case GST_DRM_BUFFERPOOL_TYPE_AFBC :
            params2.flags |= GST_MEMORY_FLAG_FMT_AFBC;
            break;
        case GST_DRM_BUFFERPOOL_TYPE_VIDEO_PLANE :
            params2.flags |= GST_MEMORY_FLAG_VIDEO_PLANE;
            break;
        }

        if (self->secure)
            params2.flags |= GST_MEMORY_FLAG_SECURE;

        if (i + 1 < GST_VIDEO_INFO_N_PLANES(&self->vinfo)) {
            size = self->vinfo.offset[i + 1] - self->vinfo.offset[i];
        } else {
            size = self->vinfo.size - self->vinfo.offset[i];
        }

        mem = gst_allocator_alloc(self->allocator, size, &params2);

        GST_LOG_OBJECT(pool, "alloc video info size is %lu", size);
        if (!mem) {
            gst_buffer_unref(buf);
            goto mem_create_failed;
        }

        gst_buffer_append_memory(buf, mem);
    }
    *buffer = buf;

    if (self->add_videometa) {
        gst_buffer_add_video_meta_full(*buffer,
                GST_VIDEO_FRAME_FLAG_NONE,
                GST_VIDEO_INFO_FORMAT(&self->vinfo),
                GST_VIDEO_INFO_WIDTH(&self->vinfo),
                GST_VIDEO_INFO_HEIGHT(&self->vinfo),
                GST_VIDEO_INFO_N_PLANES(&self->vinfo),
                self->vinfo.offset,
                self->vinfo.stride);
    }
    return GST_FLOW_OK;

/* ERROR */
not_support:
    {
        GST_ERROR_OBJECT(pool, "only support NV12");
        return GST_FLOW_ERROR;
    }
no_buffer:
    {
        GST_ERROR_OBJECT(pool, "can't create buffer");
        return GST_FLOW_ERROR;
    }
mem_create_failed:
    {
        GST_ERROR_OBJECT(pool, "Could not create drm Memory");
        return GST_FLOW_ERROR;
    }
}

static void
gst_drm_bufferpool_finalize(GObject *object)
{
    GstDRMBufferPool *self = GST_DRM_BUFFER_POOL_CAST(object);
    if (self->caps)
        gst_caps_unref (self->caps);
    if (self->allocator)
        gst_object_unref(self->allocator);

    G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_drm_bufferpool_class_init (GstDRMBufferPoolClass * klass)
{
    GObjectClass *gobject_class = (GObjectClass*) klass;
    GstBufferPoolClass *pool_class = GST_BUFFER_POOL_CLASS(klass);

    gobject_class->finalize = GST_DEBUG_FUNCPTR(gst_drm_bufferpool_finalize);;
    pool_class->get_options = GST_DEBUG_FUNCPTR(gst_drm_bufferpool_get_options);
    pool_class->set_config = GST_DEBUG_FUNCPTR(gst_drm_bufferpool_set_config);
    pool_class->alloc_buffer = GST_DEBUG_FUNCPTR(gst_drm_bufferpool_alloc_buffer);

    GST_DEBUG_CATEGORY_INIT(gst_drm_bufferpool_debug, "drmbufferpool", 0, "DRM BufferPool");
}

static void
gst_drm_bufferpool_init(GstDRMBufferPool *pool)
{
}

GstBufferPool *
gst_drm_bufferpool_new (gboolean secure, GstBufpoolFlags flag)
{
    GstDRMBufferPool *pool;

    pool = g_object_new(GST_TYPE_DRM_BUFFER_POOL, NULL);
    g_object_ref_sink(pool);
    pool->secure = secure;
    pool->flag = flag;

    return GST_BUFFER_POOL_CAST(pool);
}

