#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
 
static ngx_int_t ngx_http_zero_handler(ngx_http_request_t *r);
 
static char *
ngx_http_zero(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
 
 
/**
 * 处理nginx.conf中的配置命令解析
 * 例如：
 * location /hello {
 *  	hello
 * }
 * 当用户请求:http://127.0.0.1/hello的时候，请求会跳转到hello这个配置上
 * hello的命令行解析回调函数：ngx_http_hello
 */
static ngx_command_t ngx_http_zero_commands[] = {
		{
				ngx_string("zero"),
				NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
				ngx_http_zero,
				NGX_HTTP_LOC_CONF_OFFSET,
				0,
				NULL
		},
		ngx_null_command
};
 
 
/**
 * 模块上下文
 */
static ngx_http_module_t ngx_http_zero_module_ctx = { NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL };
 
/**
 * 模块的定义
 */
ngx_module_t ngx_http_zero_module = {
		NGX_MODULE_V1,
		&ngx_http_zero_module_ctx,
		ngx_http_zero_commands,
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
 
/**
 * 命令解析的回调函数
 * 该函数中，主要获取loc的配置，并且设置location中的回调函数handler
 */
static char *
ngx_http_zero(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
	ngx_http_core_loc_conf_t *clcf;
 
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
	/* 设置回调函数。当请求http://127.0.0.1/hello的时候，会调用此回调函数 */
	clcf->handler = ngx_http_zero_handler;
 
	return NGX_CONF_OK;
}
 
/**
 * 模块回调函数，输出hello world
 */
static ngx_int_t ngx_http_zero_handler(ngx_http_request_t *r) {
	if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD))) {
		return NGX_HTTP_NOT_ALLOWED;
	}
 
	ngx_int_t rc = ngx_http_discard_request_body(r);
	if (rc != NGX_OK) {
		return rc;
	}
 
	ngx_str_t type = ngx_string("text/html");
	ngx_str_t response = ngx_string("<html lang='zh'><head><meta charset='utf-8'><title>年来肠断秣陵舟，梦绕秦淮水上楼。</title></head><body>十日雨丝风片里，浓春艳景似残秋。</body></html>");
	r->headers_out.status = NGX_HTTP_OK;
	r->headers_out.content_length_n = response.len;
	r->headers_out.content_type = type;
 
	rc = ngx_http_send_header(r);
	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
		return rc;
	}
 
	ngx_buf_t *b;
	b = ngx_create_temp_buf(r->pool, response.len);
	if (b == NULL) {
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}
 
	ngx_memcpy(b->pos, response.data, response.len);
	b->last = b->pos + response.len;
	b->last_buf = 1;
 
	ngx_chain_t out;
	out.buf = b;
	out.next = NULL;
 
	return ngx_http_output_filter(r, &out);
}