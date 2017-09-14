#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <errno.h>
#include <semaphore.h>
// Cuenten con este codigo monolitico en una funcion
// main como punto de partida.
// Idealmente, el codigo del programa deberia estar
// adecuadamente modularizado en distintas funciones,
// e incluso en archivos separados, con dependencias
// distribuidas en headers.



void* suc_thread(void* arg);
typedef struct trans{


	char tipo;       // retiro,deposito, transferencia (r,d,t)
	char medio;	// efectivo o cuenta (e,c)
	char origen[14]; // cuenta origen
	char destino[14]; // cuenta destino
	int monto;
	
	
	
	
	int result;	//0:exitosa, 1:cuenta invalida, 2:monto invalido
	struct trans* next;

}trans;

trans* create_trans(char tipo, char medio, char origen[14], char destino[14], int monto, int result){
	trans* tr=malloc(sizeof(trans));
	if (tr==NULL){ fprintf(stderr,"FALLO DE MALLOC\n");
		exit(1);}
	tr->tipo=tipo;
	tr->medio=medio;
	strcpy(tr->origen,origen);
	strcpy(tr->destino,destino);
	tr->monto=monto;
	tr->result=result;
	tr->next=NULL;

	
	return tr;


}




void push_trans(char tipo, char medio, char origen[14], char destino[14], int monto, int result,trans* header){

	trans* temp=header;
	while(temp->next!=NULL){
		
		temp=temp->next;

		}

	temp->next=create_trans(tipo,medio,origen,destino,monto, result);

	

	}
	



typedef struct cuenta{
	int monto;
	struct cuenta* next;
	char id[14];
	sem_t lock; //lock para acceder a la cuenta en si
	
}cuenta;






typedef struct suc{

	pid_t id;
	int pipe[2];  // pipe desde donde el banco puede escribir a cada sucursal;
 	struct suc* next;
 	int cuentas;

	}suc;




suc* create_suc(pid_t suc_id, int cuentas, int pipe[2]){
	
	suc* sucur=malloc(sizeof(suc));
	if (sucur==NULL){

			fprintf(stderr,"ERRO DE MALLOC AL CREAR SUCURSAL\n");
			exit(1);


			}
	sucur->id=suc_id;
	sucur->next=NULL;
	sucur->pipe[0]=pipe[0];
	sucur->pipe[1]=pipe[1];
	sucur->cuentas=cuentas;
	return sucur;
	}



void push_suc(suc* header, pid_t suc_id, int cuentas, int pipe[2]){
	
	suc* temp=header;
	while(temp->next!=NULL){
		temp=temp->next;
				
		}
	temp->next=create_suc(suc_id,cuentas,pipe);
		
	return;
}

void print_suc(suc* header){
	
	
	suc* temp=header;
	if(temp==NULL){
		printf("-------No existen sucursales para mostrar, cree una ingresando 'init'-----\n");
		return;
		}
		
	printf("\n--------- SUCURSALES --------\n");
	int n=0;
	while(temp!=NULL){
		n+=1;
		printf("ID: %d, Rango de cuentas: 1-%d\n",(int)temp->id,temp->cuentas);
		temp=temp->next;		
		}
	printf("Numero total de sucursales: %d\n",n);
	return;
	}

int get_suc(char* id_cuenta){       //RECIBE UN ID  BBB-SSS-CCC y retorna SSS
	char str_suc[3];
	//char* acc_arr[3];
	int k=4;
	
	
	while(k<=6){
		str_suc[k-4]=id_cuenta[k];
		k+=1;
}
	int result=atoi(str_suc);
	return result;

}





cuenta* create_cuenta(int* counter, int suc, int bank){  //retorna el header* de la lista

	
	cuenta* header=malloc(sizeof(cuenta));
	
	if (header==NULL){
			fprintf(stderr,"FALLO DE MALLOC EN CUENTA\n");
		}
	*counter+=1;
	
	int random=rand()%500000000+1000;
	header->monto=random;
	sem_init(&(header->lock),0,1);
	char id[14];
	if(id==NULL){
		fprintf(stderr,"fallo de malloc en crear cuenta\n");
		exit(1);	}
	
	sprintf(id, "%03d-%03d-%06d",bank, suc,*counter);
	strcpy(header->id,id);
	
	header->next=NULL;
	
	return header;
	

}

 void push_cuenta(cuenta* header, int* counter,int suc,int bank){
		cuenta* temp=header;
		while(temp->next!=NULL){
			temp=temp->next;
		}
		cuenta* new_cuenta=create_cuenta(counter,suc,bank);
		temp->next=new_cuenta;
		return;



}









