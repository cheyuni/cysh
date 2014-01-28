#include "cysh.h"

void fatal(char * msg) {
    perror(msg);
    exit(-1);
}

void print_header() {
    int index = 0;
    char cheyuni[20][MAX] = {
        "-------------------------------------------------------------------",
        " 2013   ok7217@gmail.com                                /\\",
        " _cheyuni              ,.                              /  \\",
        " cyshell              , .                             |    |",
        "                  ' .        ~                     ---:'''':---",
        "               ~~                                     :^_^ :",
        "                           '     ~                    _:"":\\___",
        "          ~         '                  ' '      ____.' :::     '._",
        "                                 .    . *=====<<=)           \\    :",
        "    ` ~         '                      .  '      '-'-'\\_      /'._.'",
        "            =              ,.                           \\====:_ \"\"",
        "  -                  -,. -     )_                       .'     \\",
        "                   ,.            ).        ~.          :       :",
        "                 '-'  _.---._       '  )               /   :    \\",
        "  ~             ( +  .'.'  /|\\`.'.  -         ,      :   .      '.",
        "               (    :  .' / | \\ `.  :  -             :  : :      :",
        "             (  -   '.'  /  |  \\  `.'    _            :__:-:__.;--'",
        " ,          (        `. /   |   \\ .'    )       .      '-'   '-'",
        "            cyshell    `-.__|__.-'     _    ",
        "-------------------------------------------------------------------",
    };
    for(index = 0; index < 20; index++) {
        printf("%s\n",cheyuni[index]);
    }
}

void init() {
    print_header();
}

int set_params(char * buf, char * params[MAX]) {
    memset(params, 0, MAX);
    char * tocken;
    tocken = strtok(buf, " ");
    int i = 0;
    while(tocken != NULL) {
	params[i++] = tocken;
	tocken = strtok(NULL, " ");
    }
    return i;
}

void parse_and_eval(char * buf, int async) {
    int i = 0, pipe_index = 0;
    char * args[PIPE_MAX][MAX];
    char * argu_point[SMALL_MAX];
    char * params[MAX];
    enum is type = NOR;
    FILE * his;
    argu_point[pipe_index++] = buf;
    while(*buf) {
    	if(*buf == '|') {
    	    if(*buf == '|') type = PIPE;
    	    *buf = '\0';
    	    argu_point[pipe_index++] = buf + 1;
    	}
    	buf++;
    }
    if(type == NOR) {
    	i = set_params(argu_point[0], params);
    	if(params[0] != NULL) {
    	    if(!strcmp(params[0], "cd")) {
    	    	eval_cd(params, i);
    	    } else if(!strcmp(params[0], "exit")) {
    	    	exit(1);
    	    } else if(!strcmp(params[0], "history")) {
		his = fopen(his_dir, "r");
		char str[MAX];
		i = 0;
		while(fgets(str, MAX, his))
		    printf("%5d  %s", ++i,str);
		fclose(his);
	    } else {
    	    	eval(params, async);
    	    }
    	}
    } else if(type == PIPE) {
    	eval_pipe(argu_point, pipe_index, async);
    } 
}

int eval_cd(char ** params, int count) {
    if(count > 2) {
	printf("\ncd: Too many arguments.");
	return 1;
    }
    if(count == 1) {
	if(chdir(getenv("HOME")))
	    fatal("getenv");
    } else {
	if(chdir(params[1]))
	    printf("%s: bad directory.\n", params[1]);
    }
    return 1;
}

void eval(char ** params, int async) {
    pid_t pid;
    int status, i, flag = 0;
    if((pid  = fork()) == 0) {
	for(i = 0; params[i] != NULL; i++){
	    if(!strcmp(params[i], ">")) {
		redirect(params, i, 0);
		return;
	    } else if(!strcmp(params[i], ">>")) {
		redirect(params, i, 1);
		return;
	    }
	}
    	if(execvp(params[0], params) == -1) {
	    printf("%s: command not found\n", params[0]);
	    exit(0);
	}
	
    } else {
	if(!async)
	    waitpid(pid, &status, 0);
	else
	    printf("[pid %d]\n", pid);
    }
}

