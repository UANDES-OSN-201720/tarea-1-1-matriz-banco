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
// Cuenten con este codigo monolitico en una funcion
// main como punto de partida.
// Idealmente, el codigo del programa deberia estar
// adecuadamente modularizado en distintas funciones,
// e incluso en archivos separados, con dependencias
// distribuidas en headers.




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
	if (tr==NULL){ printf("FALLO DE MALLOC\n");}
	tr->tipo=tipo;
	tr->medio=medio;
	strcpy(tr->origen,origen);
	strcpy(tr->destino,destino);
	tr->monto=monto;
	tr->result=result;
	tr->next=NULL;

	printf("Se ha creado la trans: tipo: %c, medio %c, origen: %s, destino: %s, monto: %d, result:%d\n",tipo,medio,origen,destino,monto,result);
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

			printf("ERRO DE MALLOC AL CREAR SUCURSAL\n");


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

void print_suc(suc* header, int n){
	printf("\n---------NUMERO TOTAL DE SUCURSALES: %d--------\n",n);
	
	suc* temp=header;
	while(temp!=NULL){
		printf("ID: %d\n, Rango de cuentas: 1-%d",(int)temp->id,temp->cuentas);
		temp=temp->next;		
		}
	return;
	}

int get_suc(char* id_cuenta){       //RECIBE UN ID  BBB-SSS-CCC y retorna SSS
	char str_suc[3];
	if(str_suc==NULL){
		printf("Fallo de malloc");
		}
	int k=4;
	while(k<=6){
		str_suc[k-4]=id_cuenta[k];
		k+=1;
}
	int result=atoi(str_suc);
	return result;

}


int get_id_cuenta(char* id_cuenta){    // Recibe id BBB-SSS-CCC y retorna CCC
	char str_id[6];
	
	int k=8;
	while(k<=14){

		str_id[k-8]=id_cuenta[k];
		k+=1;
		}
	int cuenta=atoi(str_id);
	return cuenta;

} 