void delete_suc(int id, suc* h_suc){
	if(h_suc->id==id){
		suc* to_delete=h_suc;
		h_suc=h_suc->next;
		free(to_delete);
		return;
		}
	else{
		suc* temp_suc=h_suc;
		while(temp_suc->next!=NULL){
		
				if(temp_suc->next->id==id){
					suc* to_delete=temp_suc->next;
					
					temp_suc->next=h_suc->next->next;
					
					free(to_delete);
					return;
					}
				else{
				
					
					temp_suc=temp_suc->next;
					
					}
		
		
			}
	
	}
}


typedef struct args{
	int readpipe;   //pipe desde donde la sucursal escucha al banco
	cuenta* acc_h;		//h. de cuentas de la suc
	trans* trans_h;			//h. de trans de la suc
	
	
}args;

	
suc* buscar_suc(int id, suc* h_suc){
	suc* temp=h_suc;
	while(temp!=NULL)
	{
		if(temp->id==id)
		{
			return temp;
			
			
		}
		else
		{
		
			temp=temp->next;
		}
	
	
	}
	
	if(temp==NULL){
	
		return NULL;
	
	
	
	}
	
	
	
	
	
	
	
	

	}
	
typedef struct matrix_arg{
	suc* suc_h;
	int sucPipe;    //este int es sucPipe[0], descriptor de archivo de lectura del Pipe
}matrix_arg;




void* matrix(void* arg){ //args debe ser un struct que contenga suc_h,bankPipe

//	suc* suc_h=((matrix_arg*)arg)->suc_h;
	int sucPipe=((matrix_arg*)arg)->sucPipe;
	char readbuffer[80];
	while(true){
		int bytes=read(sucPipe,readbuffer,sizeof(readbuffer));
		if(bytes==-1){
			fprintf(stderr,"error al leer mensaje desde Pipe: %s\n",strerror(errno));
			exit(1);
	
		}
	
		char* msg_array[4];  //mensaje de la forma '<emisor>;<solicitud/respuesta>;<monto>;<cuenta_destino>;<cuenta_inicial>'
					// a traves de cuenta_destino se verifica la existencia del banco y de la cuenta en el mismo,
	
		char* token;
		token=strtok(readbuffer,";");
		
		int i=0;
		while(token!=NULL){
			msg_array[i]=malloc((sizeof(char))*(strlen(token)+1));
		
			strcpy(msg_array[i],token);
			msg_array[strlen(token)]='\0';
			token=strtok(NULL," ");
			i+=1;
	
		}
	
	}	
	
	

	free(arg);
	return;
	
	
	
	

}

typedef struct accs_lock{  // objeto que contiene lock general para leer o escribir en la lista ligada de cuentas

sem_t wlock;  // lock para entrada en la lista 
sem_t lock_rdr;	//lock para editar rdr y wrt (contadores de visitantes)
sem_t lock_wtr;
int rdr;
int wtr;
}accs_lock;

void dump(trans* h_trans);
void dump_errs(trans* header);
void dump_accs(cuenta* header);
int retiro(int monto, char id_cuenta[14], cuenta* header);
int deposito(int monto,char* id_cuenta,cuenta* header);
void acq_acc_rlock();
void acq_acc_wlock();
void rls_acc_rlock();
void rls_acc_wlock();
void acq_sucs_rlock();
void rls_sucs_rlock();



//VARIABLES GLOBALES

sem_t rand_lock;
sem_t trans_lock;  //
sem_t sucs_lock;   // semaforos parra escritura y lectura de las trans y sucs
int sucs_rdr;
sem_t sucs_rdrlock; //lock para la variaable int sucs_rdr
accs_lock accounts_lock;
int bankId;
int sucId;

