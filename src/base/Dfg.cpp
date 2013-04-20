#include <Dfg.h>

// crée un arc et le renvoie
Arc_t* new_arc(int del, t_Dep d, Node_dfg *n){
   Arc_t *arc = new Arc_t;
   arc->delai=del;
   arc->dep=d;
   arc->next=n;
   
   return arc;
}

//rend le delay entre 2 instructions pour la dépendance donnée
int get_delay(t_Dep dep, Instruction *from, Instruction *to){
   switch(dep){
   case WAW:
      return WAW_DELAY;
   case WAR:
      return WAR_DELAY;
   case MEMDEP:
      return MEMDEP_DELAY;
   case CONTROL:
      return 0;
   case NONE:
      return 0;
   case RAW:
      int ret=t_delay[from->get_type()][to->get_type()];
      if(ret<1)
	 return 1;
      return ret;
   }
   
   return 0;
}

// A REMPLIR
Dfg::Dfg(Basic_block *bb){
  
  list<Arc_t*>::iterator ita;

 
  _bb=bb;
  _index_branch=-1;
  _nb_arc=0;  
  _length=bb->get_nb_inst();;
  _read= new int[_length];
  
  bb->comput_pred_succ_dep();

  Node_dfg* branch = NULL;

  //On créée les noeuds du graphe
  for(int i=0;i<_length;i++) {
    Node_dfg* temp_node;
    Instruction* temp = bb->get_instruction_at_index(i);

    temp_node = new Node_dfg(temp);

    if(temp->is_branch()) {
      _index_branch=i;
      branch = temp_node;
    }
      
    list_node_dfg.push_back(temp_node);
  }
   
  //On créée les arcs
  list<Node_dfg*>::iterator it_node = list_node_dfg.begin();
  for(int i=0;i<_length;i++) {
    Node_dfg* node = *it_node;
    Instruction* inst = node->get_instruction();  

    //On est tombé sur une racine
    if(inst->get_nb_pred() == 0 && !inst->is_branch()) {
      //cout << "on chope une racine" << endl;
      _roots.push_back(node);
    }

    //On ajoute l'instruction suivante à la liste des delayed slots
    if(inst->is_branch()) {
      _delayed_slot.push_back(*(++it_node));
      break;
    }

    //l'instruction n'a pas de successeur et on a un branchement dans le bloc, on ajoute un arc CONTROL
    if(inst->succ_begin()==inst->succ_end() && branch){
      Arc_t* arc_branch = new_arc(0,CONTROL,branch);
      node->add_arc(arc_branch);
      branch->add_predecesseur(node);
      _nb_arc++;
      //cout << "arc de CONTROL ajouté" << endl;
    } 
    //On a des successeurs
    else {
      for(list<dep*>::iterator it_succ = inst->succ_begin(); it_succ != inst->succ_end(); it_succ++) {
	list<Node_dfg*>::iterator it_temp ;
	Node_dfg* succ;
	for(it_temp=list_node_dfg.begin();it_temp!=list_node_dfg.end();it_temp++){
	  if((*it_temp)->get_instruction() == (*it_succ)->inst){
	    succ=*it_temp;
	    break;
	  }
	}
	//t_Dep dep = inst->is_dependant(succ->get_instruction());
	t_Dep dep = (*it_succ)->type;
	int delay = get_delay(dep,inst,succ->get_instruction());
	Arc_t* arc = new_arc(delay,dep,succ);
	node->add_arc(arc);
	succ->add_predecesseur(node);
	_nb_arc++;
      }
    }
         
    it_node++; 
  }
}

// // methode auxiliaire pour la construction du Dfg, pas forcément utile, dépend de comment vous envisagez de faire...
// void Dfg::build_dfg(Node_dfg*,bool) {

//   // A REMPLIR OU NON !
//  return;
// }

Dfg::~Dfg(){}