cuenta* create_cuenta(int* counter, int suc, int bank){  //retorna el header* de la lista

	printf("CREANDO una cuenta...\n");
	cuenta* header=malloc(sizeof(cuenta));
	
	if (header==NULL){
			fprintf(stderr,"FALLO DE MALLOC EN CUENTA\n");
		}
	*counter+=1;
	int random=rand()%500000000+1000;
	header->monto=random;
	char id[14];
	if(id==NULL){
		printf("fallo de malloc en crear cuenta\n");	}
	
	sprintf(id, "%03d-%03d-%06d",bank, suc,*counter);
	strcpy(header->id,id);
	
	header->next=NULL;
	printf("Se ha creado la cuenta");
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

int retiro(int monto, char* id_cuenta, cuenta* header){ // retorna 2 si el monto es invalido
	cuenta* temp=header;
	while(temp!=NULL){
		if(get_id_cuenta(temp->id)==get_id_cuenta(id_cuenta)){
			if(temp->monto>=monto && monto>0){
					temp->monto-=monto;
					printf("SALDO: %d.... retiro: %d",temp->monto,monto);
					printf("RETIRO EXITOSO");
					
					return 0;

				}

			else{
				
				printf("MONTO INVALIDO");
				return 2;
				}

			}

		else { temp=temp->next;

				}				

		}
	printf("CUENTA INVALIDA");

	return 1;

}

int deposito(int monto,char* id_cuenta,cuenta* header){             //retorna 0 si es efectivo, 1 si la cuenta es invalida
	cuenta* temp=header;
	while(temp!=NULL){
		
		if(get_id_cuenta(temp->id)==get_id_cuenta(id_cuenta)){
			temp->monto+=monto;
			printf("Se han depositado %d a la cuenta %s, ahora su monto es %d\n",monto,id_cuenta,temp->monto);
			return 0;
		
			
			}
		else temp=temp->next;

	}
	printf("ERROR AL DEPOSITAR A: %s",id_cuenta);
	return 1;
	
}









void delete_suc(int id, suc* h_suc){      //deberia decrementar el contador num_suc de main
	if(((h_suc->id)%1000)==id){
		suc* to_delete=h_suc;
		h_suc=h_suc->next;
		free(to_delete);
		return;
		}
	else{
		suc* temp_suc=h_suc;
		while(temp_suc->next!=NULL){
		
				if(((temp_suc->next->id)%1000)==id){
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
	int pipe;   //pipe desde donde la sucursal escucha al banco
	cuenta* h_cuentas;		//h. de cuentas de la suc
	trans* h_trans;			//h. de trans de la suc
	
	
}args;
int sucId;

void dump_errs(trans* header){

	char fn[17];
	sprintf(fn,"dump_errs_%d.csv",sucId);
	FILE* f=fopen(fn,"w+");
	fprintf(f,"tipo_e,cuenta,monto\n");
	trans* temp=header;
	while(temp!=NULL){
		if(temp->result!=0){
			
			fprintf(f,"%c,%s,%d",temp->tipo,temp->destino,temp->monto);
		
		}
		temp=temp->next;
	
	
	}
	fclose(f);





}

void dump_accs(int sucID, cuenta* header){
	char fn[17];
	sprintf(fn,"dump_accs_%d.csv",sucID);
	FILE* f=fopen(fn,"w+");
	cuenta* temp=header;
	fprintf(f,"'ID','saldo'\n");

	while(temp!=NULL){
		fprintf(f,"%s,%d\n",temp->id,temp->monto);
		
		temp=temp->next;
			

		}
	


	fclose(f);	
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
		


void* escuchar(void* argumento){  //recibira un struct args*


	//casteamos los argumentos para ocuparlos
	args* argumentos=(args*)argumento;
	int pipe=argumentos->pipe;
	cuenta* h_cuentas=argumentos->h_cuentas;
	trans* h_trans=argumentos->h_trans;
	//ponemos el thread a escuchar a la matriz
	int bytes;
	char readbuffer[80];
	
	while(true){
	
		bytes = read(pipe, readbuffer, sizeof(readbuffer));
		printf("Soy la sucursal me llego mensaje '%s' de '%d' bytes.\n",
       		readbuffer,bytes);
       		if(!strncmp("kill", readbuffer, strlen("kill"))){
       				printf("matandome");
       				_exit(EXIT_SUCCESS);
       				
       				}
	
		
		else if(!strncmp("dump_accs", readbuffer, strlen("dump_accs"))){
			dump_accs(sucId,h_cuentas);
		
		
		}
		
		else if(!strncmp("dump_errs", readbuffer, strlen("dump_errs"))){
		
			dump_errs(h_trans);
		
		
		}
		
		
		
		else if(!strncmp("dump", readbuffer, strlen("dump"))){
			dump(h_trans);
		
		
		}
		
			
		

	}
		return NULL;
	
	



	}
	
	
suc* buscar_suc(int id, suc* h_suc){
	suc* temp=h_suc;
	while(temp!=NULL)
	{
		if(((temp->id)%1000)==id)
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


int main(int argc, char** argv) { ////NO SE HAN CERRADO LAS SALIDAS DE PIPE NO USADAS POR LOS RESPECTIVOS PROCESOS;
  srand(time(NULL));
  size_t bufsize = 512;
  char* commandBuf = malloc(sizeof(char)*bufsize);
  // Para guardar descriptores de pipe
  // el elemento 0 es para lectura
  // y el elemento 1 es para escritura.
  
  char readbuffer[80]; // buffer para lectura desde pipe

  // Se crea un pipe...
  
  int suc_num=0;
  suc* header_suc=NULL;
  const int bankId = getpid() % 1000;
  printf("Bienvenido a Banco '%d'\n", bankId);	
//	int status=system("gnome-terminal");
 


  while (true) {
    printf(">>");
	
    getline(&commandBuf, &bufsize, stdin);

    // Manera de eliminar el \n leido por getline
    commandBuf[strlen(commandBuf)-1] = '\0';

    printf("Comando ingresado: '%s'\n", commandBuf);

    if (!strncmp("quit", commandBuf, strlen("quit"))) {        	//QUIT
    
    	suc* temp;
    	char msg[]="kill";
    	while(header_suc!=NULL){
    		temp=header_suc;
    		header_suc=header_suc->next;
    		write(temp->pipe[1],msg,strlen(msg)+1);
    		wait(NULL);
    		printf("\nHIJO MUERTO\n");
    		suc_num-=1;
    	
    		}
        break;
        
    }
	

    else if (!strncmp("kill", commandBuf, strlen("kill"))) {        	//KILL
    //    kill(1,SIGKILL);
    
        int id_suc=atoi((char*)(commandBuf+4));
        printf("has deseado matar a la sucursal %d",id_suc);
        suc* temp=header_suc;
        char msg[]="kill";
        	while(temp!=NULL){
        		if(((temp->id)%1000)==id_suc){
        			printf("notificando a la suc para que se mate");
        			
        			write(temp->pipe[1],msg,strlen(msg)+1);
        			int status=wait(NULL);
        			printf("\n............Status= %d\n",status);
        			
        			
       				
        			
        			delete_suc(id_suc,header_suc);
        			suc_num-=1;
        			//FALtA ELIMINAR LA SUCURSAL DE LA LISTA LIGA LIGADA
        			break;
        	
        		}
        		else{
        		temp=temp->next;
        		}
        		}
        if(temp==NULL){
        	printf("Sucursal no encontrada");
        	}
        		
        	
	continue;
    }



    else if (!strncmp("list", commandBuf, strlen("list"))) {			//LIST
        printf("LIST:");
 	print_suc(header_suc,suc_num);
    }
    
    else if (!strncmp("dump_errs", commandBuf, strlen("dump_errs"))) {			//DUMP
        int suc_dump=atoi((char*)(commandBuf+10));
    	char msg[]="dump_errs";
    	suc* temp=buscar_suc(suc_dump,header_suc);
    	if(temp==NULL){
    		printf("Sucursal no encontrada\n");
    	
    		}
    	else{
    		write(temp->pipe[1],msg,strlen(msg)+1);
    	
    	
    		}
        
        
    }
    
    else if (!strncmp("dump_accs", commandBuf, strlen("dump_acss"))) {			//DUMP
        int suc_dump=atoi((char*)(commandBuf+10));
    	char msg[]="dump_accs";
    	suc* temp=buscar_suc(suc_dump,header_suc);
    	if(temp==NULL){
    		printf("Sucursal no encontrada\n");
    	
    		}
    	else{
    		write(temp->pipe[1],msg,strlen(msg)+1);
    	
    	
    		}
        
        
    }

    
    else if (!strncmp("dump", commandBuf, strlen("dump"))) {			//DUMP.. busca
    	
    	int suc_dump=atoi((char*)(commandBuf+4));
    	char msg[]="dump";
    	suc* temp=buscar_suc(suc_dump,header_suc);
    	if(temp==NULL){
    		printf("Sucursal no encontrada\n");
    	
    		}
    	else{
    		write(temp->pipe[1],msg,strlen(msg)+1);
    	
    	
    		}
    	
    	
        
        
    }
    
    

	

    else if (!strncmp("init", commandBuf, strlen("init"))) {			//INIT
      // OJO: Llamar a fork dentro de un ciclo
      // es potencialmente peligroso, dado que accidentalmente
      // pueden iniciarse procesos sin control.
      // Buscar en Google "fork bomb"
	

	  
  	int cuentas=atoi((char*)(commandBuf+4));
  	if(cuentas==0)  cuentas=1000;
  	if(cuentas<1000 || cuentas>10000){
  		printf("debe especificar un numero de cuentas entre mil y diez mil");
  	
  		}
  	
  	int bankPipe[2]; 
	pipe(bankPipe);
	
	
        pid_t sucid = fork();
	
	
      if (sucid > 0) {						//PROCESO PADRE DENTRO DE INIT
	if(suc_num==0){
		header_suc=create_suc(sucid,cuentas,bankPipe);
		suc_num+=1;
		
	}
	else {
		push_suc(header_suc,sucid,cuentas,bankPipe);
		suc_num+=1;
		int bytes;
        // Enviando saludo a la sucursal
        
        
        char msg[] = "Hola sucursal, como estas?\n";
	bytes=write(header_suc->next->pipe[1], msg, (strlen(msg)+1));
	printf("se escribieron %d bytes de saludo\n",bytes);   

		}
	
	
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
	cuenta* header=create_cuenta(&counter_accounts,(int)sucId,(int)bankId);
	while(counter_accounts<cuentas){
		usleep(1000);
		push_cuenta(header,&counter_accounts,(int)sucId,(int)bankId);

		}
	
 	trans* trans_header=create_trans('r','e',"ORIGEN","DESTINO",0,0);
	int trans_num=0;	
	
	//inicializamos el thread que se encaraga de escuchar a la matriz
	args* argumentos=malloc(sizeof(args));
	argumentos->pipe=bankPipe[0];
	argumentos->h_cuentas=header;
	argumentos->h_trans=trans_header;
	
	pthread_t* mythread=malloc(sizeof(pthread_t));
	pthread_create(mythread,NULL,escuchar,argumentos);
        while (true) {
          // 100 milisegundos...
	srand(time(NULL));
		
	char cuenta_random[14];  
	int random=rand()%cuentas+300;  // CCC random en int
	sprintf(cuenta_random, "%03d-%03d-%06d",bankId, sucId,random);
	srand(time(NULL));
	int transaccion=rand()%3+1;
	int r_trans;
	int monto=rand()%500000000+1;
	printf("\nCUENTA RANDOM: %s\n",cuenta_random);

	if(transaccion==1){	//DEPOSITO
		r_trans=deposito(monto,cuenta_random,header);
			push_trans('d','e',"EFECTIVO",cuenta_random,monto,r_trans,trans_header);
			
		
		}	
	else if(transaccion==2) { //RETIRO
		r_trans=retiro(100000,cuenta_random,header);	
		push_trans('r','e',cuenta_random,"EFECTIVO",monto,r_trans,trans_header);
		}


	else{		        	//TRANSFERENCIA
		char cuenta_random_dest[14];
		random=rand()%15;
		sprintf(cuenta_random_dest, "%03d-%03d-%06d",bankId, sucId,random);
		if((r_trans=retiro(100000,cuenta_random,header)==0)){
				r_trans=deposito(monto,cuenta_random_dest,header);
				
				push_trans('t','c',cuenta_random,cuenta_random_dest,monto,r_trans,trans_header);
					
				


			}
		else{

			push_trans('t','c',cuenta_random,cuenta_random_dest,monto,r_trans,trans_header);

				}



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

  printf("Terminando ejecucion limpiamente...\n");
  // Cerrar lado de escritura del pipe
  

  return(EXIT_SUCCESS);
}