int main(int argc, char** argv) { ////NO SE HAN CERRADO LAS SALIDAS DE PIPE NO USADAS POR LOS RESPECTIVOS PROCESOS;
  
  //inicializamos los lock
  sucs_rdr=0;
  sem_init(&rand_lock,0,1);
  sem_init(&trans_lock,0,1);
  sem_init(&sucs_lock,0,1);
  sem_init(&sucs_rdrlock,0,1);
  sem_init(&(accounts_lock.wlock),0,1);
  sem_init(&(accounts_lock.lock_rdr),0,1);
  sem_init(&(accounts_lock.lock_wtr),0,1);
  accounts_lock.rdr=0;
  accounts_lock.wtr=0;
  size_t bufsize = 512;
  char* commandBuf = malloc(sizeof(char)*bufsize);
  // Para guardar descriptores de pipe
  // el elemento 0 es para lectura
  // y el elemento 1 es para escritura.
  
  

  
  suc* suc_h=NULL;
  bankId = getpid() % 1000;
  printf("Bienvenido a Banco '%d'\n", bankId);	

	
	
 char readbuffer[80]; // buffer para lectura desde pipe

  // se crea este unico pipe desde donde TODAS las sucursales  
  int sucPipe[2];
  pipe(sucPipe);
  

  while (true) {
    printf(">>");
	
    getline(&commandBuf, &bufsize, stdin);

    // Manera de eliminar el \n leido por getline
    commandBuf[strlen(commandBuf)-1] = '\0';

    printf("Comando ingresado: '%s'\n", commandBuf);

    if (!strncmp("quit", commandBuf, strlen("quit"))) {        	//QUIT
    
    	suc* temp;
    	char msg[]="kill";
    	while(suc_h!=NULL){
    		temp=suc_h;
    		suc_h=suc_h->next;
    		write(temp->pipe[1],msg,strlen(msg)+1);
    		wait(NULL);
    		free(temp);
    		printf("\nHIJO MUERTO\n");
    		
    	
    		}
        break;
        
    }
	

    else if (!strncmp("kill", commandBuf, strlen("kill"))) {        	//KILL
   
    
        int id_suc=atoi((char*)(commandBuf+4));
        printf("has deseado matar a la sucursal %d",id_suc);
        sem_wait(&sucs_lock);
        suc* temp=suc_h;
        char msg[]="kill";
        suc* prev=NULL;
        	while(temp!=NULL){
        		if(temp->id==id_suc){
        			printf("notificando a la suc para que se mate");
        			
        			write(temp->pipe[1],msg,strlen(msg)+1);
        			
        			
        			//esperamos a que la sucursal termine...
        			int status=wait(NULL);
        			printf("\n............Status= %d\n",status);
        			
        			
       				suc* to_free=temp;
       				if (prev==NULL){
       					suc_h=temp->next;
       					// si se elimina el unico existente, hay que terminar el thread de matriz
       					
       					
       				
       				}
       				else{
       					
       					prev->next=temp->next;
       				
       				}
       				
        			free(to_free);
        			//Sucursal eliminada de la lista ligada y proceso sucursal individual terminado...
        			break;
        	
        		}
        		else{
        		prev=temp;
        		temp=temp->next;
        		}
        		}
        if(temp==NULL){
        	printf("Sucursal no encontrada");
        	}
        sem_post(&sucs_lock);
        	
	continue;
    }



    else if (!strncmp("list", commandBuf, strlen("list"))) {			//LIST
      	acq_sucs_rlock();
      	print_suc(suc_h);
 	rls_sucs_rlock();
    }
    
    else if (!strncmp("dump_errs", commandBuf, strlen("dump_errs"))) {			//DUMP
        int suc_dump=atoi((char*)(commandBuf+10));
    	char msg[]="dump_errs";
    	acq_sucs_rlock();
    	suc* temp=buscar_suc(suc_dump,suc_h);
    	if(temp==NULL){
    		printf("Sucursal no encontrada\n");
    	
    		}
    	else{
    		write(temp->pipe[1],msg,strlen(msg)+1);
    	
    	
    		}
        rls_sucs_rlock();
        
    }
    
    else if (!strncmp("dump_accs", commandBuf, strlen("dump_acss"))) {			//DUMP
        int suc_dump=atoi((char*)(commandBuf+10));
    	char msg[]="dump_accs";
    	acq_sucs_rlock();
    	suc* temp=buscar_suc(suc_dump,suc_h);
    	if(temp==NULL){
    		printf("Sucursal no encontrada\n");
    	
    		}
    	else{
    		write(temp->pipe[1],msg,strlen(msg)+1);
    	
    	
    		}
        
        rls_sucs_rlock();
    }

    
    else if (!strncmp("dump", commandBuf, strlen("dump"))) {			//DUMP.. busca
    	
    	int suc_dump=atoi((char*)(commandBuf+4));
    	char msg[]="dump";
    	acq_sucs_rlock();
    	suc* temp=buscar_suc(suc_dump,suc_h);
    	if(temp==NULL){
    		printf("Sucursal no encontrada\n");
    	
    		}
    	else{
    		write(temp->pipe[1],msg,strlen(msg)+1);
    	
    	
    		}
    	
    	
        rls_sucs_rlock();
        
    }
    
    

	

    else if (!strncmp("init", commandBuf, strlen("init"))) {			//INIT
      // OJO: Llamar a fork dentro de un ciclo
      // es potencialmente peligroso, dado que accidentalmente
      // pueden iniciarse procesos sin control.
      // Buscar en Google "fork bomb"
	
	
	  
  	int cuentas=atoi((char*)(commandBuf+4));
  	if(cuentas==0)  cuentas=1000;
  	
  	
  	int bankPipe[2]; 
	pipe(bankPipe);
	
	
        pid_t sucid = fork();
	
	
      if (sucid > 0) {		//PROCESO PADRE DENTRO DE INIT
      
      	sem_wait(&sucs_lock);			
	if(suc_h==NULL){   //si no hay suc, se crea una y se incializa el thread de matriz, hay que trabajar con lo apuntado por el header;
		
		suc_h=create_suc(sucid%1000,cuentas,bankPipe);
		matrix_arg* ma=malloc(sizeof(matrix_arg));
		ma->suc_h=suc_h;
		ma->sucPipe=sucPipe[0];
		pthread_t* matrix_t=malloc(sizeof(pthread_t));
		pthread_create(matrix_t,NULL, matrix,ma);
		
	}
	else {
		push_suc(suc_h,sucid,cuentas,bankPipe);
	        
        
        

		}
		
		
		
	
	
	sem_post(&sucs_lock);
        printf("Sucursal creada con %d cuentas, PID '%d'\n", cuentas,sucid);
	

	
        continue;
      }



      // Proceso de sucursal
      else if (!sucid) {			//HIJO AL MOMENTO DE INICIAR
      
      	//Cerramos la parte de escritura del pipe						
	close(bankPipe[1]);
		
	

        sucId = getpid() % 1000;
	
        printf("Hola, soy la sucursal '%d'\n", sucId);
        
        
        
       
        
        
        // se crean las cuentas desde 0 hasta 'cuentas' entregadas por el user, header es el header de cuentas
	int counter_accounts=0;
	cuenta* acc_h=create_cuenta(&counter_accounts,(int)sucId,(int)bankId);
	if(acc_h==NULL){
		fprintf(stderr, "FAllo de malloc");
		exit(1);
	
	}
	while(counter_accounts<cuentas){
		
		
		push_cuenta(acc_h,&counter_accounts,(int)sucId,(int)bankId);
		
		}
	
 	trans* trans_h=create_trans('r','e',"ORIGEN","DESTINO",0,0);
	
	
	//inicializamos el thread que se encaraga de escuchar a la matriz
	args* argumentos=malloc(sizeof(args));
	if(argumentos==NULL){
	
		fprintf(stderr,"Fallo de malloc");
		exit(1);
	}
	argumentos->readpipe=bankPipe[0];
	argumentos->acc_h=acc_h;
	argumentos->trans_h=trans_h;
	
	pthread_t* t1=malloc(sizeof(pthread_t));
	
	pthread_create(t1,NULL,suc_thread,argumentos);
	
        while (true) {
          // 100 milisegundos...
	srand(time(NULL));
	usleep(rand()%100000+500000);
		
	
	
	
	
	
	//CODIGO QUE ESCUCHA A LA MATRIZ
	
	int bytes;
	bytes = read(bankPipe[0], readbuffer, sizeof(readbuffer));
		printf("Soy la sucursal me llego mensaje '%s' de '%d' bytes.\n",
       		readbuffer,bytes);
       		if(!strncmp("kill", readbuffer, strlen("kill"))){
       				printf("matandome");
       				_exit(EXIT_SUCCESS);
       				
       		}
	
		
		else if(!strncmp("dump_accs", readbuffer, strlen("dump_accs"))){
			
		
			dump_accs(acc_h);
		
		
		}
		
		else if(!strncmp("dump_errs", readbuffer, strlen("dump_errs"))){
		
			dump_errs(trans_h);
		
		
		}
		
		
		
		else if(!strncmp("dump", readbuffer, strlen("dump"))){
			dump(trans_h);
		
		
		}
		
		
		else if(!strncmp("solicitud",readbuffer,strlen("solicitud"))){
		
		
		
			char solicitud[10];  
			int monto;
			char cuenta[14];
		
			char* token;
			token=strtok((readbuffer+10)," ");
			strcpy(solicitud,token);
			int k=0;
			while(token!=NULL){
				if(k==0){
				
					strcpy(solicitud,token);
				}
				
				
				else if(k==1){
				        monto=atoi(token);
				
				}
				else{
				
				
					strcpy(cuenta,token);
				}
				k+=1;
				token=strtok(NULL," ");
			}
		
			printf("se solicita: %s %d %s\n",solicitud,monto,cuenta);
		//ordenamos lo recibido para operar la solicitud
		
		
			
		
		
		
		
		}
			
			
		
		
		
		
		
		
		else if(!strncmp("respuesta",readbuffer,strlen("respuesta"))){
		
		
		
		
		
		
		}
		





	
	
	
        
//	usleep(20000000);

          // Usar usleep para dormir una cantidad de microsegundos
          usleep(2000000);
	//pthread_join(*mythread,NULL);
          // Cerrar lado de lectura del pipe
          	

          // Para terminar, el proceso hijo debe llamar a _exit,
          // debido a razones documentadas aqui:
          // https://goo.gl/Yxyuxb
        }
      }
      // error
      else {
        fprintf(stderr, "Error al crear proceso de sucursal!\n");
        return (EXIT_FAILURE);
      }
    }	
    else {
      fprintf(stderr, "Comando no reconocido.\n");
    }
    // Implementar a continuacion los otros comandos
  }
  free(commandBuf);

  printf("Terminando ejecucion limpiamente...\n");
  // Cerrar lado de escritura del pipe
   

  return(EXIT_SUCCESS);
}




