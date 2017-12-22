/*************************************************************************
> File Name: ngx_http_gearman_module.c
> Author: yy
> Mail: mengyy_linux@163.com
 ************************************************************************/

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <cJSON.h>
#include <stdlib.h>

typedef struct
{
    ngx_http_request_t              *request;
    ngx_str_t                       function_name;
    ngx_http_status_t               status;
}ngx_http_gearman_context_t;

typedef struct
{
    ngx_http_upstream_conf_t        upstream;
    ngx_str_t                       url;
}ngx_http_gearman_loc_conf_t;

static ngx_int_t ngx_http_gearman_input_filter_init(void *data);
static ngx_int_t ngx_http_gearman_input_filter(void *data, ssize_t bytes);
static void* ngx_http_gearman_create_loc_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_gearman_handler(ngx_http_request_t *r);
static void ngx_http_gearman_abort_request(ngx_http_request_t *r);
static ngx_int_t ngx_http_gearman_reinit_request(ngx_http_request_t *r);
static ngx_int_t ngx_http_gearman_create_request(ngx_http_request_t *r);
static ngx_int_t ngx_http_gearman_process_header(ngx_http_request_t *r);
static void ngx_http_gearman_finalize_request(ngx_http_request_t *r, ngx_int_t rc);
static char* ngx_http_gearman_pass(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char* ngx_http_gearman_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static ngx_int_t ngx_http_gearman_init_upstream(ngx_http_request_t *r);
static ngx_int_t ngx_http_gearman_get_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_gearman_post_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_gearman_parse_client_body(ngx_http_request_t *r);
static ngx_int_t ngx_http_gearman_process_status_line(ngx_http_request_t *r);

static ngx_str_t  ngx_http_gearman_hide_headers[] = {
    ngx_string("Date"),
    ngx_string("Server"),
    ngx_string("X-Pad"),
    ngx_string("X-Accel-Expires"),
    ngx_string("X-Accel-Redirect"),
    ngx_string("X-Accel-Limit-Rate"),
    ngx_string("X-Accel-Buffering"),
    ngx_string("X-Accel-Charset"),
    ngx_null_string
};

static ngx_command_t ngx_http_gearman_commands[] = {
    {
        ngx_string("gearman_pass"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
        ngx_http_gearman_pass,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },

    {
        ngx_string("gearman_server"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_gearman_loc_conf_t, url),
        NULL
    },

    ngx_null_command,
};

static ngx_http_module_t ngx_http_gearman_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_gearman_create_loc_conf,      /* create location configuration */
    ngx_http_gearman_merge_loc_conf,       /* merge location configuration */
};

ngx_module_t ngx_http_gearman_module = {
    NGX_MODULE_V1,
    &ngx_http_gearman_module_ctx,           /* module context */
    ngx_http_gearman_commands,              /* module directives */
    NGX_HTTP_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};

static void* ngx_http_gearman_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_gearman_loc_conf_t *glcf;

    glcf = ngx_pcalloc(cf->pool, sizeof(*glcf));
    if (glcf == NULL)
    {
        return NULL;
    }

    glcf->upstream.connect_timeout = NGX_CONF_UNSET_MSEC;
    glcf->upstream.send_timeout = NGX_CONF_UNSET_MSEC;
    glcf->upstream.read_timeout = NGX_CONF_UNSET_MSEC;
    glcf->upstream.store_access = NGX_CONF_UNSET_UINT;

    glcf->upstream.buffer_size = NGX_CONF_UNSET_SIZE;

    /* TODO */
    glcf->upstream.cyclic_temp_file = 0;
    glcf->upstream.buffering = 0;
    glcf->upstream.ignore_client_abort = 0;
    glcf->upstream.send_lowat = 0;
    glcf->upstream.bufs.num = 8;
    glcf->upstream.bufs.size = ngx_pagesize;
    glcf->upstream.busy_buffers_size = 2 * ngx_pagesize;
    glcf->upstream.max_temp_file_size = 1024 * 1024 * 1024;
    glcf->upstream.temp_file_write_size = 2 * ngx_pagesize;
    glcf->upstream.intercept_errors = 1;
    glcf->upstream.intercept_404 = 1;

    glcf->upstream.hide_headers = NGX_CONF_UNSET_PTR;
    glcf->upstream.pass_headers = NGX_CONF_UNSET_PTR;

    glcf->upstream.pass_request_headers = NGX_CONF_UNSET;
    glcf->upstream.pass_request_body = NGX_CONF_UNSET;

    ngx_str_set(&glcf->upstream.module, "gearman");

    return glcf;
}

