/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("rm.H")
#include <rm.H>
#include <namespace.H> 
#include <stdio.h>

#define DEBUG_RM 1
static int debug_rm = 0;

struct resourceTree_s
{
  class Namespace * data;
  class Namespace * children;
};

static struct resourceTree_s * resourceTree;
static class Namespace * converters;


/****************************************************************/
/*		debugging code					*/
/****************************************************************/

ATKdefineRegistry(rm, ATK, rm::InitializeClass);
#if DEBUG_RM
void Patom( const class atom  * atom );
void Plist( class atomlist  * list );
#endif
int FindResource( class atomlist  * name, struct atoms  * namecar, class atomlist  * class_c, struct atoms  * classcar, struct resourceTree_s  ** tree );
int FindManyResources( struct resourceList  * resources, class atomlist  * name, struct atoms  * namecar, class atomlist  * class_c, struct atoms  * classcar, struct resourceTree_s  ** tree );
int Convertp( class Namespace  * toconverters, class Namespace  * Namespace, int  index );
short TryConversion( struct resourceTree_s  * tree, const class atom  * type, long  * data );
static struct  resourceTree_s * FindNodeCreate( class atomlist  * path, struct resourceTree_s  * tree );
void PostResourceAt( struct resourceTree_s  * root, class atomlist  * path, long  data, const class atom  * type );

#if DEBUG_RM
void Patom( const class atom  * atom )
     {
  printf("%s",(atom)->Name());
}

void Plist( class atomlist  * list )
     {
  struct atoms * current = (list)->TraversalStart();
  if (current != NULL)
    {
      Patom((list)->TraversalAtom(current));
      for ( ; current != NULL; current = (list)->TraversalNext(current))
	{
	  putchar('.');
	  Patom((list)->TraversalAtom(current));
	}
      fflush(stdout);
    }      
}
#endif /* DEBUG_RM */

/****************************************************************/
/*		private functions				*/
/****************************************************************/
int FindResource( class atomlist  * name, struct atoms  * namecar, class atomlist  * class_c, struct atoms  * classcar, struct resourceTree_s  ** tree )
                         {
  short found;
  struct resourceTree_s * nexttree;
  struct atoms * namecdr;
  struct atoms * classcdr;
  
 tail_recursion_can_be_fun:
  
  namecdr = (name)->TraversalNext(  namecar );
  classcdr = (class_c)->TraversalNext(  classcar );
  
  if (*tree == NULL)
    found = FALSE;
  if (namecar == NULL && classcar == NULL)
    found = (*tree)->data != NULL;
  else
    {
      if (((*tree)->children)->Boundp( 
			   (name)->TraversalAtom( namecar),
			   (long *) &nexttree ) &&
	  FindResource( name, namecdr, class_c, classcdr, &nexttree ))
	{
#if DEBUG_RM
	  if (debug_rm)
	    {
	      printf("Found at name: %p ",namecar);
	      Patom((name)->TraversalAtom(namecar));
	      printf("\n");
	    }
#endif /* DEBUG_RM */
	  *tree = nexttree;
	  found = TRUE;
	}
      else
	{
	  if ((class_c)->TraversalAtom(classcar)
	      != (name)->TraversalAtom(namecar)
	      && ((*tree)->children)->Boundp( 
				  (class_c)->TraversalAtom( classcar),
				  (long *) &nexttree ) 
	      && FindResource(name, namecdr, class_c, classcdr, &nexttree ))
	    {
#if DEBUG_RM
	      if (debug_rm)
		{
		  printf("Found at class: %p ",classcar);
		  Patom((class_c)->TraversalAtom(classcar));
		  printf("\n");
		}
#endif /* DEBUG_RM */
	      *tree = nexttree;
	      found = TRUE;
	    }
	  else
	    {
	      const class atom * nameatom;
	      const class atom * classatom;
#if DEBUG_RM
	      if (debug_rm)
		{
		  printf("discarded at name: %p ",namecar);
		  Patom((name)->TraversalAtom(namecar));
		  printf("\n");
		}
#endif /* DEBUG_RM */
	      /* this optimization should win */
	      nameatom = (name)->TraversalAtom(namecar);
	      classatom = (class_c)->TraversalAtom(classcar);
	      while (nameatom == (name)->TraversalAtom(namecdr)
		     && classatom == (class_c)->TraversalAtom(classcdr))
		{
		  namecdr   = (name)->TraversalNext(namecdr);
		  classcdr  = (class_c)->TraversalNext(classcdr);
		}
	      namecar = namecdr;
	      classcar = classcdr;
	      goto tail_recursion_can_be_fun;
	    }
	}
    }
  
  return found;
}



