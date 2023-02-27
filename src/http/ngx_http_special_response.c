
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>


static ngx_int_t ngx_http_send_error_page(ngx_http_request_t *r,
    ngx_http_err_page_t *err_page);
static ngx_int_t ngx_http_send_special_response(ngx_http_request_t *r,
    ngx_http_core_loc_conf_t *clcf, ngx_uint_t err);
static ngx_int_t ngx_http_send_refresh(ngx_http_request_t *r);


static u_char ngx_http_error_full_tail[] =
"<hr><center>" NGINX_VER "</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;


static u_char ngx_http_error_build_tail[] =
"<hr><center>" NGINX_VER_BUILD "</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;


static u_char ngx_http_error_tail[] =
"<hr><center>涂山之南</center>" CRLF
"</body>" CRLF
"</html>" CRLF
;


static u_char ngx_http_msie_padding[] =
"<!-- a padding to disable MSIE and Chrome friendly error page -->" CRLF
"<!-- a padding to disable MSIE and Chrome friendly error page -->" CRLF
"<!-- a padding to disable MSIE and Chrome friendly error page -->" CRLF
"<!-- a padding to disable MSIE and Chrome friendly error page -->" CRLF
"<!-- a padding to disable MSIE and Chrome friendly error page -->" CRLF
"<!-- a padding to disable MSIE and Chrome friendly error page -->" CRLF
;


static u_char ngx_http_msie_refresh_head[] =
"<html lang='zh'><head><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><meta charset='utf-8' /><meta http-equiv=\"Refresh\" content=\"0; URL=";


static u_char ngx_http_msie_refresh_tail[] =
"\"></head><body style='font-family:KaiTi'></body></html>" CRLF;


static char ngx_http_error_301_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>北斗酌美酒，劝龙各一觞。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-times-circle-o'></i>&nbsp;富贵非所愿，与人驻颜光。</h1></center>" CRLF
;


static char ngx_http_error_302_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>愚者爱惜费，但为后世嗤。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-star-half-o'></i>&nbsp;仙人王子乔，难可与等期。</h1></center>" CRLF
;


static char ngx_http_error_303_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>翠被华灯，夜夜空相向。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-sitemap'></i>&nbsp;寂寞起来褰绣幌。月明正在梨花上。</h1></center>" CRLF
;


static char ngx_http_error_307_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>十四五，闲抱琵琶寻。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-save'></i>&nbsp;阶上簸钱阶下走，恁时相见早留心。何况到如今。</h1></center>" CRLF
;


static char ngx_http_error_308_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>不枉东风吹客泪。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-rotate-left'></i>&nbsp;相思难表，梦魂无据，惟有归来是。</h1></center>" CRLF
;


static char ngx_http_error_400_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>天下事，少年心。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-rocket'></i>&nbsp;分明点点深。</h1></center>" CRLF
;


static char ngx_http_error_401_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>故方其盛也，举天下之豪杰，莫能与之争；</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-puzzle-piece'></i>&nbsp;及其衰也，数十伶人困之，而身死国灭，为天下笑。</h1></center>" CRLF
;


static char ngx_http_error_402_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>群儿戏于庭，一儿登瓮，足跌没水中。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-power-off'></i>&nbsp;众皆弃去，光持石击瓮破之，水迸，儿得活。</h1></center>" CRLF
;


static char ngx_http_error_403_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>南山律律，飘风弗弗。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-print'></i>&nbsp;民莫不穀，我独不卒！</h1></center>" CRLF
;


static char ngx_http_error_404_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>闲凭薰笼无力。心事有谁知得。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-transgender-alt'></i>&nbsp;檀炷绕窗灯背壁。画檐残雨滴。</h1></center>" CRLF
;


static char ngx_http_error_405_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>凡事如是，难可逆见。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-mars-double'></i>&nbsp;臣鞠躬尽瘁，死而后已。至于成败利钝，非臣之明所能逆睹也。</h1></center>" CRLF
;


static char ngx_http_error_406_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>要之，死日然后是非乃定。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-linux'></i>&nbsp;书不能悉意，故略陈固陋。谨再拜。</h1></center>" CRLF
;


static char ngx_http_error_408_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>己丑晦，公宫火。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-language'></i>&nbsp;瑕甥、郤芮不获公，乃如河上，秦伯诱而杀之。</h1></center>" CRLF
;