static char* ngx_http_gearman_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_hash_init_t             hash;
    ngx_http_gearman_loc_conf_t *prev = (ngx_http_gearman_loc_conf_t*)parent;
    ngx_http_gearman_loc_conf_t *conf = (ngx_http_gearman_loc_conf_t*)child;

    ngx_conf_merge_msec_value(conf->upstream.connect_timeout,
                              prev->upstream.connect_timeout, 60000);

    ngx_conf_merge_msec_value(conf->upstream.send_timeout,
                              prev->upstream.send_timeout, 60000);

    ngx_conf_merge_msec_value(conf->upstream.read_timeout,
                              prev->upstream.read_timeout, 60000);

    ngx_conf_merge_uint_value(conf->upstream.store_access,
                              prev->upstream.store_access, 0600);

    ngx_conf_merge_size_value(conf->upstream.buffer_size,
                              prev->upstream.buffer_size,
                              (size_t) ngx_pagesize);

    ngx_conf_merge_value(conf->upstream.pass_request_headers,
                              prev->upstream.pass_request_headers, 0);
    ngx_conf_merge_value(conf->upstream.pass_request_body,
                              prev->upstream.pass_request_body, 1);

    hash.max_size = 100;
    hash.bucket_size = 1024;
    hash.name = "gearman_headers_hash";

    if (ngx_http_upstream_hide_headers_hash(cf, &conf->upstream,
                &prev->upstream, ngx_http_gearman_hide_headers, &hash) != NGX_OK)
    {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}

static char* ngx_http_gearman_pass(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_url_t                       url;
    ngx_http_core_loc_conf_t        *clcf;
    ngx_http_gearman_loc_conf_t     *glcf = (ngx_http_gearman_loc_conf_t*)conf;

    ngx_memzero(&url, sizeof(ngx_url_t));
    url.url = glcf->url;
    url.no_resolve = 1;

    glcf->upstream.upstream = ngx_http_upstream_add(cf, &url, 0);
    if (glcf->upstream.upstream == NULL)
    {
        return NGX_CONF_ERROR;
    }

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_gearman_handler;

    return NGX_CONF_OK;
}

static ngx_int_t ngx_http_gearman_handler(ngx_http_request_t *r)
{

    /* parse request method */
    switch(r->method)
    {
        case NGX_HTTP_UNKNOWN:
            return NGX_HTTP_NOT_ALLOWED;

        case NGX_HTTP_GET:
            return ngx_http_gearman_get_handler(r);

        case NGX_HTTP_HEAD:
            return NGX_HTTP_NOT_ALLOWED;

        case NGX_HTTP_POST:
            return ngx_http_gearman_post_handler(r);

        case NGX_HTTP_PUT:
            return NGX_HTTP_NOT_ALLOWED;

        case NGX_HTTP_DELETE:
            return NGX_HTTP_NOT_ALLOWED;

        case NGX_HTTP_MKCOL:
            return NGX_HTTP_NOT_ALLOWED;

        case NGX_HTTP_COPY:
            return NGX_HTTP_NOT_ALLOWED;

        case NGX_HTTP_MOVE:
            return NGX_HTTP_NOT_ALLOWED;

        default:
            return NGX_HTTP_NOT_ALLOWED;
    }
}

static void ngx_http_gearman_client_body_handler_pt(ngx_http_request_t *r)
{
    ngx_int_t           rc;

    /* parse request body. */
    rc = ngx_http_gearman_parse_client_body(r);
    if (rc != NGX_OK)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, ngx_errno,
                "client request body parse error.");

        /* discard request body */
        ngx_http_discard_request_body(r);

        r->header_only = 1;
        r->headers_out.status = NGX_HTTP_BAD_REQUEST;
        r->headers_out.content_length_n = 0;

        ngx_http_finalize_request(r, ngx_http_send_header(r));
        return;
    }

    rc = ngx_http_gearman_init_upstream(r);
    if (rc != NGX_OK)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, ngx_errno,
                "gearman init upstream failure.");
        return;
    }

    ngx_http_upstream_init(r);

    return;
}