int FindManyResources( struct resourceList  * resources, class atomlist  * name, struct atoms  * namecar, class atomlist  * class_c, struct atoms  * classcar, struct resourceTree_s  ** tree )
                              {
  short foundAll;
  struct resourceTree_s * nexttree;
  struct atoms * namecdr;
  struct atoms * classcdr;
  struct resourceTree_s * t;
  int x;
  
 tail_recursion_can_be_fun:
  
  namecdr = (name)->TraversalNext(  namecar );
  classcdr = (class_c)->TraversalNext(  classcar );
  
  if (*tree == NULL)
    foundAll = FALSE;
  if (namecar == NULL && classcar == NULL)
    {
      foundAll = TRUE;
      for (x = 0; resources[x].name != NULL; ++x)
	{
	  if (resources[x].found)
	    continue;
	  t = *tree;
	  namecar = (resources[x].name)->TraversalStart();
	  if (!FindResource( resources[x].name, namecar, resources[x].name,
			    namecar, &t ))
	    foundAll = FALSE;
	  else
	    {
	      resources[x].found = TRUE;
	      resources[x].data  = (long)t;
	    }
	}
    }
  else
    {
      if (((*tree)->children)->Boundp( 
			   (name)->TraversalAtom( namecar),
			   (long *) &nexttree ) &&
	  FindManyResources( resources, name, namecdr, class_c, classcdr, &nexttree ))
	{
#if DEBUG_RM
	  if (debug_rm)
	    {
	      printf("Found at name: %p ",namecar);
	      Patom((name)->TraversalAtom(namecar));
	      printf("\n");
	    }
#endif /* DEBUG_RM */
	  *tree = nexttree;
	  foundAll = TRUE;
	}
      else
	{
	  if ((class_c)->TraversalAtom(classcar)
	      != (name)->TraversalAtom(namecar)
	      && ((*tree)->children)->Boundp( 
				  (class_c)->TraversalAtom( classcar),
				  (long *) &nexttree ) 
	      && FindManyResources(resources, name, namecdr, class_c,
				  classcdr, &nexttree ))
	    {
#if DEBUG_RM
	      if (debug_rm)
		{
		  printf("Found at class: %p ",classcar);
		  Patom((class_c)->TraversalAtom(classcar));
		  printf("\n");
		}
#endif /* DEBUG_RM */
	      *tree = nexttree;
	      foundAll = TRUE;
	    }
	  else
	    {
	      const class atom * nameatom;
	      const class atom * classatom;
#if DEBUG_RM
	      if (debug_rm)
		{
		  printf("discarded at name: %p ",namecar);
		  Patom((name)->TraversalAtom(namecar));
		  printf("\n");
		}
#endif /* DEBUG_RM */
	      /* this optimization should win */
	      nameatom = (name)->TraversalAtom(namecar);
	      classatom = (class_c)->TraversalAtom(classcar);
	      while (nameatom == (name)->TraversalAtom(namecdr)
		     && classatom == (class_c)->TraversalAtom(classcdr))
		{
		  namecdr   = (name)->TraversalNext(namecdr);
		  classcdr  = (class_c)->TraversalNext(classcdr);
		}
	      namecar = namecdr;
	      classcar = classcdr;
	      goto tail_recursion_can_be_fun;
	    }
	}
    }
  
  return foundAll;
  /* slick, eh? */
}





boolean Convertp( class Namespace  * toconverters, class Namespace  * Namespace, int  index )
               {
  return !(toconverters)->Boundp( 
			   (Namespace)->NameAt(index),
			   NULL );
}


short TryConversion( struct resourceTree_s  * tree, const class atom  * type, long  * data )
               {
  int fromtype;
  class Namespace * toconverters;
  rm_fptr converter;
  short gotgooddata = FALSE;

  if (tree->data != NULL) {
    if ((tree->data)->Boundp(  type, data ))
      gotgooddata = TRUE;
    else
      if ((converters)->Boundp(  type, (long *) &toconverters ) &&
	  ((fromtype = (tree->data)->Enumerate( (namespace_efptr)Convertp, (long) toconverters)) >= 0))
	{
	  converter =
	    (rm_fptr)(toconverters)->GetValue( 
					  (tree->data)->NameAt(fromtype) );
	  gotgooddata =  (*converter)((tree->data)->ValueAt(fromtype),data);
	}
  }
  return gotgooddata;
}


/****************************************************************/
/*		class procedures				*/
/****************************************************************/
boolean rm::InitializeClass( )
     {
  resourceTree = (struct resourceTree_s *)malloc(sizeof( struct resourceTree_s ));
  resourceTree->data = NULL;
  resourceTree->children = new Namespace;
  converters = new Namespace;
  return TRUE;
}