static char ngx_http_error_409_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>酒贱常愁客少，月明多被云妨。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-hand-peace-o'></i>&nbsp;中秋谁与共孤光。把盏凄然北望。</h1></center>" CRLF
;


static char ngx_http_error_410_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>细雨梦回鸡塞远，小楼吹彻玉笙寒。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-gitlab'></i>&nbsp;多少泪珠何限恨，倚阑干。</h1></center>" CRLF
;


static char ngx_http_error_411_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>南箕北有斗，牵牛不负轭。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-gears'></i>&nbsp;良无盘石固，虚名复何益？</h1></center>" CRLF
;


static char ngx_http_error_412_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>信劳生、空成今古，笑我来、何事怆遗情。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-firefox'></i>&nbsp;东山老，可堪岁晚，独听桓筝。</h1></center>" CRLF
;


static char ngx_http_error_413_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>一日心期千劫在，后身缘、恐结他生里。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-file-word-o'></i>&nbsp;然诺重，君须记。</h1></center>" CRLF
;


static char ngx_http_error_414_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>情独私怀，谁者可语？</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-file-zip-o'></i>&nbsp;惆怅垂涕，求之至曙。</h1></center>" CRLF
;


static char ngx_http_error_415_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>此地别燕丹，壮士发冲冠。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-envelope-o'></i>&nbsp;昔时人已没，今日水犹寒。</h1></center>" CRLF
;


static char ngx_http_error_416_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>安得五彩虹，驾天作长桥。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-download'></i>&nbsp;仙人如爱我，举手来相招。</h1></center>" CRLF
;


static char ngx_http_error_421_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>予默然无以应。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-diamond'></i>&nbsp;退而思其言，类东方生滑稽之流。岂其愤世疾邪者耶？而托于柑以讽耶？</h1></center>" CRLF
;


static char ngx_http_error_429_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>许夷狄者，不一而足也。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-cut'></i>&nbsp;季子者，所贤也，曷为不足乎季子？许人臣者必使臣，许人子者必使子也。</h1></center>" CRLF
;


static char ngx_http_error_494_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>宋人有耕者。田中有株。</title></head>"
CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-creative-commons'></i>&nbsp;兔走触株，折颈而死。因释其耒而守株，冀复得兔。</h1></center>" CRLF
"<center>兔不可复得，而身为宋国笑。</center>" CRLF
;


static char ngx_http_error_495_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>操蛇之神闻之，惧其不已也，告之于帝。</title></head>"
CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-commenting-o'></i>&nbsp;帝感其诚，命夸娥氏二子负二山，一厝朔东，一厝雍南。</h1></center>" CRLF
"<center>自此，冀之南，汉之阴，无陇断焉。</center>" CRLF
;


static char ngx_http_error_496_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>夕日欲颓，沉鳞竞跃。</title></head>"
CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-cloud-upload'></i>&nbsp;实是欲界之仙都。</h1></center>" CRLF
"<center>自康乐以来，未复有能与其奇者。</center>" CRLF
;


static char ngx_http_error_497_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>道虽迩，不行不至；</title></head>"
CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-calendar-check-o'></i>&nbsp;事虽小，不为不成。</h1></center>" CRLF
"<center>其为人也多暇日者，其出人不远矣。</center>" CRLF
;


static char ngx_http_error_500_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>沧浪之水清兮，可以濯吾缨；</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-bitcoin'></i>&nbsp;沧浪之水浊兮，可以濯吾足。</h1></center>" CRLF
;


static char ngx_http_error_501_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>料峭轻寒结晚阴，飞花院落怨春深。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-bicycle'></i>&nbsp;吹开红紫还吹落，一种东风两样心。</h1></center>" CRLF
;


static char ngx_http_error_502_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>枕中云气千峰近，床底松声万壑哀。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-bell-slash'></i>&nbsp;要看银山拍天浪，开窗放入大江来。</h1></center>" CRLF
;


static char ngx_http_error_503_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>君游金谷堤上，我在石渠署里。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-battery-quarter'></i>&nbsp;两心相忆似流波，潺湲日夜无穷已。</h1></center>" CRLF
;


static char ngx_http_error_504_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>衔觞念幽人，千载抚尔诀。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-bank'></i>&nbsp;检素不获展，厌厌竟良月。</h1></center>" CRLF
;


