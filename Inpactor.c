#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <assert.h>
#define BUFSIZE 128

char hostname[30];

char** str_split(char* a_str, const char a_delim){
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

void step1(char dir[], int num_proc, int my_id, char db[], char result_directory[], int verbose){
	int start=0;
	int i;
	char buf[BUFSIZE];
	char dir_proc[1000];
	char db_proc[1000];
	FILE *fp;
	int num_seq = 0;
	int num_seq_per_proc = 0;
	if(my_id==0){ //master proc

		//to create step 1 directory
		char commandMK[1000] = "mkdir -p";
	    char *cur1 = commandMK;
	    char * const end1 = commandMK + sizeof commandMK;
	    cur1 += snprintf(cur1, end1-cur1, "%s ","mkdir -p");
	    cur1 += snprintf(cur1, end1-cur1, "%s/step1",result_directory);
	    if(verbose == 1){
	    	printf("Creating step 1 directory\n");
	    }
	    if ((fp = popen(commandMK, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }
	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }

	    //to count TE in directory
		char command[] = "ls ";
		strcat(command,dir);
		strcat(command,"/*_fsta.txt");
		if(verbose == 1){
			printf("executing command: %s\n",command);
		}
		if ((fp = popen(command, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        num_seq += 1;
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }

	    num_seq_per_proc = (num_seq/(num_proc-1));
	    if(verbose == 1){
	    	printf("there are %i sequences.\n",num_seq);
	    	printf("each process has to analyze approximately %i sequences\n", num_seq_per_proc);
		}

		if(num_seq < num_proc){
			if(my_id == 0){
				printf("ERROR: you must to have more sequences than threads...\n");
			}
			exit(0);
		}
	    
	    //to split sequences in process files.
	    char commandSplit[1000] = "sh Parallel_splitter ";
	    char *cur = commandSplit;
	    char * const end = commandSplit + sizeof commandSplit;
	    cur += snprintf(cur, end-cur, "%s ","sh Parallel_splitter");
	    cur += snprintf(cur, end-cur, "%s %i %s/step1 1",dir,num_proc-1,result_directory);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandSplit);
	    }	
	    if ((fp = popen(commandSplit, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }	    

	    //to send start orden to all processes
	    start=1;
	    for(i=1;i<num_proc;i++){
	    	MPI_Send(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	    }

	    //to recive all finished messages from processes
	    for(i=1;i<num_proc;i++){
	    	MPI_Recv(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	    	if(verbose == 1){
	    		printf("proc %d done!\n",i);
	    	}
	    }
	    //join all out_file.${id_proc}.tab in all_tabfiles.tab
	    char commandJoin[1000] = "cat ";
	    char *cur2 = commandJoin;
	    char * const end2 = commandJoin + sizeof commandJoin;
	    cur2 += snprintf(cur2, end2-cur2, "%s ","cat ");
	    cur2 += snprintf(cur2, end-cur2, "%s/step1/out_file.*.tab > %s/step1/all_tabfiles.tab",result_directory,result_directory);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandJoin);
	    }
	    if ((fp = popen(commandJoin, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
	    printf("Preprocessing done, please check file: %s/step1/all_tabfiles.tab\n",result_directory);

	}else{ //slave procs
		MPI_Recv(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//executing step1
		char commandStep1[1000] = "sh functions ";
	    char *cur2 = commandStep1;
	    char * const end = commandStep1 + sizeof commandStep1;
	    cur2 += snprintf(cur2, end-cur2, "%s ","sh functions");
	    cur2 += snprintf(cur2, end-cur2, "step1 %s %s %d %s/step1",dir,db,my_id,result_directory);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandStep1);
	    }
	    if ((fp = popen(commandStep1, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }
	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
	    MPI_Send(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
}

void step1_fasta(char fastafile[], int num_proc, int my_id, char db[], char result_directory[], int verbose){
	int start=0;
	int i;
	char buf[BUFSIZE];
	char dir_proc[1000];
	char db_proc[1000];
	FILE *fp;
	int num_seq = 0;
	int num_seq_per_proc = 0;
	if(my_id==0){ //master proc

		//to create step 1 directory
		char commandMK[1000] = "mkdir -p";
	    char *cur1 = commandMK;
	    char * const end1 = commandMK + sizeof commandMK;
	    cur1 += snprintf(cur1, end1-cur1, "%s ","mkdir -p");
	    cur1 += snprintf(cur1, end1-cur1, "%s/step1",result_directory);
	    if(verbose == 1){
	    	printf("Creating step 1 directory\n");
	    }
	    if ((fp = popen(commandMK, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }
	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }

	    //to split sequences in process files.
	    char commandSplit[1000] = "sh Parallel_splitter ";
	    char *cur = commandSplit;
	    char * const end = commandSplit + sizeof commandSplit;
	    cur += snprintf(cur, end-cur, "%s ","sh Parallel_splitter");
	    cur += snprintf(cur, end-cur, " %s/step1 %s %i 5",result_directory,fastafile,num_proc-1);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandSplit);
	    }	
	    if ((fp = popen(commandSplit, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }	    

	    //to send start orden to all processes
	    start=1;
	    for(i=1;i<num_proc;i++){
	    	MPI_Send(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	    }

	    //to recive all finished messages from processes
	    for(i=1;i<num_proc;i++){
	    	MPI_Recv(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	    	if(verbose == 1){
	    		printf("proc %d done!\n",i);
	    	}
	    }
	    //join all out_file.${id_proc}.tab in all_tabfiles.tab
	    char commandJoin[1000] = "cat ";
	    char *cur2 = commandJoin;
	    char * const end2 = commandJoin + sizeof commandJoin;
	    cur2 += snprintf(cur2, end2-cur2, "%s ","cat ");
	    cur2 += snprintf(cur2, end-cur2, "%s/step1/out_file.*.tab > %s/step1/all_tabfiles.tab",result_directory,result_directory);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandJoin);
	    }
	    if ((fp = popen(commandJoin, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
	    printf("Preprocessing done, please check file: %s/step1/all_tabfiles.tab\n",result_directory);

	}else{ //slave procs
		MPI_Recv(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//executing step1
		char commandStep1[1000] = "sh functions ";
	    char *cur2 = commandStep1;
	    char * const end = commandStep1 + sizeof commandStep1;
	    cur2 += snprintf(cur2, end-cur2, "%s ","sh functions");
	    cur2 += snprintf(cur2, end-cur2, "step1_fasta %s %s %d %s/step1",fastafile,db,my_id,result_directory);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandStep1);
	    }
	    if ((fp = popen(commandStep1, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }
	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
	    MPI_Send(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
}

void step1_repet(char fastafile[], int num_proc, int my_id, char db[], char result_directory[], int verbose){
	int start=0;
	int i;
	char buf[BUFSIZE];
	char dir_proc[1000];
	char db_proc[1000];
	FILE *fp;
	int num_seq = 0;
	int num_seq_per_proc = 0;
	if(my_id==0){ //master proc

		//to create step 1 directory
		char commandMK[1000] = "mkdir -p";
	    char *cur1 = commandMK;
	    char * const end1 = commandMK + sizeof commandMK;
	    cur1 += snprintf(cur1, end1-cur1, "%s ","mkdir -p");
	    cur1 += snprintf(cur1, end1-cur1, "%s/step1",result_directory);
	    if(verbose == 1){
	    	printf("Creating step 1 directory\n");
	    }
	    if ((fp = popen(commandMK, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }
	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }

	    //to split sequences in process files.
	    char commandSplit[1000] = "sh Parallel_splitter ";
	    char *cur = commandSplit;
	    char * const end = commandSplit + sizeof commandSplit;
	    cur += snprintf(cur, end-cur, "%s ","sh Parallel_splitter");
	    cur += snprintf(cur, end-cur, " %s/step1 %s %i 5",result_directory,fastafile,num_proc-1);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandSplit);
	    }	
	    if ((fp = popen(commandSplit, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }	    

	    //to send start orden to all processes
	    start=1;
	    for(i=1;i<num_proc;i++){
	    	MPI_Send(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	    }

	    //to recive all finished messages from processes
	    for(i=1;i<num_proc;i++){
	    	MPI_Recv(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	    	if(verbose == 1){
	    		printf("proc %d done!\n",i);
	    	}
	    }
	    //join all out_file.${id_proc}.tab in all_tabfiles.tab
	    char commandJoin[1000] = "cat ";
	    char *cur2 = commandJoin;
	    char * const end2 = commandJoin + sizeof commandJoin;
	    cur2 += snprintf(cur2, end2-cur2, "%s ","cat ");
	    cur2 += snprintf(cur2, end-cur2, "%s/step1/out_file.*.tab > %s/step1/all_tabfiles.tab",result_directory,result_directory);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandJoin);
	    }
	    if ((fp = popen(commandJoin, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
	    printf("Preprocessing done, please check file: %s/step1/all_tabfiles.tab\n",result_directory);

	}else{ //slave procs
		MPI_Recv(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//executing step1
		char commandStep1[1000] = "sh functions ";
	    char *cur2 = commandStep1;
	    char * const end = commandStep1 + sizeof commandStep1;
	    cur2 += snprintf(cur2, end-cur2, "%s ","sh functions");
	    cur2 += snprintf(cur2, end-cur2, "step1_repet %s %s %d %s/step1",fastafile,db,my_id,result_directory);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandStep1);
	    }
	    if ((fp = popen(commandStep1, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }
	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
	    MPI_Send(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
}

void step2(char dir[], int num_proc, int vstep1, int my_id, char tabfile[], char result_directory[], int verbose, int law808080){
	int start=0;
	int i;
	char buf[BUFSIZE];
	FILE *fp;
	int num_seq = 0;
	int num_seq_per_proc = 0;
	if(my_id == 0){ //master

		//to create step 2 directory
		char commandMK[1000] = "mkdir -p";
	    char *cur1 = commandMK;
	    char * const end = commandMK + sizeof commandMK;
	    cur1 += snprintf(cur1, end-cur1, "%s ","mkdir -p");
	    cur1 += snprintf(cur1, end-cur1, "%s/step2",result_directory);
	    if(verbose == 1){
	    	printf("Creating step 2 directory\n");
	    }
	    if ((fp = popen(commandMK, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }
	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
		if(vstep1 == 0){ //we have to split tabfile in chunks

			//call splitter
			char commandSplit[1000] = "sh Parallel_splitter ";
		    char *cur = commandSplit;
		    char * const end2 = commandSplit + sizeof commandSplit;
		    cur += snprintf(cur, end2-cur, "%s ","sh Parallel_splitter");
		    cur += snprintf(cur, end2-cur, "%s/step1 %s %d 2",result_directory,tabfile,num_proc-1);
		    if(verbose == 1){
		    	printf("executing command: %s\n",commandSplit);
		    }
		    if ((fp = popen(commandSplit, "r")) == NULL) {
		        printf("Error opening pipe!\n");
		        exit(0);
		    }

		    while (fgets(buf, BUFSIZE, fp) != NULL) {
		        printf("%s\n",buf);
		    }
		    if(pclose(fp))  {
		        printf("Command not found or exited with error status\n");
		    }
		}
			//send start orden to all processes
		    start=1;
		    for(i=1;i<num_proc;i++){
		    	MPI_Send(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		    }
		    //recive all finished messages from processes
		    for(i=1;i<num_proc;i++){
		    	MPI_Recv(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		    	if(verbose == 1){
		    		printf("proc %d done!\n",i);
		    	}
		    }
		    //join all out_file.${id_proc}.tab in all_tabfiles.tab
		    char commandJoin[4000] = "cat ";
		    char *cur2 = commandJoin;
		    char * const end3 = commandJoin + sizeof commandJoin;
		    cur2 += snprintf(cur2, end3-cur2, "%s ","cat ");
		    cur2 += snprintf(cur2, end3-cur2, "%s/step2/out_file.*.tab_ALL.RLC_RLG.TAB > %s/step2/all_tabfiles.tab_ALL.RLC_RLG.TAB; cat %s/step2/out_file.*.tab_ALL.RLC_RLG.FA > %s/step2/all_tabfiles.tab_ALL.RLC_RLG.FA; cat %s/step2/out_file.*.tab.RXX.TAB > %s/step2/all_tabfiles.tab_ALL.RXX.TAB; cat %s/step2/out_file.*.tab.RXX.FA > %s/step2/all_tabfiles.tab_ALL.RXX.FA; cat %s/step2/out_file.*.tab.TR_GAG.TAB > %s/step2/all_tabfiles.tab_ALL.TR_GAG.TAB; cat %s/step2/out_file.*.tab.TR_GAG.FA > %s/step2/all_tabfiles.tab_ALL.TR_GAG.FA; cat %s/step2/out_file.*.tab.NOC.TAB > %s/step2/all_tabfiles.tab.NOC.TAB; cat %s/step2/out_file.*.tab.NOC.FA > %s/step2/all_tabfiles.tab.NOC.FA; cat %s/step2/out_file.*.tab_ALL.RLC_RLG.TAB.families > %s/step2/all_tabfiles.tab_ALL.RLC_RLG.TAB.families; cat %s/step2/out_file.*.tab_ALL.RLC_RLG.TAB.families.FA > %s/step2/all_tabfiles.tab_ALL.RLC_RLG.TAB.families.FA; cat %s/step2/all_tabfiles.tab_ALL.RLC_RLG.TAB.families.FA %s/step2/all_tabfiles.tab_ALL.RXX.FA %s/step2/all_tabfiles.tab_ALL.TR_GAG.FA > %s/step2/classifiedTE.fa;",result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory);
		    if(verbose == 1){
		    	printf("executing command: %s\n",commandJoin);
		    }
		    if ((fp = popen(commandJoin, "r")) == NULL) {
		        printf("Error opening pipe!\n");
		        exit(0);
		    }
		    while (fgets(buf, BUFSIZE, fp) != NULL) {
		        printf("%s\n",buf);
		    }
		    if(pclose(fp))  {
		        printf("Command not found or exited with error status\n");
		    }

		    if(law808080 == 1){
		    	//to start orden to law 80-80-80
				start=1;
			    for(i=1;i<num_proc;i++){
			    	MPI_Send(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			    }
			    //recive all finished messages from processes
			    for(i=1;i<num_proc;i++){
			    	MPI_Recv(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			    	if(verbose == 1){
			    		printf("proc %d done!\n",i);
			    	}
			    }
			    //join all out_file.${id_proc}.tab in all_tabfiles.tab
			    char commandJoin2[4000] = "cat ";
			    char *cur22 = commandJoin2;
			    char * const end32 = commandJoin2 + sizeof commandJoin2;
			    cur22 += snprintf(cur22, end32-cur22, "%s ","cat ");
			    cur22 += snprintf(cur22, end32-cur22, "%s/step2/out_file.*.tab_ALL.RLC_RLG.TAB > %s/step2/all_tabfiles.tab_ALL.RLC_RLG.TAB; cat %s/step2/out_file.*.tab_ALL.RLC_RLG.FA > %s/step2/all_tabfiles.tab_ALL.RLC_RLG.FA; cat %s/step2/out_file.*.tab.RXX.TAB > %s/step2/all_tabfiles.tab_ALL.RXX.TAB; cat %s/step2/out_file.*.tab.RXX.FA > %s/step2/all_tabfiles.tab_ALL.RXX.FA; cat %s/step2/out_file.*.tab.TR_GAG.TAB > %s/step2/all_tabfiles.tab_ALL.TR_GAG.TAB; cat %s/step2/out_file.*.tab.TR_GAG.FA > %s/step2/all_tabfiles.tab_ALL.TR_GAG.FA; cat %s/step2/out_file.*.tab.NOC.TAB > %s/step2/all_tabfiles.tab.NOC.TAB; cat %s/step2/out_file.*.tab.NOC.FA > %s/step2/all_tabfiles.tab.NOC.FA; cat %s/step2/out_file.*.tab_ALL.RLC_RLG.TAB.families > %s/step2/all_tabfiles.tab_ALL.RLC_RLG.TAB.families; cat %s/step2/out_file.*.tab_ALL.RLC_RLG.TAB.families.FA > %s/step2/all_tabfiles.tab_ALL.RLC_RLG.TAB.families.FA; cat %s/step2/all_tabfiles.tab_ALL.RLC_RLG.TAB.families.FA %s/step2/all_tabfiles.tab_ALL.RXX.FA %s/step2/all_tabfiles.tab_ALL.TR_GAG.FA > %s/step2/classifiedTE.fa;",result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory,result_directory);
			    if(verbose == 1){
		    	printf("executing command: %s\n",commandJoin2);
			    }
			    if ((fp = popen(commandJoin2, "r")) == NULL) {
			        printf("Error opening pipe!\n");
			        exit(0);
			    }
			    while (fgets(buf, BUFSIZE, fp) != NULL) {
			        printf("%s\n",buf);
			    }
			    if(pclose(fp))  {
			        printf("Command not found or exited with error status\n");
			    }
		    }
		    printf("Classification done, please check files in %s/step2\n",result_directory);

	}else{//processes
		MPI_Recv(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//executing step2
		char commandStep2[1000] = "sh functions ";
	    char *cur2 = commandStep2;
	    char * const end = commandStep2 + sizeof commandStep2;
	    cur2 += snprintf(cur2, end-cur2, "%s ","sh functions ");
	    cur2 += snprintf(cur2, end-cur2, "step2 %s/step1 %d %s/step2",result_directory,my_id,result_directory);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandStep2);
	    }
	    if ((fp = popen(commandStep2, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }
	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
	    MPI_Send(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	    if(law808080 == 1){
	    	MPI_Recv(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	    	char commandStep21[1000] = "sh functions ";
		    char *cur21 = commandStep21;
		    char * const end21 = commandStep21 + sizeof commandStep21;
		    cur21 += snprintf(cur21, end-cur21, "%s ","sh functions ");
		    cur21 += snprintf(cur21, end-cur21, "80-80-80-law %s/step1 %d %s/step2",result_directory,my_id,result_directory);
		    if(verbose == 1){
		    	printf("executing command: %s\n",commandStep21);
		    }
		    if ((fp = popen(commandStep21, "r")) == NULL) {
		        printf("Error opening pipe!\n");
		        exit(0);
		    }
		    while (fgets(buf, BUFSIZE, fp) != NULL) {
		        printf("%s\n",buf);
		    }
		    if(pclose(fp))  {
		        printf("Command not found or exited with error status\n");
		    }
		    MPI_Send(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	    }
	}
}

void step3(char db[], char dir[], int num_proc, int vstep2, int my_id, char fastafile[], char result_directory[], int verbose, char references[], int RTlen, char blast_e[]){
	int start=0;
	int i;
	char buf[BUFSIZE];
	FILE *fp;
	int num_seq = 0;
	int num_seq_per_proc = 0;
	char filefasta[1000];
	if(my_id == 0){ //master
		//to create step 3 directory
		char commandMK[1000] = "mkdir -p";
	    char *cur1 = commandMK;
	    char * const end = commandMK + sizeof commandMK;
	    cur1 += snprintf(cur1, end-cur1, "%s ","mkdir -p");
	    cur1 += snprintf(cur1, end-cur1, "%s/step3",result_directory);
	    if(verbose == 1){
	    	printf("Creating step 3 directory\n");
	    }
	    if ((fp = popen(commandMK, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }
	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
		if(vstep2 == 0){ //we have to split fastafile in chunks
			strcpy(filefasta,fastafile);
			if(verbose == 1){
            	printf("fasta file: %s\n",filefasta);
           	}
		}else{
			char *cur10 = filefasta;
            char * const end10 = filefasta + sizeof filefasta;
            cur10 += snprintf(cur10, end10-cur10, "%s",result_directory);
            cur10 += snprintf(cur10, end10-cur10, "/step2/all_tabfiles.tab_ALL.RLC_RLG.FA");
            if(verbose == 1){
            	printf("fasta file: %s\n",filefasta);
           	}
		}
		//call splitter
		char commandSplit[1000] = "sh Parallel_splitter ";
	    char *cur = commandSplit;
	    char * const end2 = commandSplit + sizeof commandSplit;
	    cur += snprintf(cur, end2-cur, "%s ","sh Parallel_splitter ");
	    cur += snprintf(cur, end2-cur, "%s/step2 %s %d 3 '%s'",result_directory,filefasta,num_proc-1,references);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandSplit);
	    }
	    if ((fp = popen(commandSplit, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
		//send start orden to all processes
	    start=1;
	    for(i=1;i<num_proc;i++){
	    	MPI_Send(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	    }
	    //recive all finished messages from processes
	    for(i=1;i<num_proc;i++){
	    	MPI_Recv(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	    	if(verbose == 1){
	    		printf("proc %d done!\n",i);
	    	}
	    }
	    //join all Genome.${id_proc}.RT.fa in Genome.all_ALL.RLC_RLG.fa.rt
	    char commandJoin[1000] = "cat ";
	    char *cur2 = commandJoin;
	    char * const end3 = commandJoin + sizeof commandJoin;
	    cur2 += snprintf(cur2, end3-cur2, "%s ","cat ");
	    cur2 += snprintf(cur2, end3-cur2, "%s/step3/Genome.*.RT.fa %s > %s/step3/Genome.all_ALL.RLC_RLG.fa.rt",result_directory,db,result_directory);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandJoin);
	    }
	    if ((fp = popen(commandJoin, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
	    printf("Domain Extraction done, please check file: %s/step3/Genome.all_ALL.RLC_RLG.fa.rt\n",result_directory);

	}else{//processes
		MPI_Recv(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//executing step3
		char commandStep3[1000] = "sh functions ";
	    char *cur2 = commandStep3;
	    char * const end = commandStep3 + sizeof commandStep3;
	    cur2 += snprintf(cur2, end-cur2, "%s ","sh functions ");
	    cur2 += snprintf(cur2, end-cur2, "step3 %s %d %s/step3 %s/step2 %d %s",db,my_id,result_directory,result_directory,RTlen,blast_e);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandStep3);
	    }
	    if ((fp = popen(commandStep3, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
	    MPI_Send(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
}

void step4(char tabfile[], int vstep2, int vstep3, int my_id, char dir[],char result_directory[], int num_proc, int verbose, char input[], char substitution_rate[]){
	int start=0;
	int i;
	char buf[BUFSIZE];
	FILE *fp;
	int num_seq = 0;
	int num_seq_per_proc = 0;
	char genomeS4[1000];
	char tabfile_s4[1000];
    if(my_id == 0){ //master
        char *cur10 = genomeS4;
        char * const end10 = genomeS4 + sizeof genomeS4;
        cur10 += snprintf(cur10, end10-cur10, "%s",result_directory);
        cur10 += snprintf(cur10, end10-cur10, "/step3/Genome.all_ALL.RLC_RLG.fa.rt");
        if(verbose == 1){
        	printf("genome file: %s\n",genomeS4);
       	}
	
	//to create step 3 directory
        char commandMK[1000] = "mkdir -p";
	    char *cur1 = commandMK;
	    char * const end = commandMK + sizeof commandMK;
	    cur1 += snprintf(cur1, end-cur1, "%s ","mkdir -p");
	    cur1 += snprintf(cur1, end-cur1, "%s/step4",result_directory);
	    if(verbose == 1){
	    	printf("Creating step 4 directory\n");
	    }
	    if ((fp = popen(commandMK, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
		if(vstep2 == 0){ //we must to use parameter tabfile
			strcpy(tabfile_s4,tabfile);
		}else{
			char *cur11 = tabfile_s4;
	        char * const end11 = tabfile_s4 + sizeof tabfile_s4;
	        cur11 += snprintf(cur11, end11-cur11, "%s",result_directory);
	        cur11 += snprintf(cur11, end11-cur11, "/step2/all_tabfiles.tab_ALL.RLC_RLG.TAB");
		}
		if(verbose == 1){
	        	printf("tabfile for step 4 is: %s\n",tabfile_s4);
	    }
		//call splitter
		char commandSplit[1000] = "sh Parallel_splitter ";
	    char *cur = commandSplit;
	    char * const end2 = commandSplit + sizeof commandSplit;
	    cur += snprintf(cur, end2-cur, "%s ","sh Parallel_splitter");
	    cur += snprintf(cur, end2-cur, "%s/step2 %s %i 4",result_directory,tabfile_s4,num_proc-1);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandSplit);
	    }
	    if ((fp = popen(commandSplit, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }

		//send start orden to all processes
	    start=1;
	    for(i=1;i<num_proc;i++){
	    	MPI_Send(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	    }
	    //recive all finished messages from processes
	    for(i=2;i<num_proc;i++){
	    	MPI_Recv(&start, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	    	if(verbose == 1){
	    		printf("proc %d done!\n",i);
	    	}
	    }
	    //join all partial results in only one
	    char commandJoin[1000] = "sh functions ";
	    char *cur2 = commandJoin;
	    char * const end3 = commandJoin + sizeof commandJoin;
	    cur2 += snprintf(cur2, end3-cur2, "%s ","sh functions ");
	    cur2 += snprintf(cur2, end3-cur2,  "step4 %d %s/step4 %s %d",my_id,result_directory,genomeS4,num_proc);
	    if(verbose == 1){
	    	printf("executing command: %s\n",commandJoin);
	    }
	    if ((fp = popen(commandJoin, "r")) == NULL) {
	        printf("Error opening pipe!\n");
	        exit(0);
	    }

	    while (fgets(buf, BUFSIZE, fp) != NULL) {
	        printf("%s\n",buf);
	    }
	    if(pclose(fp))  {
	        printf("Command not found or exited with error status\n");
	    }
	    printf("Distribution file done, please check file: %s/step4/Distribution-in-age.tab\n",result_directory);
		MPI_Recv(&start, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	    printf("Tree file done, please check file: %s/step4/multiple_align.tree\n",result_directory);
	}else{//processes
		MPI_Recv(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//executing step4
		char commandStep4[1000] = "sh functions ";
		char *cur2 = commandStep4;
		char * const end = commandStep4 + sizeof commandStep4;
		cur2 += snprintf(cur2, end-cur2, "%s ","sh functions ");
		cur2 += snprintf(cur2, end-cur2, "step4 %d %s/step4 %s/step2 %s %s",my_id,result_directory,result_directory,input,substitution_rate);
		if(verbose == 1){
		  printf("executing command: %s\n",commandStep4);
		}
		if ((fp = popen(commandStep4, "r")) == NULL) {
		      printf("Error opening pipe!\n");
		      exit(0);
		}

		while (fgets(buf, BUFSIZE, fp) != NULL) {
			printf("%s\n",buf);
		}
		if(pclose(fp))  {
		     printf("Command not found or exited with error status\n");
		}
		MPI_Send(&start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
	   
}

void cleaner(char result_directory[], int verbose){
	char buf[BUFSIZE];
	FILE *fp;
	char commandMK[1000] = "sh functions ";
    char *cur1 = commandMK;
    char * const end = commandMK + sizeof commandMK;
    cur1 += snprintf(cur1, end-cur1, "%s ","sh functions");
    cur1 += snprintf(cur1, end-cur1, "cleaner %s",result_directory);
    if(verbose == 1){
    	printf("executing cleaner: %s\n",commandMK);
    }
    if ((fp = popen(commandMK, "r")) == NULL) {
        printf("Error opening pipe!\n");
        exit(0);
    }

    while (fgets(buf, BUFSIZE, fp) != NULL) {
        printf("%s\n",buf);
    }
    if(pclose(fp))  {
        printf("Command not found or exited with error status\n");
    }
}

void printHelp(){
	printf("Usage: mpirun -np num_processes Inpactor configuration_file\n");
	printf("NOTE: Parameter num_processes has to be greater than 1\n");
	printf("Example of configuration file:\n");
	printf("######################### Configuration file #####################\n");
	printf("input=/home/user/input_data\n");
	printf("result_directory=/home/user/Inpactor_output\n");
	printf("verbose=true\n");
	printf("clean=true\n");
    printf("#Input type can be LTR_STRUC, repet or fasta\n");
    printf("input_type=LTR_STRUC\n");
	printf("######################## Preprocessing ########################\n");
	printf("preprocessing=true\n");
	printf("database=/home/user/cores-database-wickercode.Lineage_Bianca.fa\n");
	printf("######################### Classification ########################\n");
	printf("classification=true\n");
	printf("#this tabfile isn't necessary if preprocessing is true\n");
	printf("tabfile=/home/user/Inpactor_output/step1/all_tabfiles.tab\n");
	printf("80-80-80-rule=false\n");
	printf("####################### Domain Extraction ######################\n");
	printf("extraction=true\n");
	printf("RTdatabase=/home/user/RTcores-database-wickercode.Lineage_Bianca.fa\n");
	printf("references=/home/user/references.fasta\n");
	printf("#this fastafile isn't necessary if classification is true\n");
	printf("fastafile=/home/user/Inpactor_output/step2/all_tabfiles.tab_ALL.RLC_RLG.FA\n");
	printf("#filter RT size. For a lot of RT, is better RTlength > 200, \n");
	printf("#for less RTlength > 180 or 150\n");
	printf("RTlength=200\n");
	printf("Blast_evalue=1e-4\n");
	printf("###### Insertion Time analysis and Phylogenetic tree creation ########\n");
	printf("insertion=true\n");
	printf("substitution_rate=0.000000013\n");
	printf("#this tabfile isn't necessary if classification is true\n");
	printf("tabfileS4=/home/user/Inpactor_output/step2/all_tabfiles.tab_ALL.RLC_RLG.TAB\n");
}

int main (int argc, char *argv[]){
	int i, my_id, nproc, namelen, cont;
	char directory[1000] = "";
	char database[1000] = "";
	char tab_fileS2[1000] = "";
	char result_directory[1000] = "";
	char databaseRT[1000] = "";
	char tab_fileS3[1000] = "";
	char tab_fileS4[1000] = "";
	char references[1000] = "";
	char blast_evalue[1000] = "";
	char input_type[1000] = "";
	char substitution_rate[1000] = "";
	int law808080 = 0;
	int RTlen = 200;
	int par_step1 = 0;
	int par_step2 = 0;
	int par_step3 = 0;
	int par_step4 = 0;
	int verbose = 0;
	int clean = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	MPI_Get_processor_name(hostname, &namelen);

	FILE *file;
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
    if(strcmp(argv[1],"help") == 0){
    	if(my_id == 0){
    		printHelp();
    	}
    	exit(0);
    }
    if(nproc < 2){
    	if(my_id == 0){
    		printf("You must to indicate at least 2 MPI processes!!!!\n");
			printHelp();
		}
		exit(0);
    }
    if(argc < 2){
		if(my_id == 0){
			printf("You must to indicate the configuration file!!!!\n");
			printHelp();
		}
		exit(0);
	}else{
		//read configuration file
		if(my_id == 0){	
			printf("Reading configuration file: %s ...\n",argv[1]);
		}
		file = fopen(argv[1], "r");
		if (file == NULL){
			exit(0);
		}
        
	    while ((read = getline(&line, &len, file)) != -1) {
	    	if(line[0] != '#' && line[0] != ' '){ //non-comment lines and non-space white lines
	    		char parameter_line[strlen(line)];
	    		strcpy(parameter_line, line);
	    		char** parameters = str_split(parameter_line, '=');
		    	if(strcmp(*(parameters), "preprocessing") == 0){ // preprocessing module 
	    			if(strcmp(*(parameters+1), "true\n") == 0){
		    			par_step1 = 1;
		    		}else{
		    			par_step1 = 0;
		    		}
	    			if(my_id == 0){	
		    			printf("preprocessing module 1 was selected? %s",*(parameters+1));
		    		}
		    	}
		    	if(strcmp(*(parameters), "input_type") == 0){ // input type 
	    			strcpy(input_type, *(parameters+1));
		    		if(input_type[strlen(input_type)-1] == '\n' ){
						input_type[strlen(input_type)-1] = '\0';
					}
	    			if(my_id == 0){	
		    			printf("Input Type is: %s\n", input_type);
		    		}
		    	}
		    	if(strcmp(*(parameters), "verbose") == 0){ // step 1 parameter 
	    			if(strcmp(*(parameters+1), "true\n") == 0){
		    			verbose = 1;
		    		}else{
		    			verbose = 0;
		    		}
	    			if(my_id == 0){	
		    			printf("verbose was actived? %s",*(parameters+1));
		    		}
		    	}
		    	if(strcmp(*(parameters), "clean") == 0){ // Cleaner parameter 
	    			if(strcmp(*(parameters+1), "true\n") == 0){
		    			clean = 1;
		    		}else{
		    			clean = 0;
		    		}
	    			if(my_id == 0){	
		    			printf("clean was actived? %s",*(parameters+1));
		    		}
		    	}
		    	if(strcmp(*(parameters), "result_directory") == 0){ // result directory parameter 
	    			strcpy(result_directory, *(parameters+1));
		    		if(result_directory[strlen(result_directory)-1] == '\n' ){
						result_directory[strlen(result_directory)-1] = '\0';
					}
	    			if(my_id == 0){	
		    			printf("Result directory is: %s\n", result_directory);
		    		}
		    	}
		    	if(strcmp(*(parameters), "input") == 0){ // directory parameter 
		    		strcpy(directory, *(parameters+1));
		    		if(directory[strlen(directory)-1] == '\n' ){
						directory[strlen(directory)-1] = '\0';
					}
	    			if(my_id == 0){	
		    			printf("Input is: %s\n", directory);
		    		}
		    	}
		    	if(strcmp(*(parameters), "database") == 0){ // database parameter 
		    		strcpy(database, *(parameters+1));
		    		if(database[strlen(database)-1] == '\n' ){
						database[strlen(database)-1] = '\0';
					}
		    		if(my_id == 0){
		    			printf("database is: %s\n", database);
		    		}
		    	}
		    	if(strcmp(*(parameters), "classification") == 0){ // classification module parameter 
	    			if(strcmp(*(parameters+1), "true\n") == 0){
		    			par_step2 = 1;
		    		}else{
		    			par_step2 = 0;
		    		}
	    			if(my_id == 0){	
		    			printf("Classification module was selected? %s",*(parameters+1));
		    		}
		    	}
		    	if(strcmp(*(parameters), "80-80-80-rule") == 0){ // 80-80-80 rule parameter
	    			if(strcmp(*(parameters+1), "true\n") == 0){
		    			law808080 = 1;
		    		}else{
		    			law808080 = 0;
		    		}
	    			if(my_id == 0){	
		    			printf("80-80-80 rule is enabled? %s",*(parameters+1));
		    		}
		    	}
		    	if(strcmp(*(parameters), "tabfile") == 0){ // tabfile parameter 
		    		strcpy(tab_fileS2, *(parameters+1));
		    		if(tab_fileS2[strlen(tab_fileS2)-1] == '\n' ){
						tab_fileS2[strlen(tab_fileS2)-1] = '\0';
					}
		    		if(my_id == 0){
		    			printf("tabfile is: %s\n", tab_fileS2);
		    		}
		    	}
		    	if(strcmp(*(parameters), "extraction") == 0){ // extraction module parameter 
	    			if(strcmp(*(parameters+1), "true\n") == 0){
		    			par_step3 = 1;
		    		}else{
		    			par_step3 = 0;
		    		}
	    			if(my_id == 0){	
		    			printf("Extraction module was selected? %s",*(parameters+1));
		    		}
		    	}
		    	if(strcmp(*(parameters), "RTdatabase") == 0){ // RT database parameter 
		    		strcpy(databaseRT, *(parameters+1));
		    		if(databaseRT[strlen(databaseRT)-1] == '\n' ){
						databaseRT[strlen(databaseRT)-1] = '\0';
					}
		    		if(my_id == 0){
		    			printf("RT database is: %s\n", databaseRT);
		    		}
		    	}
		    	if(strcmp(*(parameters), "RTlength") == 0){ // RT database parameter 
		    		RTlen = atoi(*(parameters+1));
		    		if(my_id == 0){
		    			printf("RT length is: %d\n", RTlen);
		    		}
		    	}
                if(strcmp(*(parameters), "Blast_evalue") == 0){ // blast e-value parameter 
				strcpy(blast_evalue, *(parameters+1));
                        if(blast_evalue[strlen(blast_evalue)-1] == '\n' ){
                                        blast_evalue[strlen(blast_evalue)-1] = '\0';
                                }
                        if(my_id == 0){
                                printf("BLAST E-value is: %s\n", blast_evalue);
                        }
                }
		    	if(strcmp(*(parameters), "fastafile") == 0){ // fasta file parameter 
		    		strcpy(tab_fileS3, *(parameters+1));
		    		if(tab_fileS3[strlen(tab_fileS3)-1] == '\n' ){
						tab_fileS3[strlen(tab_fileS3)-1] = '\0';
					}
		    		if(my_id == 0){
		    			printf("fastafile is: %s\n", tab_fileS3);
		    		}
		    	}
		    	if(strcmp(*(parameters), "insertion") == 0){ // insertion module parameter 
	    			if(strcmp(*(parameters+1), "true\n") == 0){
		    			par_step4 = 1;
		    		}else{
		    			par_step4 = 0;
		    		}
	    			if(my_id == 0){	
		    			printf("Insertion module was selected? %s",*(parameters+1));
		    		}
		    	}
		    	if(strcmp(*(parameters), "substitution_rate") == 0){ // substitution rate 
	    			strcpy(substitution_rate, *(parameters+1));
		    		if(substitution_rate[strlen(substitution_rate)-1] == '\n' ){
						substitution_rate[strlen(substitution_rate)-1] = '\0';
					}
		    		if(my_id == 0){
		    			printf("substitution rate is: %s\n", substitution_rate);
		    		}
		    	}
		    	if(strcmp(*(parameters), "tabfileS4") == 0){ // tabular file parameter 
		    		strcpy(tab_fileS4, *(parameters+1));
		    		if(tab_fileS4[strlen(tab_fileS4)-1] == '\n' ){
						tab_fileS4[strlen(tab_fileS4)-1] = '\0';
					}
		    		if(my_id == 0){
		    			printf("tabular file for step 4 is: %s\n", tab_fileS4);
		    		}
		    	}
		    	if(strcmp(*(parameters), "references") == 0){ // tabular file parameter 
		    		strcpy(references, *(parameters+1));
		    		if(references[strlen(references)-1] == '\n' ){
						references[strlen(references)-1] = '\0';
					}
		    		if(my_id == 0){
		    			printf("references are: %s\n", references);
		    		}
		    	}
       		}
	    }
	    fclose(file);
	    if (line){
	        free(line);
	    }

	    //to validate parameters 
		if(strcmp(result_directory,"") == 0 || strcmp(directory,"") == 0 || strcmp(input_type,"") == 0){
			if(my_id == 0){
				printf("ERROR: Input directory, Input type and result directory are required!!!!\n");
			}
			exit(0);
		}
		if(par_step1 == 1){
			if(strcmp(database,"") == 0){
				if(my_id == 0){
					printf("ERROR: If Preprocessing module is selected, database is required!!!!\n");
				}
				exit(0);
			}
			FILE *tdatabase_file;
			tdatabase_file = fopen(database, "r");
			if (tdatabase_file == NULL){
				if(my_id == 0){
					printf("ERROR: %s has a problem\n", database);
				}
				exit(0);
			}
			fclose(tdatabase_file);
		}

		if(par_step2 == 1){
			if(par_step1 == 0 && strcmp(tab_fileS2,"") == 0){
				if(my_id == 0){
					printf("ERROR: If Classification is selected and Preprocessing isn't selected, tabfile is required!!!!\n");
				}
				exit(0);
			}
			if(par_step1 == 0){
			FILE *tabfile;
			tabfile = fopen(tab_fileS2, "r");
			if (tabfile == NULL){
				if(my_id == 0){
					printf("ERROR: %s has a problem\n", tab_fileS2);
				}
				exit(0);
			}
			fclose(tabfile);
			}
		}

		if(par_step3 == 1){
			if(par_step2 == 0 && strcmp(tab_fileS3,"") == 0){
				if(my_id == 0){
					printf("ERROR: If Extraction is selected and Classification isn't selected, fastafile is required!!!!\n");
				}
				exit(0);
			}
			if(par_step2 == 0){
			FILE *tabfile3;
			tabfile3 = fopen(tab_fileS3, "r");
			if (tabfile3 == NULL){
				if(my_id == 0){
					printf("ERROR: %s has a problem\n", tab_fileS3);
				}
				exit(0);
			}
			fclose(tabfile3);
			}
			if(strcmp(databaseRT,"") == 0){
				if(my_id == 0){
					printf("ERROR: If Extraction is selected, databaseRT is required!!!!\n");
				}
				exit(0);
			}
			FILE *tabfile4;
			tabfile4 = fopen(databaseRT, "r");
			if (tabfile4 == NULL){
				if(my_id == 0){
					printf("ERROR: %s has a problem\n", databaseRT);
				}
				exit(0);
			}
			fclose(tabfile4);
		}

		if(par_step4 == 1){
			if(par_step2 == 0 && strcmp(tab_fileS4,"") == 0){
				if(my_id == 0){
					printf("ERROR: If Insertion is selected and Classification isn't selected, tab_fileS4 is required!!!!\n");
				}
				exit(0);
			}
			if(par_step2 == 0){
			FILE *tabfile_file4;
			tabfile_file4 = fopen(tab_fileS4, "r");
			if (tabfile_file4 == NULL){
				if(my_id == 0){
					printf("ERROR: %s has a problem\n", tab_fileS4);
				}
				exit(0);
			}
			fclose(tabfile_file4);
			}
			if(strcmp(substitution_rate,"") == 0){
				if(my_id == 0){
					printf("ERROR: You must to indicate the substitution rate\n");
				}
				exit(0);
			}
		}

		if(par_step4 == 1 && par_step3 == 0){
			if(my_id == 0){
				printf("ERROR: if Insertion is selected, Extraction is required!\n");
			}
			exit(0);
		}

	    //to create the result directory
		if(my_id == 0){
			printf("finished reading configuration file\n");
			int start=0;
			int i;
			char buf[BUFSIZE];
			FILE *fp;
			char command[1000] = "mkdir -p";
		    char *cur1 = command;
		    char * const end = command + sizeof command;
		    cur1 += snprintf(cur1, end-cur1, "%s ","mkdir -p");
		    cur1 += snprintf(cur1, end-cur1, "%s",result_directory);
		    if(verbose == 1){
		    	printf("Creating result directory: %s\n",command);
			}
		    if ((fp = popen(command, "r")) == NULL) {
		        printf("Error opening pipe!\n");
		        exit(0);
		    }
		    while (fgets(buf, BUFSIZE, fp) != NULL) {
		        printf("%s\n",buf);
		    }
		    if(pclose(fp))  {
		        printf("Command not found or exited with error status\n");
		    }
		}

		//call to step1
	    if(par_step1 == 1){
	    	if(strcmp(input_type,"fasta") == 0){
	    		step1_fasta(directory,nproc,my_id,database,result_directory,verbose);
	    	}else if(strcmp(input_type,"repet") == 0){
	    		step1_repet(directory,nproc,my_id,database,result_directory,verbose);
	    	} else if(strcmp(input_type,"LTR_STRUC") == 0){
	    		step1(directory,nproc,my_id,database,result_directory,verbose);
	    	} else{
	    		if(my_id == 0){
					printf("ERROR: Input type must be fasta, repet or LTR_STRUC!\n");
				}
			exit(0);
	    	}
	    }

	    //call to step2
	    if(par_step2 == 1){
	    	step2(directory,nproc,par_step1,my_id,tab_fileS2,result_directory,verbose,law808080);
	    }

	    //call to step3
	    if(par_step3 == 1){
	    	step3(databaseRT,directory,nproc,par_step2,my_id,tab_fileS3,result_directory,verbose,references,RTlen,blast_evalue);
	    }

	    //call to step4
	    if(par_step4 == 1){
	    	if(strcmp(input_type,"repet") != 0){
	    		step4(tab_fileS4,par_step2,par_step3,my_id,directory,result_directory,nproc,verbose,input_type,substitution_rate);
	    	}else{
	    		if(my_id == 0){
					printf("I cannot run step 4 with repet input type!\n");
				}
	    	}
	    }

	    //call to cleaner
	    if(clean == 1){
            if(my_id == 0){
    	    	cleaner(result_directory,verbose);
            }
	    }
	}
	MPI_Finalize();
}
