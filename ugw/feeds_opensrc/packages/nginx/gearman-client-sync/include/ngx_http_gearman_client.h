/*************************************************************************
> File Name: ngx_http_gearman_client.h
> Author: yy
> Mail: mengyy_linux@163.com
 ************************************************************************/

#ifndef _GEARMAN_CLIENT
#define _GEARMAN_CLIENT

void* ngx_gearman_initialize(const char *host, in_port_t port);
void ngx_gearman_deinitialize(void *ctx);
int ngx_gearman_run_task(void *client, void *context, const char *function, void *workload, size_t workload_size);

#endif