void dump(trans* h_trans){    //FALTA ARREGLAR LA FORMA EN QUE SE INICIALIZA LA LISTA LIGADA DE TRANSACCIONES
	trans* temp=h_trans;
	temp=temp->next; /// ESTA MAL INICIALIZADA.. POR ESO PARTIMOS DE LA SEGUNDA
	//creamos el archivo
	char fname[12];
	sprintf(fname,"dump_%d.txt",sucId);
	FILE* f=fopen(fname,"w+");
	fprintf(f,"tipo,medio,origen,destino\n");
	if(f==NULL){
		fprintf(stderr,"no se pudo abrir el archivo");
		exit(1);
		}
		
		
		
	while(temp!=NULL){
		if(temp->result==0){
		
			fprintf(f,"%c, %c, %s, %s\n",temp->tipo,temp->medio,temp->origen,temp->destino);
					
		}
		temp=temp->next;
	
	
	}
	fclose(f);
	
	return;
		}
		






void dump_errs(trans* header){

	char fn[17];
	sprintf(fn,"dump_errs_%d.csv",sucId);
	FILE* f=fopen(fn,"w+");
	fprintf(f,"tipo_e,cuenta,monto\n");
	trans* temp=header;
	temp=temp->next;
	while(temp!=NULL){
		if(temp->result!=0){
			
			fprintf(f,"%c,%s,%d\n",temp->tipo,temp->destino,temp->monto);
		
		}
		temp=temp->next;
	
	
	}
	fclose(f);





}