void rm::PostConverter( const class atom  * from, const class atom  * to, rm_fptr  converter )
                    {
	ATKinit;

  class Namespace * toconverters;

  if (!(converters)->Boundp(  to, (long *) &toconverters ))
    {
      toconverters = new Namespace;
      (converters)->SetValue(  to, (long) toconverters );
    }

  (toconverters)->SetValue(  from, (long) converter );
}



static struct  resourceTree_s * FindNodeCreate( class atomlist  * path, struct resourceTree_s  * tree )
          {
  struct atoms * car = (path )->TraversalStart( );

  while (car != NULL)
    {
      if (!(tree->children)->Boundp( 
			    (path)->TraversalAtom(car), (long *) &tree ))
	{
	  struct resourceTree_s * newTree;
	  newTree = (struct resourceTree_s *)malloc(sizeof( struct resourceTree_s ));
	  newTree->data = NULL;
	  newTree->children = new Namespace;
	  (tree->children)->SetValue( 
			     (path)->TraversalAtom(car),
			     (long) newTree );
	  tree = newTree;
	}
      car = (path)->TraversalNext(  car );
    }
  return tree;
}


void PostResourceAt( struct resourceTree_s  * root, class atomlist  * path, long  data, const class atom  * type )
                    {
  struct resourceTree_s * tree = FindNodeCreate( path, root );

  if (tree->data != NULL)
    (tree->data )->Clear( );
  else
    tree->data = new Namespace;
  (tree->data)->SetValue(  type, data );
}


void rm::PostManyResources( struct resourceList  * resources, class atomlist  * context )
               {
	ATKinit;

  int x;
  struct resourceTree_s * tree = FindNodeCreate(context, resourceTree);

  for (x = 0;
       resources[x].name != NULL;
       ++x)
    PostResourceAt( tree, resources[x].name, resources[x].data,
		   resources[x].type );
}


void rm::PostResource( class atomlist  * path, long  data, const class atom  * type )
                    {
	ATKinit;

#if DEBUG_RM
  if (debug_rm)
    {
      printf("Posting resource: ");
      Plist( path );
      printf("\nType: %p ",type);
      Patom(type);
      putchar('\n');
    }
#endif /* DEBUG_RM */
  PostResourceAt( resourceTree, path, data, type );
}


/* name and class had better be the same length! */
short rm::GetResource( class atomlist  * name, class atomlist  * class_c, const class atom  * type, long  * data )
                         {
	ATKinit;

  struct resourceTree_s * tree = resourceTree;
  int x;
  int y = 0;

#if DEBUG_RM
  if (debug_rm)
    {
      printf("Seeking resource: ");
      Plist( name );
      printf("\nClass: ");
      Plist(class_c);
      printf("\nType: %p ",type);
      Patom(type);
      printf("\n");
    }
#endif /* DEBUG_RM */

  x = FindResource( name, (name)->TraversalStart(),
		   class_c, (class_c)->TraversalStart(),
		   &tree );
#if DEBUG_RM
  if (debug_rm)
    {
      if (x)
	printf("Found such a resource %p\n", tree);
      else
	printf("no such resource\n");
    }
#endif /* DEBUG_RM */

  if (x) y  = TryConversion( tree, type, data );

#if DEBUG_RM
  if (debug_rm)
    {
      if (x && y)
	printf("Conversion succeeded\n");
      else
	printf("Conversion failed\n");
    }
#endif /* DEBUG_RM */

  return x && y;
}


void rm::GetManyResources( struct resourceList * resources, class atomlist  * name, class atomlist  * class_c)
                    {
	ATKinit;

  int x;
  struct resourceTree_s * tree = resourceTree;

  for (x = 0; resources[x].name != NULL; ++x)
    resources[x].found = 0;

  (void)FindManyResources( resources, name, (name)->TraversalStart(),
		   class_c, (class_c)->TraversalStart(),
		   &tree );

  for (x = 0; resources[x].name != NULL; ++x)
    if (resources[x].found)
      {
	resources[x].found =
	  TryConversion( (struct resourceTree_s *)resources[x].data,
			resources[x].type,
			&resources[x].data );
      }

}



void
rm::ContextualPostResource( class atomlist  * context, class atomlist  * path, long  data, const class atom  * type )
                         {
	ATKinit;

  struct atoms * m;
  m = (path)->Mark();
  if (context != NULL)
    (path)->JoinToBeginning(context);
  rm::PostResource(path,data,type);
  (path)->Cut(m);
}
