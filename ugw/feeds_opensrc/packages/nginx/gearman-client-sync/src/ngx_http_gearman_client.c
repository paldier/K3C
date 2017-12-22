/*************************************************************************
> File Name: ngx_http_gearman_client.c
> Author: yy
> Mail: mengyy_linux@163.com
 ************************************************************************/

#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgearman/gearman.h>

#define TRUE    (1)
#define FALSE   (0)

static int writen(int fd, const void *buf, size_t count)
{
    ssize_t nret;
    const char *tmp_pbuf = buf;
    size_t size;

    if(fd < 0 || !tmp_pbuf)
    {
        return -1;
    }

    size = 0;
    while(count)
    {
        nret = write(fd, tmp_pbuf, count);
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

static gearman_return_t client_created_cb(gearman_task_st *task)
{
    return GEARMAN_SUCCESS;
}

static gearman_return_t client_data_cb(gearman_task_st *task)
{
    size_t size;
    void *context = NULL;
    const void *data = NULL;

    if (!task)
    {
        return GEARMAN_FAIL;
    }

    context = gearman_task_context(task);
    data = gearman_task_data(task);
    size = gearman_task_data_size(task);

    int fd = *((int*)context);
    if (fd > 0)
    {
        writen(fd, &size, sizeof(size_t));
        writen(fd, data, size);
    }

    return GEARMAN_SUCCESS;
}

static gearman_return_t client_warning_cb(gearman_task_st *task)
{
    void *context = NULL;
    size_t size;
    const void *data = NULL;

    if (!task)
    {
        return GEARMAN_FAIL;
    }

    context = gearman_task_context(task);
    data = gearman_task_data(task);
    size = gearman_task_data_size(task);

    int fd = *((int*)context);
    if (fd > 0)
    {
        writen(fd, &size, sizeof(size_t));
        writen(fd, data, size);
    }

    return GEARMAN_SUCCESS;
}

static gearman_return_t client_status_cb(gearman_task_st *task)
{
    //unsigned int rate;

    if (!task)
    {
        return GEARMAN_FAIL;
    }

    //rate = (gearman_task_numerator(task) * 100) / gearman_task_denominator(task);
    /* TODO */

    return GEARMAN_SUCCESS;
}

static gearman_return_t client_fail_cb(gearman_task_st *task)
{
    void *context = NULL;
    const char *fail_string = "Job failed!";
    size_t size = strlen(fail_string) + 1;

    if (!task)
    {
        return GEARMAN_FAIL;
    }

    context = gearman_task_context(task);

    int fd = *((int*)context);
    writen(fd, &size, sizeof(size_t));
    writen(fd, fail_string, size);

    return GEARMAN_SUCCESS;
}

/*
 * @function: initialize gearman client context.
 * @params[in]  host: the ip addr of gearman job server.
 *              port: the port of gearman job server listening port.
 * @params[out] null
 * @return the allocated gearman client context.
 */
void* ngx_gearman_initialize(const char *host, in_port_t port)
{
    int timeout = 0;
    int ssl = FALSE;
    gearman_client_st *client_ctx = NULL;

    if (!host || port <= 0)
    {
        return NULL;
    }

    client_ctx = (gearman_client_st*)calloc(1, sizeof(*client_ctx));
    gearman_client_create(client_ctx);

    if (timeout > 0)
    {
        gearman_client_set_timeout(client_ctx, timeout);
    }

    if (gearman_failed(gearman_client_add_server(client_ctx, host, port)))
    {
        gearman_client_free(client_ctx);
        return NULL;
    }

    if (ssl)
    {
        gearman_client_add_options(client_ctx, GEARMAN_CLIENT_SSL);
    }

    gearman_client_set_created_fn(client_ctx, client_created_cb);
    gearman_client_set_data_fn(client_ctx, client_data_cb);
    gearman_client_set_warning_fn(client_ctx, client_warning_cb);
    gearman_client_set_status_fn(client_ctx, client_status_cb);
    gearman_client_set_complete_fn(client_ctx, client_data_cb);
    gearman_client_set_exception_fn(client_ctx, client_warning_cb);
    gearman_client_set_fail_fn(client_ctx, client_fail_cb);

    return (void*)client_ctx;
}

/*
 * @function: destroy gearman client context
 * @params[in]  ctx: the gearman client context
 * @params[out] null
 * @return  null
 */
void ngx_gearman_deinitialize(void *ctx)
{
    if (ctx)
    {
        gearman_client_free(ctx);
    }
}

/*
 * @function: add tasks and run the tasks
 * @params[in]  client: the gearman client context
 *              context: the caller context
 *              function: the gearman worker function
 *              workload: the data sending to the worker
 *              workload_size: the size of workload_size
 * @params[out] null
 * @return  TRUE: successed; FALSE: failed
 */
int ngx_gearman_run_task(void *client, void *context, const char *function, void *workload, size_t workload_size)
{
    gearman_return_t ret;
    const char *unique = NULL;

    if (!client || !context || !function || !workload)
    {
        return FALSE;
    }

    gearman_client_add_task(client,     /* client */
            NULL,                       /* task */
            context,                    /* context */
            function,                   /* function name */
            unique,                     /* unique */
            workload,                   /* workload */
            workload_size,              /* workload_size */
            &ret);                      /* ret_ptr */

    if (gearman_failed(gearman_client_run_tasks(client)))
    {
        return FALSE;
    }

    return TRUE;
}
