#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct 		//配置项结构体 – 每个模块相关的配置项集中管理用的结构体
{
    ngx_str_t output_words;
} ngx_http_hello_world_loc_conf_t;

static char* ngx_http_hello_world(ngx_conf_t* cf, ngx_command_t* cmd, void* conf);

static void* ngx_http_hello_world_create_loc_conf(ngx_conf_t* cf);

static char* ngx_http_hello_world_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child);

static ngx_command_t ngx_http_hello_world_commands[] = 
{	//commands结构体，设定配置项特定的处理方式，定义该处理项出现时的处理函数
    {
	//配置项的名字
	ngx_string("hello_world"), 
	//NGX_HTTP_LOC_CONF表示指令在位置配置部分出现是合法的 ，NGX_CONF_TAKE1: 指令读入一个参数
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
	//遇到该指令名字时调用的函数	
        ngx_http_hello_world, 
	//存储位置，配置结构体+offset指示变量的具体存储位置
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_hello_world_loc_conf_t, output_words),
        NULL
    },
    ngx_null_command
};

//实现ngx_http_module_t接口，来管理http模块的配置项，在http框架初始化时，会调用该模块定义的回调方法
static ngx_http_module_t ngx_http_hello_world_module_ctx = 
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ngx_http_hello_world_create_loc_conf,
    ngx_http_hello_world_merge_loc_conf
};

//定义ngx_module_t模块，主要工作是利用前面定义的ngx_http_hello_world_module_ctx和ngx_http_hello_world_commands
//来对其中的ctx和commands成员变量进行赋值
ngx_module_t ngx_http_hello_world_module = {
    NGX_MODULE_V1,
    &ngx_http_hello_world_module_ctx,
    ngx_http_hello_world_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};

//处理请求的回调函数
static ngx_int_t ngx_http_hello_world_handler(ngx_http_request_t* r) {
    ngx_int_t rc;
    ngx_buf_t* b;
    ngx_chain_t out[2];

    ngx_http_hello_world_loc_conf_t* hlcf;
    hlcf = ngx_http_get_module_loc_conf(r, ngx_http_hello_world_module);

    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char*)"text/plain";

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    out[0].buf = b;
    out[0].next = &out[1];
    //建立ngx_buf_t,直接指向原内存地址，不对数据进行复制，节省内存
    b->pos = (u_char*)"Hello World";
    b->last = b->pos + sizeof("Hello World") - 1;
    b->memory = 1;

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    out[1].buf = b;
    out[1].next = NULL;

    b->pos = hlcf->output_words.data;
    b->last = hlcf->output_words.data + (hlcf->output_words.len);
    b->memory = 1;
    b->last_buf = 1;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = hlcf->output_words.len + sizeof("hello_world, ") - 1;
    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out[0]);
}

static void* ngx_http_hello_world_create_loc_conf(ngx_conf_t* cf) {
    ngx_http_hello_world_loc_conf_t* conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hello_world_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    conf->output_words.len = 0;
    conf->output_words.data = NULL;

    return conf;
}

static char* ngx_http_hello_world_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child) 
{
    ngx_http_hello_world_loc_conf_t* prev = parent;
    ngx_http_hello_world_loc_conf_t* conf = child;
    ngx_conf_merge_str_value(conf->output_words, prev->output_words, "Nginx");
    return NGX_CONF_OK;
}

static char* ngx_http_hello_world(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) 
{
	ngx_http_core_loc_conf_t* clcf;
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    	//将ngx_http_core_loc_conf_t结构体的回调函数设为ngx_http_hello_world_handle，
 	//在NGX_HTTP_CONTENT_PHASE阶段，如果请求的主机名，URI与配置项所在的配置块相匹配时，就调用该回调方法
    	clcf->handler = ngx_http_hello_world_handler;	
    	ngx_conf_set_str_slot(cf, cmd, conf);
    	return NGX_CONF_OK;
}
