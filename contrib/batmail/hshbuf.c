#include "com.h"
#include "hshbuf.h"

static long hashBuf(buf,len)
char *buf;
int len;
{
	unsigned long hv = 0, ov;

	while(len-->0) {
		hv = (hv<<5)+(*buf++);
		ov = hv&0xf0000000;
		if (ov != 0) {
			hv ^= (ov>>27);
			hv ^= ov;
		}
	}
	return hv;
}

	struct hshbuf *
hshbuf_CreateMax(hb,buf,len,buflen)
	struct hshbuf *hb;
	char *buf;
	int len;
{
	if(hb==NULL)
		hb=(struct hshbuf *)
				malloc(sizeof(struct hshbuf)+buflen-1);
	hb->len=len;
	hb->hashVal=hashBuf(buf, len);
	memcpy(hb->buf, buf, len);
	return hb;
}

	struct hshbuf *
hshbuf_Create(buf,len)
	char *buf;
	int len;
{
	return hshbuf_CreateMax(NULL, buf, len, len);
}

	struct hshbuf *
hshbuf_Copy(hb)
	struct hshbuf *hb;
{
	return hshbuf_CreateMax(NULL, hb->buf, 
			hb->len, hb->len);
}
