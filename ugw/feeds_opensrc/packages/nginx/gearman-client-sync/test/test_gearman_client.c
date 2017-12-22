/*************************************************************************
> File Name: test_gearman_client.c
> Author: yy
> Mail: mengyy_linux@163.com
 ************************************************************************/

/*
 * @PURPOSE:
 *      test gearman client wrapper API.
 * @COMPILE:
 *      gcc test_gearman_client.c gearman_client.c -lgearman -o test
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

#include "ngx_http_gearman_client.h"

typedef struct
{
    const char*         host;
    in_port_t           port;
    const char*         function;
    int                 fd[2];
    void*               client;
}client_ctx_t;

ssize_t readn(int fd, void *buf, size_t count)
{
    ssize_t nret;
    size_t size;

    size = 0;
    char *tmp_pbuf = buf;
    while(count)
    {
        nret = read(fd, tmp_pbuf, count);
        if (nret < 0)
        {
        }
        else if (nret == 0)
        {
            /* TODO */
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

int main(int argc, char**argv)
{
    const char *workload = "hello world!";
    size_t workload_size = strlen(workload) + 1;

    client_ctx_t ctx;

    memset(&ctx, 0, sizeof(ctx));
    ctx.host = "127.0.0.1";
    ctx.port = (in_port_t)4730;
    ctx.function = "nginx";

    pipe(ctx.fd);

    ctx.client = ngx_gearman_initialize(ctx.host, ctx.port);

    while(1)
    {
        ngx_gearman_run_task(ctx.client, (void*)(&ctx.fd[1]), ctx.function, (void*)workload, workload_size);

        size_t buffer_size;
        char *buffer = NULL;

        readn(ctx.fd[0], &buffer_size, sizeof(size_t));
        fprintf(stderr, "buffer size:%lu\n", buffer_size);

        buffer = calloc(1, buffer_size);
        assert(buffer != NULL);
        readn(ctx.fd[0], buffer, buffer_size);
        fprintf(stderr, "buffer content:\t%s\n", buffer);

        free(buffer);
        buffer = NULL;

        //sleep(1);
        break;
    }

    close(ctx.fd[0]);
    close(ctx.fd[1]);
    ctx.fd[0] = -1;
    ctx.fd[1] = -1;

    ngx_gearman_deinitialize(ctx.client);

    return 0;
}
