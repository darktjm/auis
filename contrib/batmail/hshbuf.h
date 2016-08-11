#define MAXHSHBUF 100

struct hshbuf {
    long hashVal;
    int len;
    char buf[1];
};

#define hshbuf_FILL(hb,buf,l) \
  ((hb)=hshbuf_CreateMax(hb,buf,l,MAXHSHBUF),(hb))

#define hshbuf_EQ(hb1,hb2) \
  ((hb1)->hashVal==(hb2)->hashVal && \
   (hb1)->len==(hb2)->len && \
   bcmp((hb1)->buf,(hb2)->buf,(hb1)->len)==0)

#define hshbuf_Len(hb) ((hb)->len)
#define hshbuf_Buf(hb) ((hb)->buf)

struct hshbuf *hshbuf_CreateMax(), *hshbuf_Copy(), *hshbuf_Create();