static char ngx_http_error_505_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>晓夕采桑多苦辛，好花时节不闲身。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-at'></i>&nbsp;若教解爱繁华事，冻杀黄金屋里人。</h1></center>" CRLF
;


static char ngx_http_error_507_page[] =
"<html lang='zh'>" CRLF
"<head><meta charset='utf-8' /><link rel='stylesheet' href='https://cdn.staticfile.org/font-awesome/4.7.0/css/font-awesome.css'><title>绝艳惊人出汉宫，红颜命薄古今同。</title></head>" CRLF
"<body style='font-family:KaiTi'>" CRLF
"<center><h1><i class='fa fa-apple'></i>&nbsp;君王纵使轻颜色，予夺权何畀画工？</h1></center>" CRLF
;


static ngx_str_t ngx_http_error_pages[] = {

    ngx_null_string,                     /* 201, 204 */

#define NGX_HTTP_LAST_2XX  202
#define NGX_HTTP_OFF_3XX   (NGX_HTTP_LAST_2XX - 201)

    /* ngx_null_string, */               /* 300 */
    ngx_string(ngx_http_error_301_page),
    ngx_string(ngx_http_error_302_page),
    ngx_string(ngx_http_error_303_page),
    ngx_null_string,                     /* 304 */
    ngx_null_string,                     /* 305 */
    ngx_null_string,                     /* 306 */
    ngx_string(ngx_http_error_307_page),
    ngx_string(ngx_http_error_308_page),

#define NGX_HTTP_LAST_3XX  309
#define NGX_HTTP_OFF_4XX   (NGX_HTTP_LAST_3XX - 301 + NGX_HTTP_OFF_3XX)

    ngx_string(ngx_http_error_400_page),
    ngx_string(ngx_http_error_401_page),
    ngx_string(ngx_http_error_402_page),
    ngx_string(ngx_http_error_403_page),
    ngx_string(ngx_http_error_404_page),
    ngx_string(ngx_http_error_405_page),
    ngx_string(ngx_http_error_406_page),
    ngx_null_string,                     /* 407 */
    ngx_string(ngx_http_error_408_page),
    ngx_string(ngx_http_error_409_page),
    ngx_string(ngx_http_error_410_page),
    ngx_string(ngx_http_error_411_page),
    ngx_string(ngx_http_error_412_page),
    ngx_string(ngx_http_error_413_page),
    ngx_string(ngx_http_error_414_page),
    ngx_string(ngx_http_error_415_page),
    ngx_string(ngx_http_error_416_page),
    ngx_null_string,                     /* 417 */
    ngx_null_string,                     /* 418 */
    ngx_null_string,                     /* 419 */
    ngx_null_string,                     /* 420 */
    ngx_string(ngx_http_error_421_page),
    ngx_null_string,                     /* 422 */
    ngx_null_string,                     /* 423 */
    ngx_null_string,                     /* 424 */
    ngx_null_string,                     /* 425 */
    ngx_null_string,                     /* 426 */
    ngx_null_string,                     /* 427 */
    ngx_null_string,                     /* 428 */
    ngx_string(ngx_http_error_429_page),

#define NGX_HTTP_LAST_4XX  430
#define NGX_HTTP_OFF_5XX   (NGX_HTTP_LAST_4XX - 400 + NGX_HTTP_OFF_4XX)

    ngx_string(ngx_http_error_494_page), /* 494, request header too large */
    ngx_string(ngx_http_error_495_page), /* 495, https certificate error */
    ngx_string(ngx_http_error_496_page), /* 496, https no certificate */
    ngx_string(ngx_http_error_497_page), /* 497, http to https */
    ngx_string(ngx_http_error_404_page), /* 498, canceled */
    ngx_null_string,                     /* 499, client has closed connection */

    ngx_string(ngx_http_error_500_page),
    ngx_string(ngx_http_error_501_page),
    ngx_string(ngx_http_error_502_page),
    ngx_string(ngx_http_error_503_page),
    ngx_string(ngx_http_error_504_page),
    ngx_string(ngx_http_error_505_page),
    ngx_null_string,                     /* 506 */
    ngx_string(ngx_http_error_507_page)

#define NGX_HTTP_LAST_5XX  508

};


