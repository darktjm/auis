char *fakedom_ReplaceAddr();
#define fakedom_RewriteAddrs(buf) rewriteAddrs(buf,fakedom_ReplaceAddr)
