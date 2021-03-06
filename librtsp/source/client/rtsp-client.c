#include "rtsp-client.h"
#include "rtsp-client-internal.h"
#include "rtsp-parser.h"
#include "sdp.h"
#include <stdlib.h>
#include <assert.h>

void* rtsp_client_create(const struct rtsp_client_handler_t *handler, void* param)
{
	struct rtsp_client_t *rtsp;
	rtsp = (struct rtsp_client_t*)malloc(sizeof(*rtsp));
	if(NULL == rtsp)
		return NULL;

	memset(rtsp, 0, sizeof(*rtsp));
	memcpy(&rtsp->handler, handler, sizeof(rtsp->handler));
	rtsp->state = RTSP_INIT;
	rtsp->param = param;
	rtsp->cseq = 1;
	return rtsp;
}

void rtsp_client_destroy(void* p)
{
	struct rtsp_client_t *rtsp;
	rtsp = (struct rtsp_client_t*)p;

	if(rtsp->media_ptr)
		free(rtsp->media_ptr);

	free(rtsp);
}

int rtsp_client_open(void* p, const char* uri, const char* sdp)
{
	int r;
	struct rtsp_client_t *rtsp;
	rtsp = (struct rtsp_client_t*)p;

	strlcpy(rtsp->uri, uri, sizeof(rtsp->uri));
	if(NULL == sdp || 0 == *sdp)
		r = rtsp_client_describe(rtsp);
	else
		r = rtsp_client_setup(rtsp, sdp);
	return r;
}

int rtsp_client_close(void* p)
{
	struct rtsp_client_t *rtsp;
	rtsp = (struct rtsp_client_t*)p;

	assert(RTSP_TEARDWON != rtsp->state);
	return rtsp_client_teardown(rtsp);
}

int rtsp_client_input(void* p, void* parser)
{
	struct rtsp_client_t *rtsp;
	rtsp = (struct rtsp_client_t*)p;

	switch (rtsp->state)
	{
	case RTSP_ANNOUNCE:	return rtsp_client_announce_onreply(rtsp, parser);
	case RTSP_DESCRIBE: return rtsp_client_describe_onreply(rtsp, parser);
	case RTSP_SETUP:	return rtsp_client_setup_onreply(rtsp, parser);
	case RTSP_PLAY:		return rtsp_client_play_onreply(rtsp, parser);
	case RTSP_PAUSE:	return rtsp_client_pause_onreply(rtsp, parser);
	case RTSP_TEARDWON: return rtsp_client_teardown_onreply(rtsp, parser);
	case RTSP_OPTIONS:	return rtsp_client_options_onreply(rtsp, parser);
	case RTSP_GET_PARAMETER: return rtsp_client_get_parameter_onreply(rtsp, parser);
	case RTSP_SET_PARAMETER: return rtsp_client_set_parameter_onreply(rtsp, parser);
	default: assert(0); return -1;
	}
}

int rtsp_client_media_count(void* p)
{
	struct rtsp_client_t *rtsp;
	rtsp = (struct rtsp_client_t*)p;
	return rtsp->media_count;
}

const struct rtsp_header_transport_t* rtmp_client_get_media_transport(void* p, int media)
{
	struct rtsp_client_t *rtsp;
	rtsp = (struct rtsp_client_t*)p;
	if(media < 0 || media >= rtsp->media_count)
		return NULL;
	return &rtsp_get_media(rtsp, media)->transport;
}

const char* rtsp_client_get_media_encoding(void* p, int media)
{
	struct rtsp_client_t *rtsp;
	rtsp = (struct rtsp_client_t*)p;
	if (media < 0 || media >= rtsp->media_count)
		return NULL;
	return rtsp_get_media(rtsp, media)->avformats[0].encoding;
}

int rtsp_client_get_media_payload(void* p, int media)
{
	struct rtsp_client_t *rtsp;
	rtsp = (struct rtsp_client_t*)p;
	if (media < 0 || media >= rtsp->media_count)
		return -1;
	return rtsp_get_media(rtsp, media)->avformats[0].fmt;
}
