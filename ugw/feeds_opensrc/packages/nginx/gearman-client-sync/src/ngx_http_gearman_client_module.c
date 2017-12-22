/*************************************************************************
> File Name: ngx_http_gearman_client_module.c
> Author: yy
> Mail: mengyy_linux@163.com
 ************************************************************************/

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <errno.h>
#include <assert.h>
#include <unistd.h>

#include <ngx_http_gearman_client.h>

#define TRUE    (1)
#define FALSE   (0)

struct ngx_http_gearman_context_s
{
    const char      *host;
    in_port_t       port;
    char            res[2];
    const char      *function;
    int             fd[2];
    void            *client;
    int             initialized;
};
typedef struct ngx_http_gearman_context_s ngx_http_gearman_context_t;

ngx_http_gearman_context_t ngx_http_gearman_ctx; /* TODO */

static int ngx_http_gearman_initialize_context(void);
static void ngx_http_gearman_deinitialize_context(void);
static int ngx_http_gearman_send_buffer_to_upstream(void);
static ssize_t ngx_http_gearman_readn(int fd, void *buf, size_t count);
static ngx_int_t ngx_http_gearman_client_handler(ngx_http_request_t *r);
static size_t ngx_http_gearman_recv_buffer_size_from_upstream_sync(ngx_http_request_t *r);
static ngx_buf_t* ngx_http_gearman_recv_buffer_from_upstream_sync(ngx_http_request_t *r);
static char* ngx_http_gearman_client(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t ngx_http_gearman_client_commands[] = {
    {
        ngx_string("gearman_client"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
        ngx_http_gearman_client,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },

    ngx_null_command,
};

static ngx_http_module_t ngx_http_gearman_client_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL,                                  /* merge location configuration */
};

ngx_module_t ngx_http_gearman_client_module = {
    NGX_MODULE_V1,
    &ngx_http_gearman_client_module_ctx,    /* module context */
    ngx_http_gearman_client_commands,       /* module directives */
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

static char* ngx_http_gearman_client(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_gearman_client_handler;

    return NGX_CONF_OK;
}

static ngx_int_t ngx_http_gearman_client_handler(ngx_http_request_t *r)
{
    if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD)))
    {
        return NGX_HTTP_NOT_ALLOWED;
    }

    ngx_int_t rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK)
    {
        return rc;
    }

    int ret;
    size_t size;

    ret = ngx_http_gearman_initialize_context();
    if (ret != TRUE)
    {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ret = ngx_http_gearman_send_buffer_to_upstream();
    if (ret != TRUE)
    {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    size = ngx_http_gearman_recv_buffer_size_from_upstream_sync(r);
    ngx_log_error(NGX_LOG_INFO, r->connection->log, ngx_errno,
            "nginx gearman upstream buffer size:%lu", size);

    ngx_str_t type = ngx_string("text/plain");
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = size;
    r->headers_out.content_type = type;

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK)
    {
        return rc;
    }

    ngx_buf_t *b = NULL;
    b = ngx_http_gearman_recv_buffer_from_upstream_sync(r);
    if (b == NULL)
    {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_http_gearman_deinitialize_context();

    ngx_chain_t out;
    out.buf = b;
    out.next = NULL;

    return ngx_http_output_filter(r, &out);
}

static int ngx_http_gearman_initialize_context(void)
{
    int ret;

    ngx_http_gearman_ctx.host = "127.0.0.1";
    ngx_http_gearman_ctx.port = (in_port_t)4730;
    ngx_http_gearman_ctx.function = "nginx";

    ngx_http_gearman_ctx.client = ngx_gearman_initialize(
            ngx_http_gearman_ctx.host,
            (in_port_t)ngx_http_gearman_ctx.port);
    if (!ngx_http_gearman_ctx.client)
    {
        return FALSE;
    }

    ret = pipe(ngx_http_gearman_ctx.fd);
    if (ret < 0)
    {
        return FALSE;
    }

    ngx_http_gearman_ctx.initialized = TRUE;

    return TRUE;
}

static void ngx_http_gearman_deinitialize_context(void)
{
    ngx_gearman_deinitialize(ngx_http_gearman_ctx.client);
    ngx_http_gearman_ctx.client = NULL;

    close(ngx_http_gearman_ctx.fd[0]);
    close(ngx_http_gearman_ctx.fd[1]);
    ngx_http_gearman_ctx.fd[0] = -1;
    ngx_http_gearman_ctx.fd[1] = -1;

    ngx_http_gearman_ctx.initialized = FALSE;
}

static int ngx_http_gearman_send_buffer_to_upstream(void)
{
    int ret;
    void *context = NULL;
    ngx_str_t request = ngx_string("nginx gearman module");

    if (ngx_http_gearman_ctx.initialized != TRUE)
    {
        return FALSE;
    }

    context = &ngx_http_gearman_ctx.fd[1];
    ret = ngx_gearman_run_task(
            ngx_http_gearman_ctx.client,         /* the gearman client context */
            context,                        /* the pipe write fd */
            ngx_http_gearman_ctx.function,       /* the gearman worker registed function */
            request.data,                   /* the data sending to the worker */
            request.len);                   /* the size of workload */

    return ret;
}

static size_t ngx_http_gearman_recv_buffer_size_from_upstream_sync(ngx_http_request_t *r)
{
    int fd;
    size_t size;
    ssize_t nret;

    fd = ngx_http_gearman_ctx.fd[0];

    size_t length = sizeof(size_t);
    nret = ngx_http_gearman_readn(fd, &size, length);
    if (nret != (ssize_t)length)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, ngx_errno,
                "nginx gearman receive buffer error. received size:%d expected size:%d", nret, size);
    }

    return size;
}

static ngx_buf_t* ngx_http_gearman_recv_buffer_from_upstream_sync(ngx_http_request_t *r)
{
    int fd;
    ngx_buf_t *b = NULL;
    size_t size;
    ssize_t nret;

    size = r->headers_out.content_length_n;
    b = ngx_create_temp_buf(r->pool, size);
    if (b == NULL)
    {
        return NULL;
    }

    fd = ngx_http_gearman_ctx.fd[0];
    nret = ngx_http_gearman_readn(fd, b->pos, size);
    if (nret != (ssize_t)size)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, ngx_errno,
                "nginx gearman receive buffer error. received size:%d expected size:%d", nret, size);
    }

    b->last = b->pos + nret;
    b->last_buf = 1;

    return b;
}

static ssize_t ngx_http_gearman_readn(int fd, void *buf, size_t count)
{
    ssize_t nret;
    size_t size;

    size = 0;
    char *tmp_pbuf = (char*)buf;
    while(count)
    {
        nret = read(fd, tmp_pbuf, count);
        if (nret < 0)
        {
            assert(0);
        }
        else if (nret == 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            count -= nret;
            tmp_pbuf += nret;
            size += nret;
        }
    }

    return size;
}
