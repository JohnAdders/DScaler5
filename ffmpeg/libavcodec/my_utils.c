#include "avcodec.h"

av_malloc_fc av_malloc=NULL;
av_free_fc av_free=NULL;
av_realloc_fc av_realloc=NULL;

void av_set_memory(av_malloc_fc mal,av_free_fc fre,av_realloc_fc rel)
{
 av_malloc=mal;
 av_free=fre;
 av_realloc=rel;
}

static char av_datetime[]=__DATE__" "__TIME__;
void getVersion(char **version,char **build,char **datetime)
{
 if (version) *version=LIBAVCODEC_VERSION;
 if (build) *build=AV_STRINGIFY(LIBAVCODEC_BUILD);
 if (datetime) *datetime=av_datetime;
}