void dump_accs(cuenta* header){
	char fn[17];
	sprintf(fn,"dump_accs_%d.csv",sucId);
	FILE* f=fopen(fn,"w+");
	cuenta* temp=header;
	acq_acc_rlock();
	fprintf(f,"'ID','saldo'\n");

	while(temp!=NULL){
		fprintf(f,"%s,%d\n",temp->id,temp->monto);
		
		temp=temp->next;
			

		}
	
	
	fclose(f);	
	rls_acc_rlock();
}


int retiro(int monto, char id_cuenta[14], cuenta* header){ // retorna 0 si es exitoso,1 cuenta invalida, 2 monto invalido
	cuenta* temp=header;
	acq_acc_wlock();
	while(temp!=NULL){
		if(temp->id==id_cuenta){
			sem_wait(&temp->lock);
			if(temp->monto>=monto && monto>0){
					temp->monto-=monto;
					rls_acc_wlock();
					sem_post(&temp->lock);
					return 0;

				}

			else{
				sem_post(&temp->lock);
				rls_acc_wlock();
				return 2;
				}

			}
			

		else { temp=temp->next;

				}				

		}
	
	rls_acc_wlock();
	return 1;

}

int deposito(int monto,char* id_cuenta,cuenta* header){             //retorna 0 si es efectivo, 1 si la cuenta es invalida
	cuenta* temp=header;
	while(temp!=NULL){
		
		if(temp->id==id_cuenta){
			sem_wait(&temp->lock);
			temp->monto+=monto;
			sem_post(&temp->lock);
			return 0;
		
			
			}
		else temp=temp->next;

	}

	return 1;
	
}


