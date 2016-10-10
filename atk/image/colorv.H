#ifndef _colorv_H_
#define _colorv_H_
class colorv : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


  
	static boolean InitializeClass();
	static boolean atk_inited;

  
	colorv();
  
	virtual ~colorv();

   void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
   void Update();
};


#endif /* _colorv_H_ */
