char *falias_Alias(), *falias_Unalias();
void falias_Read();
char *rewriteAddrs();

#define falias_RewriteAddrs(buf) rewriteAddrs(buf,falias_Unalias)