static ngx_int_t ngx_http_gearman_post_handler(ngx_http_request_t *r)
{
    ngx_int_t       rc;

#if 0
    /* check request content length. */
    if (r->headers_in.content_length_n > (ngx_int_t)ngx_pagesize)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, ngx_errno,
                      "request buffer length is too big.");
        return NGX_HTTP_NOT_ALLOWED;
    }
#endif

    rc = ngx_http_read_client_request_body(r, ngx_http_gearman_client_body_handler_pt);
    if (rc >= NGX_HTTP_SPECIAL_RESPONSE)
    {
        return rc;
    }

    return NGX_DONE;
}

static ngx_int_t ngx_http_gearman_get_handler(ngx_http_request_t *r)
{
    ngx_int_t       rc;

    rc = ngx_http_gearman_init_upstream(r);
    if (rc != NGX_OK)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, ngx_errno,
                "gearman init upstream failure.");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    /* start gearman upstream */
    r->main->count++;
    ngx_http_upstream_init(r);

    return NGX_DONE;
}

static ngx_int_t ngx_http_gearman_init_upstream(ngx_http_request_t *r)
{
    ngx_http_upstream_t             *u;
    ngx_http_gearman_context_t      *ctx;
    ngx_http_gearman_loc_conf_t     *glcf;

    if (ngx_http_set_content_type(r) != NGX_OK)
    {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (ngx_http_upstream_create(r) != NGX_OK)
    {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    u = r->upstream;

    u->peer.log = r->connection->log;
    u->peer.log_error = NGX_ERROR_ERR;
    u->output.tag = &ngx_http_gearman_module;

    glcf = ngx_http_get_module_loc_conf(r, ngx_http_gearman_module);
    u->conf = &glcf->upstream;

    u->create_request = ngx_http_gearman_create_request;
    u->reinit_request = ngx_http_gearman_reinit_request;
    u->process_header = ngx_http_gearman_process_status_line;
    u->abort_request = ngx_http_gearman_abort_request;
    u->finalize_request = ngx_http_gearman_finalize_request;

    ctx = ngx_http_get_module_ctx(r, ngx_http_gearman_module);
    if (ctx == NULL)
    {
        ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
        if (ctx == NULL)
        {
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }

        ctx->request = r;
        ngx_http_set_ctx(r, ctx, ngx_http_gearman_module);
    }

    u->input_filter_init = ngx_http_gearman_input_filter_init;
    u->input_filter = ngx_http_gearman_input_filter;
    u->input_filter_ctx = ctx;

    return NGX_OK;
}

static ngx_int_t ngx_http_gearman_parse_client_body(ngx_http_request_t *r)
{
    cJSON                           *json, *id;
    ngx_http_gearman_context_t      *ctx;
    ngx_buf_t                       *b;
    ngx_chain_t                     *body;
    ngx_int_t                       len;
    ngx_temp_file_t                 *tf;
    off_t                           offset;
    ssize_t                         n, total;

    ctx = ngx_http_get_module_ctx(r, ngx_http_gearman_module);
    if (ctx == NULL)
    {
        ctx = ngx_pcalloc(r->pool, sizeof(*ctx));
        if (ctx == NULL)
        {
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }

        ctx->request = r;
        ngx_http_set_ctx(r, ctx, ngx_http_gearman_module);
    }

    /* parse request body if exit.*/
    if (r->method == NGX_HTTP_POST
            && r->headers_in.content_length_n > 0)
    {

        /*
         * do not limit request body size.
         * do copy the request json-rpc message to one buffer and decode it.
         */
        b = ngx_create_temp_buf(r->pool, r->headers_in.content_length_n+8);
        if (b == NULL)
        {
            return NGX_ERROR;
        }

        tf = r->request_body->temp_file;

        if (tf == NULL)
        {
            body = r->request_body->bufs;

            while(body)
            {
                len = body->buf->last - body->buf->pos;
                ngx_memcpy(b->last, body->buf->pos, len);
                b->last = b->last + len;

                body = body->next;
            }

            *b->last++ = '\0';
        }
        else
        {
            n = 0;
            total = 0;
            offset = 0;

            while(1)
            {
                n = ngx_read_file(&tf->file, b->last, ngx_pagesize, offset);
                if (n == NGX_ERROR)
                {
                    ngx_log_error(NGX_LOG_ERR, r->connection->log, ngx_errno,
                            "gearman read temp file error.");
                    return NGX_ERROR;
                }

                offset = offset + n;
                total = total + n;
                b->last = b->last + n;

                if (total == r->headers_in.content_length_n)
                {
                    break;
                }
            }
            ngx_log_error(NGX_LOG_INFO, r->connection->log, ngx_errno,
                    "request body size:%d read buffer size:%d",
                    r->headers_in.content_length_n,
                    total);

            *b->last++ = '\0';
        }

        ngx_log_error(NGX_LOG_INFO, r->connection->log, ngx_errno,
                "request body:%s", b->pos);

        json = cJSON_Parse((const char*)b->pos);
        if (!json)
        {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, ngx_errno,
                    "request body is invalid json object");
            return NGX_ERROR;
        }
        else
        {
            id = cJSON_GetObjectItem(json, "type");
            if (!id)
            {
                ngx_log_error(NGX_LOG_ERR, r->connection->log, ngx_errno,
                        "do not find gearman worker identify.");
                return NGX_ERROR;
            }

            char *out = cJSON_Print(id);
            ngx_log_error(NGX_LOG_INFO, r->connection->log, ngx_errno,
                    "gearman worker function name:%s", out);

            ctx->function_name.len = ngx_strlen(out) - 1;
            ctx->function_name.data = ngx_pcalloc(r->pool, ctx->function_name.len);
            ngx_memcpy(ctx->function_name.data, (void*)(out + 1), ngx_strlen(out) - 2);
            ctx->function_name.data[ctx->function_name.len] = '\0';

            ngx_free(out);
            out = NULL;

            return NGX_OK;
        }
    }
    else
    {
        return NGX_OK;
    }
}

static ngx_int_t ngx_http_gearman_create_request(ngx_http_request_t *r)
{
    ngx_buf_t                   *b;
    ngx_chain_t                 *cl;
    ngx_str_t                   url;
    ngx_chain_t                 *body;
    ngx_http_upstream_t         *u;
    ngx_http_gearman_loc_conf_t *glcf;
    ngx_http_gearman_context_t  *ctx;
    ngx_str_t                   *function_name;

    ngx_log_error(NGX_LOG_INFO, r->connection->log, ngx_errno,
            "call %s start.", __func__);

    glcf = ngx_http_get_module_loc_conf(r, ngx_http_gearman_module);
    ctx = ngx_http_get_module_ctx(r, ngx_http_gearman_module);
    if (ctx == NULL)
    {
        ngx_log_error(NGX_LOG_INFO, r->connection->log, ngx_errno,
                "ngx gearan get ctx error.");
        return NGX_ERROR;
    }
    function_name = &ctx->function_name;

    /* build headers. */
    b = ngx_create_temp_buf(r->pool, ngx_pagesize);
    if (b == NULL)
    {
        return NGX_ERROR;
    }

    b->last = ngx_sprintf(b->pos,
                          "%s /%s HTTP/1.1\r\n",
                          (r->method & NGX_HTTP_POST) ? "POST" : "GET",
                          (function_name->data != NULL) ? (char*)function_name->data : "worker");

    url = glcf->url;
    b->last = ngx_sprintf(b->last, "Host: %s\r\n", url.data);
    b->last = ngx_sprintf(b->last, "Accept: */*\r\n");
    b->last = ngx_sprintf(b->last, "User-Agent: nginx/1.6.2\r\n");
    b->last = ngx_sprintf(b->last, "X-Gearman-Unique: %d\r\n", random());
    b->last = ngx_sprintf(b->last, "X-Gearman-Background: false\r\n");
    b->last = ngx_sprintf(b->last, "X-Gearman-Priority: high\r\n");

    if (r->method == NGX_HTTP_POST
            && r->headers_in.content_length_n > 0)
    {
        b->last = ngx_sprintf(b->last,
                              "Content-Length: %d\r\n",
                              r->headers_in.content_length_n);
    }
    b->last = ngx_sprintf(b->last, "\r\n");

    ngx_log_error(NGX_LOG_INFO, r->connection->log, ngx_errno,
            "headers:%s", b->pos);

    cl = ngx_alloc_chain_link(r->pool);
    if (cl == NULL)
    {
        return NGX_ERROR;
    }
    cl->buf = b;
    cl->next = NULL;
    r->header_hash = 1;

    /* pass request body if the method is POST. */
    u = r->upstream;
    if (r->method == NGX_HTTP_POST
            && r->headers_in.content_length_n > 0)
    {
        body = u->request_bufs;
        u->request_bufs = cl;

        while(body)
        {
            b = ngx_alloc_buf(r->pool);
            if (b == NULL)
            {
                return NGX_ERROR;
            }

            ngx_memcpy(b, body->buf, sizeof(ngx_buf_t));

            cl->next = ngx_alloc_chain_link(r->pool);
            if (cl->next == NULL)
            {
                return NGX_ERROR;
            }

            cl = cl->next;
            cl->buf = b;

            body = body->next;
        }
    }
    else if (r->method == NGX_HTTP_GET)
    {
        r->upstream->request_bufs = cl;
    }
    else
    {
        /* do not support other http methods. */
    }
    cl->next = NULL;

    ngx_log_error(NGX_LOG_INFO, r->connection->log, ngx_errno,
            "call %s end.", __func__);

    return NGX_OK;
}

static ngx_int_t ngx_http_gearman_reinit_request(ngx_http_request_t *r)
{
    ngx_log_error(NGX_LOG_INFO, r->connection->log, ngx_errno,
            "call %s", __func__);
    return NGX_OK;
}

static ngx_int_t ngx_http_gearman_process_status_line(ngx_http_request_t *r)
{
    size_t                              len;
    ngx_int_t                           rc;
    ngx_http_upstream_t                 *u;
    ngx_http_gearman_context_t          *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_http_gearman_module);
    if (ctx == NULL)
    {
        return NGX_ERROR;
    }

    u = r->upstream;

    rc = ngx_http_parse_status_line(r, &u->buffer, &ctx->status);
    if (rc == NGX_AGAIN)
    {
        return rc;
    }

    if (rc == NGX_ERROR)
    {
        r->http_version = NGX_HTTP_VERSION_9;
        u->state->status = NGX_HTTP_OK;
        u->headers_in.connection_close = 1;

        return NGX_OK;
    }

    if (u->state)
    {
        u->state->status = ctx->status.code;
    }

    u->headers_in.status_n = ctx->status.code;

    len = ctx->status.end - ctx->status.start;
    u->headers_in.status_line.len = len;

    u->headers_in.status_line.data = ngx_pnalloc(r->pool, len);
    if (u->headers_in.status_line.data == NULL)
    {
        return NGX_ERROR;
    }

    ngx_memcpy(u->headers_in.status_line.data, ctx->status.start, len);

    u->process_header = ngx_http_gearman_process_header;

    return ngx_http_gearman_process_header(r);
}