void redirect(char **params, int index, int is_ltlt) {
    int fid;
    pid_t pid ,j ;
    char target[MAX];
    strcpy(target, params[index + 1]);

    params[index] = NULL;
    if(is_ltlt) {
	fid = open(target, O_RDWR | O_APPEND | O_CREAT,0640);
    } else {
	fid = open(target, O_RDWR | O_TRUNC | O_CREAT,0640);
    }
    if(fid < 0) {
	fprintf(stderr, "error creating %s\n", target);
    }
    close(1);
    dup(fid);
    close(fid);
    execvp(params[0], params);
}

void eval_pipe(char * argu_point[], int pipe_index, int async) {
    pid_t pid[pipe_index];
    int i = 0, j = 0, k = 0;
    int pfd[pipe_index - 1][2], status;
    char * params[MAX];
    for(i = 0; i < pipe_index - 1; i++) {
	if(pipe(pfd[i]) == -1)
	    fatal("pipe");
    }
    for(i = 0; i <= pipe_index - 1; i++) {
	set_params(argu_point[i], params);
	pid[i] = fork();
	if(pid[i] == -1)
	    fatal("fork multi pipe");
	else if(pid[i] == 0) {
	    if(i != 0) {
	    	close(0);
	    	dup(pfd[i-1][0]);
	    }
	    if(i != (pipe_index - 1)) {
	    	close(1);
	    	dup(pfd[i][1]);
            }

	    for(j = 0; j < pipe_index - 1; j++) {
	    	close(pfd[j][0]);
	    	close(pfd[j][1]);
	    }

	    
	    if(i == (pipe_index - 1)) {
	    	for(k = 0; params[k] != NULL; k++){
	    	    if(!strcmp(params[k], ">")) {
	    		redirect(params, k, 0);
	    	    } else if(!strcmp(params[k], ">>")) {
	    		redirect(params, k, 1);
	    	    }
	    	}
	    }
	    
	    execvp(params[0], params);
	    exit(1);
	}
    }
    for(j=0; j < pipe_index - 1; j++) {
    	close(pfd[j][0]);
    	close(pfd[j][1]);
    }
    if (async) {
	pid_t id;
    	for(j=0; j <= pipe_index - 1; j++) {
    	    if(pid[j] > 0) {
		id = pid[j];
	    }
    	}
	printf("[pid %d]\n", id);
    }
    for(i = 0; i < pipe_index; i++) {
	waitpid(pid[i], &status, 0);
    }
}

void run(char * buf) {
    int async = 0;
    char *start = buf;
    buf = start;
    while(*buf) {
    	if(*buf == '\0' || *buf == '\n' || *buf == '&' || *buf == ';') {
    	    if(*buf == '&') async = 1;
    	    *buf = '\0';
    	    parse_and_eval(start, async);
    	    start = buf + 1;
    	    async = 0;
    	}
    	buf++;
    }
}

int print_prompt() {
    char prompt[MAX];
    if(getcwd(prompt, sizeof(prompt)) == NULL)
    	fatal("getcwd");
    else {
	printf("%s> ", prompt);
	return 1;
    }
}
char * set_buf(char *buf) {
    char * start = buf;
    char tmp[MAX], tmp2[MAX];
    DIR * dirptr;
    struct dirent * dir;
    char this[SMALL_MAX];
    while(*buf) {
    	if(*buf == '~') {
    	    strcpy(tmp , buf + 1);
    	    if(*buf == '~') {
    		*buf = '\0';
    		strcat(buf, getenv("HOME"));
    	    }   
    	    strcat(buf, tmp);
    	}
    	buf++;
    }
    return start;
}

int main(int argc, char *argv[]){
    char buf[MAX];
    init();
    struct passwd * pw = getpwuid(getuid());
    his_dir = pw->pw_dir;
    strcat(his_dir, "/.history");
    FILE * history;
    history = fopen(his_dir, "a");
    while(print_prompt() && fgets(buf, MAX, stdin)) {
	fputs(buf, history);
    	run(set_buf(buf));
    }
    fclose(history);
    return 0;
}
