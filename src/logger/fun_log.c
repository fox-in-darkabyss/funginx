
#include <fun_log.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_string.h>

void fun_log(ngx_str_t str)
{
   FILE *fp = NULL;
   fp = fopen("/nnggiinnxx/fun_log.txt", "w+");
   fputs((const char *)str.data, fp);
   fclose(fp);
}