static ngx_int_t ngx_http_gearman_process_header(ngx_http_request_t *r)
{

    ngx_int_t                       rc;
    ngx_table_elt_t                 *h;
    ngx_http_upstream_header_t      *hh;
    ngx_http_upstream_main_conf_t   *umcf;

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
            "call %s", __func__);

    umcf = ngx_http_get_module_main_conf(r, ngx_http_upstream_module);

    while(1)
    {
        rc = ngx_http_parse_header_line(r, &r->upstream->buffer, 1);

        if (rc == NGX_OK)
        {
            /* a header line has been parsed successfully */
            h = ngx_list_push(&r->upstream->headers_in.headers);
            if (h == NULL)
            {
                return NGX_ERROR;
            }

            h->hash = r->header_hash;

            h->key.len = r->header_name_end - r->header_name_start;
            h->value.len = r->header_end - r->header_start;

            h->key.data = ngx_pnalloc(r->pool,
                    h->key.len + 1 + h->value.len + 1 + h->key.len);
            if (h->key.data == NULL)
            {
                return NGX_ERROR;
            }

            h->value.data = h->key.data + h->key.len + 1;
            h->lowcase_key = h->key.data + h->key.len + 1 + h->value.len + 1;

            ngx_memcpy(h->key.data, r->header_name_start, h->key.len);
            h->key.data[h->key.len] = '\0';
            ngx_memcpy(h->value.data, r->header_start, h->value.len);
            h->value.data[h->value.len] = '\0';

            if (h->key.len == r->lowcase_index)
            {
                ngx_memcpy(h->lowcase_key, r->lowcase_header, h->key.len);
            }
            else
            {
                ngx_strlow(h->lowcase_key, h->key.data, h->key.len);
            }
            hh = ngx_hash_find(&umcf->headers_in_hash, h->hash,
                    h->lowcase_key, h->key.len);

            if (hh && hh->handler(r, h, hh->offset) != NGX_OK)
            {
                return NGX_ERROR;
            }

            ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
                    "http gearman header: \"%V: %V\"",
                    &h->key, &h->value);

            continue;
        }

        if (rc == NGX_HTTP_PARSE_HEADER_DONE)
        {
            if (r->upstream->headers_in.server == NULL)
            {
                h = ngx_list_push(&r->upstream->headers_in.headers);
                if (h == NULL)
                {
                    return NGX_ERROR;
                }

                h->hash = ngx_hash(ngx_hash(ngx_hash(ngx_hash(
                                    ngx_hash('s', 'e'), 'r'), 'v'), 'e'), 'r');

                ngx_str_set(&h->key, "Server");
                ngx_str_null(&h->value);
                h->lowcase_key = (u_char *) "server";
            }

            if (r->upstream->headers_in.date == NULL)
            {
                h = ngx_list_push(&r->upstream->headers_in.headers);
                if (h == NULL)
                {
                    return NGX_ERROR;
                }

                h->hash = ngx_hash(ngx_hash(ngx_hash('d', 'a'), 't'), 'e');

                ngx_str_set(&h->key, "Date");
                ngx_str_null(&h->value);
                h->lowcase_key = (u_char *) "date";
            }

            /* a whole header has been parsed successfully */
            ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
                    "http gearman header done");
            return NGX_OK;
        }

        if (rc == NGX_AGAIN)
        {
            return NGX_AGAIN;
        }

        /* there was error while a header line parsing */
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "upstream sent invalid header");

        return NGX_HTTP_UPSTREAM_INVALID_HEADER;
    }
}

