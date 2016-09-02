#include <andrewos.h>
#include <avalue.H>
#include <aaction.H>

#define DEFINE_MULTI_AVALUEARGS(type,args)  \
class type : public avalueargs { \
  public: \
type() : avalueargs(){ \
args; \
    } \
};

class A {
  public:
    void foo(const avalueflex &in,avalueflex &out) {
	printf("blah..%d\n", in[0].Integer());
	printf("blah..%s\n", in[0].CString());
    }
};
#if 0
DEFINE_AACTION_CLASS(textv,A);

textv_action blah[]={
    textv_action(&A::foo,"Blah","A","Dummy Function"),
    textv_action(&A::foo,9L,"A","Dummy Function"),
};

#define ATKNEWOBJECT(c,b) ((b *)(ATK::IsTypeByName(Stringize(c),Stringize(b))?ATK::NewObject(Stringize(c)):NULL))
#endif
int main() {
    printf("main\n");
    ATKregister(aaction);
    ATKregister(objectaction);
 //   ATKregister(textv);
   ATKregister(nestedmark);
#if 0
   textv *p=ATKNEWOBJECT(textv,textv);
    A v;
    p->Set(&v,&blah[0]);
    avalueargs out;
    (*p)(*p->act[0][0].List(),out);
#endif
}