//metodos para sincronizar las intervenciones sobre la estructura de datos de las accounts
void acq_acc_rlock(){
	
	sem_wait(&accounts_lock.lock_rdr);
	if(accounts_lock.rdr==0){
		accounts_lock.rdr+=1;
		sem_wait(&accounts_lock.wlock);
		
	}
	else {accounts_lock.rdr+=1;
		
		
		
	}
	
	sem_post(&accounts_lock.lock_rdr);
	
	return;
	


}


void acq_acc_wlock(){
	sem_wait(&accounts_lock.lock_wtr);
	accounts_lock.wtr+=1;
	if(accounts_lock.wtr==1){
		sem_wait(&accounts_lock.wlock);
		
	}
	
	sem_post(&accounts_lock.lock_wtr);
	return;
	

}


void rls_acc_rlock(){
	sem_wait(&accounts_lock.lock_rdr);
	accounts_lock.rdr-=1;
	if(accounts_lock.rdr==0){
		
		sem_post(&accounts_lock.wlock);
	}
	
	
	sem_post(&accounts_lock.lock_rdr);
	
	return;
}

void rls_acc_wlock(){
	
	sem_wait(&accounts_lock.lock_wtr);
	accounts_lock.wtr-=1;
	if(accounts_lock.wtr==0){
		
		sem_post(&accounts_lock.wlock);
	}
	
	
	sem_post(&accounts_lock.lock_wtr);
	return;
	

}

void acq_sucs_rlock(){
	sem_wait(&sucs_rdrlock);
	sucs_rdr+=1;
	if(sucs_rdr==0){
		sem_wait(&sucs_lock);
		
		}
	sem_post(&sucs_rdrlock);

	return;

}	


void rls_sucs_rlock(){
	sem_wait(&sucs_rdrlock);
	sucs_rdr-=1;
	if(sucs_rdr==0){
		sem_post(&sucs_lock);
		
		}
	sem_post(&sucs_rdrlock);
	return;
}
void* suc_thread(void* argumento){ //arg necesita ser un objeto que contenga el acc_h, trans_h,pipeBank[0] 
	printf("Se ha iniciado un thread concurrente en sucursal\n");
 	
	args* argumentos=(args*)argumento;
	int pipe=argumentos->readpipe;

	
	cuenta* acc_h=argumentos->acc_h;
	
	trans* trans_h=argumentos->trans_h;
	if(trans_h==NULL){
		fprintf(stderr,"trans_null");
		exit(1);
	
		}
	int random;
	while(true){	
		
		usleep(500000);
		
		char cuenta_random[14];  
		srand(time(NULL));
		random=rand()%1500;  // CCC random en int
		sprintf(cuenta_random, "%03d-%03d-%06d",bankId, sucId,random);
		

		usleep(1000);
		srand(time(NULL));
		int transaccion=rand()%3+1;
		int r_trans;
	
		int monto=rand()%500000000+1;
		
		if(transaccion==1){	//DEPOSITO
			
			r_trans=deposito(monto,cuenta_random,acc_h);
			
			push_trans('d','e',"EFECTIVO",cuenta_random,monto,r_trans,trans_h);
			
				
			
		
		}
		else if(transaccion==2) { //RETIRO
			
			r_trans=retiro(100000,cuenta_random,acc_h);
			
			push_trans('r','e',cuenta_random,"EFECTIVO",monto,r_trans,trans_h);
			}


		else{		        	//TRANSFERENCIA
			
			char cuenta_random_dest[14];
			random=rand()%1500;
			sprintf(cuenta_random_dest, "%03d-%03d-%06d",bankId, sucId,random);
			if((r_trans=retiro(100000,cuenta_random,acc_h)==0)){
					r_trans=deposito(monto,cuenta_random_dest,acc_h);
					
					push_trans('t','c',cuenta_random,cuenta_random_dest,monto,r_trans,trans_h);
					
				


				}
			else{

				push_trans('t','c',cuenta_random,cuenta_random_dest,monto,r_trans,trans_h);

					}



			}
		usleep(100000);
	
	}

}




