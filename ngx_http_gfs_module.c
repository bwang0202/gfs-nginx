#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#define DEFAULT_BATCH 4
#define DEFAULT_CHUNKSIZE 64 * 1024 * 1024
#define DEFAULT_CSID "default_csid"
#define DEFAULT_ROOTDIR "/tmp/"

typedef struct {
    ngx_str_t csid;
    size_t chunksize;
    ngx_uint_t max_batch;
    ngx_str_t root_dir;
} ngx_http_gfs_loc_conf_t;

// install module and handler
static char* ngx_http_gfs(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// malloc 
static void* ngx_http_gfs_create_loc_conf(ngx_conf_t *cf);

static char* ngx_http_gfs_merge_loc_conf(ngx_conf_t *cf,
    void *parent, void *child);

static ngx_int_t
ngx_http_gfs_handler(ngx_http_request_t *r)
{
    ngx_buf_t *b;
    ngx_chain_t out;

    ngx_http_gfs_loc_conf_t  *gfslcf;
    gfslcf = ngx_http_get_module_loc_conf(r, ngx_http_gfs_module);

    // TODO: process r
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, 
        "Failed to allocate response buffer.");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    char* read = ngx_pcalloc(r->pool, gfslcf->chunksize);
    read[1] = '1';
    b->pos = read;
    b->last = read + gfslcf->chunksize;

    b->memory = 1; /* content is in read-only memory */
    /* (i.e., filters should copy it rather than rewrite in place) */

    b->last_buf = 1; /* there will be no more buffers in the request */

    out.buf = b;
    out.next = NULL;
    return ngx_http_output_filter(r, &out);
}

// module directives
static ngx_command_t  ngx_http_gfs_commands[] = {
    {   ngx_string("gfs"),
        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
        ngx_http_gfs,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL },
    {
        ngx_string("csid"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_gfs_loc_conf_t, csid),
        NULL },
    {   ngx_string("chunksize"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_size_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_gfs_loc_conf_t, chunksize),
        NULL },
    {   ngx_string("max_batch"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_gfs_loc_conf_t, chunksize),
        NULL },
    {
        ngx_string("root_dir"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_gfs_loc_conf_t, root_dir),
        NULL },
    ngx_null_command
};

// module context
static ngx_http_module_t  ngx_http_gfs_module_ctx = {
    NULL,                          /* preconfiguration */
    NULL,                          /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */

    ngx_http_gfs_create_loc_conf,  /* create location configuration */
    ngx_http_gfs_merge_loc_conf /* merge location configuration */
};

// module definition
ngx_module_t  ngx_http_gfs_module = {
    NGX_MODULE_V1,
    &ngx_http_gfs_module_ctx, /* module context */
    ngx_http_gfs_commands,   /* module directives */
    NGX_HTTP_MODULE,               /* module type */
    NULL,                          /* init master */
    NULL,                          /* init module */
    NULL,                          /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    NULL,                          /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING
};

static char* ngx_http_gfs(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_gfs_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_gfs_handler;

    return NGX_CONF_OK;
}

static void *
ngx_http_gfs_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_gfs_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_gfs_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    conf->chunksize = NGX_CONF_UNSET_SIZE;
    conf->max_batch = NGX_CONF_UNSET_UINT;
    // conf->csid = NGX_CONF_UNSET;
    // conf->root_dir = NGX_CONF_UNSET;
    return conf;
}

static char* ngx_http_gfs_merge_loc_conf(ngx_conf_t *cf,
    void *parent, void *child)
{
    ngx_http_gfs_loc_conf_t *prev = parent;
    ngx_http_gfs_loc_conf_t *conf = child;

    ngx_conf_merge_size_value(conf->chunksize, prev->chunksize, DEFAULT_CHUNKSIZE);
    ngx_conf_merge_uint_value(conf->max_batch, prev->max_batch, DEFAULT_BATCH);
    ngx_conf_merge_str_value(conf->csid, prev->csid, DEFAULT_CSID);
    ngx_conf_merge_str_value(conf->root_dir, prev->root_dir, DEFAULT_ROOTDIR);

    if (conf->max_batch < 1) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, 
            "max_batch must be equal or more than 1");
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}