void Dfg::display(Node_dfg * node, bool first){
   
   
   list<Node_dfg*>::iterator it;
   it=_roots.begin();

   if(first)	
      for(int i=0; i<_length; i++)	
	 _read[i]=0;  	
   
   for (unsigned int j=0; j<_roots.size();j++ ){ 
      if(first) node = *it;	
			

      if(!_read[node->get_instruction()->get_index()]){
	 _read[node->get_instruction()->get_index()]=1;
	 cout<<"pour i"<<node->get_instruction()->get_index()<<endl;
	 cout<<"l'instruction "<<node->get_instruction()->get_content()<<endl;
			
	 //On affiche ses successeurs s'il en a
	 for(int i=0;i<node->get_nb_arcs();i++){
	    if (node->get_arc(i)){
	       cout<< " -> succ i"
		   << node->get_arc(i)->next->get_instruction()->get_index()
		   << " : "
		   << node->get_arc(i)->next->get_instruction()->get_content()
		   << endl;
	    }
	 }
	 for(int i=0;i<node->get_nb_arcs();i++){
	    if (node->get_arc(i))
	       display(node->get_arc(i)->next, false);
	 }
      }
      it++;

   }
}

#define DEP(x) ((x==NONE)?"":((x==RAW)?"raw":\
				   ((x==WAR)?"war":\
				    ((x==WAW)?"waw": \
				     ((x==MEMDEP)?"mem": "control")))))


//Pour générer le fichier .dot: dot -Tps graph.dot -o graph.ps
void Dfg::restitute(Node_dfg * node, string const filename, bool first){
   if(first)
      remove(filename.c_str());
   ofstream monflux(filename.c_str(), ios::app);
   list<Node_dfg*>::iterator it;
 
   if(first && _length){
     
      for(int i=0; i<_length; i++)
	 _read[i]=0;
      
      it = _delayed_slot.begin();
      
      int index_min = _length;
      
      for(unsigned int i=0; i < _delayed_slot.size(); i++){
	 _read[(*it)->get_instruction()->get_index()] = 1;
	 if (index_min > (*it)->get_instruction()->get_index())
	    index_min = (*it)->get_instruction()->get_index();
	 it++;
      }

      monflux<<"digraph G1 {"<<endl;
      for(int i=0; i<index_min; i++){
	 monflux<<"i"<<i<< ";"<<endl;
	 
      }
   }	
   it=_roots.begin();
   for (unsigned int j=0; j<_roots.size();j++ ){ 		

      if(first) node = *it;
		
      if(monflux){			
	 //monflux.open(filename.c_str(), ios::app);
	 if(!_read[node->get_instruction()->get_index()]){
	    _read[node->get_instruction()->get_index()]=1;
					
	    //On affiche ses successeurs s'il en a
	    for(int i=0; i<node->get_nb_arcs(); i++){
	       if (node->get_arc(i)){
	   
		  monflux<<"i"<<node->get_instruction()->get_index();
		  monflux<<" ->  i" << node->get_arc(i)->next->get_instruction()->get_index();

		  // monflux<<"i"<<node->get_instruction()->get_index()<<"_"<<node->get_weight();
		  // monflux<<" ->  i" << node->get_arc(i)->next->get_instruction()->get_index();
		  // monflux<<"_"<<node->get_arc(i)->next->get_weight();

		  monflux<<" [label= \""<< DEP(node->get_arc(i)->dep) << node->get_arc(i)->delai<<"\"];"<<endl;
	       }
	    }
	    monflux.close();
	
	    for(int i=0;i<node->get_nb_arcs();i++){
	       if (node->get_arc(i))
		  restitute(node->get_arc(i)->next,filename.c_str(),false);	
	    }
	 }
      }
      if((j+1) < _roots.size())	monflux.open(filename.c_str(), ios::app);
      it++;
   }

   if (first && _length){
      monflux.open(filename.c_str(), ios::app);
      monflux<<"}"<<endl;
      monflux.close();
   }
   return;
 
}

bool Dfg::read_test(){
  for(int i=0; i<_length; i++)	if(_read[i]==0)	return true;
  return false;
}


