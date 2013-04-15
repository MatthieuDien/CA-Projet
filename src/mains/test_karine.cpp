#include <iostream>
#include <OPLabel.h>
#include <Program.h>
#include <OPExpression.h>
#include <OPRegister.h>
#include <OPImmediate.h>
#include <Cfg.h>

/* 
parsing d'un fichier assembleur 
affichage du programme
calcul des fonction, affichage du nombre de fonction
EXEMPLE DE TEST des méthodes que vous devez coder
*/

int main(int argc, char ** argv){
  if (argc < 2){
    cerr << "pas de fichier assembleur en paramètre, pb !" << endl;
    return -1;
  }
  Program p2(argv[1]) ;
  p2.display() ;

 

  /* Test DU CALCUL DES FONCTIONS */
   /* Calcul des fonctions contenues dans le fichier parsé */

  
  p2.comput_function();
  cout << "dans le fichier " << argv[1]  << " nb function : " << p2.nbr_func() << endl;

  /* TEST DU CALCUL DES BB d'une fonction */
  /* Pour tester le calcul des BB d'une fonction */ 
  
  
  Function * fct = p2.get_function(0);
  fct -> comput_basic_block();
  cout << "dans le fichier " << argv[1]  << ", première fonction, nb BB : " << fct->nbr_BB() << endl;
  

                /* EXEMPLE DE TEST */
  /* Pour tester le calcul des blocs de bases puis du calcul des BB pred/succ des BB */ 
  

  
  
  Function * fct2 = p2.get_function(2);
  fct2 -> comput_basic_block(); 
  fct2 -> comput_succ_pred_BB();
  fct2 -> test(); 
  
  
  
  // COMPUTATION de toutes les fonctions du programme, sinon comput_cfg crache
  int size = p2.nbr_func();
  list<Function*>::iterator it = p2.function_list_begin();
  Function *current;
  for(int i=0;i<size;i++){
    cout << endl << endl << "NEW ITERATION" << endl << endl;
    current = *it;
    current->comput_basic_block();
    current->comput_succ_pred_BB();
    current->test();
    it++;
  }
  
  
  
  /* TEST DE LA CONSTRUCTION DES CFG et donc des succ/pred des BB */

  // JUSTE LA PREMIERE FCT COMPUTEE, D'OU LA MERDE
  
  if (p2.nbr_func() > 0){
    p2.get_function(0)->display();
    p2.comput_CFG();
    cout<<"comput_cfg ok"<<endl;
    Cfg * cfg = p2.get_CFG(0);
    cout<<"cfg chopé"<<endl;
    cfg -> restitution(NULL,"./tmp/graph2.dot");
  }

  p2.get_function(0)->get_firstBB()->comput_pred_succ_dep();

  return 0;
}
