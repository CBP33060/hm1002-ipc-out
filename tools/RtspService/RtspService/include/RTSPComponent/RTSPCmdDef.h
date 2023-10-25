#ifndef __CR_RTSPCMDDEF_H__
#define __CR_RTSPCMDDEF_H__

const char RTSP_SDP_VIDEO[] = "\
m=video 0 RTP/AVP 158\r\n\
a=control:trackID=%d\r\n\
a=framerate:%d\r\n\
a=rtpmap:158 H264/90000\r\n\
a=fmtp:158 packetization-mode=1;profile-level-id=4D0020;sprop-parameter-sets=%s,%s\r\n";

const char RTSP_SDP_VIDEO1[] = "\
m=video 0 RTP/AVP 96\r\n\
c=IN IP4 0.0.0.0\r\n\
a=rtpmap:96 H265/90000\r\n\
a=fmtp:96 packetization-mode=1;\r\n\
a=control:trackID=0\r\n\r\n";

const char RTSP_SDP_VIDEO2[] = "\
m=video 0 RTP/AVP 100\r\n\
a=rtpmap:100 H265/90000\r\n\
a=fmtp:100 packetization-mode=1;profile-level-id=4D0020;sprop-parameter-sets=%s,%s\r\n\
a=recvonly\r\n\r\n";

const char RTSP_SDP_AUDIO[] = "\
m=audio 0 RTP/AVP %d\r\n\
a=control:trackID=%d\r\n\
a=rtpmap:%d %s/8000\r\n";


// const char OPTIONS_RSP[] = "\
// RTSP/1.0 200 OK\r\n\
// CSeq: %d\r\n\
// Server: Crearo Rtsp Server\r\n\
// Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER\r\n\r\n";
const char OPTIONS_RSP[] = "\
RTSP/1.0 200 OK\r\n\
CSeq: %d\r\n\
Server: Ingenic Rtsp Server\r\n\
Public: OPTIONS, DESCRIBE, PLAY, PAUSE, SETUP, TEARDOWN\r\n\r\n";

const char DESCRIBE_OK[] = "\
RTSP/1.0 200 OK\r\n\
CSeq: %d\r\n\
Content-Base: %s\r\n\
Content-Type: application/sdp\r\n\
Content-Length: %d\r\n\r\n%s";

const char RTSP_SDP_HEAD[] = "\
v=0\r\n\
o=- %s %s IN IP4 %s\r\n\
s=RTSP Session of HeFei Ingenic Technology CO.,LTD.\r\n\
c=IN IP4 %s\r\n\
t=0 0\r\n\
a=control:*\r\n\
a=range:npt=%s-%s\r\n\
a=packetization-supported:DH\r\n";

// const char RTSP_SDP_HEAD1[] = "\
// v=0\r\n\
// o=- %s %s IN IP4 %s\r\n\
// s=Media Presentation\r\n\
// o=- %s %s IN IP4 %s\r\n\
// a=range:npt=%s-%s\r\n";
const char RTSP_SDP_HEAD1[] = "\
v=0\r\n\
o=- %s %s IN IP4 %s\r\n\
t=0 0\r\n\
a=control:*\r\n\
a=type:broadcast\r\n";

const char RTSP_SETUP[] = "\
RTSP/1.0 200 OK\r\n\
CSeq: %d\r\n\
Session: %s;timeout=60\r\n\
Transport: RTP/AVP;unicast;destination=%s;client_port=%d-%d;source=%s;server_port=%d-%d;mode=\"play\"\r\n\r\n";

const char RTSP_SETUP_TCP[] = "\
RTSP/1.0 200 OK\r\n\
CSeq: %d\r\n\
Session: %s;timeout=60\r\n\
Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n";

const char RTSP_TEARDOWN[] = "\
RTSP/1.0 200 OK\r\n\
CSeq: %d\r\n\
Session: %s\r\n\r\n";

const char RTSP_PLAY[] = "\
RTSP/1.0 200 OK\r\n\
CSeq: %d\r\n\
Range:%s\r\n\
Session: %s; timeout=60\r\n\r\n";


const char RTSP_PLAY2[] = "\
RTSP/1.0 200 OK\r\n\
CSeq: %d\r\n\
Scale: %s\r\n\
Session: %s\r\n\
RTP-Info: url=%s;seq=0;rtptime=0\r\n\
Date: %s\r\n\r\n";

const char RTSP_PLAY3[] = "\
RTSP/1.0 200 OK\r\n\
CSeq: %d\r\n\
Session: %s\r\n\
Range: npt=%s-%s\r\n\
RTP-Info: url=%s;seq=0;rtptime=0\r\n\
Date: %s\r\n\r\n";

const char RTSP_PLAY4[] = "\
RTSP/1.0 200 OK\r\n\
CSeq: %d\r\n\
Session: %s\r\n\
Range: npt=0.000-\r\n\
RTP-Info: url=%s;seq=0;rtptime=0\r\n\
Date: %s\r\n\r\n";

const char RTSP_PLAY5[] = "\
RTSP/1.0 200 OK\r\n\
CSeq: %d\r\n\
Session: %s\r\n\
Range: clock==%s-%s\r\n\
RTP-Info: url=%s;seq=0;rtptime=0\r\n\
Date: %s\r\n\r\n";

const char RTSP_PAUSE[] = "\
RTSP/1.0 200 OK\r\n\
CSeq: %d\r\n\
Session: %s\r\n\r\n";

const char RTSP_ANNOUNCE[] = "\
ANNOUNCE %s RTSP/1.0\r\n\
CSeq: %d\r\n\
Session: %s\r\n\
Event-Type: 2001 End-Of-Stream\r\n\r\n";

const char RTSP_GET_PARAMETER[] = "\
RTSP/1.0 200 OK\r\n\
CSeq: %d\r\n\
Session: %s\r\n\
Date: %s\r\n\r\n";

#endif/*__CR_RTSPCMDDEF_H__*/