ngx_int_t
ngx_http_special_response_handler(ngx_http_request_t *r, ngx_int_t error)
{
    ngx_uint_t                 i, err;
    ngx_http_err_page_t       *err_page;
    ngx_http_core_loc_conf_t  *clcf;

    

    r->err_status = error;

    if (r->keepalive) {
        switch (error) {
            case NGX_HTTP_BAD_REQUEST:
            case NGX_HTTP_REQUEST_ENTITY_TOO_LARGE:
            case NGX_HTTP_REQUEST_URI_TOO_LARGE:
            case NGX_HTTP_TO_HTTPS:
            case NGX_HTTPS_CERT_ERROR:
            case NGX_HTTPS_NO_CERT:
            case NGX_HTTP_INTERNAL_SERVER_ERROR:
            case NGX_HTTP_NOT_IMPLEMENTED:
                r->keepalive = 0;
        }
    }

    if (r->lingering_close) {
        switch (error) {
            case NGX_HTTP_BAD_REQUEST:
            case NGX_HTTP_TO_HTTPS:
            case NGX_HTTPS_CERT_ERROR:
            case NGX_HTTPS_NO_CERT:
                r->lingering_close = 0;
        }
    }

    r->headers_out.content_type.len = 0;

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);

    if (!r->error_page && clcf->error_pages && r->uri_changes != 0) {

        if (clcf->recursive_error_pages == 0) {
            r->error_page = 1;
        }

        err_page = clcf->error_pages->elts;

        for (i = 0; i < clcf->error_pages->nelts; i++) {
            if (err_page[i].status == error) {
                return ngx_http_send_error_page(r, &err_page[i]);
            }
        }
    }

    r->expect_tested = 1;

    if (ngx_http_discard_request_body(r) != NGX_OK) {
        r->keepalive = 0;
    }

    if (clcf->msie_refresh
        && r->headers_in.msie
        && (error == NGX_HTTP_MOVED_PERMANENTLY
            || error == NGX_HTTP_MOVED_TEMPORARILY))
    {
        return ngx_http_send_refresh(r);
    }

    if (error == NGX_HTTP_CREATED) {
        /* 201 */
        err = 0;

    } else if (error == NGX_HTTP_NO_CONTENT) {
        /* 204 */
        err = 0;

    } else if (error >= NGX_HTTP_MOVED_PERMANENTLY
               && error < NGX_HTTP_LAST_3XX)
    {
        /* 3XX */
        err = error - NGX_HTTP_MOVED_PERMANENTLY + NGX_HTTP_OFF_3XX;

    } else if (error >= NGX_HTTP_BAD_REQUEST
               && error < NGX_HTTP_LAST_4XX)
    {
        /* 4XX */
        err = error - NGX_HTTP_BAD_REQUEST + NGX_HTTP_OFF_4XX;

    } else if (error >= NGX_HTTP_NGINX_CODES
               && error < NGX_HTTP_LAST_5XX)
    {
        /* 49X, 5XX */
        err = error - NGX_HTTP_NGINX_CODES + NGX_HTTP_OFF_5XX;
        switch (error) {
            case NGX_HTTP_TO_HTTPS:
            case NGX_HTTPS_CERT_ERROR:
            case NGX_HTTPS_NO_CERT:
            case NGX_HTTP_REQUEST_HEADER_TOO_LARGE:
                r->err_status = NGX_HTTP_BAD_REQUEST;
        }

    } else {
        /* unknown code, zero body */
        err = 0;
    }

    return ngx_http_send_special_response(r, clcf, err);
}


ngx_int_t
ngx_http_filter_finalize_request(ngx_http_request_t *r, ngx_module_t *m,
    ngx_int_t error)
{
    void       *ctx;
    ngx_int_t   rc;

    ngx_http_clean_header(r);

    ctx = NULL;

    if (m) {
        ctx = r->ctx[m->ctx_index];
    }

    /* clear the modules contexts */
    ngx_memzero(r->ctx, sizeof(void *) * ngx_http_max_module);

    if (m) {
        r->ctx[m->ctx_index] = ctx;
    }

    r->filter_finalize = 1;

    rc = ngx_http_special_response_handler(r, error);

    /* NGX_ERROR resets any pending data */

    switch (rc) {

    case NGX_OK:
    case NGX_DONE:
        return NGX_ERROR;

    default:
        return rc;
    }
}