static void ngx_http_gearman_abort_request(ngx_http_request_t *r)
{
    ngx_log_error(NGX_LOG_INFO, r->connection->log, ngx_errno,
            "call %s", __func__);
    return;
}

static void ngx_http_gearman_finalize_request(ngx_http_request_t *r, ngx_int_t rc)
{
    ngx_log_error(NGX_LOG_INFO, r->connection->log, ngx_errno,
            "call %s", __func__);
    return;
}

static ngx_int_t ngx_http_gearman_input_filter_init(void *data)
{
    ngx_http_request_t              *r;
    ngx_http_upstream_t             *u;
    ngx_http_gearman_context_t      *ctx;

    ctx = (ngx_http_gearman_context_t*)data;
    r = ctx->request;
    u = r->upstream;

    ngx_log_error(NGX_LOG_INFO, r->connection->log, ngx_errno,
            "call %s", __func__);

    u->length = u->headers_in.content_length_n;

    return NGX_OK;
}

static ngx_int_t ngx_http_gearman_input_filter(void *data, ssize_t bytes)
{
    ngx_buf_t                       *b;
    ngx_http_upstream_t             *u;
    ngx_http_request_t              *r;
    ngx_http_gearman_context_t      *ctx;
    ngx_chain_t                     *cl, **ll;

    ctx = (ngx_http_gearman_context_t*)data;
    r = ctx->request;
    u = r->upstream;

    ngx_log_error(NGX_LOG_INFO, r->connection->log, ngx_errno,
            "call %s", __func__);

    for (cl = u->out_bufs, ll = &u->out_bufs; cl; cl = cl->next)
    {
        ll = &cl->next;
    }

    cl = ngx_chain_get_free_buf(r->pool, &u->free_bufs);
    if (cl == NULL)
    {
        return NGX_ERROR;
    }

    *ll = cl;

    cl->buf->flush = 1;
    cl->buf->memory = 1;

    b = &u->buffer;

    cl->buf->pos = b->last;
    b->last += bytes;
    cl->buf->last = b->last;
    cl->buf->tag = u->output.tag;

    if (u->length == -1)
    {
        return NGX_OK;
    }

    u->length -= bytes;

    if (u->length == 0)
    {
        u->keepalive = !u->headers_in.connection_close;
    }

    return NGX_OK;
}