bool contains(list<Node_dfg*>* l, Node_dfg* n){
   list<Node_dfg*>::iterator it;
   
   for(it=l->begin(); it!= l->end(); it++){
      if( (*it)==n ){
	 return true;
      }
   }
   return false;
}

list<Node_dfg*> Dfg::get_inverse_topologic_order() {
  list<Node_dfg*> l, r;//r = liste résultat
  list<Node_dfg*>::iterator it;

  //On ajoute les noeuds sans successeurs à la liste
  for(it=list_node_dfg.begin();it!=list_node_dfg.end();it++) {
    if((*it)->get_nb_arcs() == 0){
      r.push_back(*it);
      l.push_back(*it);
      _read[(*it)->get_instruction()->get_index()] = 1;
    } else {
      _read[(*it)->get_instruction()->get_index()] = 0;
    }
  }

  while(!l.empty()){
    //Noeud candidat à la prochaine place
    Node_dfg* candidat = l.front();
    l.pop_front();
    bool succ_treated = true;
    list<Arc_t*>::iterator it_succ;
    for(it_succ=candidat->arcs_begin();it_succ!=candidat->arcs_end();it_succ++){
      succ_treated &= (_read[(*it_succ)->next->get_instruction()->get_index()]==1);
      //Si un des successeurs du candidat n'a pas été traité il faut le traité avant lui
      if(!succ_treated){
	l.push_front(candidat);
	l.push_front((*it_succ)->next);
	break;
      }
    }
    if(succ_treated){
      //les successeurs du candidats ont été traité, on ajoute le candidat à la liste et ses
      //prédecesseurs à la liste des candidats a traiter
      if( _read[candidat->get_instruction()->get_index()]!=1){
	r.push_back(candidat);
	_read[candidat->get_instruction()->get_index()]=1;
      }
      list<Node_dfg*>::iterator preds;
      for(preds=candidat->pred_begin();preds!=candidat->pred_end();preds++)
	if(_read[(*preds)->get_instruction()->get_index()]==0)
	  l.push_back(*preds);
    }
  }

  return r;
}


void Dfg::comput_critical_path(){
  list<Node_dfg*>::iterator it_node;
  list<Node_dfg*> list_node_ordered = get_inverse_topologic_order();

  for(it_node=list_node_dfg.begin();it_node!=list_node_dfg.end();it_node++)
    (*it_node)->set_weight(0);
  
  for(it_node=list_node_ordered.begin();it_node!=list_node_ordered.end();it_node++){
    //cout << (*it_node)->get_instruction()->get_index() << endl;
    if((*it_node)->get_nb_arcs() == 0)
      (*it_node)->set_weight((*it_node)->get_instruction()->get_latency());
    else {
      list<Arc_t*>::iterator it_succ;
      for(it_succ=(*it_node)->arcs_begin();it_succ!=(*it_node)->arcs_end();it_succ++)
	(*it_node)->set_weight(max((*it_node)->get_weight(),(*it_succ)->delai+(*it_succ)->next->get_weight()));
     }
  }

#ifdef DEBUG
  it=list_node_dfg.begin();
  for(unsigned int k = 0; k < list_node_dfg.size(); k++, it++){
    Node_dfg* node = *it;
    cout << "node inst " << node -> get_instruction() -> get_index() << " poids=" << node->get_weight() << " nb succ=" << node->get_nbr_arc() << endl;
  }
#endif
}


int Dfg::get_critical_path(){
  list<Node_dfg*>::iterator it_node;
  comput_critical_path();
  int criticalpath =0;
  for(it_node=_roots.begin();it_node!=_roots.end();it_node++)
    criticalpath = max(criticalpath,(*it_node)->get_weight());

  return criticalpath;
}


void  Dfg::scheduling(){
}

void Dfg::display_sheduled_instr(){
   list <Node_dfg*>::iterator it;
   Instruction *inst;
   for(it=new_order.begin(); it!=new_order.end(); it++){
      inst=(*it)->get_instruction();
      cout<<"i"<<inst->get_index()<<": "<<inst->get_content()<<endl;
   }
}