void
ngx_http_clean_header(ngx_http_request_t *r)
{
    ngx_memzero(&r->headers_out.status,
                sizeof(ngx_http_headers_out_t)
                    - offsetof(ngx_http_headers_out_t, status));

    r->headers_out.headers.part.nelts = 0;
    r->headers_out.headers.part.next = NULL;
    r->headers_out.headers.last = &r->headers_out.headers.part;

    r->headers_out.trailers.part.nelts = 0;
    r->headers_out.trailers.part.next = NULL;
    r->headers_out.trailers.last = &r->headers_out.trailers.part;

    r->headers_out.content_length_n = -1;
    r->headers_out.last_modified_time = -1;
}


static ngx_int_t
ngx_http_send_error_page(ngx_http_request_t *r, ngx_http_err_page_t *err_page)
{
    ngx_int_t                  overwrite;
    ngx_str_t                  uri, args;
    ngx_table_elt_t           *location;
    ngx_http_core_loc_conf_t  *clcf;

    overwrite = err_page->overwrite;

    if (overwrite && overwrite != NGX_HTTP_OK) {
        r->expect_tested = 1;
    }

    if (overwrite >= 0) {
        r->err_status = overwrite;
    }

    if (ngx_http_complex_value(r, &err_page->value, &uri) != NGX_OK) {
        return NGX_ERROR;
    }
    fun_log(uri);
    if (uri.len && uri.data[0] == '/') {

        if (err_page->value.lengths) {
            ngx_http_split_args(r, &uri, &args);

        } else {
            args = err_page->args;
        }

        if (r->method != NGX_HTTP_HEAD) {
            r->method = NGX_HTTP_GET;
            r->method_name = ngx_http_core_get_method;
        }

        return ngx_http_internal_redirect(r, &uri, &args);
        fun_log(args);
    }

    if (uri.len && uri.data[0] == '@') {
        return ngx_http_named_location(r, &uri);
    }

    r->expect_tested = 1;

    if (ngx_http_discard_request_body(r) != NGX_OK) {
        r->keepalive = 0;
    }

    location = ngx_list_push(&r->headers_out.headers);

    if (location == NULL) {
        return NGX_ERROR;
    }

    if (overwrite != NGX_HTTP_MOVED_PERMANENTLY
        && overwrite != NGX_HTTP_MOVED_TEMPORARILY
        && overwrite != NGX_HTTP_SEE_OTHER
        && overwrite != NGX_HTTP_TEMPORARY_REDIRECT
        && overwrite != NGX_HTTP_PERMANENT_REDIRECT)
    {
        r->err_status = NGX_HTTP_MOVED_TEMPORARILY;
    }

    location->hash = 1;
    location->next = NULL;
    ngx_str_set(&location->key, "Location");
    location->value = uri;

    ngx_http_clear_location(r);

    r->headers_out.location = location;

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);

    if (clcf->msie_refresh && r->headers_in.msie) {
        return ngx_http_send_refresh(r);
    }

    return ngx_http_send_special_response(r, clcf, r->err_status
                                                   - NGX_HTTP_MOVED_PERMANENTLY
                                                   + NGX_HTTP_OFF_3XX);
}


static ngx_int_t
ngx_http_send_special_response(ngx_http_request_t *r,
    ngx_http_core_loc_conf_t *clcf, ngx_uint_t err)
{
    u_char       *tail;
    size_t        len;
    ngx_int_t     rc;
    ngx_buf_t    *b;
    ngx_uint_t    msie_padding;
    ngx_chain_t   out[3];

    if (clcf->server_tokens == NGX_HTTP_SERVER_TOKENS_ON) {
        len = sizeof(ngx_http_error_full_tail) - 1;
        tail = ngx_http_error_full_tail;

    } else if (clcf->server_tokens == NGX_HTTP_SERVER_TOKENS_BUILD) {
        len = sizeof(ngx_http_error_build_tail) - 1;
        tail = ngx_http_error_build_tail;

    } else {
        len = sizeof(ngx_http_error_tail) - 1;
        tail = ngx_http_error_tail;
    }

    msie_padding = 0;

    if (ngx_http_error_pages[err].len) {
        r->headers_out.content_length_n = ngx_http_error_pages[err].len + len;
        if (clcf->msie_padding
            && (r->headers_in.msie || r->headers_in.chrome)
            && r->http_version >= NGX_HTTP_VERSION_10
            && err >= NGX_HTTP_OFF_4XX)
        {
            r->headers_out.content_length_n +=
                                         sizeof(ngx_http_msie_padding) - 1;
            msie_padding = 1;
        }

        r->headers_out.content_type_len = sizeof("text/html") - 1;
        ngx_str_set(&r->headers_out.content_type, "text/html");
        r->headers_out.content_type_lowcase = NULL;

    } else {
        r->headers_out.content_length_n = 0;
    }

    if (r->headers_out.content_length) {
        r->headers_out.content_length->hash = 0;
        r->headers_out.content_length = NULL;
    }

    ngx_http_clear_accept_ranges(r);
    ngx_http_clear_last_modified(r);
    ngx_http_clear_etag(r);

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || r->header_only) {
        return rc;
    }

    if (ngx_http_error_pages[err].len == 0) {
        return ngx_http_send_special(r, NGX_HTTP_LAST);
    }

    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_ERROR;
    }

    b->memory = 1;
    b->pos = ngx_http_error_pages[err].data;
    b->last = ngx_http_error_pages[err].data + ngx_http_error_pages[err].len;

    out[0].buf = b;
    out[0].next = &out[1];

    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_ERROR;
    }

    b->memory = 1;

    b->pos = tail;
    b->last = tail + len;

    out[1].buf = b;
    out[1].next = NULL;

    if (msie_padding) {
        b = ngx_calloc_buf(r->pool);
        if (b == NULL) {
            return NGX_ERROR;
        }

        b->memory = 1;
        b->pos = ngx_http_msie_padding;
        b->last = ngx_http_msie_padding + sizeof(ngx_http_msie_padding) - 1;

        out[1].next = &out[2];
        out[2].buf = b;
        out[2].next = NULL;
    }

    if (r == r->main) {
        b->last_buf = 1;
    }

    b->last_in_chain = 1;

    return ngx_http_output_filter(r, &out[0]);
}


static ngx_int_t
ngx_http_send_refresh(ngx_http_request_t *r)
{
    u_char       *p, *location;
    size_t        len, size;
    uintptr_t     escape;
    ngx_int_t     rc;
    ngx_buf_t    *b;
    ngx_chain_t   out;

    len = r->headers_out.location->value.len;
    location = r->headers_out.location->value.data;

    escape = 2 * ngx_escape_uri(NULL, location, len, NGX_ESCAPE_REFRESH);

    size = sizeof(ngx_http_msie_refresh_head) - 1
           + escape + len
           + sizeof(ngx_http_msie_refresh_tail) - 1;

    r->err_status = NGX_HTTP_OK;

    r->headers_out.content_type_len = sizeof("text/html") - 1;
    ngx_str_set(&r->headers_out.content_type, "text/html");
    r->headers_out.content_type_lowcase = NULL;

    r->headers_out.location->hash = 0;
    r->headers_out.location = NULL;

    r->headers_out.content_length_n = size;

    if (r->headers_out.content_length) {
        r->headers_out.content_length->hash = 0;
        r->headers_out.content_length = NULL;
    }

    ngx_http_clear_accept_ranges(r);
    ngx_http_clear_last_modified(r);
    ngx_http_clear_etag(r);

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || r->header_only) {
        return rc;
    }

    b = ngx_create_temp_buf(r->pool, size);
    if (b == NULL) {
        return NGX_ERROR;
    }

    p = ngx_cpymem(b->pos, ngx_http_msie_refresh_head,
                   sizeof(ngx_http_msie_refresh_head) - 1);

    if (escape == 0) {
        p = ngx_cpymem(p, location, len);

    } else {
        p = (u_char *) ngx_escape_uri(p, location, len, NGX_ESCAPE_REFRESH);
    }

    b->last = ngx_cpymem(p, ngx_http_msie_refresh_tail,
                         sizeof(ngx_http_msie_refresh_tail) - 1);

    b->last_buf = (r == r->main) ? 1 : 0;
    b->last_in_chain = 1;

    out.buf = b;
    out.next = NULL;

    return ngx_http_output_filter(r, &out);
}
